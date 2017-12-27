'''
Created on Nov 28, 2017

@author: Salim
'''
from abc import ABCMeta, abstractmethod


class Dictionary(object):
    
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
    
    
    ######################### OBJECT
    
    @abstractmethod
    def write_object_start(self, numfields, outstream):
        raise Exception("Not Implemented!!!")
    
    @abstractmethod
    def write_object_end(self, numfields, outstream):
        raise Exception("Not Implemented!!!")
    
    @abstractmethod
    def write_object_field_separator(self, name, value, outstream):
        raise Exception("Not Implemented!!!")
    
    @abstractmethod
    def write_object_field_name(self, name, outstream):
        raise Exception("Not Implemented!!!")
    
    @abstractmethod
    def write_object_name_value_separator(self, name, value, outstream):
        raise Exception("Not Implemented!!!")

    ######################### ARRAY
    
    @abstractmethod 
    def write_array_start(self, length, outstream):
        raise Exception("Not Implemented!!!")
    
    @abstractmethod 
    def write_array_end(self, length, outstream):
        raise Exception("Not Implemented!!!")
    
    @abstractmethod 
    def write_array_field_separator(self, value, outstream):
        raise Exception("Not Implemented!!!")
     
    
    ######################### DICT
    
    @abstractmethod
    def write_dict_start(self, numfields, outstream):
        raise Exception("Not Implemented!!!")
    
    @abstractmethod
    def write_dict_end(self, numfields, outstream):
        raise Exception("Not Implemented!!!")
    
    @abstractmethod
    def write_dict_field_separator(self, name, value, outstream):
        raise Exception("Not Implemented!!!")    
    
    @abstractmethod
    def write_dict_field_name(self, name, outstream):
        raise Exception("Not Implemented!!!")
    
    @abstractmethod
    def write_dict_name_value_separator(self, name, value, outstream):
        raise Exception("Not Implemented!!!")

    
    @abstractmethod
    def write_value(self, val, outstream):
        raise Exception("Not Implemented!!!")    
    