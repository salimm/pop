'''
Created on Dec 26, 2017

@author: Salim
'''
from pylods.deserialize import Typed

def rename_attr(oldname, newname):
    def inner_rename_attr(cls):
        name = oldname
        if name.startswith("__"):
            name = "_" + cls.__name__ + oldname
        _create_pylods_property(cls, 'namedecode')
        _create_pylods_property(cls, 'nameencode')
        cls._pylods[cls]['nameencode'][name] = newname
        cls._pylods[cls]['namedecode'][newname] = name
        return cls
        
    return inner_rename_attr



def ignore_attr(name):
    def inner_ignore_attr(cls):
        _create_pylods_property(cls, 'ignore')
        cls._pylods[cls]['ignore'][name] = True
        return cls
    return inner_ignore_attr

def order_attr(name, order):
    def inner_order_attr(cls):
        _create_pylods_property(cls, 'order')
        cls._pylods[cls]['order'][name] = order
        return cls
    return inner_order_attr


def use_serializer(serializer):
    def inner_use_serializer(cls):
        _create_pylods_property(cls, 'serializer')
        cls._pylods[cls]['serializer'] = serializer()
        return cls
    return inner_use_serializer

def type_attr(name, typecls):
    def inner_type_attr(cls):
        Typed.register_type(name, typecls, cls)
        return cls
    return inner_type_attr

def use_deserializer(deserializer):
    def inner_use_deserializer(cls):
        _create_pylods_property(cls, 'deserializer')
        cls._pylods[cls]['deserializer'] = deserializer()
        return cls
    return inner_use_deserializer

def _create_pylods_property(cls, name):
    _create_pylods(cls)
    if not name in cls._pylods[cls]:
        cls._pylods[cls][name] = {}
        
def _create_pylods(cls):
    if(not hasattr(cls, "_pylods")):
        cls._pylods = {}
    if not cls in cls._pylods:
        cls._pylods[cls]={}





    
