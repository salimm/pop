#include "pylodsc.h"

PyObject* read_value(PyObject* events, PyObject* pdict){
    //increasing pointers to args
    Py_INCREF(events);
    Py_INCREF(pdict);

    PyObject* next = PyIter_Next(events);
    PyObject* method = PyString_FromString("read_value");

    // calling pdict's read value to parse value from event
    PyObject* value = PyObject_CallMethodObjArgs(pdict, method, next, NULL);

    // decreasing points back
    Py_DECREF(next);
    Py_DECREF(method);
    Py_DECREF(events);
    Py_DECREF(pdict);


    return value;
}

PyObject* read_this_value(PyObject* event, PyObject* pdict){
    //increasing pointers to args
    //std::cout<< "========== read this 1\n";
    Py_INCREF(event);
    Py_INCREF(pdict);
    //std::cout<< "========== read this 2\n";
    PyObject* method = PyString_FromString("read_value");
    // calling pdict's read value to parse value from event
    PyObject* value = PyObject_CallMethodObjArgs(pdict, method, event, NULL);
    //std::cout<< "========== read this 3\n";
    // decreasing points back
    Py_DECREF(method);
    Py_DECREF(event);
    Py_DECREF(pdict);
    //std::cout<< "========== read this 4\n";
    return value;
}



PyObject* read_obj_property_name(PyObject* events,  PyObject* pdict){
    //increasing pointers to args
    Py_INCREF(events);
    Py_INCREF(pdict);
    // calling read_value to get the string value as name of property
    PyObject* val = read_value(events, pdict);

    // decreasing points back
    Py_DECREF(events);
    Py_DECREF(pdict);

    return val;
}




PyObject* read_array(PyObject* events, PyObject* cls, PyObject* propname, PyObject* ctxt,  PyObject* pdict,  PyObject* deserializers, PyObject* TYPED, enum ParserState  state){
    //std::cout<< "==========1\n";
    Py_INCREF(events);
    Py_XINCREF(cls);
    Py_XINCREF(propname);
    Py_XINCREF(ctxt);
    Py_INCREF(deserializers);    
    //std::cout<< "==========2\n";
    // final result
    PyObject* res = PyList_New(0); // new ref
    //std::cout<< "==========3\n";
    if (state == EXPECTING_ARRAY_START){
        //std::cout<< "==========4\n";
        PyObject* first =  PyIter_Next(events);// new ref

        if (!call_pdict_check(pdict, (char *)"is_array_start",first )){
            //std::cout<< "==========5\n";
            // raise exception
        }
        Py_DECREF(first);

    }

    state = EXPECTING_VALUE_OR_ARRAY_END;
    PyObject* event = PyIter_Next(events); // new ref
    //std::cout<< "==========6\n";
    while (event != NULL){
        //std::cout<< "==========7\n";


        if (call_pdict_check(pdict, (char *)"is_array_end",event)){
            //std::cout<< "==========8\n";
            // check valid state
            if (state != EXPECTING_VALUE_OR_ARRAY_END){
                //std::cout<< "==========9\n";
                // raise UnexpectedStateException(POPState.EXPECTING_VALUE_OR_ARRAY_END, state, " wasn't expecting a end of object")
            }
            break;
        }else if (call_pdict_check(pdict, (char *)"is_value",event)){
            //std::cout<< "==========10\n";
            if (state != EXPECTING_VALUE_OR_ARRAY_END){
                //std::cout<< "==========11\n";
                // raise UnexpectedStateException(state, POPState.EXPECTING_VALUE_OR_ARRAY_END, " wasn't expecting a value")
            }
            PyObject* val = read_this_value(event,pdict);
            PyList_Append(res,  val);
            Py_DECREF(val);
        }else if (call_pdict_check(pdict, (char *)"is_obj_start",event)){
            //std::cout<< "==========12\n";
            PyObject* val = read_obj_as_value(events, cls, propname, ctxt, pdict, deserializers, TYPED);
            PyList_Append(res, val);
            Py_DECREF(val);
        }else if (call_pdict_check(pdict, (char *)"is_array_start",event)){
            //std::cout<< "==========13\n";
            PyObject* val = read_array(events,cls, propname,ctxt,pdict, deserializers, TYPED, EXPECTING_VALUE_OR_ARRAY_END);
            PyList_Append(res, val);
            Py_DECREF(val);
        }else{
            // raise ParseException('Unexpected event')
        }

        //std::cout<< "==========14\n";
        Py_DECREF(event); // dec ref
        event = PyIter_Next(events); // new ref
    }

    //std::cout<< "==========15\n";

    Py_DECREF(events);
    Py_XDECREF(cls);
    Py_XDECREF(propname);
    Py_XDECREF(ctxt);
    Py_DECREF(deserializers);    

    return res;
}


