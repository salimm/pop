'''
Created on Dec 5, 2017

@author: Salim
'''
from abc import abstractmethod, ABCMeta


class Serializer():
    '''
        Base Serializer class. It only specifies that it needs a handled_class method
    '''
    
    __metaclass__ = ABCMeta
    
    @abstractmethod
    def serialize(self, gen, obj, outstream):
        '''
        Is called by the serializer when trying to serialize this class
        :param gen:
        :param outstream:
        '''
        raise Exception('Not implemented');
    
    


class DataFormatGenerator():
    '''
        class that helps writing 
    '''
    
    __metaclass__ = ABCMeta
    
    __slots__ = ['_pdict', '_serializers']
    
    
    def __init__(self, pdict):
        self._pdict = pdict
        self._serializers = {}
    
    
    def write(self, val, outstream):
        if(self._isobject(val)):
            self._write_object(val, outstream)
        elif(self._isdict(val)):
            self._write_dict(val, outstream)
        elif(self._isarray(val)):
            self._write_array(val, outstream)
        else:
            self._write_value(val, outstream)
     
    def _isobject(self, val):
        return hasattr(val, '__dict__')  or hasattr(val, '__slots__')
    
    def _isdict(self, val):
        return isinstance(val, dict)
    
    def _isarray(self, val):
        return isinstance(val, list) or isinstance(val, tuple)
    
    def _write_value(self, val, outstream):
        self._pdict.write_value(val, outstream)
    
    
    def _write_array(self, val, outstream):
        length = len(val)
        self.write_array_start(length, outstream)
        if length > 0:
            self.write(val[0], outstream)
            idx = 1
            while idx < length:
                self.write_array_field_separator(val[idx], outstream)
                self.write(val[idx], outstream)
                idx += 1
        self.write_array_end(length, outstream)

    def _write_object(self, val, outstream):
        '''
        Write object into the stream
        :param val:
        :param outstream:
        '''
        # first attempt using an existing custom serializer
        serializer = self.__lookup_serializer(getattr(val, '__class__'))
        if serializer is not None:
            serializer.serialize(self, val, outstream)
            return
        
        fields = self._fetch_obj_fields(val)
        length = len(fields)
        self.write_object_start(length, outstream)
        count = 0;
        if length > 0:
            mappendname = self._encode_field_name(val, fields[0])
            if mappendname is not None:
                count += 1
                self.write_object_field(mappendname, getattr(val, fields[0]), outstream)
            idx = 1
            while idx < length:                
                name = fields[idx]
                mappedname = self._encode_field_name(val, name)
                if mappedname is not None:
                    value = getattr(val, name)
                    if count > 0:
                        self.write_object_field_separator(mappedname, value, outstream)
                    self.write_object_field(mappedname, value, outstream)
                    count += 1
                idx += 1
        self.write_object_end(length, outstream)
        
    def write_object_field(self, name, value, outstream):
        self.write_object_field_name(name, outstream)
        self.write_object_name_value_separator(name, value, outstream)
        self.write(value, outstream)
        
        
    def _write_dict(self, val, outstream):
        '''
        write a dictionary
        :param val:
        :param outstream:
        '''
        fields = val.keys()
        length = len(fields)
        self.write_dict_start(length, outstream)
        if length > 0:
            self.write_dict_field(fields[0], val[fields[0]], outstream)
            idx = 1
            while idx < length:
                name = fields[idx]
                value = val[fields[idx]]
                self.write_dict_field_separator(name, value, outstream)
                self.write_dict_field(name, value, outstream)
                idx += 1
        self.write_dict_end(length, outstream)        
    
    
    def write_dict_field(self, name, value, outstream):
        self.write_dict_field_name(name, outstream)
        self.write_dict_name_value_separator(name, value, outstream)
        self.write(value, outstream)
    
    def _fetch_obj_fields(self, obj):
        
        if  hasattr(obj, '__dict__'):
            fields = [f for f in obj.__dict__.keys() if not callable(obj.__dict__[f])]
        elif hasattr(obj, '__slots__'):
            fields = [f for f in obj.__slots__ if not callable(getattr(obj,f))]
        else:
            return [];
        
        fields = self._sort_obj_fields(obj, fields)
        return fields
    
    def _sort_obj_fields(self, obj, fields):
        cls = getattr(obj, '__class__')
        if hasattr(cls, '_pylods') and  cls in cls._pylods and  'order' in cls._pylods[cls]:
            tmp = self.__attach_order(cls, fields)
            tmp = sorted(tmp, key=lambda item: item[1])
            fields = [i[0] for i in tmp]
        return fields
    
    
    def __attach_order(self, cls, fields):
        attached = [None] * len(fields)
        for i in range(len(fields)):
            name = fields[i]
            if name in cls._pylods[cls]['order']:
                attached[i] = (name,cls._pylods[cls]['order'][name])
            else:
                attached[i] = (name,9999999999)     
        return attached           
    
    def _encode_field_name(self, obj, name):
        mappedname = name;
        
        # 1. check to rename fields using decorator
