from helpers import *
import struct
import socket

# HOST_T layout: 16-byte id | 4-byte ipv4 | 2-byte port
def _host_payload(ip='127.0.0.1', port=SOCKET_PORT):
    return b'\x00' * 16 + socket.inet_aton(ip) + struct.pack('>H', port)

def run():
    with test("TEST (unix): sending AUTH should work and return a OK op with host id in dest_host"):
        resp = unix_connection(Packet(op=OP_AUTH, flags=FL_PEER_TO_PEER | FL_HOST, payload=_host_payload()))
        assert_server_response(resp, op=OP_OK)
        assert resp.dest_host != 0, f"dest_host should contain the host id: {resp}"
    host_id = resp.dest_host

    with test("TEST (tcp): sending AUTH with id should return empty payload"):
        resp = tcp_connection(Packet(op=OP_AUTH, flags=FL_PEER_TO_PEER | FL_HOST, id=host_id))
        assert_server_response(resp, op=OP_OK, payload=b'')

    with test("TEST (tcp): sending AUTH with wrong id should return ERROR: host not found"):
        resp = tcp_connection(Packet(op=OP_AUTH, flags=FL_PEER_TO_PEER | FL_HOST, id=0x12345))
        assert_server_response(resp, op=OP_ERROR, payload=b'host not found')

    with test("TEST (tcp): sending AUTH should return a message after adding too many hosts"):
        for _ in range(HOST_MAX_COUNT - 2):  # two already added
            resp = tcp_connection(Packet(op=OP_AUTH, flags=FL_PEER_TO_PEER | FL_HOST, payload=_host_payload()))
            assert resp.op == OP_OK, f"expected OK: {resp}"
            host_id = resp.dest_host

        resp = tcp_connection(Packet(op=OP_AUTH, flags=FL_PEER_TO_PEER | FL_HOST, payload=_host_payload()))
        assert_server_response(resp, op=OP_ERROR, payload=b'host limit has been reached')

    return host_id
