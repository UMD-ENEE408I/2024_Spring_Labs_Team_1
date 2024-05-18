import socket
import time

# server constants
HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 8090  # Port to listen on (non-privileged ports are > 1023)

DUAL_FATES_LEFT = b"Dual fates left"
DUAL_FATES_RIGHT = b"Dual fates right"
DUAL_FATES_RESULT = b"Dual fates result"
DUAL_FATES_DONE = b"Duel fates done"

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
s.send(DUAL_FATES_LEFT)
s.close()

time.sleep(5)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
s.send(DUAL_FATES_RIGHT)
s.close()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
s.send(DUAL_FATES_RESULT)
msg = s.recv(1024)
print(msg)
s.close()
