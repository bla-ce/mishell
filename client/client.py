import socket
from packet import (
    Packet, recv_packet,
    MAGIC, WRONG_MAGIC,
    OP_HELLO, OP_WELCOME,
    OP_AUTH, OP_AUTH_OK,
    OP_REGISTER, OP_REGISTER_OK,
    OP_ERROR,
    FL_CLIENT_TO_SERVER, FL_SERVER_TO_CLIENT,
    FL_USER, FL_HOST, FL_SERVER,
    SOCKET_PATH, SOCKET_PORT
)

HOST_MAX_LEN = 8

# -- TCP tests --

print("TEST (tcp): sending invalid magic value should fail")
with socket.create_connection(('127.0.0.1', SOCKET_PORT)) as sock:
    sock.sendall(Packet(magic=WRONG_MAGIC, op=OP_HELLO,
                        flags=FL_CLIENT_TO_SERVER | FL_HOST).pack())
    sock.shutdown(socket.SHUT_WR)
    resp = recv_packet(sock)
    assert resp.magic == MAGIC, f"wrong magic: {resp}"
    assert resp.op == OP_ERROR, f"expected ERROR op: {resp}"
    assert resp.flags == (FL_SERVER_TO_CLIENT | FL_SERVER), f"wrong flags: {resp}"
    assert resp.payload == b'invalid magic value in packet', f"wrong payload: {resp}"

print("TEST (tcp): sending wrong direction should fail")
with socket.create_connection(('127.0.0.1', SOCKET_PORT)) as sock:
    sock.sendall(Packet(op=OP_HELLO,
                        flags=FL_SERVER_TO_CLIENT | FL_HOST).pack())
    sock.shutdown(socket.SHUT_WR)
    resp = recv_packet(sock)
    assert resp.op == OP_ERROR, f"expected ERROR op: {resp}"
    assert resp.flags == (FL_SERVER_TO_CLIENT | FL_SERVER), f"wrong flags: {resp}"
    assert resp.payload == b'invalid direction flag in packet', f"wrong payload: {resp}"

print("TEST (tcp): sending wrong mode should fail")
with socket.create_connection(('127.0.0.1', SOCKET_PORT)) as sock:
    sock.sendall(Packet(op=OP_HELLO,
                        flags=FL_CLIENT_TO_SERVER | FL_SERVER).pack())
    sock.shutdown(socket.SHUT_WR)
    resp = recv_packet(sock)
    assert resp.op == OP_ERROR, f"expected ERROR op: {resp}"
    assert resp.flags == (FL_SERVER_TO_CLIENT | FL_SERVER), f"wrong flags: {resp}"
    assert resp.payload == b'invalid mode flag in packet', f"wrong payload: {resp}"

print("TEST (tcp): sending wrong op should fail")
with socket.create_connection(('127.0.0.1', SOCKET_PORT)) as sock:
    sock.sendall(Packet(op=0x4,
                        flags=FL_CLIENT_TO_SERVER | FL_USER).pack())
    sock.shutdown(socket.SHUT_WR)
    resp = recv_packet(sock)
    assert resp.op == OP_ERROR, f"expected ERROR op: {resp}"
    assert resp.flags == (FL_SERVER_TO_CLIENT | FL_SERVER), f"wrong flags: {resp}"
    assert resp.payload == b'invalid op value in packet', f"wrong payload: {resp}"

print("TEST (tcp): sending HELLO should work and return a WELCOME op")
with socket.create_connection(('127.0.0.1', SOCKET_PORT)) as sock:
    sock.sendall(Packet(op=OP_HELLO,
                        flags=FL_CLIENT_TO_SERVER | FL_USER).pack())
    sock.shutdown(socket.SHUT_WR)
    resp = recv_packet(sock)
    assert resp.magic == MAGIC, f"wrong magic: {resp}"
    assert resp.op == OP_WELCOME, f"expected WELCOME: {resp}"
    assert resp.flags == (FL_SERVER_TO_CLIENT | FL_SERVER), f"wrong flags: {resp}"
    assert resp.payload == b'', f"unexpected payload: {resp}"

host_id = 0

print("TEST (unix): sending AUTH should work and return a AUTH_OK op with 16 bytes payload")
with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
    sock.connect(SOCKET_PATH)
    sock.sendall(Packet(op=OP_AUTH,
                        flags=FL_CLIENT_TO_SERVER | FL_USER).pack())
    sock.shutdown(socket.SHUT_WR)
    resp = recv_packet(sock)
    assert resp.magic == MAGIC, f"wrong magic: {resp}"
    assert resp.op == OP_AUTH_OK, f"expected AUTH_OK: {resp}"
    assert resp.flags == (FL_SERVER_TO_CLIENT | FL_SERVER), f"wrong flags: {resp}"
    assert resp.id == 0, f"wrong id: {resp}"
    assert len(resp.payload) == 16, f"payload should be 16 bytes long: {resp}"

    host_id = int.from_bytes(resp.payload, byteorder='little')

