'''
Created on Nov 28, 2017

@author: Salim
'''
from abc import ABCMeta, abstractmethod
from pylods.core import EventStream



class Serializer():
    '''
        Base Serializer class. It only specifies that it needs a handled_class method
    '''
    
    __metaclass__ = ABCMeta
    
    
    @abstractmethod
    def handled_class(self):
        raise Exception('Not implemented'); 
    
    
    
    
    
    
    
    
class Deserializer():
    '''
        Base Deserializer class. It only specifies that it needs a handled_class method
    '''
    __metaclass__ = ABCMeta
    
    @abstractmethod
    def execute(self, events, pdict, count):
        raise Exception('Not implemented');
    
    @abstractmethod
    def handled_class(self):
        raise Exception('Not implemented');
    
    
    

class EventBasedDeserializer(Deserializer):
    '''
        EventBasedDeserializer will execute deserializer over an iterator over events corresponding to the given object
    '''
    __metaclass__ = ABCMeta
    
    def execute(self, events, pdict, count):
        tmp = EventStream(ClassEventIterator(events,pdict, count))
        val = self.deserialize(tmp, pdict)
        try:
            while tmp.next():
                pass
        except StopIteration:
            pass
            
        return val
            
    
    @abstractmethod
    def deserialize(self, events, pdict):
        raise Exception('Not implemented');
    
    
class ClassEventIterator(object):
    
    def __init__(self, events,pdict, count=0):
        self._events = events
        self._count = count
        self._pdict = pdict
        self._isdone = False
        if self._count == 0:
            if  self._pdict.is_obj_start(events.next()):
                self._count = self._count + 1
            else:
                raise Exception("Expected a object start event but didn't receive one")
    
    def __iter__(self):
        return self

    def __next__(self):
        if self._isdone:
            raise StopIteration()
        event = self._events.next()
        if event:
            if self._pdict.is_obj_start(event):
                self._count = self._count + 1
            elif self._pdict.is_obj_end(event):
                self._count = self._count - 1
                if self._count == 0:
                    self._isdone = True
                    return None
            return event
        else:
            return event
        
        

    next = __next__  # Python 2
    
