from helpers import *

def run(host_id, service_id):
    with test("TEST (tcp): sending START with empty id should return unauthorized error"):
        resp = tcp_connection(Packet(op=OP_START, flags=FL_PEER_TO_PEER | FL_HOST, id=0))
        assert_server_response(resp, op=OP_ERROR, payload=b'user is unauthorized to perform this request')

    with test("TEST (tcp): sending START with wrong server host id should return service not found"):
        payload = (0x12345).to_bytes(16, 'little') + service_id.to_bytes(16, 'little')
        resp = tcp_connection(Packet(op=OP_START, flags=FL_PEER_TO_PEER | FL_HOST, id=host_id, payload=payload))
        assert_server_response(resp, op=OP_ERROR, payload=b'service not found')

    with test("TEST (tcp): sending START with correct host id but wrong service id should return service not found"):
        payload = host_id.to_bytes(16, 'little') + (0x12345).to_bytes(16, 'little')
        resp = tcp_connection(Packet(op=OP_START, flags=FL_PEER_TO_PEER | FL_HOST, id=host_id, payload=payload))
        assert_server_response(resp, op=OP_ERROR, payload=b'service not found')

    with test("TEST (tcp): sending START with correct host id and correct service id should return OK"):
        payload = host_id.to_bytes(16, 'little') + service_id.to_bytes(16, 'little')
        resp = tcp_connection(Packet(op=OP_START, flags=FL_PEER_TO_PEER | FL_HOST, id=host_id, payload=payload))
        assert_server_response(resp, op=OP_OK, payload=b'')

    with test("TEST (tcp): sending STOP with wrong server host id should return service not found"):
        payload = (0x12345).to_bytes(16, 'little') + service_id.to_bytes(16, 'little')
        resp = tcp_connection(Packet(op=OP_STOP, flags=FL_PEER_TO_PEER | FL_HOST, id=host_id, payload=payload))
        assert_server_response(resp, op=OP_ERROR, payload=b'service not found')

    with test("TEST (tcp): sending STOP with correct host id but wrong service id should return service not found"):
        payload = host_id.to_bytes(16, 'little') + (0x12345).to_bytes(16, 'little')
        resp = tcp_connection(Packet(op=OP_STOP, flags=FL_PEER_TO_PEER | FL_HOST, id=host_id, payload=payload))
        assert_server_response(resp, op=OP_ERROR, payload=b'service not found')

    with test("TEST (tcp): sending UNREGISTER on running service should return error"):
        payload = host_id.to_bytes(16, 'little') + service_id.to_bytes(16, 'little')
        resp = tcp_connection(Packet(op=OP_UNREGISTER, flags=FL_PEER_TO_PEER | FL_HOST, id=host_id, payload=payload))
        assert_server_response(resp, op=OP_ERROR, payload=b'stop service before unregistering it')

    with test("TEST (tcp): sending STOP with correct host id and correct service id should return OK"):
        payload = host_id.to_bytes(16, 'little') + service_id.to_bytes(16, 'little')
        resp = tcp_connection(Packet(op=OP_STOP, flags=FL_PEER_TO_PEER | FL_HOST, id=host_id, payload=payload))
        assert_server_response(resp, op=OP_OK, payload=b'')

    with test("TEST (tcp): sending UNREGISTER on stopped service should return OK"):
        payload = host_id.to_bytes(16, 'little') + service_id.to_bytes(16, 'little')
        resp = tcp_connection(Packet(op=OP_UNREGISTER, flags=FL_PEER_TO_PEER | FL_HOST, id=host_id, payload=payload))
        assert_server_response(resp, op=OP_OK, payload=b'')

    with test("TEST (tcp): sending START on unregistered service should return ERROR"):
        payload = host_id.to_bytes(16, 'little') + service_id.to_bytes(16, 'little')
        resp = tcp_connection(Packet(op=OP_START, flags=FL_PEER_TO_PEER | FL_HOST, id=host_id, payload=payload))
        assert_server_response(resp, op=OP_ERROR, payload=b'service not found')
