'''
Created on Dec 26, 2017

@author: Salim
'''
from pylods.deserialize import EventStream, Typed, POPState
from umsgpack import UnsupportedTypeException
from pylods.error import UnexpectedStateException,\
    ObjectDeserializationException, ParseException
from abc import ABCMeta
from pylods.serialize import DataFormatGenerator


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

class ObjectMapper(DataFormatGenerator):
    '''
        Main Parser class to deserialize to objects, values and etc. This parser requires a dictionary to serialize/ deserialize from an input stream
    '''
    
    __classmeta__ = ABCMeta
    
    __slots__ = [ '_deserializers', '_events']
    
    
    def __init__(self, pdict, events):
        super(ObjectMapper, self).__init__(pdict) 
        self._deserializers = {}
        self._events = self.prepare_input(events)
    
    
    def read_value(self):
        val = self._pdict.read_value(self._events.next())
        return val
            

    def read_obj_propery_name(self):
        propname = self._pdict.read_value(self._events.next());
        return propname
    
        
    def read_obj(self, cls=dict, state=POPState.EXPECTING_OBJ_START):
        cnt = 0
        if state is POPState.EXPECTING_OBJ_START:
            cnt = 0;
        elif state is POPState.EXPECTING_VALUE:
            cnt = 1
        else:
            raise Exception("Couldn't start reading an obejct at this state: "+str(state))
        deserializer = self._deserializers.get(cls, None)
        if deserializer:
            val = deserializer.execute(self._events, self._pdict, count=cnt)
        else:    
            val = self._read_obj(cls, state)
        return val    
        
    def _read_obj(self, cls=dict, state=POPState.EXPECTING_OBJ_START):
        
        events = self._events
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
                    return  obj
                elif self._pdict.is_obj_property_name(event):
                    # check valid state
                    if state is not POPState.EXPECTING_OBJ_PROPERTY_OR_END:
                        raise UnexpectedStateException(state, POPState.EXPECTING_OBJ_PROPERTY_OR_END, " the received object property name was not expected")
                    propname = self._pdict.read_value(event);
                    propname=self._decode_field_name(obj, propname)
                    # setting next state
                    state = POPState.EXPECTING_VALUE
                else:
                    raise ParseException(" Unexpected event when expected state is: " + str(state) + " while event was: " + str(event))
            elif state is POPState.EXPECTING_VALUE:
                val = None
                if self._pdict.is_array_start(event):
                    val = self.read_array(POPState.EXPECTING_VALUE_OR_ARRAY_END, cls, propname)
                elif self._pdict.is_value(event):
                    # check valid state
                    if state is not POPState.EXPECTING_VALUE:
                        raise UnexpectedStateException(state, POPState.EXPECTING_VALUE, " wasn't expecting a value")
                    # read value
                    val = self._pdict.read_value(event)
                elif self._pdict.is_obj_start(event):
                    val = self._read_obj_as_value(cls, propname)
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
            
    def read_array(self, state=POPState.EXPECTING_ARRAY_START, cls=None, propname=None):
        events = self._events
        res = []
        
        if state is POPState.EXPECTING_ARRAY_START:
            # setting to object start state
            first = events.next();
            # check if input is object
            if not self._pdict.is_array_start(first):
                raise UnexpectedStateException(POPState.EXPECTING_ARRAY_START, " didn't received start of array!!!")
        
        state = POPState.EXPECTING_VALUE_OR_ARRAY_END    
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
                val = self._read_obj_as_value( cls, propname)
                res.append(val)
            else:
                raise Exception('Unexpected event')
            
            
            
            event = events.next()

    

    def _read_obj_as_value(self, cls=None, valname=None):
        if cls is not None and valname is not None:
            # attempt to resolve class of value
            valcls = Typed.resolve(valname, cls)
        if cls is not None and valname is None:
            valcls = cls
        if valcls is None:  # if not resolved simply convert to dictionary
            valcls = dict
        deserializer = self._deserializers.get(valcls, None)
        if deserializer:
            val = deserializer.execute(self._events, self._pdict, count=1)
        else:    
            val = self._read_obj(valcls, POPState.EXPECTING_OBJ_PROPERTY_OR_END)
        return val
        
    def prepare_input(self, events):    
        if isinstance(events, EventStream):
            return events
        else:
            raise UnsupportedTypeException('Input can only be an ' + str(EventStream) + ' instance!!')
        
    
    def register_module(self, module):
        for serializer in module.serializers:
            self.register_serializer(serializer)
            
        for deserializer in module.deserializers:
            self.register_deserializer(deserializer)
        
    
    def register_deserializer(self, deserializer):
        self._deserializers[deserializer.handled_class()] = deserializer