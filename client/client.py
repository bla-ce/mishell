import socket, struct

# Magic
MAGIC = 0xCAFE
WRONG_MAGIC = 0xBEEF

# Request OP codes
OP_HELLO = 0x00

# Return OP codes
OP_WELCOME = 0x0
OP_ERROR = 0x1

# Flags
FL_CLIENT_TO_SERVER = 0b0000
FL_SERVER_TO_CLIENT = 0b0001
FL_USER = 0b0010
FL_HOST = 0b0100
FL_SERVER = 0b1000

def make_error_packet(message: bytes) -> bytes:
    header = struct.pack('<HBBH', MAGIC, OP_ERROR, FL_SERVER_TO_CLIENT | FL_SERVER, len(message))
    return header + message

# -- TCP tests --

print("TEST (tcp): sending invalid magic value should fail")
with socket.create_connection(('127.0.0.1', 7474)) as sock:
    header = struct.pack('<HBBH', WRONG_MAGIC, OP_HELLO, FL_CLIENT_TO_SERVER | FL_HOST, 0x00)
    sock.sendall(header)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)
    expected = make_error_packet(b'invalid magic value in packet')
    assert msg == expected, f"Expected: {expected!r}\n  Actual: {msg!r}"

print("TEST (tcp): sending wrong direction should fail")
with socket.create_connection(('127.0.0.1', 7474)) as sock:
    header = struct.pack('<HBBH', MAGIC, OP_HELLO, FL_SERVER_TO_CLIENT | FL_HOST, 0x00)
    sock.sendall(header)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)
    expected = make_error_packet(b'invalid direction flag in packet')
    assert msg == expected, f"Expected: {expected!r}\n  Actual: {msg!r}"

print("TEST (tcp): sending wrong mode should fail")
with socket.create_connection(('127.0.0.1', 7474)) as sock:
    header = struct.pack('<HBBH', MAGIC, OP_HELLO, FL_CLIENT_TO_SERVER | FL_SERVER, 0x00)
    sock.sendall(header)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)
    expected = make_error_packet(b'invalid mode flag in packet')
    assert msg == expected, f"Expected: {expected!r}\n  Actual: {msg!r}"

print("TEST (tcp): sending wrong op should fail")
with socket.create_connection(('127.0.0.1', 7474)) as sock:
    header = struct.pack('<HBBH', MAGIC, 0x4, FL_CLIENT_TO_SERVER | FL_USER, 0x00)
    sock.sendall(header)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)
    expected = make_error_packet(b'invalid op value in packet')
    assert msg == expected, f"Expected: {expected!r}\n  Actual: {msg!r}"

print("TEST (tcp): sending HELLO should work and return a WELCOME op")
with socket.create_connection(('127.0.0.1', 7474)) as sock:
    header = struct.pack('<HBBH', MAGIC, OP_HELLO, FL_CLIENT_TO_SERVER | FL_USER, 0x00)
    sock.sendall(header)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)

    expected = struct.pack('<HBBH', MAGIC, OP_WELCOME, FL_SERVER_TO_CLIENT | FL_SERVER, 0x00)
    assert msg == expected, f"Expected: {expected!r}\n  Actual: {msg!r}"

print("All tests passed!")
