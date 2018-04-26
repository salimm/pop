#include "pylodsc.h"
#include "Dictionary.h"



PyObject* read_value(PyObject* events, Dictionary* dict){
    //increasing pointers to args
    Py_INCREF(events);

    PyObject* next = PyIter_Next(events);
    // calling pdict's read value to parse value from event
    PyObject* value = dict->read_value(next);

    // decreasing points back
    Py_DECREF(next);
    Py_DECREF(events);


    return value;
}

PyObject* read_this_value(PyObject* event, Dictionary* dict){
    //increasing pointers to args
    Py_INCREF(event);
    PyObject* value = dict->read_value(event);
    // decreasing points back
    Py_DECREF(event);
    return value;
}


PyObject* read_array(PyObject* events, PyObject* cls, PyObject* propname, PyObject* ctxt,  Dictionary* dict,  PyObject* deserializers, PyObject* TYPED, enum ParserState  state){
    Py_INCREF(events);
    Py_XINCREF(cls);
    Py_XINCREF(propname);
    Py_XINCREF(ctxt);
    Py_INCREF(deserializers);    
    // final result
    PyObject* res = PyList_New(0); // new ref
    if (state == EXPECTING_ARRAY_START){
        PyObject* first =  PyIter_Next(events);// new ref

        if (!dict->is_array_start(first)){
            // raise exception
        }
        Py_DECREF(first);

    }
    state = EXPECTING_VALUE_OR_ARRAY_END;
    PyObject* event = PyIter_Next(events); // new ref
    while (event != NULL){
        if (dict->is_array_end(event)){
            // check valid state
            if (state != EXPECTING_VALUE_OR_ARRAY_END){
                // raise UnexpectedStateException(POPState.EXPECTING_VALUE_OR_ARRAY_END, state, " wasn't expecting a end of object")
            }
            break;
        }else if (dict->is_value(event)){
            if (state != EXPECTING_VALUE_OR_ARRAY_END){
                // raise UnexpectedStateException(state, POPState.EXPECTING_VALUE_OR_ARRAY_END, " wasn't expecting a value")
            }
            PyObject* val = read_this_value(event,dict);
            PyList_Append(res,  val);
            Py_DECREF(val);
        }else if (dict->is_obj_start(event)){
            PyObject* val = read_obj_as_value(events, cls, propname, ctxt, dict, deserializers, TYPED);
            PyList_Append(res, val);
            Py_DECREF(val);
        }else if (dict->is_array_start(event)){
            PyObject* val = read_array(events,cls, propname,ctxt,dict, deserializers, TYPED, EXPECTING_VALUE_OR_ARRAY_END);
            PyList_Append(res, val);
            Py_DECREF(val);
        }else{
            // raise ParseException('Unexpected event')
        }
        Py_DECREF(event); // dec ref
        event = PyIter_Next(events); // new ref
    }

    Py_DECREF(events);
    Py_XDECREF(cls);
    Py_XDECREF(propname);
    Py_XDECREF(ctxt);
    Py_DECREF(deserializers);    
    return res;
}


