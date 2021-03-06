'''
Created on Nov 26, 2017
@author: Salim
'''
from abc import ABCMeta, abstractmethod
from enum import Enum
from _pyio import __metaclass__
import types
from io import BytesIO
from pylods.serialize import DataFormatGenerator
    
    
class DeserializationContext():
    
    def __init__(self):
        self.__attributes = {}
        
        
    def set_attribute(self, key, val):
        self.__attributes[key] = val

    def get_attribute(self, key, default=None):
        '''
            returns attribute from the attribute map. It will return None if it doens't exist
        :param key:
        '''
        return self.__attributes.get(key, default)
        
        
    def copy_all(self, ctxt):
        '''
         copies all values into the current context from given context
        :param ctxt:
        '''
        for key, value  in ctxt.iteritems:
            self.set_attribute(key, value)
        
        
    @classmethod 
    def create_context(cls, items=[]):
        ctxt = DeserializationContext()
        if items is not None:
            for key, value in items:
                ctxt.set_attribute(key, value)
        return ctxt
    
    
    
        
class Deserializer():
    '''
        Base Deserializer class. It only specifies that it needs a handled_class method
    '''
    __metaclass__ = ABCMeta
    
    @abstractmethod
    def execute(self, events, pdict, count, ctxt):
        raise Exception('Not implemented');
    
    
    

class EventBasedDeserializer(Deserializer):
    '''
        EventBasedDeserializer will execute deserializer over an iterator over events corresponding to the given object
    '''
    __metaclass__ = ABCMeta
    
    def execute(self, events, pdict, ctxt):        
        val = self.deserialize(events, pdict, ctxt)
        try:
            while next(events):
                pass
        except StopIteration:
            pass
        return val
            
    
    @abstractmethod
    def deserialize(self, events, pdict, ctxt=None):
        raise Exception('Not implemented');
    
    
class ClassEventIterator(object):
    
    def __init__(self, events, pdict, count=0):
        self._events = events
        self._count = count
        self._pdict = pdict
        self._isdone = False
        if self._count == 0:
            if  self._pdict.is_obj_start(next(events)):
                self._count = self._count + 1
            else:
                raise Exception("Expected a object start event but didn't receive one")
    
    def __iter__(self):
        return self

    def __next__(self):
        
        if self._isdone:
            raise StopIteration()
        event = next(self._events)
        if event:
            if self._pdict.is_obj_start(event):
                self._count = self._count + 1
            elif self._pdict.is_obj_end(event):
                self._count = self._count - 1
                if self._count == 0:
                    self._isdone = True
                    raise StopIteration()
            return event
        else:
            return None                

    next = __next__  # Python 2
    

        
    
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
        


class Parser():
    '''
        Used for parsing to events
    '''
    
    __slots__ = ['_pdict']
    
    
    def __init__(self, pdict):
        self._pdict = pdict
        
        
    
    def parse(self, data):
        
            
        if type(data) is types.StringType:
            data = BytesIO(data);
            data.seek(0)
        if hasattr(data, 'read'):
            pass
        else:
            raise Exception('input should be of one the types [str, IOBase, file] but ' + str(data.__class__) + " was received!!")
        
        return self._gen_events(data)
        
    def _gen_events(self, instream):
        '''
            generates events using the specific parser
        :param instream:
        '''
        return self._pdict.gen_events(instream)
        
    
class EventStream():
    '''
        Wrapper Iterator as output of Parser
    '''
    
    def __init__(self, events): 
        print("{{}{}{}}{{}{{}{{}{}")
        self._events = events
        
    
    def __iter__(self):
        return self

    def next(self):
        e= next(self._events)
        return  e
    
    __next__ = next  # Python 2
    
    

class Module():
    '''
        A Module contains customer serializer/deserializers.
    '''
    
    __metaclass__ = ABCMeta
    __slots__ = ['_serializers', '_deserializers']
    
    def __init__(self):
        self._serializers = {}
        self._deserializers = {}
    
    def add_serializer(self, cls, serializer):
        self._serializers[cls ] = serializer
    
    def add_deserializer(self, cls, deserializer):
        self._deserializers[cls ] = deserializer
    
    
    def get_serializers(self):
        return self._serializers
    
    def get_deserializers(self):
        return self._deserializers
    
    serializers = property(get_serializers)
    deserializers = property(get_deserializers)



class DecoratorsModule(Module):
    '''
        Default module for decorators    
    '''
    
    all_serializers = {}
    all_deserializers = {}
    
    def __init__(self):
        self._serializers = self.all_serializers
        self._deserializers = self.all_deserializers
        
    @classmethod
    def register_serializer(cls, scls, serializer):
        cls.all_serializers[scls ] = serializer();
    
    @classmethod
    def register_deserializer(cls, dcls, deserializer):
        cls.all_deserializers[dcls ] = deserializer();



class ObjectMapperBackend(DataFormatGenerator):
    
    __metaclass__ = ABCMeta
    def __init__(self, pdict):
        super(ObjectMapperBackend, self).__init__(pdict)
        
    @abstractmethod
    def read_value(self, events):
        pass
            
    @abstractmethod
    def read_obj_property_name(self, events):
        pass
    
        
    @abstractmethod
    def read_obj(self, events, cls=dict, state=POPState.EXPECTING_OBJ_START, ctxt=DeserializationContext.create_context()):
        pass
    
    
    @abstractmethod
    def read_array(self, events, state=POPState.EXPECTING_ARRAY_START, cls=None, propname=None, ctxt=DeserializationContext.create_context()):
        pass
    
    
    @abstractmethod
    def register_module(self, module):
        pass
        
    @abstractmethod
    def register_deserializer(self, cls, deserializer):
        pass
        
    @abstractmethod    
    def copy(self):
        pass
    
     
