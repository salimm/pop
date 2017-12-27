'''
Created on Dec 26, 2017

@author: Salim
'''

def rename_attr(name, newname):
    def inner_rename_attr(cls):
        _create_pylods_property(cls,'namedecode')
        _create_pylods_property(cls,'nameencode')
        cls._pylods['nameencode'][name] = newname
        cls._pylods['namedecode'][newname] = name
        return cls
        
    return inner_rename_attr



def ignore_attr(name):
    def inner_ignore_attr(cls):
        _create_pylods_property(cls,'ignore')
        cls._pylods['ignore'][name] = True
        return cls
    return inner_ignore_attr

def _create_pylods_property(cls, name):
    if(not hasattr(cls,"_pylods")):
        cls._pylods = {}
    if not name in cls._pylods:
        cls._pylods[name] = {}
        
        

    