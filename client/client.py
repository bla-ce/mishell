import socket, struct

MAGIC = 0xCAFE
WRONG_MAGIC = 0xBEEF

# Test invalid magic
with socket.create_connection(('127.0.0.1', 7474)) as sock:
    header = struct.pack('<HBH', WRONG_MAGIC, 0x00, 0x00)
    sock.sendall(header)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)
    assert msg == b'invalid magic value', f"Unexpected response: {msg}"

# Send real payload
with socket.create_connection(('127.0.0.1', 7474)) as sock:
    payload = b"good evening sir"
    header = struct.pack('<HBH', MAGIC, 0x00, len(payload))
    sock.sendall(header + payload)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)
    assert msg == payload, f"Unexpected response: {msg}"


# Same tests with unix socket
SOCKET_PATH = '../mishell.sock'

with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
    sock.connect(SOCKET_PATH)
    header = struct.pack('<HBH', WRONG_MAGIC, 0x00, 0x00)
    sock.sendall(header)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)
    assert msg == b'invalid magic value', f"Unexpected response: {msg}"

with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
    sock.connect(SOCKET_PATH)
    payload = b"good evening sir"
    header = struct.pack('<HBH', MAGIC, 0x00, len(payload))
    sock.sendall(header + payload)
    sock.shutdown(socket.SHUT_WR)
    msg = sock.recv(1024)
    assert msg == payload, f"Unexpected response: {msg}"
