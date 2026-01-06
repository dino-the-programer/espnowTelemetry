import ctypes
from dataclasses import dataclass, field
from typing import List

from cdataclass import BigEndianCDataMixIn, meta


# Simple big endian C structure
@dataclass
class Item(BigEndianCDataMixIn):
    number: int = field(metadata=meta(ctypes.c_uint32))
    msg: bytes = field(metadata=meta(ctypes.c_char * 10))

item = Item(0,b'abc')
buff = item.to_bytes()
arr = bytearray()
arr[0:0]= buff
print(item.to_bytes())
item2 = Item.from_buffer(arr)
print(item2)

