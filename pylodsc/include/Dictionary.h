#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <Python.h>
 


 // Circle class declaration
class Dictionary {
private:   // Accessible by members of this class only
   
public:    // Accessible by ALL
	
	// Dictionary();
	/**
            indicates if the given or a tuple representing an start of object event
            :param event:
    */
	virtual int is_obj_start(PyObject* event) = 0;

	/**
            indicates if the given or a tuple representing an end of object event
            :param event
    */
	virtual int is_obj_end(PyObject* event) = 0;

	/**
            indicates if the given or tuple representing an event that represents a raw value
            :param event:
    */
	virtual int is_value(PyObject* event) = 0;

	/**
            indicates if the given event or tuple representing an event is a property name. 
          This library expects to receive the value of the property next. If this value is 
          an object or array, start event for an array or map is expected.
          
        :param event:
    */
	virtual int is_obj_property_name(PyObject* event) = 0;

	/**
             indicates if the given or tuple representing an event that indicates start of an array
            :param event:
    */
	virtual int is_array_start(PyObject* event) = 0;

	/**
                indicates if the given or tuple representing an event that indicates end of an array
            :param event:
    */
	virtual int is_array_end(PyObject* event) = 0;

	/**
            Returns the value in the given event or tuple representing the event
    */
	virtual PyObject* read_value(PyObject* event) = 0;


	virtual PyObject* get_pdict() = 0;

	virtual ~Dictionary() = 0;


};


#endif