#         print(obj)
        cls = getattr(obj,"__class__")
        if hasattr(obj, '_pylods') and cls in cls._pylods :
            if 'ignore' in obj.__class__._pylods[cls] and name in obj._pylods[cls]['ignore']:
                return None 
            if 'nameencode' in obj.__class__._pylods[cls] and name in obj._pylods[cls]['nameencode']:
                return obj._pylods[cls]['nameencode'][name]
        # 2. check to ignore fields based on decorators
        # 3. remove private convention _
        if mappedname.startswith('_'):
            mappedname = mappedname[1:]
        return mappedname
    
    def _decode_field_name(self, obj, name):
        if isinstance(obj, dict):
            return name
        # 1. check to rename fields using decorator
        cls = getattr(obj,"__class__")
        if hasattr(obj, '_pylods'):            
            if 'namedecode' in obj.__class__._pylods[cls] and name in obj._pylods[cls]['namedecode']:
                return obj._pylods[cls]['namedecode'][name]
        # 2. check to ignore fields based on decorators
        # 3. remove private convention _
        fields = self._fetch_obj_fields(obj)
        properties=[p for p in dir(getattr(obj,"__class__")) if isinstance(getattr(getattr(obj,"__class__"),p),property)]
        if name in fields or name in properties:
            return name
        elif "_" + name in fields or properties:
            return "_" + name
        else:
            raise Exception("property \"" + str(name) + "\" couldn't be mapped to a property in object " + str(getattr(obj,"__class__")))
        
    ######################### ARRAY
    
    def write_array_start(self, length, outstream):
        self._pdict.write_array_start(length, outstream)
    
    def write_array_end(self, length, outstream):
        self._pdict.write_array_end(length, outstream)
    
    def write_array_field_separator(self, value, outstream):
        self._pdict.write_array_field_separator(value, outstream)
    
    
    ######################### OBJECT
     
    def write_object_start(self, numfields, outstream):
        self._pdict.write_object_start(numfields, outstream)
    
    def write_object_end(self, numfields, outstream):
        self._pdict.write_object_end(numfields, outstream)
    
    def write_object_field_separator(self, name, value, outstream):
        self._pdict.write_object_field_separator(name, value, outstream)
    
    def write_object_name_value_separator(self, name, value, outstream):
        self._pdict.write_object_name_value_separator(name, value, outstream)
        
    def write_object_field_name(self, name, outstream):
        self._pdict.write_object_field_name(name, outstream)
        
    ######################### DICT    
        
    def write_dict_start(self, numfields, outstream):
        self._pdict.write_dict_start(numfields, outstream)
    
    def write_dict_end(self, numfields, outstream):
        self._pdict.write_dict_end(numfields, outstream)
    
    def write_dict_field_separator(self, name, value, outstream):
        self._pdict.write_dict_field_separator(name, value, outstream)
        
    def write_dict_name_value_separator(self, name, value, outstream):
        self._pdict.write_dict_name_value_separator(name, value, outstream)    
    
    def write_dict_field_name(self, name, outstream):
        self._pdict.write_dict_field_name(name, outstream)
        
        
    def register_serializer(self, cls, serializer):
        self._serializers[cls] = serializer


    def __lookup_serializer(self, cls):
        deserializer = self._serializers.get(cls, None)
        if deserializer is None and hasattr(cls, '_pylods'):
            return cls._pylods[cls].get('serializer',None)
        
        return None

    
    
    

    
     
