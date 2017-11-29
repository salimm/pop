'''
Created on Nov 26, 2017

@author: Salim
'''
from abc import ABCMeta, abstractmethod
from pylods.error import ObjectDeserializationException, UnexpectedStateException, \
    ParseException
from enum import Enum
from _pyio import __metaclass__


        
        
    
class POPState(Enum):
    
    EXPECTING_OBJ_START = 1
    EXPECTING_OBJ_PROPERTY_OR_END = 3
    EXPECTING_VALUE = 4
    EXPECTING_ARRAY_START = 5
    EXPECTING_VALUE_OR_ARRAY_END = 6
    


class Typed():
    '''
        Basic class for classes that will support typed object based serialization. 
        Extending this class is not required to support type resolution however, 
        it will make the code cleaner
    '''
    
    _types = {}
        
        
    @classmethod
    def resolve(cls, propname, objcls=None):
        '''
            resolve type of the class property for the class. If objcls is not set then 
            assume that cls argument (class that the function is called from) is the class 
            we are trying to resolve the type for 
        :param cls:
        :param objcls:
        :param propname:
        '''
        # object class is not given
        if objcls is None:
            objcls = cls
        # get type
        typemap = cls._types.get(objcls)
        if(typemap is not None):
            # nothing registered for this property
            return typemap.get(propname, None)
        # type map didn't exist so none
        return None
    
    @classmethod
    def register_type(cls, propname, valcls, objcls=None):
        '''
            register type for class 
             
        :param cls:
        :param propname:
        :param valcls:
        :param objcls:
        '''
        # object class is not given
        if objcls is None:
            objcls = cls
        # get type map for objcls
        typemap = cls._types.get(objcls, None)
        # if type map doesn't exist then remove it
        if not typemap:
            typemap = {}
            cls._types[objcls] = typemap
        # register value class for property
        typemap[propname] = valcls
        
        



class Module():
    '''
        A Module contains customer serializer/deserializers.
    '''
    
    __metaclass__ = ABCMeta
    __slots__ = ['_serializers', '_deserializers']
    
    def __init__(self):
        self._serializers = []
        self._deserializers = []
    
    def add_serializer(self, serializer):
        self._serializers.append(serializer)
    
    def add_deserializer(self, deserializer):
        self._deserializers.append(deserializer)
    
    
    def get_serializers(self):
        return self._serializers
    
    def get_deserializers(self):
        return self._deserializers
    
    serializers = property(get_serializers)
    deserializers = property(get_deserializers)






class Parser():
    '''
        Main Parser class. This parser requires a dictionary to serialize/ deserialize from an input stream
    '''
    
    __classmeta__ = ABCMeta
    
    __slots__ = ['_pdict','_serializers', '_deserializers']
    
    def __init__(self, pdict):
        self._pdict = pdict
        self._serializers = {}
        self._deserializers = {}
    
    
    def parse(self, instream, cls):
        '''
            If called it will parse input and generate output
        '''
        events = self._gen_events(instream)
        return self.parse_obj(events, cls)
    
    
        
            
        
    def parse_obj(self, events, cls, state=POPState.EXPECTING_OBJ_START):
        obj = cls()
        if state is POPState.EXPECTING_OBJ_START:
            # setting to object start state
            first = events.next();
            # check if input is object
            if not self._pdict.is_obj_start(first):
                raise ObjectDeserializationException("Expected the content to start with an object!! But first event was: " + str(first))
        
        state = POPState.EXPECTING_OBJ_PROPERTY_OR_END
        propname = None
        event = events.next()
        while event:
            if state is POPState.EXPECTING_OBJ_PROPERTY_OR_END:
                # end of object
                if self._pdict.is_obj_end(event):
                    # check valid state
                    if state is not POPState.EXPECTING_OBJ_PROPERTY_OR_END:
                        raise UnexpectedStateException(state, POPState.EXPECTING_OBJ_PROPERTY_OR_END, " wasn't expecting a end of object")
                    return obj
                elif self._pdict.is_obj_property_name(event):
                    # check valid state
                    if state is not POPState.EXPECTING_OBJ_PROPERTY_OR_END:
                        raise UnexpectedStateException(state, POPState.EXPECTING_OBJ_PROPERTY_OR_END, " the received object property name was not expected")
                    propname = self._pdict.read_value(event);
                    # setting next state
                    state = POPState.EXPECTING_VALUE
                else:
                    raise ParseException(" Unexpected event when expected state is: " + str(state) + " while event was: " + str(event))
            elif state is POPState.EXPECTING_VALUE:
                val = None
                if self._pdict.is_array_start(event):
                    val = self.parse_array(events, POPState.EXPECTING_VALUE_OR_ARRAY_END)
                elif self._pdict.is_value(event):
                    # check valid state
                    if state is not POPState.EXPECTING_VALUE:
                        raise UnexpectedStateException(state, POPState.EXPECTING_VALUE, " wasn't expecting a value")
                    # read value
                    val = self._pdict.read_value(event)
                elif self._pdict.is_obj_start(event):
                    # attempt to resolve class of value
                    valcls = Typed.resolve(propname, cls)
                    if valcls is None:  # if not resolved simply convert to dictionary
                        valcls = dict
                    deserializer = self._deserializers.get(valcls, None)
                    if deserializer:
                        val = deserializer.execute(events, self._pdict, count=1)
                    else:    
                        val = self.parse_obj(events, valcls, POPState.EXPECTING_OBJ_PROPERTY_OR_END)
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
#             try:
            event = events.next()
#             except StopIteration:
#                 event = None
            
        raise UnexpectedStateException(state, POPState.EX, " wasn't expecting a value")
            
    def parse_array(self, events, state=POPState.EXPECTING_ARRAY_START):
        res = []
        
        if state is POPState.EXPECTING_ARRAY_START:
            # setting to object start state
            first = events.next();
            # check if input is object
            if not self._pdict.is_array_start(first):
                raise UnexpectedStateException(POPState.EXPECTING_ARRAY_START, " didn't received start of array!!!")
            
        event = events.next()
        while event:
            # end of object
            if self._pdict.is_array_end(event):
                # check valid state
                if state is not POPState.EXPECTING_VALUE_OR_ARRAY_END:
                    raise UnexpectedStateException(POPState.EXPECTING_VALUE_OR_ARRAY_END, state, " wasn't expecting a end of object")
                return res
            elif self._pdict.is_value(event):
                # check valid state
                if state is not POPState.EXPECTING_VALUE_OR_ARRAY_END:
                    raise UnexpectedStateException(state, POPState.EXPECTING_VALUE_OR_ARRAY_END, " wasn't expecting a value")
                val = self._pdict.read_value(event)
                res.append(val)
            elif self._pdict.is_obj_start(event):
                val = self.parse_obj(events, dict, POPState.EXPECTING_OBJ_PROPERTY_OR_END)
                res.append(val)
            else:
                raise Exception('Unexpected event')
            
            
            
            event = events.next()
        
        
        
    def _gen_events(self, instream):
        '''
            generates events using the specific parser
        :param instream:
        '''
        return self._pdict.gen_events(instream)
        
    
      
    
    def register_module(self, module):
        for serializer in module.serializers:
            self.register_serializer(serializer)
            
        for deserializer in module.deserializers:
            self.register_deserializer(deserializer)
        
    
    def register_serializer(self,serializer):
        self._serializers[serializer.handled_class()] = serializer
    
    def register_deserializer(self,deserializer):
        self._deserializers[deserializer.handled_class()] = deserializer




