import serial
from type import *
import signal
import sys
import time
import threading
from queue import Queue

sendQueue = Queue()
rxbuf = bytearray()
connectedNodes:list = []

ser = serial.Serial("COM5", 115200, timeout=0)

def syncNode(msg):
    print(msg.broadcast_addr)
    node = -1
    if msg.broadcast_addr not in connectedNodes:
        connectedNodes.append(msg.broadcast_addr)
        savemsg = SerialMessage(
            SerialCommand.SAVE,
            msg.broadcast_addr,
            5,
            b""
        )
        sendQueue.put(savemsg)
    node = connectedNodes.index(msg.broadcast_addr)
    print(node)
    print(ConfigNode(node).pack())
    initmsg = SerialMessage(
        SerialCommand.INIT,
        msg.broadcast_addr,
        5,
        ConfigNode(node).pack()
    )
    sendQueue.put(initmsg)

def sendHello(stop_event):
    while len(connectedNodes)==0:
        if stop_event.wait(timeout=1.0): # Blocks for up to 1 second, returns True if set
            return
    time.sleep(5)
    i = 0
    while True:
        if stop_event.wait(timeout=1.0): # Blocks for up to 1 second, returns True if set
            return
        string = f"c={i}"
        msg = SerialMessage(
            SerialCommand.SEND,
            connectedNodes[0],
            5,
            string.encode()
        )
        i+=1
        sendQueue.put(msg)
        time.sleep(5)
        

def sendSerialLoop(stop_event):
    global sendQueue
    while True:
        if stop_event.wait(timeout=1.0): # Blocks for up to 1 second, returns True if set
            return
        if not sendQueue.empty():
            msg = sendQueue.get()
            ser.write(msg.pack())
            sendQueue.task_done()

def readSerialLoop(stop_event):
    global rxbuf
    while True:
        if stop_event.wait(timeout=1.0): # Blocks for up to 1 second, returns True if set
            return
        rxbuf += ser.read(256)
        frame = unpack_frame(rxbuf)
        if frame:
            msg_type, payload = frame

            if msg_type == FRAME_SERIAL:
                sm = SerialMessage.unpack(payload)
                if sm.command == SerialCommand.STARTED:
                    print("access point node started")
                elif sm.command == SerialCommand.SAVE:
                    syncNode(sm)
                elif sm.command == SerialCommand.DATA_RCV:
                    espPacket = EspNowMessage.unpack(sm.data)
                    (msg,) = struct.unpack("<10s",espPacket.data[:10])
                    print(msg)

                    pass

            elif msg_type == FRAME_ESPNOW:
                em = EspNowMessage.unpack(payload)
                print("ESPNOW:", em.command)

stop_event = threading.Event()

t1 = threading.Thread(target=sendSerialLoop, args=(stop_event,))
t2 = threading.Thread(target=readSerialLoop, args=(stop_event,))
t3 = threading.Thread(target=sendHello, args=(stop_event,))

def signal_handler(sig, frame):
    print('\nSIGINT (Ctrl+C) detected. Performing cleanup and exiting.')
    stop_event.set()
    ser.close()
    sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)

t1.start()
t2.start()
t3.start()

while True:
    pass
