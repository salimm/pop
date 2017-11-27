'''
Created on Nov 26, 2017

@author: Salim
'''
from abc import ABCMeta, abstractmethod
from pop.error import ObjectDeserializationException, UnexpectedStateException, \
    ParseException
from enum import Enum


class POPDictionary(object):
    
    __classmeta__ = ABCMeta
    
    
    @abstractmethod
    def gen_events(self, instream):
        '''
            generates events from parsing an input stream
        :param instream:
        '''
        
    
    @abstractmethod
    def is_obj_start(self, event):
        '''
            indicates if the given or a tuple representing an start of object event
            :param event:
        '''
    @abstractmethod
    def is_obj_end(self, event):
        '''
            indicates if the given or a tuple representing an end of object event
            :param event:
        '''
        
    @abstractmethod
    def is_value(self, event):
        '''
            indicates if the given or tuple representing an event that represents a raw value
            :param event:
        '''
    @abstractmethod    
    def is_obj_property_name(self, event):
        '''
         indicates if the given event or tuple representing an event is a property name. 
          This library expects to receive the value of the property next. If this value is 
          an object or array, start event for an array or map is expected.
          
        :param event:
        '''
         
    @abstractmethod
    def is_array_start(self, event):
        '''
            indicates if the given or tuple representing an event that indicates start of an array
            :param event:
        '''
    
    @abstractmethod
    def is_array_end(self, event):
        '''
            indicates if the given or tuple representing an event that indicates end of an array
            :param event:
        '''
        
    def read_value(self, event):
        '''
            Returns the value in the given event or tuple representing the event
        '''
        
        
    
class POPState(Enum):
    
    EXPECTING_OBJ_START = 1
    EXPECTING_OBJ_PROPERTY_OR_END = 3
    EXPECTING_VALUE = 4
    EXPECTING_ARRAY_START = 5
    EXPECTING_VALUE_OR_ARRAY_END = 6
    


class Typed():
    
    _types = {}
        
        
    @classmethod
    def resolve(cls, objcls, propname):
        typemap = cls._types.get(objcls)
        if(typemap is not None):
            return typemap.get(propname,None)
        return None
    
    @classmethod
    def register_type(cls, propname, valcls):
        typemap = cls._types.get(cls,None)
        if not typemap:
            typemap = {}
            cls._types[cls] = typemap
        typemap[propname] = valcls
        
        


class POPParser():
    
    __classmeta__ = ABCMeta
    
    def __init__(self, popdict):
        self._popdict = popdict
    
    
    def parse(self, instream, cls):
        '''
            If called it will parse input and generate output
        '''
        events = self._gen_events(instream)
        return self._parse_obj(events, cls)
        
            
        
    def _parse_obj(self, events, cls, state=POPState.EXPECTING_OBJ_START):
        obj = cls()
        if state is POPState.EXPECTING_OBJ_START:
            # setting to object start state
            first = events.next();
            # check if input is object
            if not self._popdict.is_obj_start(first):
                raise ObjectDeserializationException("Expected the content to start with an object!! But first event was: " + str(first))
        
        state = POPState.EXPECTING_OBJ_PROPERTY_OR_END
        propname = None
        event = events.next()
        while event:
#             print(event)
#             print(state)
            if state is POPState.EXPECTING_OBJ_PROPERTY_OR_END:
                # end of object
                if self._popdict.is_obj_end(event):
                    # check valid state
                    if state is not POPState.EXPECTING_OBJ_PROPERTY_OR_END:
                        raise UnexpectedStateException(state, POPState.EXPECTING_OBJ_PROPERTY_OR_END, " wasn't expecting a end of object")
                    return obj
                elif self._popdict.is_obj_property_name(event):
                    # check valid state
                    if state is not POPState.EXPECTING_OBJ_PROPERTY_OR_END:
                        raise UnexpectedStateException(state, POPState.EXPECTING_OBJ_PROPERTY_OR_END, " the received object property name was not expected")
                    propname = self._popdict.read_value(event);
                    # setting next state
                    state = POPState.EXPECTING_VALUE
                else:
                    raise ParseException(" Unexpected event when expected state is: " + str(state) + " while event was: " + str(event))
            elif state is POPState.EXPECTING_VALUE:
                val = None
                if self._popdict.is_array_start(event):
                    val = self._parse_array(events, POPState.EXPECTING_VALUE_OR_ARRAY_END)
                elif self._popdict.is_value(event):
                    # check valid state
                    if state is not POPState.EXPECTING_VALUE:
                        raise UnexpectedStateException(state, POPState.EXPECTING_VALUE, " wasn't expecting a value")
                    val = self._popdict.read_value(event)
                elif self._popdict.is_obj_start(event):
                    type = Typed.resolve(cls,propname)
                    if type is None:
                        type = dict
                    val = self._parse_obj(events, type, POPState.EXPECTING_OBJ_PROPERTY_OR_END)
                else:
                    raise Exception('unrecognized event when reading value: ' + str(event))
                # setting value
                if cls is dict:
                    obj[propname] = val
                else:
                    setattr(obj, propname, val)
                propname = None
                state = POPState.EXPECTING_OBJ_PROPERTY_OR_END
            
            # continue
            event = events.next()
            
        raise UnexpectedStateException(state, POPState.EX, " wasn't expecting a value")
            
    def _parse_array(self, events, state=POPState.EXPECTING_ARRAY_START):
        res = []
        
        if state is POPState.EXPECTING_ARRAY_START:
            # setting to object start state
            first = events.next();
            # check if input is object
            if not self._popdict.is_array_start(first):
                raise UnexpectedStateException(POPState.EXPECTING_ARRAY_START, " didn't received start of array!!!")
            
        event = events.next()
        while event:
            # end of object
            if self._popdict.is_array_end(event):
                # check valid state
                if state is not POPState.EXPECTING_VALUE_OR_ARRAY_END:
                    raise UnexpectedStateException(POPState.EXPECTING_VALUE_OR_ARRAY_END, state, " wasn't expecting a end of object")
                return res
            elif self._popdict.is_value(event):
                # check valid state
                if state is not POPState.EXPECTING_VALUE_OR_ARRAY_END:
                    raise UnexpectedStateException(state, POPState.EXPECTING_VALUE_OR_ARRAY_END, " wasn't expecting a value")
                val = self._popdict.read_value(event)
                res.append(val)
            elif self._popdict.is_obj_start(event):
                val = self._parse_obj(events, dict, POPState.EXPECTING_OBJ_PROPERTY_OR_END)
                res.append(val)
            else:
                raise Exception('Unexpected event')
            
            
            
            event = events.next()
        
        
            
            

        
        
    @abstractmethod
    def _gen_events(self, instream):
        '''
            generates events using the specific parser
        :param instream:
        '''
        return self._popdict.gen_events(instream)
        
    
        
