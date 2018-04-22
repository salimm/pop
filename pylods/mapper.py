'''
Created on Dec 26, 2017

@author: Salim
'''

from abc import ABCMeta
from pylods.deserialize import POPState, DeserializationContext
    
        
        
        
class ObjectMapper():
    '''
        Main Parser class to deserialize to objects, values and etc. This parser requires a dictionary to serialize/ deserialize from an input stream
    '''
    
    __classmeta__ = ABCMeta
    
    __slots__ = [ '__deserializers', "_backend"]
    
    def __init__(self, backend=None):
        self._backend = backend
    
    
    def read_value(self, events):
        return self._backend.read_value(events);
            
    def read_obj_property_name(self, events):
        return self._backend.read_obj_property_name( events);
    
        
    def read_obj(self, events, cls=dict, state=POPState.EXPECTING_OBJ_START, ctxt=DeserializationContext.create_context()):
        return self._backend.read_obj( events, cls, state, ctxt);
    
    
    def read_array(self, events, state=POPState.EXPECTING_ARRAY_START, cls=None, propname=None, ctxt=DeserializationContext.create_context()):
        return self._backend.read_array( events, state, cls, propname, ctxt)
    
    
    def register_module(self, module):
        return self._backend.register_module(module)
    
    def write(self, val, outstream):
        self._backend.write(val, outstream)
    
        
    def write_object_field(self, name, value, outstream):
        self._backend.write_object_field(name, value, outstream)
      
    
    def write_dict_field(self, name, value, outstream):
        self._backend.write_dict_field( name, value, outstream)
    
    ######################### ARRAY
    
    def write_array_start(self, length, outstream):
        self._backend.write_array_start( length, outstream)
    
    def write_array_end(self, length, outstream):
        self._backend.write_array_end( length, outstream)
    
    def write_array_field_separator(self, value, outstream):
        self._backend.write_array_field_separator( value, outstream)
    
    
    ######################### OBJECT
     
    def write_object_start(self, numfields, outstream):
        self._backend.write_object_start( numfields, outstream)
    
    def write_object_end(self, numfields, outstream):
        self._backend.write_object_end( numfields, outstream)
    
    def write_object_field_separator(self, name, value, outstream):
        self._backend.write_object_field_separator( name, value, outstream)
    
    def write_object_name_value_separator(self, name, value, outstream):
        self._backend.write_object_name_value_separator( name, value, outstream)
        
    def write_object_field_name(self, name, outstream):
        self._backend.write_object_field_name( name, outstream)
        
    ######################### DICT    
        
    def write_dict_start(self, numfields, outstream):
        self._backend.write_dict_start( numfields, outstream)
    
    def write_dict_end(self, numfields, outstream):
        self._backend.write_dict_end( numfields, outstream)
    
    def write_dict_field_separator(self, name, value, outstream):
        self._backend.write_dict_field_separator( name, value, outstream)
        
    def write_dict_name_value_separator(self, name, value, outstream):
        self._backend.write_dict_name_value_separator( name, value, outstream)
    
    def write_dict_field_name(self, name, outstream):
        self._backend.write_dict_field_name( name, outstream)
        
        
    def register_serializer(self, cls, serializer):
        self._backend.register_serializer(cls, serializer)
        
    def register_deserializer(self, cls, deserializer):
        return self._backend.register_deserializer(cls, deserializer)
        
    def copy(self):
        return self._backend.copy()
    