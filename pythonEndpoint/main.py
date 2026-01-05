import serial
from type import *
import signal
import sys
import time

ser = serial.Serial("COM5", 115200, timeout=0)
def signal_handler(sig, frame):
    print('\nSIGINT (Ctrl+C) detected. Performing cleanup and exiting.')
    ser.close()
    sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)

rxbuf = bytearray()

# msg = SerialMessage(
#     SerialCommand.SEND,
#     b"\xff\xff\xff\xff\xff\xff",
#     5,
#     b"HELLO"
# )

# time.sleep(5)

# ser.write(b"hello\n")



while True:
    rxbuf += ser.read(256)

    frame = unpack_frame(rxbuf)

    if frame:
        msg_type, payload = frame

        if msg_type == FRAME_SERIAL:
            sm = SerialMessage.unpack(payload)
            print("SERIAL:", sm.command, sm.data, sm.broadcast_addr)

        elif msg_type == FRAME_ESPNOW:
            em = EspNowMessage.unpack(payload)
            print("ESPNOW:", em.command)