PyObject* read_obj(PyObject* events, PyObject* cls,  PyObject* ctxt,  Dictionary* dict,  PyObject* deserializers, PyObject* TYPED, enum ParserState  state){
    //increasing pointers to args
    Py_INCREF(events);
    Py_INCREF(cls);
    Py_INCREF(ctxt);
    Py_INCREF(deserializers);
    PyObject* obj =PyObject_CallObject(cls, NULL); // here is the problem :(((
    if (state == EXPECTING_OBJ_START){
        //setting to object start state
        PyObject* first =  PyIter_Next(events);
        // check if input is object
        if (!dict->is_obj_start(first)){   
            Py_DECREF(events);
            Py_DECREF(cls);
            Py_DECREF(ctxt);
            Py_DECREF(deserializers);

            Py_DECREF(first);
            // raise ObjectDeserializationException("Expected the content to start with an object!! But first event was: " + str(first))
        }        

        Py_DECREF(first);
    }
    state = EXPECTING_OBJ_PROPERTY_OR_END;    
    PyObject* propname = NULL;

    PyObject* event = PyIter_Next(events);
    while (event != NULL){
        if (state == EXPECTING_OBJ_PROPERTY_OR_END){
            //end of object
            if (dict->is_obj_end(event)){
                //check valid state
                break;
            }else if (dict->is_obj_property_name(event)){
                //check valid state                
                propname = read_this_value(event, dict); // new ref
                PyObject* decoded = decode_field_name(obj, propname);
                Py_DECREF(propname);
                propname = decoded;

                //setting next state
                state = EXPECTING_VALUE;
            }else{
                Py_DECREF(events);
                Py_DECREF(cls);
                Py_DECREF(ctxt);
                Py_DECREF(deserializers);
                
                Py_DECREF(event);
                 // raise ParseException(" Unexpected event when expected state is: " + str(state) + " while event was: " + str(event))
            }
        }else if (state == EXPECTING_VALUE){
            PyObject* val = NULL;

            if (dict->is_array_start(event)){
                val = read_array(events,cls, propname,ctxt,dict, deserializers, TYPED, EXPECTING_VALUE_OR_ARRAY_END);
            }else if (dict->is_value(event)){ 
                //  read value
                val = read_this_value(event, dict);
            }else if (dict->is_obj_start(event)){
                val = read_obj_as_value(events, cls, propname, ctxt, dict, deserializers, TYPED);
            }else {
                Py_DECREF(events);
                Py_DECREF(cls);
                Py_DECREF(ctxt);
                Py_DECREF(deserializers);

                Py_DECREF(propname);
                Py_DECREF(event);
                // raise ParseException('unrecognized event when reading value: ' + str(event))
            }
            // setting to None if Null
            if(val == NULL){
                Py_INCREF(Py_None);
                val = Py_None;
            }
            // setting value        
            if(PyObject_IsInstance(obj, (PyObject* )&PyDict_Type)){                
               PyDict_SetItem(obj, propname, val);
            }else{
                PyObject_SetAttr(obj, propname, val);
            }
            Py_DECREF(propname);
            Py_DECREF(val);
            propname = NULL;
            state = EXPECTING_OBJ_PROPERTY_OR_END;

        }
        Py_DECREF(event);
        event = PyIter_Next(events);
    }

    // decreasing points back
    Py_DECREF(events);
    Py_DECREF(cls);
    Py_DECREF(ctxt);
    Py_DECREF(deserializers);
    return obj;
}



int contains(PyObject* list, PyObject* value){
    for(int i=0; i< PyList_Size(list); i++){
        PyObject* item = PyList_GetItem(list, i);
        if(PyObject_RichCompareBool(value, item, Py_EQ)){
            return 1;
        }
    }
    return 0;
}



PyObject* decode_field_name(PyObject* obj, PyObject* name){    
    Py_INCREF(name);
    Py_INCREF(obj);
    PyObject* result = NULL;

    if(PyObject_IsInstance(obj,(PyObject* )&PyDict_Type)){
        Py_INCREF(name);
        result = name;
    }else{
        //1. check to rename fields using decorator
        PyObject* cls = PyObject_GetAttrString(obj, (char *)"__class__"); // new ref
        if(PyObject_HasAttrString(cls, "_pylods")){
            PyObject* pylods = PyObject_GetAttrString(cls, "_pylods"); // new ref
            
            PyObject* namedecodemap = PyDict_GetItemString(PyDict_GetItem(pylods,cls),"namedecode"); // borrowed ref
            if (namedecodemap!=NULL){
                result = PyDict_GetItem(namedecodemap, name); // borrowed ref
                if(result!=NULL){
                    Py_INCREF(result);
                }
            }

            Py_DECREF(pylods); // dec ref
        }
        if(result==NULL){
            //# 2. check to ignore fields based on decorators TODO
            //# 3. remove private convention _
            PyObject* fields = fetch_obj_fields(obj); // new ref
            PyObject* properties = extract_property_names(obj);
            if (contains(fields,name) || contains(properties, name)){
                Py_INCREF(name);
                result = name;        
            }else {
                PyObject* privatename =PyString_FromString((char*)"_");
                PyString_Concat(  &privatename, name);
                if (contains(fields, privatename) || contains(properties, privatename) ){
                    Py_INCREF(privatename);
                    result = privatename;
                }else{
                    Py_DECREF(name);
                    Py_DECREF(obj);
                    Py_DECREF(fields);
                    Py_DECREF(cls); // dec ref
                    Py_DECREF(privatename);
                    // raise the exception
                    // PyErr_SetObject();
                    // raise Exception("property \"" + str(name) + "\" couldn't be mapped to a property in object " + str(getattr(obj,"__class__")))
                }
                Py_DECREF(privatename);
            }
            Py_DECREF(fields);
            Py_DECREF(cls); // dec ref
        }
    }
    Py_DECREF(name);
    Py_DECREF(obj);
    Py_INCREF(name);
    

    return result;
}


