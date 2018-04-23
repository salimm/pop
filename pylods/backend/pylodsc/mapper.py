'''
Created on Dec 26, 2017

@author: Salim
'''
from pylods.deserialize import  POPState, DeserializationContext,DecoratorsModule    ,\
    ObjectMapperBackend
from pylods.error import ParseException
from abc import ABCMeta
import pylodscbackend


    
        


class CObjectMapper(ObjectMapperBackend):
    '''
        Main Parser class to deserialize to objects, values and etc. This parser requires a dictionary to serialize/ deserialize from an input stream
    '''
    
    __classmeta__ = ABCMeta
    
    __slots__ = [ '__deserializers']
    
    
    def __init__(self, pdict):
        super(CObjectMapper, self).__init__(pdict)
        pdict.mapper_backend = self
        self.__deserializers = {}
#         events = self.prepare_input(events)
        self.register_module(DecoratorsModule())
        
        
    
#     def prepare_input(self, events):
#         if events is None:
#             return None   
#         if isinstance(events, EventStream):
#             return events
#         else:
#             raise UnsupportedTypeException('Input can only be an ' + str(EventStream) + ' instance!!')
    
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
            val = deserializer.execute(pylodscbackend.create_ClassEventIterator(events, cnt, self._pdict), self._pdict, ctxt=ctxt)
        else:    
            val = self._read_obj(events, cls, state, ctxt)
            
        return val    
    
    
        
    def _read_obj(self, events, cls=dict, state=POPState.EXPECTING_OBJ_START, ctxt=DeserializationContext.create_context()):
        return pylodscbackend.read_obj(events, cls, ctxt,self._pdict,self.__deserializers, state.value  );

            
    def read_array(self, events, state=POPState.EXPECTING_ARRAY_START, cls=None, propname=None, ctxt=DeserializationContext.create_context()):
        return pylodscbackend.read_array(events, cls, propname, ctxt,self._pdict,self.__deserializers, state.value  );

    

    
     
    
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
    
    
        
        
