#include <Python.h>
#include "pylodsc.h"


enum ParserState convert_state(PyObject* state){
    return static_cast<ParserState>(int(PyInt_AsLong(state)));
}


PyObject* read_obj_api(PyObject *self, PyObject *args){

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

    PyObject*  result = read_obj(events,cls,ctxt, pdict, deserializers, TYPED, converted_state);


    Py_DECREF(TYPED);
    Py_DECREF(state);

    return result;

}

PyObject* read_array_api(PyObject *self, PyObject *args){

    
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

    PyObject*  result = read_array(events, cls, propname,ctxt, pdict,  deserializers, TYPED, converted_state);

    Py_DECREF(TYPED);
    Py_DECREF(state);

    return result;

}

PyMODINIT_FUNC
initpylodscext(void)
{
    static PyMethodDef FindMethods[] = {

        {"read_obj",  read_obj_api, METH_VARARGS,
            "read object"},
        {"read_array",  read_array_api, METH_VARARGS,
            "read array"},
        {NULL, NULL, 0, NULL}        /* Sentinel */
    };

    (void) Py_InitModule("pylodscext", FindMethods);
}


int
main(int argc, char *argv[])
{
    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(argv[0]);

    /* Initialize the Python interpreter.  Required. */
    Py_Initialize();

    /* Add a static module */
    initpylodscext();         
    
    // process(NULL, NULL);
}

