

#include "Dictionary.h"
#include "pylodsc.h"
#include "PyDictionary.h"

#ifndef Py_PYLODSCAPI_H
#define Py_PYLODSCAPI_H
#ifdef __cplusplus
extern "C" {
#endif



/* Header file for pylodscapi */

/* C API functions */

#define PYLODSC_ReadArray_NUM 0
#define PYLODSC_ReadArray_RETURN PyObject*
#define PYLODSC_ReadArray_PROTO (PyObject* events, PyObject* cls, PyObject* propname, PyObject* ctxt,  Dictionary* pdict,  PyObject* deserializers, PyObject* TYPED, enum ParserState  state)

#define PYLODSC_ReadObject_NUM 1
#define PYLODSC_ReadObject_RETURN PyObject*
#define PYLODSC_ReadObject_PROTO (PyObject* events, PyObject* cls,  PyObject* ctxt,  Dictionary* pdict,  PyObject* deserializers, PyObject* TYPED, enum ParserState  state)

#define PYLODSC_CreateClassEventIterator_NUM 2
#define PYLODSC_CreateClassEventIterator_RETURN PyObject*
#define PYLODSC_CreateClassEventIterator_PROTO (PyObject* events, Dictionary* dict, long int count)

/* Total number of C API pointers */
#define PYLODSC_API_pointers 3


#ifdef PYLODSC_MODULE
/* This section is used when compiling pylodscapi.cpp */

static PYLODSC_ReadArray_RETURN PYLODSC_ReadArray PYLODSC_ReadArray_PROTO;
static PYLODSC_ReadObject_RETURN PYLODSC_ReadObject PYLODSC_ReadObject_PROTO;
static PYLODSC_CreateClassEventIterator_RETURN PYLODSC_CreateClassEventIterator PYLODSC_CreateClassEventIterator_PROTO;

#else
/* This section is used in modules that use pylodsc module's API */

static void **PYLODSC_API;

#define PYLODSC_ReadArray \
 (*(PYLODSC_ReadArray_RETURN (*)PYLODSC_ReadArray_PROTO) PYLODSC_API[PYLODSC_ReadArray_NUM])

#define PYLODSC_ReadObject \
 (*(PYLODSC_ReadObject_RETURN (*)PYLODSC_ReadObject_PROTO) PYLODSC_API[PYLODSC_ReadObject_NUM])

#define PYLODSC_CreateClassEventIterator \
 (*(PYLODSC_CreateClassEventIterator_RETURN (*)PYLODSC_CreateClassEventIterator_PROTO) PYLODSC_API[PYLODSC_CreateClassEventIterator_NUM])
/* Return -1 and set exception on error, 0 on success. */
static int import_pylodsc(void){
    PyObject *module = PyImport_ImportModule("pylodscbackend");
    if (module != NULL) {
        PyObject *c_api_object = PyObject_GetAttrString(module, "_api");
        if (c_api_object == NULL)
            return -1;
        if (PyCObject_Check(c_api_object))
            PYLODSC_API = (void **)PyCObject_AsVoidPtr(c_api_object);
        Py_DECREF(c_api_object);
    }
    return 0;
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* !defined(Py_PYLODSCAPI_H) */
