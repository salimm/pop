#ifndef PY_DICTIONARY_H
#define PY_DICTIONARY_H

#include "Dictionary.h"
#include <Python.h>
#include <stdexcept>
#include <iostream>
 


 // Circle class declaration
class PyDictionary : public Dictionary{
private:   // Accessible by members of this class only

    PyObject* pdict;
   
public:    // Accessible by ALL

    PyDictionary(PyObject* pdict);
	/**
            indicates if the given or a tuple representing an start of object event
            :param event:
    */
    int is_obj_start(PyObject* event) ;

	/**
            indicates if the given or a tuple representing an end of object event
            :param event
    */
	int is_obj_end(PyObject* event) ;

	/**
            indicates if the given or tuple representing an event that represents a raw value
            :param event:
    */
	int is_value(PyObject* event) ;

	/**
            indicates if the given event or tuple representing an event is a property name. 
          This library expects to receive the value of the property next. If this value is 
          an object or array, start event for an array or map is expected.
          
        :param event:
    */
	int is_obj_property_name(PyObject* event) ;

	/**
             indicates if the given or tuple representing an event that indicates start of an array
            :param event:
    */
	int is_array_start(PyObject* event);

	/**
                indicates if the given or tuple representing an event that indicates end of an array
            :param event:
    */
	int is_array_end(PyObject* event) ;

	/**
            Returns the value in the given event or tuple representing the event
    */
	PyObject* read_value(PyObject* event);


    PyObject* get_pdict();

    ~PyDictionary(){};


};


#endif