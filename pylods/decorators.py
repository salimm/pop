'''
Created on Dec 26, 2017

@author: Salim
'''
from pylods.mapper import DecoratorsModule
from pylods.deserialize import Typed

def rename_attr(name, newname):
    def inner_rename_attr(cls):
        _create_pylods_property(cls, 'namedecode')
        _create_pylods_property(cls, 'nameencode')
        cls._pylods['nameencode'][name] = newname
        cls._pylods['namedecode'][newname] = name
        return cls
        
    return inner_rename_attr



def ignore_attr(name):
    def inner_ignore_attr(cls):
        _create_pylods_property(cls, 'ignore')
        cls._pylods['ignore'][name] = True
        return cls
    return inner_ignore_attr

def order_attr(name, order):
    def inner_order_attr(cls):
        _create_pylods_property(cls, 'order')
        cls._pylods['order'][name] = order
        return cls
    return inner_order_attr


def use_serializer(serializer):
    def inner_use_serializer(cls):
        _create_pylods_property(cls, 'serializer')
        cls._pylods['serializer'] = serializer()
        return cls
    return inner_use_serializer

def type_attr(name, typecls):
    def inner_type_attr(cls):
        Typed.register_type( name, typecls, cls)
        return cls
    return inner_type_attr

def use_deserializer(deserializer):
    def inner_use_deserializer(cls):
        _create_pylods_property(cls, 'deserializer')
        cls._pylods['deserializer'] = deserializer()
        return cls
    return inner_use_deserializer

def _create_pylods_property(cls, name):
    _create_pylods(cls)
    if not name in cls._pylods:
        cls._pylods[name] = {}
        
def _create_pylods(cls):
    if(not hasattr(cls, "_pylods")):
        cls._pylods = {}





    
