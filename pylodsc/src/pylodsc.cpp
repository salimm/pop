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
    //std::cout << "------- read this value: 1\n";
    //PyObject_Print(event,stdout, 0);
    //std::cout << "------- read this value: 2\n";
    PyObject* value = dict->read_value(event);
    //PyObject_Print(value,stdout, 0);
    //std::cout << "\n------- read this value: 3\n";
    // decreasing points back
    Py_DECREF(event);
    //std::cout << "\n------- read this value: 4\n";
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
    //std::cout << "--- read obj 1\n";    
    //PyObject_Print(cls, stdout, 0);
    //std::cout << "\n--- read obj 1.1\n";
    //increasing pointers to args
    Py_INCREF(events);
    Py_INCREF(cls);
    Py_INCREF(ctxt);
    Py_INCREF(deserializers);
    PyObject* obj =PyObject_CallObject(cls, NULL); // here is the problem :(((
    //PyObject_Print(obj, stdout, 0);
    //std::cout << "\n--- read obj 1.2\n";
    if (state == EXPECTING_OBJ_START){
        //std::cout << "--- read obj 2\n";
        //setting to object start state
        PyObject* first =  PyIter_Next(events);
        // check if input is object
        if (!dict->is_obj_start(first)){   
            //std::cout << "--- read obj 3\n";
            Py_DECREF(events);
            Py_DECREF(cls);
            Py_DECREF(ctxt);
            Py_DECREF(deserializers);

            Py_DECREF(first);
            // raise ObjectDeserializationException("Expected the content to start with an object!! But first event was: " + str(first))
        }        

        Py_DECREF(first);
    }
    //std::cout << "--- read obj 4\n";
    state = EXPECTING_OBJ_PROPERTY_OR_END;    
    PyObject* propname = NULL;

    PyObject* event = PyIter_Next(events);
    //PyObject_Print(obj, stdout, 0);
    //std::cout << "\n--- read obj 5\n";
    while (event != NULL){
        //std::cout << "\n--- read obj 6\n";
        //PyObject_Print(obj, stdout, 0);
        //std::cout << "\n--- read obj 6\n";
        if (state == EXPECTING_OBJ_PROPERTY_OR_END){
            //std::cout << "--- read obj 7\n";
            //end of object
            if (dict->is_obj_end(event)){
                //std::cout << "--- read obj 8\n";
                //check valid state
                break;
            }else if (dict->is_obj_property_name(event)){
                //std::cout << "--- read obj 9\n";
                //check valid state                
                propname = read_this_value(event, dict); // new ref
                //PyObject_Print(propname,stdout, 0);
                PyObject* decoded = decode_field_name(obj, propname);
                Py_DECREF(propname);
                propname = decoded;
                //PyObject_Print(propname,stdout, 0);
                //setting next state
                state = EXPECTING_VALUE;
            }else{
                //std::cout << "--- read obj 10\n";
                Py_DECREF(events);
                Py_DECREF(cls);
                Py_DECREF(ctxt);
                Py_DECREF(deserializers);
                
                Py_DECREF(event);
                 // raise ParseException(" Unexpected event when expected state is: " + str(state) + " while event was: " + str(event))
            }
        }else if (state == EXPECTING_VALUE){
            //std::cout << "--- read obj 11\n";
            PyObject* val = NULL;

            if (dict->is_array_start(event)){
                //std::cout << "--- read obj 12\n";
                val = read_array(events,cls, propname,ctxt,dict, deserializers, TYPED, EXPECTING_VALUE_OR_ARRAY_END);
            }else if (dict->is_value(event)){ 
                //std::cout << "--- read obj 13\n";
                //  read value
                val = read_this_value(event, dict);
            }else if (dict->is_obj_start(event)){
                //std::cout << "--- read obj 14\n";
                val = read_obj_as_value(events, cls, propname, ctxt, dict, deserializers, TYPED);
            }else {
                //std::cout << "--- read obj 15\n";
                Py_DECREF(events);
                Py_DECREF(cls);
                Py_DECREF(ctxt);
                Py_DECREF(deserializers);

                Py_DECREF(propname);
                Py_DECREF(event);
                // raise ParseException('unrecognized event when reading value: ' + str(event))
            }
            //std::cout << "--- read obj 16\n";
            // setting to None if Null
            if(val == NULL){
                //std::cout << "--- read obj 16.1\n";
                Py_INCREF(Py_None);
                val = Py_None;
            }
            //std::cout << "--- read obj 16.2\n";
            // setting value        
            //PyObject_Print(obj,stdout,0);
            if(PyObject_IsInstance(obj, (PyObject* )&PyDict_Type)){                
                //std::cout << "--- read obj 17\n";
               PyDict_SetItem(obj, propname, val);
            }else{
                //std::cout << "--- read obj 18\n";
                PyObject_SetAttr(obj, propname, val);
            }
            //std::cout << "--- read obj 19\n";
            Py_DECREF(propname);
            Py_DECREF(val);
            propname = NULL;
            state = EXPECTING_OBJ_PROPERTY_OR_END;

        }
        //std::cout << "--- read obj 20\n";
        Py_DECREF(event);
        event = PyIter_Next(events);
    }

    //std::cout << "--- read obj 21\n";
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
    // std::cout << "===== decode 1\n";
    // PyObject_Print(obj,stdout,0);
    // std::cout << "\n===== decode 1\n";
    Py_INCREF(obj);
    PyObject* result = NULL;

    // std::cout << "===== decode 2\n";
    if(PyObject_IsInstance(obj,(PyObject* )&PyDict_Type)){
        // std::cout << "===== decode 3\n";
        Py_INCREF(name);
        result = name;
    }else{
        // std::cout << "===== decode 4\n";
        //1. check to rename fields using decorator
        PyObject* cls = PyObject_GetAttrString(obj, (char *)"__class__"); // new ref
        if(PyObject_HasAttrString(cls, "_pylods")){
            // std::cout << "===== decode 5\n";
            PyObject* pylods = PyObject_GetAttrString(cls, "_pylods"); // new ref
            
            PyObject* namedecodemap = PyDict_GetItemString(PyDict_GetItem(pylods,cls),"namedecode"); // borrowed ref
            if (namedecodemap!=NULL){
                // std::cout << "===== decode 6\n";
                // PyObject_Print(namedecodemap,stdout,0);
                // PyObject_Print(name,stdout,0);
                result = PyDict_GetItem(namedecodemap, name); // borrowed ref
                // PyObject_Print(result,stdout,0);
                if(result!=NULL){
                    Py_INCREF(result);
                }
            }

            Py_DECREF(pylods); // dec ref
            //std::cout << "===== decode 7\n";
        }
        if(result==NULL){
            //# 2. check to ignore fields based on decorators TODO
            //# 3. remove private convention _
            //std::cout << "===== decode 8.0\n";   
            PyObject* fields = fetch_obj_fields(obj); // new ref
            //std::cout << "===== decode 8.0.1\n";   
            PyObject* properties = extract_property_names(obj);
            //std::cout << "===== decode 8\n";
            if (contains(fields,name) || contains(properties, name)){
                //std::cout << "===== decode 9\n";
                Py_INCREF(name);
                result = name;        
            }else {
                //std::cout << "===== decode 10\n";
                PyObject* privatename =PyString_FromString((char*)"_");
                PyString_Concat(  &privatename, name);
                //PyObject_Print(fields,stdout,0);
                //PyObject_Print(properties,stdout,0);
                //PyObject_Print(privatename,stdout,0);
                //std::cout << "\n===== decode 11\n";
                if (contains(fields, privatename) || contains(properties, privatename) ){
                    //std::cout << "\n===== decode 12\n";
                    Py_INCREF(privatename);
                    result = privatename;
                }else{
                    //std::cout << "\n===== decode 13\n";
                    Py_DECREF(name);
                    Py_DECREF(obj);
                    Py_DECREF(fields);
                    Py_DECREF(cls); // dec ref
                    Py_DECREF(privatename);
                    // raise the exception
                    // PyErr_SetObject();
                    // raise Exception("property \"" + str(name) + "\" couldn't be mapped to a property in object " + str(getattr(obj,"__class__")))
                }
                //std::cout << "\n===== decode 14\n";
                Py_DECREF(privatename);
                //std::cout << "\n===== decode 15\n";
            }
            //std::cout << "\n===== decode 16\n";
            Py_DECREF(fields);
            Py_DECREF(cls); // dec ref
        }
    }
    //std::cout << "\n===== decode 17\n";
    Py_DECREF(name);
    Py_DECREF(obj);
    Py_INCREF(name);
    

    //std::cout << "===== decode last\n";
    //PyObject_Print(obj,stdout,0);
    //std::cout << "\n===== decode last\n";
    return result;
}


