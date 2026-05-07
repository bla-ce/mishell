import socket, struct

MAGIC = 0xCAFE
WRONG_MAGIC = 0xBEEF

def recv_all(sock):
    data = b""
    while chunk := sock.recv(1024):
        data += chunk
    return data

# Test invalid magic
with socket.create_connection(('127.0.0.1', 7474)) as sock:
    header = struct.pack('<HBH', WRONG_MAGIC, 0x00, 0x00)
    sock.sendall(header)
    sock.shutdown(socket.SHUT_WR)
    msg = recv_all(sock)
    assert msg == b'invalid magic value', f"Unexpected response: {msg}"

# Send real payload
with socket.create_connection(('127.0.0.1', 7474)) as sock:
    payload = b"good evening sir"
    header = struct.pack('<HBH', MAGIC, 0x00, len(payload))
    sock.sendall(header + payload)