PyObject* read_obj_as_value(PyObject* events, PyObject* cls, PyObject* valname, PyObject* ctxt, Dictionary* dict,  PyObject* deserializers, PyObject* TYPED){
    Py_XINCREF(deserializers);
    Py_XINCREF(ctxt);
    Py_XINCREF(cls);
    Py_XINCREF(valname);

    PyObject* val = NULL;
    PyObject* valcls = resolve(cls, valname, TYPED);
    PyObject* deserializer = lookup_deserializer(deserializers, valcls);
    if (deserializer != NULL){
        PyObject* count = PyInt_FromLong(1);
        PyObject* method = PyString_FromString("execute");
        PyObject* clseventitr = create_class_event_iterator(events, dict ,PyInt_AsLong(count));
        val = PyObject_CallMethodObjArgs(deserializer, method, clseventitr, dict->get_pdict(), ctxt, NULL);
        Py_DECREF(clseventitr);        
        Py_DECREF(deserializer);
        Py_DECREF(method);
        Py_DECREF(count);
    }else{
        val = read_obj(events, valcls , ctxt, dict, deserializers, TYPED,EXPECTING_OBJ_PROPERTY_OR_END);
    }

    Py_XDECREF(valname);
    Py_XDECREF(cls);
    Py_DECREF(deserializers);
    Py_DECREF(ctxt);
    return val;
}

PyObject* lookup_deserializer(PyObject* deserializers, PyObject* cls){
    PyObject* deserializer = PyDict_GetItem(deserializers, cls);
    if (deserializer == NULL){
        if(PyObject_HasAttrString(cls, "_pylods")){
            PyObject* pylods = PyObject_GetAttrString(cls,"_pylods");
            deserializer = PyDict_GetItemString(PyDict_GetItem(pylods,cls),"deserializer");
            Py_XINCREF(deserializer);
            Py_DECREF(pylods);
        }
    }else{
        Py_INCREF(deserializer);
    }
    return deserializer;
}

PyObject* resolve(PyObject* cls, PyObject* valname, PyObject* TYPED){
    Py_XINCREF(cls);
    Py_XINCREF(valname);
    Py_XINCREF(TYPED);
    PyObject* valcls = NULL;
    if ((cls != NULL) && (valname != NULL) && (valname != Py_None)) {
        //attempt to resolve class of value
        PyObject* method = PyString_FromString("resolve");
        valcls = PyObject_CallMethodObjArgs(TYPED, method, valname, cls, NULL);
        Py_DECREF(method);
        if(valcls == Py_None){      
            if(PyObject_HasAttrString(cls, "_pylods")){
                PyObject* pylods = PyObject_GetAttrString(cls, "_pylods");
                PyObject* typemap = PyDict_GetItemString(PyDict_GetItem(pylods,cls),"type");
                if (typemap!=NULL){
                    valcls = PyDict_GetItem(typemap, valname);
                }
                Py_DECREF(pylods);
            }
        }
    }else if((cls != NULL) && ((valname == NULL)|| (valname == Py_None)) ){
        valcls = cls;        
    }
    
    if (valcls == NULL){   
        valcls = (PyObject* )&PyDict_Type;
    }
    // increasing pointers
    Py_INCREF(valcls);
    
    // decreasing pointers
    Py_XDECREF(cls);
    Py_XDECREF(valname);
    Py_XDECREF(TYPED);

    return valcls;
}





PyObject* attach_order(PyObject* ordermap, PyObject* fields){
    Py_INCREF(fields); // inc ref
    Py_INCREF(ordermap); // inc ref
    PyObject* attached = PyList_New(0); // inc ref

    for(int i=0; i< PyList_Size(fields); i++){
        PyObject* name = PyList_GetItem(fields, i); // borrowed ref
        PyObject* entry = PyDict_GetItem(ordermap, name); // borrowed ref
        PyObject* order;
        if (entry != NULL ){       
            order = entry;            
        }else{
            order = PyInt_FromLong(999999); // new ref
        }
        PyObject* tmp  = PyTuple_Pack(2, name, order);
        PyList_Append(attached, tmp); // creates new ref of args  -> decref
        Py_DECREF(tmp); // dec ref
    }


    Py_DECREF(fields); // inc ref
    Py_DECREF(ordermap); // inc ref
    return attached;
}    

