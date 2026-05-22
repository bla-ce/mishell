from helpers import *

def run():
    with test("TEST (tcp): sending invalid magic value should fail"):
        resp = tcp_connection(Packet(magic=WRONG_MAGIC, op=OP_HELLO, flags=FL_CLIENT_TO_SERVER | FL_HOST))
        assert_server_response(resp, op=OP_ERROR, payload=b'invalid magic value in packet')

    with test("TEST (tcp): sending wrong direction should fail"):
        resp = tcp_connection(Packet(op=OP_HELLO, flags=FL_SERVER_TO_CLIENT | FL_HOST))
        assert_server_response(resp, op=OP_ERROR, payload=b'invalid direction flag in packet')

    with test("TEST (tcp): sending wrong mode should fail"):
        resp = tcp_connection(Packet(op=OP_HELLO, flags=FL_CLIENT_TO_SERVER | FL_SERVER))
        assert_server_response(resp, op=OP_ERROR, payload=b'invalid mode flag in packet')

    with test("TEST (tcp): sending wrong op should fail"):
        resp = tcp_connection(Packet(op=OP_COUNT, flags=FL_CLIENT_TO_SERVER | FL_USER))
        assert_server_response(resp, op=OP_ERROR, payload=b'invalid op value in packet')

    with test("TEST (tcp): sending HELLO should work and return a OK op"):
        resp = tcp_connection(Packet(op=OP_HELLO, flags=FL_CLIENT_TO_SERVER | FL_USER))
        assert_server_response(resp, op=OP_OK, payload=b'')
