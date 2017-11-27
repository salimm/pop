'''
Created on Nov 26, 2017

@author: Salim
'''


class ObjectDeserializationException(Exception):
    '''
        Exception to FormatTypeNotAcceptable
    '''
    def __init__(self, msg):
        Exception.__init__(self, msg)
        
        
class UnexpectedStateException(Exception):
    '''
        Exception to FormatTypeNotAcceptable
    '''
    def __init__(self, expected, state=None, msg=None):
        txt = "Parser originally expected " + str(expected)
        if state is not None:
            txt = txt + " but state was " + str(state)
        
        if msg is not None:
            txt = txt + " with message: " + msg;
        Exception.__init__(self, txt)

class ParseException(Exception):
    '''
        Exception to FormatTypeNotAcceptable
    '''
    def __init__(self, msg):
        Exception.__init__(self, msg)