PyObject* read_obj_as_value(PyObject* events, PyObject* cls, PyObject* valname, PyObject* ctxt, Dictionary* dict,  PyObject* deserializers, PyObject* TYPED){
    //std::cout << "--- read obj as val 1\n";
    //PyObject_Print(cls, stdout, 0);
    Py_XINCREF(deserializers);
    Py_XINCREF(ctxt);
    Py_XINCREF(cls);
    Py_XINCREF(valname);

    PyObject* val = NULL;
    PyObject* valcls = resolve(cls, valname, TYPED);
    PyObject* deserializer = lookup_deserializer(deserializers, valcls);
    //std::cout << "--- read obj as val 2\n";
    if (deserializer != NULL){
        //std::cout << "--- read obj as val 3\n";
        PyObject* count = PyInt_FromLong(1);
        PyObject* method = PyString_FromString("execute");
        //std::cout << "----- create call 1\n";
        //PyObject_Print(events, stdout, 0);
        //PyObject_Print(count, stdout, 0);
        PyObject* clseventitr = create_class_event_iterator(events, dict ,PyInt_AsLong(count));
        //std::cout << "----- create call 2\n";
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
    //std::cout << "--- read obj as val 5\n";
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
    //std::cout << "---- attach 1\n";
    PyObject* attached = PyList_New(0); // inc ref

    for(int i=0; i< PyList_Size(fields); i++){
        //std::cout << "---- attach 2\n";
        PyObject* name = PyList_GetItem(fields, i); // borrowed ref
        PyObject* entry = PyDict_GetItem(ordermap, name); // borrowed ref
        PyObject* order;
        //std::cout << "---- attach 3\n";
        if (entry != NULL ){       
            //std::cout << "---- attach 4\n";                 
            order = entry;            
        }else{
            //std::cout << "---- attach 5\n";
            order = PyInt_FromLong(999999); // new ref
        }
        //std::cout << "---- attach 6\n";
        //PyObject_Print(order, stdout, 0);
        PyObject* tmp  = PyTuple_Pack(2, name, order);
        //PyObject_Print(tmp, stdout, 0);
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
    //std::cout << "+++++++++++  extract 1\n";
    for(int i=0; i< PyList_Size(attached); i++){
        //std::cout << "+++++++++++  extract 2\n";
        PyObject* entry = PyList_GetItem(attached, i); // borrowed ref
        //PyObject_Print(entry,stdout, 0);
        PyObject* name = PyTuple_GetItem(entry,0);// borrowed
        //PyObject_Print(name,stdout, 0);
        PyList_Append(fields, name); // creates new ref of args  -> decref
    }
    //std::cout << "+++++++++++  extract 3\n";
    Py_DECREF(attached); // dec ref
    return fields;
}

PyObject* sort_obj_fields(PyObject* obj, PyObject* fields){
    //std::cout << "***** sort1\n";
    Py_INCREF(fields);
    Py_INCREF(obj);

    PyObject* result = NULL;
    PyObject* cls = PyObject_GetAttrString(obj, (char *)"__class__");// new ref
    //std::cout << "*****  sort2\n";
    if(PyObject_HasAttrString(cls, "_pylods")){
        //std::cout << "*****  sort3\n";
        PyObject* pylods = PyObject_GetAttrString(cls, "_pylods");// new ref
        PyObject* ordermap = PyDict_GetItemString(PyDict_GetItem(pylods,cls),"order");
        //std::cout << "*****  sort4\n";
        if(ordermap!=NULL){
            //std::cout << "*****  sort5\n";
            PyObject* attached = attach_order(ordermap, fields); // new ref
            //std::cout << PyList_Size(attached);
            //PyObject_Print(PyList_GetItem(attached,0), stdout, 0);
            //PyObject_Print(attached, stdout, 0);
            //std::cout << "*****  sort5.1\n";
            PyObject* sorted = sort_tmp_obj_fields(attached);// new ref
            //PyObject_Print(sorted, stdout, 0);
            //std::cout << "*****  sort5.2\n";
            Py_DECREF(attached); // dec ref
            //std::cout << "*****  sort5.3\n";
            result = extract_names(sorted);// new ref
            //PyObject_Print(result, stdout, 0);
            //std::cout << "*****  sort5.4\n";
            Py_DECREF(sorted);// dec ref
        }else{
            //std::cout << "*****  sort6\n";
            Py_INCREF(fields);
            result = fields;
        }
        //std::cout << "*****  sort7\n";
        Py_DECREF(pylods); // dec ref
    }else{
        //std::cout << "*****  sort8\n";
        Py_INCREF(fields);
        result = fields;
    }

    //std::cout << "*****  sort9\n";
    Py_DECREF(cls);// dec ref
    Py_DECREF(fields);// dec ref
    Py_DECREF(obj);// dec ref

    return result;
}


PyObject* fetch_obj_fields(PyObject* obj){
    //std::cout << "===== fetch 0\n";
    Py_INCREF(obj);

    PyObject* fields = PyList_New(0);// new ref
    //std::cout << "===== fetch 1\n";
    if(PyObject_HasAttrString(obj,(char *)"__dict__")){
        //std::cout << "===== fetch 2\n";
        PyObject* dict = PyObject_GetAttrString(obj, (char *)"__dict__"); // new ref
        PyObject* keys = PyDict_Keys(dict);// new ref
        //std::cout << "===== fetch 3\n";
        for(int i=0; i< PyList_Size(keys); i++){
            //std::cout << "===== fetch 4\n";
            if(!PyCallable_Check(PyDict_GetItem(dict,PyList_GetItem(keys,i)))){
                //std::cout << "===== fetch 5\n";
                PyList_Append(fields,PyList_GetItem(keys,i));
            }
        }
        //std::cout << "===== fetch 6\n";
        Py_DECREF(dict); // dec ref
        Py_DECREF(keys); // dec ref
    }else if(PyObject_HasAttrString(obj,(char *)"__slots__")){
        //std::cout << "===== fetch 7\n";
        PyObject* slots = PyObject_GetAttrString(obj, (char *)"__slots__");
        PyObject* keys = PyDict_Keys(slots);
        for(int i=0; i< PyList_Size(slots); i++){
            if(!PyCallable_Check(PyDict_GetItem(slots,PyList_GetItem(keys,i)))){
                PyList_Append(fields,PyList_GetItem(keys,i));
            }
        }
        Py_DECREF(slots);
        Py_DECREF(keys);
    }else{
        //std::cout << "===== fetch 8\n";
        // do nothing
    }
    //std::cout << "===== fetch 9\n";
    //PyObject_Print(obj,stdout,0);
    //PyObject_Print(fields,stdout,0);
    PyObject* sorted = sort_obj_fields(obj, fields);
    //std::cout << "===== fetch 10\n";
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
    //std::cout << "----- next 1\n";

    if(p->isdone){
        //std::cout << "----- next 2\n";
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }
    //std::cout << "----- next 3\n";    
    PyObject* event = PyIter_Next(p->events);
    //PyObject_Print(event, stdout, 0);
    if(p->dict->is_obj_start(event)){
        //std::cout << "----- next 4\n";
        p->count = p->count +1;
    }else if(p->dict->is_obj_end(event)){
        //std::cout << "----- next 5\n";
        p->count = p->count -1;
        if(p->count ==0){
            //std::cout << "----- next 6\n";
            p->isdone =1;            
            Py_DECREF(event);
            PyErr_SetNone(PyExc_StopIteration);
            return NULL;
        }                    
    }
    //std::cout << "----- next 7\n";

    return event;        
}


 void classeventiterator_dealloc(PyObject* self)
{
    /* We need XDECREF here because when the generator is exhausted,
     * rgstate->sequence is cleared with Py_CLEAR which sets it to NULL.
    */
    //std::cout << "----- dealloc 1";
    pylodscbackend_ClassEventIterator *p = (pylodscbackend_ClassEventIterator *)self;
    Py_DECREF(p->events);  
    //std::cout << "----- dealloc 2";  
    // Py_DECREF(p->dict->get_pdict());
    //std::cout << "----- dealloc 3";
    // delete p->dict;
    // //std::cout << "----- dealloc 4";
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
    //std::cout << "==== create 1\n";
    pylodscbackend_ClassEventIterator *p;
    p = PyObject_New(pylodscbackend_ClassEventIterator, &pylodscbackend_ClassEventIteratorType);
    if (!p) return NULL;

    //std::cout << "==== create 2\n";
    if (!PyObject_Init((PyObject *)p, &pylodscbackend_ClassEventIteratorType)) {
        //std::cout << "==== create 3\n";
        Py_DECREF(p);
        return NULL;
    }
    //std::cout << "==== create 4\n";
    Py_INCREF(events);
    p->events = events;
    p->dict =  dict;
    p->count = count;
    p->isdone = 0;
    //std::cout << "==== create 5\n";
    if(count==0){
        //std::cout << "==== create 6\n";
        PyObject* event = PyIter_Next(events);
        int isobjstart = dict->is_obj_start(event);
        Py_DECREF(event);
        //std::cout << "==== create 7\n";
        if(isobjstart){
            //std::cout << "==== create 8\n";
            p->count = p->count+1;
        }else{
            //std::cout << "==== create 9\n";
            PyErr_SetString(PyExc_ValueError, "Expected a object start event but didn't receive one");
            return NULL;
        }
    }
    //std::cout << "==== create 10\n";

    return (PyObject *)p;
}
