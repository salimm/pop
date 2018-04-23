#include <Python.h>
#define PYLODSC_MODULE
#include "pylodscapi.h"



enum ParserState convert_state(PyObject* state){
    return static_cast<ParserState>(int(PyInt_AsLong(state)));
}

static PyObject* PYLODSC_ReadArray(PyObject* events, PyObject* cls, PyObject* propname, PyObject* ctxt,  Dictionary* dict,  PyObject* deserializers, PyObject* TYPED, enum ParserState  state){
    return read_array(events, cls, propname,ctxt, dict,  deserializers, TYPED, state);
}


static PyObject* PYLODSC_ReadObject(PyObject* events, PyObject* cls,  PyObject* ctxt,  Dictionary* dict,  PyObject* deserializers, PyObject* TYPED, enum ParserState  state){
    return read_obj(events,cls,ctxt, dict, deserializers, TYPED, state);
}

static PyObject* read_object_api(PyObject *self, PyObject *args){
    PyObject* events;
    PyObject* cls;
    PyObject* ctxt;
    PyObject* pdict;
    PyObject* deserializers;
    PyObject*  state;

    PyObject* pName = PyString_FromString("pylods.deserialize"); // new ref
    PyObject*  pModule = PyImport_Import(pName); // new ref
    Py_DECREF(pName); // dec ref
    if (pModule == NULL) {
        // throw exception
    }    

    PyObject* TYPED = PyObject_GetAttrString(pModule, "Typed");
    Py_DECREF(pModule);

    if (!PyArg_ParseTuple(args, "OOOOOO",  &events, &cls, &ctxt, &pdict, &deserializers, &state))
        return NULL;

    enum ParserState  converted_state = convert_state(state);
    PyDictionary* dict = new PyDictionary(pdict);
    PyObject*  result = PYLODSC_ReadObject(events,cls,ctxt, (Dictionary*)dict, deserializers, TYPED, converted_state);
    delete dict;

    Py_DECREF(TYPED);
    Py_DECREF(state);
    return result;
}


static PyObject* read_array_api(PyObject *self, PyObject *args){

    
    PyObject* events;
    PyObject* cls;
    PyObject* propname;
    PyObject* ctxt;
    PyObject* pdict;
    PyObject* deserializers;
    PyObject*  state;

    PyObject* pName = PyString_FromString("pylods.deserialize"); // new ref
    PyObject*  pModule = PyImport_Import(pName); // new ref
    Py_DECREF(pName); // dec ref

    if (pModule == NULL) {
        // throw exception
    }    
    PyObject* TYPED = PyObject_GetAttrString(pModule, "Typed");
    Py_DECREF(pModule);    

    if (!PyArg_ParseTuple(args, "OOOOOOO",  &events, &cls, &propname, &ctxt, &pdict, &deserializers, &state))
        return NULL;

    enum ParserState  converted_state = convert_state(state);

    PyDictionary* dict = new PyDictionary(pdict);
    PyObject*  result = PYLODSC_ReadArray(events, cls, propname,ctxt, (Dictionary*)dict,  deserializers, TYPED, converted_state);
    delete dict;

    Py_DECREF(TYPED);
    Py_DECREF(state);

    return result;

}

PyObject * create_Class_event_iterator_api(PyObject *self, PyObject *args){
    PyObject* events;
    long int count;
    PyObject* pdict;
    if (!PyArg_ParseTuple(args, "OlO", &events, &count, &pdict))  return NULL;
    Py_INCREF(pdict);
    PyDictionary* dict = new PyDictionary(pdict);
    return create_class_event_iterator(events,dict, count );
}

static PyObject * PYLODSC_CreateClassEventIterator(PyObject* events, Dictionary* dict, long int count){
    return create_class_event_iterator(events,dict, count );
}

static PyObject * create_class_event_iterator_api(PyObject *self, PyObject *args){
    PyObject* events;
    long int count;
    PyObject* pdict;
    if (!PyArg_ParseTuple(args, "OlO", &events, &count, &pdict))  return NULL;
    Py_INCREF(pdict);
    PyDictionary* dict = new PyDictionary(pdict);
    return PYLODSC_CreateClassEventIterator(events,dict, count );
}



PyMODINIT_FUNC
initpylodscbackend(void){
    static PyMethodDef FindMethods[] = {

        {"read_obj",  read_object_api, METH_VARARGS,
            "read object"},
        {"read_array",  read_array_api, METH_VARARGS,
            "read array"},
        {"create_ClassEventIterator",  create_class_event_iterator_api, METH_VARARGS,
            "creates iterator for custom class parsers"},
        {NULL, NULL, 0, NULL}        /* Sentinel */
    };


    PyObject *m;
    static void *PYLODSC_API[PYLODSC_API_pointers];
    PyObject *c_api_object;

    m=Py_InitModule("pylodscbackend", FindMethods);
    if (m == NULL)
        return;

    /* Initialize the C API pointer array */
    PYLODSC_API[PYLODSC_ReadArray_NUM] = (void *)PYLODSC_ReadArray;    
    PYLODSC_API[PYLODSC_ReadObject_NUM] = (void *)PYLODSC_ReadObject;
    PYLODSC_API[PYLODSC_CreateClassEventIterator_NUM] = (void *)PYLODSC_CreateClassEventIterator;

    /* Create a CObject containing the API pointer array's address */
    c_api_object = PyCObject_FromVoidPtr((void *)PYLODSC_API, NULL);
    if (c_api_object != NULL)
        PyModule_AddObject(m, "_api", c_api_object);

}


int main(int argc, char *argv[])
{
    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(argv[0]);

    /* Initialize the Python interpreter.  Required. */
    Py_Initialize();

    /* Add a static module */
    initpylodscbackend();         
}

