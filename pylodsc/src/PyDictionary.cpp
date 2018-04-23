#include "PyDictionary.h"
#include <Python.h>
 

int call_pdict_check(PyObject* pdict, char* name,PyObject* args);
 

PyDictionary::PyDictionary(PyObject* pdict){
    this->pdict = pdict;
}

int PyDictionary::is_obj_start(PyObject* event){
    return call_pdict_check(this->pdict, (char*)"is_obj_start", event);
}

int PyDictionary::is_obj_end(PyObject* event){
    return call_pdict_check(this->pdict, (char*)"is_obj_end", event);    
}


int PyDictionary::is_value(PyObject* event){
    return call_pdict_check(this->pdict, (char*)"is_value", event);
}


int PyDictionary::is_obj_property_name(PyObject* event){
    return call_pdict_check(this->pdict, (char*)"is_obj_property_name", event);
}


int PyDictionary::is_array_start(PyObject* event){
    return call_pdict_check(this->pdict, (char*)"is_array_start", event);
}


int PyDictionary::is_array_end(PyObject* event){
    return call_pdict_check(this->pdict, (char*)"is_array_end", event);
}


PyObject* PyDictionary::read_value(PyObject* event){
    Py_INCREF(event);
    PyObject* method = PyString_FromString("read_value");

    // calling pdict's read value to parse value from event
    PyObject* value = PyObject_CallMethodObjArgs(pdict, method, event, NULL);

    Py_DECREF(method);
    Py_DECREF(event);

    return value;
}


PyObject* PyDictionary::get_pdict(){
    return this->pdict;
}



int call_pdict_check(PyObject* pdict, char* name,PyObject* args){
    Py_INCREF(pdict);
    PyObject* method = PyString_FromString(name);

    PyObject* val = PyObject_CallMethodObjArgs(pdict,method, args,NULL); // new ref
    
    Py_DECREF(method);    

    int flag = 0;


    if(val == Py_True){
        flag = 1;
    }else if(val == Py_False){
        flag = 0;
    }else{
        Py_DECREF(pdict);
        Py_DECREF(val);
        throw std::runtime_error(" the pdict method should have returned Py_True or Py_False!!!");
    }
    Py_DECREF(pdict);
    Py_DECREF(val);

    return flag;
}