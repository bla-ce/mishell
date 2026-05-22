from helpers import *

def run():
    with test("TEST (unix): sending AUTH should work and return a OK op with 16 bytes payload"):
        resp = unix_connection(Packet(op=OP_AUTH, flags=FL_CLIENT_TO_SERVER | FL_USER))
        assert_server_response(resp, op=OP_OK)
        assert len(resp.payload) == 16, f"payload should be 16 bytes long: {resp}"
    host_id = int.from_bytes(resp.payload, byteorder='little')

    with test("TEST (tcp): sending AUTH with id should return empty payload"):
        resp = tcp_connection(Packet(op=OP_AUTH, flags=FL_CLIENT_TO_SERVER | FL_USER, id=host_id))
        assert_server_response(resp, op=OP_OK, payload=b'')

    with test("TEST (tcp): sending AUTH with wrong id should return ERROR: host not found"):
        resp = tcp_connection(Packet(op=OP_AUTH, flags=FL_CLIENT_TO_SERVER | FL_USER, id=0x12345))
        assert_server_response(resp, op=OP_ERROR, payload=b'host not found')

    with test("TEST (tcp): sending AUTH should return a message after adding too many hosts"):
        for _ in range(HOST_MAX_COUNT - 1):  # one already added
            resp = tcp_connection(Packet(op=OP_AUTH, flags=FL_CLIENT_TO_SERVER | FL_USER))
            assert resp.op == OP_OK, f"expected OK: {resp}"
            host_id = int.from_bytes(resp.payload, byteorder='little')

        resp = tcp_connection(Packet(op=OP_AUTH, flags=FL_CLIENT_TO_SERVER | FL_USER))
        assert_server_response(resp, op=OP_ERROR, payload=b'host limit has been reached')

    return host_id
