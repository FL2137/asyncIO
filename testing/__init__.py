import os
import sys
import pytest
import socket

assert(sys.argv[1])
assert(sys.argv[2])

ADDRESS = sys.argv[1]
PORT = int(sys.argv[2])


def test_accept():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    addr = (ADDRESS, PORT)
    s.connect(("127.0.0.1", 5001))
    msg = "hejka :3"
    s.send(msg)
    rcv = s.recv(1024)
    print(rcv)
    s.close()
    return 1


if __name__ == "__main__":
    test_accept()