print("TEST (tcp): sending AUTH with id should return empty payload")
with socket.create_connection(('127.0.0.1', SOCKET_PORT)) as sock:
    sock.sendall(Packet(op=OP_AUTH,
                        flags=FL_CLIENT_TO_SERVER | FL_USER,
                        id=host_id).pack())
    sock.shutdown(socket.SHUT_WR)
    resp = recv_packet(sock)
    assert resp.magic == MAGIC, f"wrong magic: {resp}"
    assert resp.op == OP_AUTH_OK, f"expected AUTH_OK: {resp}"
    assert resp.flags == (FL_SERVER_TO_CLIENT | FL_SERVER), f"bad flags: {resp}"
    assert resp.id == 0, f"bad id: {resp}"
    assert len(resp.payload) == 0, f"payload should be empty: {resp}"

print("TEST (tcp): sending AUTH with wrong id should return ERROR: host not found")
with socket.create_connection(('127.0.0.1', SOCKET_PORT)) as sock:
    sock.sendall(Packet(op=OP_AUTH,
                        flags=FL_CLIENT_TO_SERVER | FL_USER,
                        id=0x12345).pack())
    sock.shutdown(socket.SHUT_WR)
    resp = recv_packet(sock)
    assert resp.magic == MAGIC, f"bad magic: {resp}"
    assert resp.op == OP_ERROR, f"expected ERROR: {resp}"
    assert resp.flags == (FL_SERVER_TO_CLIENT | FL_SERVER), f"bad flags: {resp}"
    assert resp.id == 0, f"bad id: {resp}"
    assert resp.payload == b'host not found', f"bad payload: {resp}"

print("TEST (tcp): sending AUTH should return a message after registering too many hosts")
for _ in range(HOST_MAX_LEN-1): # we already registered one
    with socket.create_connection(('127.0.0.1', SOCKET_PORT)) as sock:
        sock.sendall(Packet(op=OP_AUTH,
                            flags=FL_CLIENT_TO_SERVER | FL_USER).pack())
        sock.shutdown(socket.SHUT_WR)
        resp = recv_packet(sock)
        assert resp.op == OP_AUTH_OK, f"expected AUTH_OK: {resp}"

with socket.create_connection(('127.0.0.1', SOCKET_PORT)) as sock:
    sock.sendall(Packet(op=OP_AUTH,
                        flags=FL_CLIENT_TO_SERVER | FL_USER).pack())
    sock.shutdown(socket.SHUT_WR)
    resp = recv_packet(sock)
    assert resp.op == OP_ERROR, f"expected ERROR: {resp}"
    assert resp.payload == b'max host limit has been reached', f"bad payload: {resp}"

print("TEST (tcp): sending REGISTER with id should return empty payload and REGISTER_OK")
with socket.create_connection(('127.0.0.1', SOCKET_PORT)) as sock:
    sock.sendall(Packet(op=OP_REGISTER,
                        flags=FL_CLIENT_TO_SERVER | FL_USER,
                        id=host_id).pack())
    sock.shutdown(socket.SHUT_WR)
    resp = recv_packet(sock)
    assert resp.magic == MAGIC, f"wrong magic: {resp}"
    assert resp.op == OP_REGISTER_OK, f"expected REGISTER_OK: {resp}"
    assert resp.flags == (FL_SERVER_TO_CLIENT | FL_SERVER), f"wrong flags: {resp}"
    assert resp.id == 0, f"wrong id: {resp}"
    assert len(resp.payload) == 0, f"payload should be empty: {resp}"

print("TEST (tcp): sending REGISTER with empty id should return unauthorized error")
with socket.create_connection(('127.0.0.1', SOCKET_PORT)) as sock:
    sock.sendall(Packet(op=OP_REGISTER,
                        flags=FL_CLIENT_TO_SERVER | FL_USER,
                        id=0).pack())
    sock.shutdown(socket.SHUT_WR)
    resp = recv_packet(sock)
    assert resp.magic == MAGIC, f"wrong magic: {resp}"
    assert resp.op == OP_ERROR, f"expected ERROR: {resp}"
    assert resp.flags == (FL_SERVER_TO_CLIENT | FL_SERVER), f"wrong flags: {resp}"
    assert resp.payload == b"user is unauthorized to perform this request", f"wrong payload: {resp}"

print("All tests passed!")
