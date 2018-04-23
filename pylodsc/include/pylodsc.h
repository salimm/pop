
#ifndef PYLODC_H_INCLUDED
#define PYLODC_H_INCLUDED

#include "Dictionary.h"
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
PyObject* read_array(PyObject* events, PyObject* cls, PyObject* propname, PyObject* ctxt,  Dictionary* pdict,  PyObject* deserializers, PyObject* TYPED, enum ParserState  state=EXPECTING_ARRAY_START);
PyObject* read_obj(PyObject* events, PyObject* cls,  PyObject* ctxt,  Dictionary* pdict,  PyObject* deserializers, PyObject* TYPED, enum ParserState  state);

// obj related
PyObject* read_obj_as_value(PyObject* events, PyObject* cls, PyObject* valname, PyObject* ctxt, Dictionary* dict,  PyObject* deserializers, PyObject* TYPED);

// functions to use pdict
PyObject* read_value(PyObject* events, Dictionary* dict);
PyObject* read_obj_property_name(PyObject* events,  PyObject* pdict,  PyObject* deserializers);

// type related
PyObject* resolve(PyObject* cls, PyObject* valname, PyObject* TYPED);
PyObject* fetch_obj_fields(PyObject* obj);
PyObject* decode_field_name(PyObject* obj, PyObject* name);
PyObject* extract_property_names(PyObject* obj);
// fields related


PyObject* lookup_deserializer(PyObject* deserializers, PyObject* cls);
PyObject* convert_state(enum ParserState);

// Iterator
PyObject* pylodscbackend_ClassEventIterator_iter(PyObject *self);

PyObject* pylodscbackend_ClassEventIterator_next(PyObject *self) ; 

void classeventiterator_dealloc(PyObject* self);

PyObject * create_class_event_iterator(PyObject* events, Dictionary* dict, long int count);



#endif

