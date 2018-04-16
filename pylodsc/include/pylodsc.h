
#ifndef PYLODC_H_INCLUDED
#define PYLODC_H_INCLUDED

#include <iostream>
#include <Python.h>


enum ParserState{
    EXPECTING_OBJ_START = 1,
    EXPECTING_OBJ_PROPERTY_OR_END = 3,
    EXPECTING_VALUE = 4,
    EXPECTING_ARRAY_START = 5,
    EXPECTING_VALUE_OR_ARRAY_END = 6
};


// main function to be used in python
PyObject* read_array(PyObject* events, PyObject* cls, PyObject* propname, PyObject* ctxt,  PyObject* pdict,  PyObject* deserializers, PyObject* TYPED, enum ParserState  state=EXPECTING_ARRAY_START);
PyObject* read_obj(PyObject* events, PyObject* cls,  PyObject* ctxt,  PyObject* pdict,  PyObject* deserializers, PyObject* TYPED, enum ParserState  state);

// obj related
PyObject* read_obj_as_value(PyObject* events, PyObject* cls, PyObject* valname, PyObject* ctxt, PyObject* pdict,  PyObject* deserializers, PyObject* TYPED);

// functions to use pdict
PyObject* read_value(PyObject* events, PyObject* pdict);
PyObject* call_pdict(PyObject* pdict, char* name,PyObject* args);
int call_pdict_check(PyObject* pdict, char* name,PyObject* args);
PyObject* read_obj_property_name(PyObject* events,  PyObject* pdict,  PyObject* deserializers);

// type related
PyObject* resolve(PyObject* cls, PyObject* valname, PyObject* TYPED);
PyObject* fetch_obj_fields(PyObject* obj);
PyObject* decode_field_name(PyObject* obj, PyObject* name);
PyObject* extract_property_names(PyObject* obj);
// fields related


PyObject* lookup_deserializer(PyObject* deserializers, PyObject* cls);
PyObject* convert_state(enum ParserState);






#endif