PyObject* read_obj(PyObject* events, PyObject* cls,  PyObject* ctxt,  PyObject* pdict,  PyObject* deserializers, PyObject* TYPED, enum ParserState  state){
    //increasing pointers to args
    Py_INCREF(events);
    Py_INCREF(cls);
    Py_INCREF(ctxt);
    Py_INCREF(pdict);
    Py_INCREF(deserializers);

    //std::cout<< "------------1\n";
    PyObject* obj =PyObject_CallObject(cls, NULL); // here is the problem :(((
    //std::cout<< "------------2\n";
    if (state == EXPECTING_OBJ_START){
        //std::cout<< "------------3\n";
        //setting to object start state
        PyObject* first =  PyIter_Next(events);
        // check if input is object
        if (!call_pdict_check(pdict, (char *)"is_obj_start",first )){   
            //std::cout<< "------------4\n";
            Py_DECREF(events);
            Py_DECREF(cls);
            Py_DECREF(ctxt);
            Py_DECREF(pdict);
            Py_DECREF(deserializers);

            Py_DECREF(first);
            // raise ObjectDeserializationException("Expected the content to start with an object!! But first event was: " + str(first))
        }        

        Py_DECREF(first);
    }
    //std::cout<< "------------5\n";

    state = EXPECTING_OBJ_PROPERTY_OR_END;    
    PyObject* propname = NULL;
    PyObject* event = PyIter_Next(events);
    //std::cout<< "------------6\n";
    while (event != NULL){
        //PyObject_Print(event, stdout, 0);                
        //std::cout<< "------------7\n";
        if (state == EXPECTING_OBJ_PROPERTY_OR_END){
            //std::cout<< "------------8\n";
            //end of object
            if (call_pdict_check(pdict, (char *)"is_obj_end",event)){
                //std::cout<< "------------9\n";
        //         //check valid state
                break;
            }else if (call_pdict_check(pdict, (char *)"is_obj_property_name",event)){
                //std::cout<< "------------10\n";
                //check valid state                
                propname = read_this_value(event, pdict); // new ref
                //std::cout  << "propname #####################";
                //PyObject_Print(propname, stdout, 0);
                // //std::cout<< "------------10.1\n";
                PyObject* decoded = decode_field_name(obj, propname);
                // //PyObject_Print(decoded, stdout, 0);                
                Py_DECREF(propname);
                propname = decoded;
                //setting next state
                state = EXPECTING_VALUE;
                //std::cout<< "------------10.3\n";
            }else{
                //std::cout<< "------------11\n";
                Py_DECREF(events);
                Py_DECREF(cls);
                Py_DECREF(ctxt);
                Py_DECREF(pdict);
                Py_DECREF(deserializers);
                
                Py_DECREF(event);
                 // raise ParseException(" Unexpected event when expected state is: " + str(state) + " while event was: " + str(event))
            }
                   
        }else if (state == EXPECTING_VALUE){
            //std::cout<< "------------12\n";
            PyObject* val = NULL;

            if (call_pdict_check(pdict, (char *)"is_array_start",event)){
                //std::cout<< "------------13\n";
                val = read_array(events,cls, propname,ctxt,pdict, deserializers, TYPED, EXPECTING_VALUE_OR_ARRAY_END);
            }else if (call_pdict_check(pdict, (char *)"is_value",event)){                
                //std::cout<< "------------14\n";
        //         //  read value
                val = read_this_value(event, pdict);
            }else if (call_pdict_check(pdict, (char *)"is_obj_start",event)){
                //std::cout<< "------------15\n";
                val = read_obj_as_value(events, cls, propname, ctxt, pdict, deserializers, TYPED);
            }else {
                //std::cout<< "------------16\n";
                Py_DECREF(events);
                Py_DECREF(cls);
                Py_DECREF(ctxt);
                Py_DECREF(pdict);
                Py_DECREF(deserializers);

                Py_DECREF(propname);
                Py_DECREF(event);
                // raise ParseException('unrecognized event when reading value: ' + str(event))
            }
            //std::cout<< "------------17\n";
            // setting to None if Null
            if(val == NULL){
                //std::cout<< "------------18\n";
                Py_INCREF(Py_None);
                val = Py_None;
            }
            //std::cout<< "------------19\n";
            // setting value        
            if(PyObject_IsInstance(obj, (PyObject* )&PyDict_Type)){                
                //std::cout<< "------------20\n";
               PyDict_SetItem(obj, propname, val);
            }else{
                //std::cout<< "------------21\n";
                PyObject_SetAttr(obj, propname, val);
            }
            //std::cout<< "------------22\n";
            Py_DECREF(propname);
            Py_DECREF(val);
            propname = NULL;
            state = EXPECTING_OBJ_PROPERTY_OR_END;

        }
        //std::cout<< "------------23\n";

        Py_DECREF(event);
        event = PyIter_Next(events);
    }

    //std::cout<< "------------24\n";

    // decreasing points back
    Py_DECREF(events);
    Py_DECREF(cls);
    Py_DECREF(ctxt);
    Py_DECREF(pdict);
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
    //std::cout<< "------------ decode 1\n";
    PyObject* result = NULL;

    if(PyObject_IsInstance(obj,(PyObject* )&PyDict_Type)){
        //std::cout<< "------------ decode 2\n";
        Py_INCREF(name);
        result = name;
    }else{
        //std::cout<< "------------ decode 3\n";
        //1. check to rename fields using decorator
        PyObject* cls = PyObject_GetAttrString(obj, (char *)"__class__"); // new ref
        if(PyObject_HasAttrString(cls, "_pylods")){
            //std::cout<< "------------ decode 4\n";
            PyObject* pylods = PyObject_GetAttrString(cls, "_pylods"); // new ref
            
            PyObject* namedecodemap = PyDict_GetItemString(PyDict_GetItem(pylods,cls),"namedecode"); // borrowed ref
            if (namedecodemap!=NULL){
                //std::cout<< "------------ decode 5\n";                
                result = PyDict_GetItem(namedecodemap, name); // borrowed ref
                Py_INCREF(result);
            }

            Py_DECREF(pylods); // dec ref
        }
        //std::cout<< "------------ decode 6\n";
        //# 2. check to ignore fields based on decorators TODO
        //# 3. remove private convention _

        PyObject* fields = fetch_obj_fields(obj); // new ref
        //PyObject_Print(fields, stdout, 0);
        //std::cout<< "------------ decode 7\n";

        PyObject* properties = extract_property_names(obj);

        //std::cout<< "------------ decode 8\n";
        if (contains(fields,name) || contains(properties, name)){
            //std::cout<< "------------ decode 9\n";
            Py_INCREF(name);
            result = name;        
        }else {
            //std::cout<< "------------ decode 10\n";
            // Py_INCREF(name);
            PyObject* privatename =PyString_FromString((char*)"_");
            PyString_Concat(  &privatename, name);
            if (contains(fields, privatename) || contains(properties, privatename) ){
                //std::cout<< "------------ decode 11\n";
                Py_INCREF(privatename);
                result = privatename;
            }else{
                //std::cout<< "------------ decode 12\n";
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
        //std::cout<< "------------ decode 13\n";


        Py_DECREF(fields);
        Py_DECREF(cls); // dec ref
    }
    //std::cout<< "------------ decode 4\n";
    Py_DECREF(name);
    Py_DECREF(obj);

    Py_INCREF(name);
    result = name;

    return result;

}



PyObject* call_pdict(PyObject* pdict, char* name,PyObject* args){
    Py_INCREF(pdict);
    
    PyObject* method = PyString_FromString(name);
    PyObject* val = PyObject_CallMethodObjArgs(pdict,method, args,NULL); // new ref
    
    Py_DECREF(method);
    Py_DECREF(pdict);
    return val;
}

int call_pdict_check(PyObject* pdict, char* name,PyObject* args){
    Py_INCREF(pdict);
    //std::cout << "+++++ check: 1\n";
    PyObject* method = PyString_FromString(name);

    PyObject* val = PyObject_CallMethodObjArgs(pdict,method, args,NULL); // new ref
    
    Py_DECREF(method);    

    //std::cout << "+++++ check: 2\n";
    int flag = 0;
    //std::cout << PyString_AS_STRING(PyObject_GetAttrString(PyObject_GetAttrString(pdict, (char *) "__class__"), (char *) "__name__"));
    //std::cout << PyString_AS_STRING(PyObject_GetAttrString(PyObject_GetAttrString(val, (char *) "__class__"), (char *) "__name__"));


    //std::cout << "+++++ check: 3\n";
    if(val == Py_True){
        //std::cout << "+++++ check: 4\n";
        flag = 1;
    }else if(val == Py_False){
        //std::cout << "+++++ check: 5\n";
        flag = 0;
    }else{
        //std::cout << "+++++ check: 6\n";
        Py_DECREF(pdict);
        Py_DECREF(val);
        throw std::runtime_error(" the pdict method should have returned Py_True or Py_False!!!");
    }
    //std::cout << "+++++ check: 7\n";
    Py_DECREF(pdict);
    Py_DECREF(val);

    return flag;
}


PyObject* read_obj_as_value(PyObject* events, PyObject* cls, PyObject* valname, PyObject* ctxt, PyObject* pdict,  PyObject* deserializers, PyObject* TYPED){

    Py_XINCREF(deserializers);
    Py_XINCREF(pdict);
    Py_XINCREF(ctxt);
    Py_XINCREF(cls);
    Py_XINCREF(valname);

    PyObject* val = NULL;
    
    PyObject* valcls = resolve(cls, valname, TYPED);

        
    PyObject* deserializer = lookup_deserializer(deserializers, valcls);
    //std::cout << "@@@@@@@@@@@@@@@@@@@@@@@";
    //PyObject_Print(deserializer, stdout, 0);


    if (deserializer != NULL){
        PyObject* count = PyInt_FromLong(1);
        PyObject* method = PyString_FromString("execute");

        val = PyObject_CallMethodObjArgs(deserializer, method, events, pdict, count, ctxt, NULL);

        Py_DECREF(deserializer);
        Py_DECREF(method);
        Py_DECREF(count);
    }else{
        val = read_obj(events, valcls , ctxt, pdict, deserializers, TYPED,EXPECTING_OBJ_PROPERTY_OR_END);
    }


        

    Py_XDECREF(valname);
    Py_XDECREF(cls);
    Py_DECREF(deserializers);
    Py_DECREF(pdict);
    Py_DECREF(ctxt);

    return val;
}

PyObject* lookup_deserializer(PyObject* deserializers, PyObject* cls){
    PyObject* deserializer = PyDict_GetItem(deserializers, cls);
    //std::cout << "-------- lookup 1\n";
    if (deserializer == NULL){
        //std::cout << "-------- lookup 2\n";
        //PyObject_Print(cls, stdout, 0);
        if(PyObject_HasAttrString(cls, "_pylods")){
            //std::cout << "-------- lookup 3\n";
            PyObject* pylods = PyObject_GetAttrString(cls,"_pylods");
            //PyObject_Print(pylods, stdout, 0);
            deserializer = PyDict_GetItemString(PyDict_GetItem(pylods,cls),"deserializer");
            //PyObject_Print(deserializer, stdout, 0);
            Py_XINCREF(deserializer);
            Py_DECREF(pylods);
        }
    }else{
        //std::cout << "-------- lookup 4\n";
        Py_INCREF(deserializer);
    }
    //std::cout << "-------- lookup 5\n";
    return deserializer;
}

PyObject* resolve(PyObject* cls, PyObject* valname, PyObject* TYPED){
    // PyObject *module = PyImport_ImportModule("msgpackstream.defs");
    // if (!module)
    //   throw std::runtime_error("can't import ");

    Py_XINCREF(cls);
    Py_XINCREF(valname);
    Py_XINCREF(TYPED);

    //std::cout << "------------- resolve 1\n";
    //PyObject_Print(cls, stdout,0);

    PyObject* valcls = NULL;
    if ((cls != NULL) && (valname != NULL)) {
        //std::cout << "------------- resolve 2\n";
        //attempt to resolve class of value
        PyObject* method = PyString_FromString("resolve");
        //PyObject_Print(TYPED, stdout,0);
        //PyObject_Print(method, stdout,0);
        //PyObject_Print(valname, stdout,0);
        //PyObject_Print(cls, stdout,0);
        valcls = PyObject_CallMethodObjArgs(TYPED, method, valname, cls, NULL);
        Py_DECREF(method);
        //PyObject_Print(valcls, stdout,0);

        if(valcls == Py_None){            
            //std::cout << "------------- resolve 3\n";
            if(PyObject_HasAttrString(cls, "_pylods")){
                //std::cout << "------------- resolve 4\n";
                PyObject* pylods = PyObject_GetAttrString(cls, "_pylods");
                //PyObject_Print(pylods, stdout,0);
                PyObject* typemap = PyDict_GetItemString(PyDict_GetItem(pylods,cls),"type");
                //PyObject_Print(typemap, stdout,0);
                if (typemap!=NULL){
                    //std::cout << "------------- resolve 5\n";
                    valcls = PyDict_GetItem(typemap, valname);
                    //PyObject_Print(valcls, stdout,0);
                }
                Py_DECREF(pylods);
            }

        }
        //std::cout << "------------- resolve 6\n";

    }else if((cls != NULL) && (valname == NULL) ){
        //std::cout << "------------- resolve 7\n";
        valcls = cls;        
    }

    //std::cout << "------------- resolve 8\n";

    
    if (valcls == NULL){   
        //std::cout << "------------- resolve 9\n"; 
        valcls = (PyObject* )&PyDict_Type;
    }
    // increasing pointers
    Py_INCREF(valcls);
    
    // decreasing pointers
    Py_XDECREF(cls);
    Py_XDECREF(valname);
    Py_XDECREF(TYPED);

    //std::cout << "------------- resolve 10\n";
    return valcls;

}





PyObject* attach_order(PyObject* ordermap, PyObject* fields){
    Py_INCREF(fields); // inc ref
    Py_INCREF(ordermap); // inc ref

    PyObject* attached = PyList_New(PyList_Size(fields)); // inc ref

    for(int i=0; i< PyList_Size(ordermap); i++){
        PyObject* name = PyList_GetItem(fields, i); // borrowed ref
        PyObject* entry = PyDict_GetItem(ordermap, name); // borrowed ref
        PyObject* order;
        if (entry != NULL ){            
            order = entry;            
        }else{
            order = PyInt_FromLong(9999999999); // new ref
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
    // PyObject* properties = PyList_New(0);
    //     PyObject* dirlist = PyObject_Dir(obj);
    //     for(int i; i< PyList_Size(dirlist); i++){            
    //         PyObject* prop = PyObject_GetAttr(cls, PyList_GetItem(dirlist,i));
    //         if(PyObject_IsInstance(prop, )){

    //             }
    //     isinstance(getattr(getattr(obj,"__class__"),p),property)            
    //     }
    Py_INCREF(obj);// inc ref
    PyObject* pValue =NULL;

    PyObject* pName = PyString_FromString("pylods.serialize"); // new ref
    //PyObject_Print(pName, stdout, 0);
    PyObject*  pModule = PyImport_Import(pName); // new ref
    //PyObject_Print(pModule, stdout, 0);
    Py_DECREF(pName); // dec ref

    if (pModule != NULL) {
        PyObject* pFunc = PyObject_GetAttrString(pModule, "extract_property_names"); // new ref
        //PyObject_Print(pFunc, stdout, 0);
        if (pFunc && PyCallable_Check(pFunc)) {
            pValue = PyObject_CallFunctionObjArgs(pFunc, obj,NULL);
            //PyObject_Print(pValue, stdout, 0);
        }
        Py_DECREF(pFunc);// dec ref
    }

    Py_DECREF(obj); // dec ref
    Py_DECREF(pModule); // dec ref

    return pValue;
}

     
PyObject* sort_tmp_obj_fields(PyObject* fields){
    //std::cout << "+++++ sort tmp: 1\n";
    Py_INCREF(fields);// inc ref
    PyObject* pValue = NULL;

    //std::cout << "+++++ sort tmp: 2\n";

    PyObject* pName = PyString_FromString("pylods.serialize"); // new ref
    PyObject*  pModule = PyImport_Import(pName); // new ref
    Py_DECREF(pName); // dec ref

    //std::cout << "+++++ sort tmp: 3\n";
    if (pModule != NULL) {
        //std::cout << "+++++ sort tmp: 4\n";
        PyObject* pFunc = PyObject_GetAttrString(pModule, "sort_obj_fields"); // new ref
        if (pFunc && PyCallable_Check(pFunc)) {
            //std::cout << "+++++ sort tmp: 5\n";
            pValue = PyObject_CallFunctionObjArgs(pFunc, fields, NULL);
        }
        Py_DECREF(pFunc);// dec ref
    }

    //std::cout << "+++++ sort tmp: 6\n";
    Py_DECREF(pModule); // dec ref
    Py_DECREF(fields);// dec ref

    return pValue;
}


PyObject* extract_names(PyObject* attached){
    Py_INCREF(attached); // inc ref
    PyObject* fields = PyList_New(PyList_Size(attached));// new ref

    for(int i=0; i< PyList_Size(attached); i++){
        PyObject* entry = PyList_GetItem(attached, i); // borrowed ref

        PyObject* name = PyTuple_GetItem(entry,0);// borrowed

        PyList_Append(attached, name); // creates new ref of args  -> decref
    }

    Py_DECREF(attached); // dec ref
    return fields;
}

PyObject* sort_obj_fields(PyObject* obj, PyObject* fields){
    //std::cout << "+++++ sort: 1\n";
    Py_INCREF(fields);
    Py_INCREF(obj);

    PyObject* result = NULL;

    PyObject* cls = PyObject_GetAttrString(obj, (char *)"__class__");// new ref
    //std::cout << "+++++ sort: 2\n";
    if(PyObject_HasAttrString(cls, "_pylods")){
        //std::cout << "+++++ sort: 3\n";

        PyObject* pylods = PyObject_GetAttrString(cls, "_pylods");// new ref
        //PyObject_Print(pylods, stdout, 0);
        //PyObject_Print(cls, stdout, 0);
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
    //std::cout << "+++++ sort: 4\n";

    Py_DECREF(cls);// dec ref
    Py_DECREF(fields);// dec ref
    Py_DECREF(obj);// dec ref

    return result;
}


PyObject* fetch_obj_fields(PyObject* obj){
    //std::cout << "+++++ fetch fields: 1\n";
    Py_INCREF(obj);

    PyObject* fields = PyList_New(0);// new ref

    //std::cout << "+++++ fetch fields: 2\n";

    if(PyObject_HasAttrString(obj,(char *)"__dict__")){
        
        //std::cout << "+++++ fetch fields: 3\n";

        PyObject* dict = PyObject_GetAttrString(obj, (char *)"__dict__"); // new ref
        //std::cout << "+++++ fetch fields: 3.1\n";
        PyObject* keys = PyDict_Keys(dict);// new ref
        //std::cout << "+++++ fetch fields: 3.2\n";
        for(int i=0; i< PyList_Size(keys); i++){
            //std::cout << "+++++ fetch fields: 3.3\n";
            if(!PyCallable_Check(PyDict_GetItem(dict,PyList_GetItem(keys,i)))){
                //std::cout << "+++++ fetch fields: 3.4\n";
                PyList_Append(fields,PyList_GetItem(keys,i));
            }
        }
        //std::cout << "+++++ fetch fields: 3.5\n";
        Py_DECREF(dict); // dec ref
        Py_DECREF(keys); // dec ref
    }else if(PyObject_HasAttrString(obj,(char *)"__slots__")){
        
        //std::cout << "+++++ fetch fields: 4\n";

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
        //std::cout << "+++++ fetch fields: 5\n";
        // do nothing
    }
    //PyObject_Print(fields, stdout, 0);

    PyObject* sorted = sort_obj_fields(obj, fields);
    Py_DECREF(fields);
    
    Py_DECREF(obj);

    //std::cout << "+++++ fetch fields: 6\n";

    return sorted;

}

