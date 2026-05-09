import socket, struct

# Magic
MAGIC = 0xCAFE
WRONG_MAGIC = 0xBEEF

# Op codes
OP_ERROR = 0x05

# Flags
FLAG_CLIENT_TO_SERVER = 0x00
FLAG_SERVER_TO_CLIENT = 0x01

def make_error_packet(message: bytes) -> bytes:
    header = struct.pack('<HBBH', MAGIC, OP_ERROR, FLAG_SERVER_TO_CLIENT, len(message))
    return header + message

EXPECTED_ERROR = make_error_packet(b'internal error')

# -- TCP tests --

print("TEST (tcp): sending invalid magic value should fail")
with socket.create_connection(('127.0.0.1', 7474)) as sock:
    header = struct.pack('<HBBH', WRONG_MAGIC, 0x00, FLAG_CLIENT_TO_SERVER, 0x00)
    sock.sendall(header)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)
    assert msg == EXPECTED_ERROR, f"Unexpected response: {msg!r}"

print("TEST (tcp): sending wrong direction should fail")
with socket.create_connection(('127.0.0.1', 7474)) as sock:
    header = struct.pack('<HBBH', WRONG_MAGIC, 0x00, FLAG_SERVER_TO_CLIENT, 0x00)
    sock.sendall(header)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)
    assert msg == EXPECTED_ERROR, f"Unexpected response: {msg!r}"

print("TEST (tcp): sending wrong op should fail")
with socket.create_connection(('127.0.0.1', 7474)) as sock:
    header = struct.pack('<HBBH', WRONG_MAGIC, 0x22, FLAG_SERVER_TO_CLIENT, 0x00)
    sock.sendall(header)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)
    assert msg == EXPECTED_ERROR, f"Unexpected response: {msg!r}"

# -- Unix socket tests --

SOCKET_PATH = '../mishell.sock'

print("TEST (unix): sending invalid magic value should fail")
with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
    sock.connect(SOCKET_PATH)
    header = struct.pack('<HBBH', WRONG_MAGIC, 0x00, FLAG_CLIENT_TO_SERVER, 0x00)
    sock.sendall(header)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)
    assert msg == EXPECTED_ERROR, f"Unexpected response: {msg!r}"

print("TEST (unix): sending wrong direction should fail")
with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
    sock.connect(SOCKET_PATH)
    header = struct.pack('<HBBH', WRONG_MAGIC, 0x00, FLAG_SERVER_TO_CLIENT, 0x00)
    sock.sendall(header)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)
    assert msg == EXPECTED_ERROR, f"Unexpected response: {msg!r}"

print("TEST (unix): sending wrong op should fail")
with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
    sock.connect(SOCKET_PATH)
    header = struct.pack('<HBBH', WRONG_MAGIC, 0x22, FLAG_SERVER_TO_CLIENT, 0x00)
    sock.sendall(header)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)
    assert msg == EXPECTED_ERROR, f"Unexpected response: {msg!r}"

print("All tests passed!")