PyObject* extract_property_names(PyObject* obj){
    Py_INCREF(obj);// inc ref
    PyObject* pValue =NULL;

    PyObject* pName = PyString_FromString("pylods.serialize"); // new ref
    PyObject*  pModule = PyImport_Import(pName); // new ref
    Py_DECREF(pName); // dec ref

    if (pModule != NULL) {
        PyObject* pFunc = PyObject_GetAttrString(pModule, "extract_property_names"); // new ref
        if (pFunc && PyCallable_Check(pFunc)) {
            pValue = PyObject_CallFunctionObjArgs(pFunc, obj,NULL);
        }
        Py_DECREF(pFunc);// dec ref
    }

    Py_DECREF(obj); // dec ref
    Py_DECREF(pModule); // dec ref
    return pValue;
}

     
PyObject* sort_tmp_obj_fields(PyObject* fields){
    Py_INCREF(fields);// inc ref
    PyObject* pValue = NULL;

    PyObject* pName = PyString_FromString("pylods.serialize"); // new ref
    PyObject*  pModule = PyImport_Import(pName); // new ref
    Py_DECREF(pName); // dec ref

    if (pModule != NULL) {
        PyObject* pFunc = PyObject_GetAttrString(pModule, "sort_obj_fields"); // new ref
        if (pFunc && PyCallable_Check(pFunc)) {
            pValue = PyObject_CallFunctionObjArgs(pFunc, fields, NULL);
        }
        Py_DECREF(pFunc);// dec ref
    }

    Py_DECREF(pModule); // dec ref
    Py_DECREF(fields);// dec ref
    return pValue;
}


PyObject* extract_names(PyObject* attached){
    Py_INCREF(attached); // inc ref
    PyObject* fields = PyList_New(0);// new ref
    for(int i=0; i< PyList_Size(attached); i++){
        PyObject* entry = PyList_GetItem(attached, i); // borrowed ref
        PyObject* name = PyTuple_GetItem(entry,0);// borrowed
        PyList_Append(fields, name); // creates new ref of args  -> decref
    }
    Py_DECREF(attached); // dec ref
    return fields;
}

PyObject* sort_obj_fields(PyObject* obj, PyObject* fields){
    Py_INCREF(fields);
    Py_INCREF(obj);

    PyObject* result = NULL;
    PyObject* cls = PyObject_GetAttrString(obj, (char *)"__class__");// new ref
    if(PyObject_HasAttrString(cls, "_pylods")){
        PyObject* pylods = PyObject_GetAttrString(cls, "_pylods");// new ref
        PyObject* ordermap = PyDict_GetItemString(PyDict_GetItem(pylods,cls),"order");
        if(ordermap!=NULL){
            PyObject* attached = attach_order(ordermap, fields); // new ref
            PyObject* sorted = sort_tmp_obj_fields(attached);// new ref
            Py_DECREF(attached); // dec ref
            result = extract_names(sorted);// new ref
            Py_DECREF(sorted);// dec ref
        }else{
            Py_INCREF(fields);
            result = fields;
        }
        Py_DECREF(pylods); // dec ref
    }else{
        Py_INCREF(fields);
        result = fields;
    }

    Py_DECREF(cls);// dec ref
    Py_DECREF(fields);// dec ref
    Py_DECREF(obj);// dec ref

    return result;
}


PyObject* fetch_obj_fields(PyObject* obj){
    Py_INCREF(obj);

    PyObject* fields = PyList_New(0);// new ref
    if(PyObject_HasAttrString(obj,(char *)"__dict__")){
        PyObject* dict = PyObject_GetAttrString(obj, (char *)"__dict__"); // new ref
        PyObject* keys = PyDict_Keys(dict);// new ref
        for(int i=0; i< PyList_Size(keys); i++){
            if(!PyCallable_Check(PyDict_GetItem(dict,PyList_GetItem(keys,i)))){
                PyList_Append(fields,PyList_GetItem(keys,i));
            }
        }
        Py_DECREF(dict); // dec ref
        Py_DECREF(keys); // dec ref
    }else if(PyObject_HasAttrString(obj,(char *)"__slots__")){
        PyObject* slots = PyObject_GetAttrString(obj, (char *)"__slots__");
        for(int i=0; i< PyList_Size(slots); i++){
            if(!PyCallable_Check(PyList_GetItem(slots,i))){
                PyList_Append(fields,PyList_GetItem(slots,i));
            }
        }
        Py_DECREF(slots);
    }else{
        // do nothing
    }
    PyObject* sorted = sort_obj_fields(obj, fields);
    Py_DECREF(fields);    
    Py_DECREF(obj);
    return sorted;
}



