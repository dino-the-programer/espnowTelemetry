import struct
import zlib
from enum import IntEnum

# ==================================================
# ENUMS (MATCH ESP32 EXACTLY)
# ==================================================

class SerialCommand(IntEnum):
    SEND = 0
    SENT = 1
    FAILED = 2
    SAVE = 3
    INIT = 4
    STARTED = 5
    DATA_RCV = 6


class EspNowCommand(IntEnum):
    SYNC = 0
    ACK = 1
    DATA = 2
    INIT_CMD = 3


# ==================================================
# CRC32 (ESP32 COMPATIBLE)
# ==================================================

def crc32_le(data: bytes, crc: int = 0) -> int:
    """
    Matches ESP32 esp_crc32_le()
    """
    return zlib.crc32(data, crc) & 0xFFFFFFFF


def frame_crc(msg_type: int, payload: bytes) -> int:
    crc = 0
    crc = crc32_le(struct.pack("<B", msg_type), crc)
    crc = crc32_le(struct.pack("<H", len(payload)), crc)
    crc = crc32_le(payload, crc)
    return crc


# ==================================================
# STRUCT FORMATS (ESP32-COMPATIBLE)
# ==================================================

# dataHeader
HEADER_FORMAT = "<6sI"        # uint8_t[6], uint32_t
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)

# serialMessage
SERIAL_MSG_FORMAT = "<I6s2xI210s2x"
SERIAL_MSG_SIZE = struct.calcsize(SERIAL_MSG_FORMAT)

# espNowMessage (aligned)
ESP_NOW_MSG_FORMAT = "<HBBI200s"
ESP_NOW_MSG_SIZE = struct.calcsize(ESP_NOW_MSG_FORMAT)

# configNode
CONFIG_NODE_FORMAT = "<i"
CONFIG_NODE_SIZE = struct.calcsize(CONFIG_NODE_FORMAT)


# ==================================================
# FRAMING CONSTANTS
# ==================================================

PREAMBLE  = b"\xAA\x55"
POSTAMBLE = b"\x55\xAA"

FRAME_HEADER_FMT = "<2sBH"    # preamble, type, length
FRAME_HEADER_SIZE = struct.calcsize(FRAME_HEADER_FMT)

CRC_SIZE = 4

FRAME_SERIAL = 0x01
FRAME_ESPNOW = 0x02
FRAME_CONFIG = 0x03


# ==================================================
# FRAMING FUNCTIONS
# ==================================================

def pack_frame(msg_type: int, payload: bytes) -> bytes:
    header = struct.pack(
        FRAME_HEADER_FMT,
        PREAMBLE,
        msg_type,
        len(payload)
    )

    crc = frame_crc(msg_type, payload)

    return (
        header +
        payload +
        struct.pack("<I", crc) +
        POSTAMBLE
    )


def unpack_frame(rxbuf: bytearray):
    while True:
        if len(rxbuf) < FRAME_HEADER_SIZE:
            return None

        if rxbuf[:2] != PREAMBLE:
            rxbuf.pop(0)
            continue

        _, msg_type, length = struct.unpack(
            FRAME_HEADER_FMT,
            rxbuf[:FRAME_HEADER_SIZE]
        )

        frame_len = FRAME_HEADER_SIZE + length + CRC_SIZE + 2

        if len(rxbuf) < frame_len:
            return None

        payload = rxbuf[
            FRAME_HEADER_SIZE:
            FRAME_HEADER_SIZE + length
        ]

        crc_recv = struct.unpack(
            "<I",
            rxbuf[
                FRAME_HEADER_SIZE + length:
                FRAME_HEADER_SIZE + length + CRC_SIZE
            ]
        )[0]

        if rxbuf[frame_len - 2:frame_len] != POSTAMBLE:
            rxbuf.pop(0)
            continue

        crc_calc = frame_crc(msg_type, payload)

        if crc_calc != crc_recv:
            rxbuf.pop(0)
            continue

        del rxbuf[:frame_len]
        return msg_type, payload


# ==================================================
# MESSAGE CLASSES
# ==================================================

class SerialMessage:
    size = SERIAL_MSG_SIZE

    def __init__(self, command, broadcast_addr, length, data_bytes):
        self.command = SerialCommand(command)
        self.broadcast_addr = broadcast_addr
        self.length = length
        self.data = data_bytes

    def pack(self) -> bytes:
        payload = struct.pack(
            SERIAL_MSG_FORMAT,
            int(self.command),
            self.broadcast_addr,
            self.length,
            self.data.ljust(210, b'\x00')
        )
        return pack_frame(FRAME_SERIAL, payload)

    @classmethod
    def unpack(cls, payload: bytes):
        if len(payload) != SERIAL_MSG_SIZE:
            raise ValueError(
                f"Invalid SerialMessage size: {len(payload)} != {SERIAL_MSG_SIZE}"
            )

        cmd, addr, length, data = struct.unpack(
            SERIAL_MSG_FORMAT, payload
        )

        return cls(
            command=SerialCommand(cmd),
            broadcast_addr=addr,
            length=length,
            data_bytes=data[:length]
        )



class EspNowMessage:
    size = ESP_NOW_MSG_SIZE

    def __init__(self, node_id, message_id, command, data_bytes):
        self.node_id = node_id
        self.message_id = message_id
        self.command = EspNowCommand(command)
        self.data = data_bytes

    def pack(self) -> bytes:
        payload = struct.pack(
            ESP_NOW_MSG_FORMAT,
            self.node_id,
            self.message_id,
            0,  # alignment padding
            int(self.command),
            self.data.ljust(200, b'\x00')
        )
        return pack_frame(FRAME_ESPNOW, payload)

    @classmethod
    def unpack(cls, payload: bytes):
        node, mid, _, cmd, data = struct.unpack(
            ESP_NOW_MSG_FORMAT, payload
        )
        return cls(
            node_id=node,
            message_id=mid,
            command=EspNowCommand(cmd),
            data_bytes=data
        )


class ConfigNode:
    size = CONFIG_NODE_SIZE

    def __init__(self, node_id):
        self.node_id = node_id

    def pack(self) -> bytes:
        return struct.pack(CONFIG_NODE_FORMAT, self.node_id)

    @classmethod
    def unpack(cls, payload: bytes):
        (node_id,) = struct.unpack(CONFIG_NODE_FORMAT, payload)
        return cls(node_id)
