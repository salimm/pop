'''
Created on Dec 26, 2017

@author: Salim
'''
from pylods.deserialize import  Typed, POPState, DeserializationContext,\
    DecoratorsModule, ObjectMapperBackend, ClassEventIterator
from pylods.error import UnexpectedStateException, \
    ObjectDeserializationException, ParseException
from abc import ABCMeta


    
        

class PyObjectMapper(ObjectMapperBackend):
    '''
        Main Parser class to deserialize to objects, values and etc. This parser requires a dictionary to serialize/ deserialize from an input stream
    '''
    
    __classmeta__ = ABCMeta
    
    __slots__ = [ '__deserializers']
    
    
    def __init__(self, pdict):
        pdict.mapper_backend = self
        super(PyObjectMapper, self).__init__(pdict) 
        self.__deserializers = {}
#         events = self.prepare_input(events)
        self.register_module(DecoratorsModule())
        
    
    def read_value(self, events):
        val = self._pdict.read_value(next(events))
        return val
            

    def read_obj_property_name(self, events):
        propname = self._pdict.read_value(next(events));
        return propname
    
        
    def read_obj(self, events, cls=dict, state=POPState.EXPECTING_OBJ_START, ctxt=DeserializationContext.create_context()):
        cnt = 0
        if state is POPState.EXPECTING_OBJ_START:
            cnt = 0;
        elif state is POPState.EXPECTING_OBJ_PROPERTY_OR_END:
            cnt = 1
        else:
            raise ParseException("Couldn't start reading an object at this state: " + str(state))
        deserializer = self.__lookup_deserializer(cls)
        if deserializer:
            val = deserializer.execute(ClassEventIterator(events, self._pdict, cnt), self._pdict, ctxt=ctxt)
        else:    
            val = self._read_obj(events, cls, state, ctxt)
            
        return val    
    
    
        
    def _read_obj(self, events, cls=dict, state=POPState.EXPECTING_OBJ_START, ctxt=DeserializationContext.create_context()):
        try:
            obj = cls()
        except:
            raise Exception("Failed to instantiate cls" + str(cls))
        if state is POPState.EXPECTING_OBJ_START:
            # setting to object start state
            first = next(events);
            # check if input is object
            if not self._pdict.is_obj_start(first):
                raise ObjectDeserializationException("Expected the content to start with an object!! But first event was: " + str(first))
        
        state = POPState.EXPECTING_OBJ_PROPERTY_OR_END
        propname = None
        event = next(events)
        while event:
            if state is POPState.EXPECTING_OBJ_PROPERTY_OR_END:
                # end of object
                if self._pdict.is_obj_end(event):
                    # check valid state
                    return  obj
                elif self._pdict.is_obj_property_name(event):
                    # check valid state
                    propname = self._pdict.read_value(event);
                    propname = self._decode_field_name(obj, propname)
                    # setting next state
                    state = POPState.EXPECTING_VALUE
                else:
                    raise ParseException(" Unexpected event when expected state is: " + str(state) + " while event was: " + str(event))
            elif state is POPState.EXPECTING_VALUE:
                val = None
                if self._pdict.is_array_start(event):
                    val = self.read_array(events, POPState.EXPECTING_VALUE_OR_ARRAY_END, cls, propname, ctxt)
                elif self._pdict.is_value(event):
                    # check valid state
                    # read value
                    val = self._pdict.read_value(event)
                elif self._pdict.is_obj_start(event):
                    val = self._read_obj_as_value(events, cls, propname, ctxt)
                else:
                    raise ParseException('unrecognized event when reading value: ' + str(event))
                # setting value
                if cls is dict:
                    obj[propname] = val
                else:
                    setattr(obj, propname, val)
                propname = None
                state = POPState.EXPECTING_OBJ_PROPERTY_OR_END
            
            # continue
            event = next(events)
            
        raise UnexpectedStateException(state, POPState.EX, " wasn't expecting a value")
            
    def read_array(self, events, state=POPState.EXPECTING_ARRAY_START, cls=None, propname=None, ctxt=DeserializationContext.create_context()):
        res = []
        if state is POPState.EXPECTING_ARRAY_START:
            # setting to object start state
            first = next(events);
            # check if input is object
            if not self._pdict.is_array_start(first):
                raise UnexpectedStateException(POPState.EXPECTING_ARRAY_START, " didn't received start of array!!!")
        
        state = POPState.EXPECTING_VALUE_OR_ARRAY_END    
        event = next(events)
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
                val = self._read_obj_as_value(events, cls, propname, ctxt)
                res.append(val)
            elif self._pdict.is_array_start(event):
                val = self.read_array(events, POPState.EXPECTING_VALUE_OR_ARRAY_END, cls, propname, ctxt)
                res.append(val)
            else:
                raise ParseException('Unexpected event')
            try:
                event = next(events)
            except StopIteration:
                break  # Iterator exhausted: stop the loop    

    

    def _read_obj_as_value(self, events, cls=None, valname=None, ctxt=DeserializationContext.create_context()):
        valcls = None
        if cls is not None and valname is not None:
            # attempt to resolve class of value
            valcls = Typed.resolve(valname, cls)
            if valcls is None:
                if hasattr(cls, '_pylods'):            
                    if 'type' in cls._pylods[cls] and valname in cls._pylods[cls]['type']:
                        valcls = cls._pylods[cls]['type'][valname]  
        elif cls is not None and valname is None:
            valcls = cls
            
        if valcls is None:
            valcls = dict
        deserializer = self.__lookup_deserializer(valcls)
        if deserializer:
            val = deserializer.execute(ClassEventIterator(events, self._pdict, 1), self._pdict, ctxt=ctxt)
        else:    
            val = self._read_obj(events, valcls, POPState.EXPECTING_OBJ_PROPERTY_OR_END, ctxt)
        return val
     
    
    def __lookup_deserializer(self, cls):
        deserializer = self.__deserializers.get(cls, None)
        if deserializer is None and hasattr(cls, '_pylods'):
            return cls._pylods[cls].get('deserializer', None)
        
        return deserializer;
    
    
    def register_module(self, module):
        for serializer in module.serializers.items():
            self.register_serializer(serializer[0], serializer[1])
            
        for deserializer in module.deserializers.items():
            self.register_deserializer(deserializer[0], deserializer[1])
        
    
    def register_deserializer(self, cls, deserializer):
        self.__deserializers[cls] = deserializer
        
        
    def copy(self):
        '''
         makes a clone copy of the mapper. It won't clone the serializers or deserializers and it won't copy the events
        '''
        try:
            tmp = self.__class__()
        except Exception:
            tmp = self.__class__(self._pdict)
            
        tmp._serializers = self._serializers
        tmp.__deserializers = self.__deserializers

        return tmp
    
    
        
        