//####################################################################################
//####################################### iterator ###################################
//####################################################################################



typedef struct {
    PyObject_HEAD
    PyObject* events;
    Dictionary* dict;
    long int count;
    int isdone;
} pylodscbackend_ClassEventIterator;

 PyObject* pylodscbackend_ClassEventIterator_iter(PyObject *self){
    Py_INCREF(self);        
    return self;
}


 PyObject* pylodscbackend_ClassEventIterator_next(PyObject *self){
    pylodscbackend_ClassEventIterator *p = (pylodscbackend_ClassEventIterator *)self;

    if(p->isdone){
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }
    PyObject* event = PyIter_Next(p->events);
    if(p->dict->is_obj_start(event)){
        p->count = p->count +1;
    }else if(p->dict->is_obj_end(event)){
        p->count = p->count -1;
        if(p->count ==0){
            p->isdone =1;            
            Py_DECREF(event);
            PyErr_SetNone(PyExc_StopIteration);
            return NULL;
        }                    
    }

    return event;        
}


 void classeventiterator_dealloc(PyObject* self)
{
    /* We need XDECREF here because when the generator is exhausted,
     * rgstate->sequence is cleared with Py_CLEAR which sets it to NULL.
    */
    pylodscbackend_ClassEventIterator *p = (pylodscbackend_ClassEventIterator *)self;
    Py_DECREF(p->events);  
}

PyTypeObject pylodscbackend_ClassEventIteratorType = {
    PyObject_HEAD_INIT(NULL)
    0,                                                         /*ob_size*/
    "pylodscbackend._ClassEventIterator",                      /*tp_name*/
    sizeof(pylodscbackend_ClassEventIterator),                 /*tp_basicsize*/
    0,                                                         /*tp_itemsize*/
    (destructor)classeventiterator_dealloc,                    /*tp_dealloc*/
    0,                                                         /*tp_print*/
    0,                                                         /*tp_getattr*/
    0,                                                         /*tp_setattr*/
    0,                                                         /*tp_compare*/
    0,                                                         /*tp_repr*/
    0,                                                         /*tp_as_number*/
    0,                                                         /*tp_as_sequence*/
    0,                                                         /*tp_as_mapping*/
    0,                                                         /*tp_hash */
    0,                                                         /*tp_call*/
    0,                                                         /*tp_str*/
    0,                                                         /*tp_getattro*/
    0,                                                         /*tp_setattro*/
    0,                                                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER,
      /* tp_flags: Py_TPFLAGS_HAVE_ITER tells python to
         use tp_iter and tp_iternext fields. */
    "C base ClassEventIterator object",                        /* tp_doc */
    0,                                                         /* tp_traverse */
    0,                                                         /* tp_clear */
    0,                                                         /* tp_richcompare */
    0,                                                         /* tp_weaklistoffset */
    pylodscbackend_ClassEventIterator_iter,                    /* tp_iter: __iter__() method */
    pylodscbackend_ClassEventIterator_next                     /* tp_iternext: next() method */
};

PyObject* create_class_event_iterator(PyObject* events, Dictionary* dict, long int count){
    pylodscbackend_ClassEventIterator *p;
    p = PyObject_New(pylodscbackend_ClassEventIterator, &pylodscbackend_ClassEventIteratorType);
    if (!p) return NULL;

    if (!PyObject_Init((PyObject *)p, &pylodscbackend_ClassEventIteratorType)) {
        Py_DECREF(p);
        return NULL;
    }
    Py_INCREF(events);
    p->events = events;
    p->dict =  dict;
    p->count = count;
    p->isdone = 0;
    if(count==0){
        PyObject* event = PyIter_Next(events);
        int isobjstart = dict->is_obj_start(event);
        Py_DECREF(event);
        if(isobjstart){
            p->count = p->count+1;
        }else{
            PyErr_SetString(PyExc_ValueError, "Expected a object start event but didn't receive one");
            return NULL;
        }
    }

    return (PyObject *)p;
}