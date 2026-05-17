import socket
from packet import *
from service import *

HOST_MAX_COUNT = 5
SERVICE_MAX_COUNT_PER_HOST = 5

def tcp_connection(packet):
    with socket.create_connection(('127.0.0.1', SOCKET_PORT)) as sock:
        sock.sendall(packet.pack())
        sock.shutdown(socket.SHUT_WR)
        return recv_packet(sock)

def unix_connection(packet):
    with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
        sock.connect(SOCKET_PATH)
        sock.sendall(packet.pack())
        sock.shutdown(socket.SHUT_WR)
        return recv_packet(sock)

def assert_server_response(resp, *, op, payload=None, id=0):
    assert resp.magic == MAGIC,                          f"wrong magic: {resp}"
    assert resp.op == op,                                f"expected {op}: {resp}"
    assert resp.flags == (FL_SERVER_TO_CLIENT | FL_SERVER), f"wrong flags: {resp}"
    assert resp.id == id,                                f"wrong id: {resp}"
    if payload is not None:
        assert resp.payload == payload,                  f"wrong payload: {resp}"

print("TEST (tcp): sending invalid magic value should fail")
resp = tcp_connection(Packet(magic=WRONG_MAGIC, op=OP_HELLO, flags=FL_CLIENT_TO_SERVER | FL_HOST))
assert_server_response(resp, op=OP_ERROR, payload=b'invalid magic value in packet')

print("TEST (tcp): sending wrong direction should fail")
resp = tcp_connection(Packet(op=OP_HELLO, flags=FL_SERVER_TO_CLIENT | FL_HOST))
assert_server_response(resp, op=OP_ERROR, payload=b'invalid direction flag in packet')

print("TEST (tcp): sending wrong mode should fail")
resp = tcp_connection(Packet(op=OP_HELLO, flags=FL_CLIENT_TO_SERVER | FL_SERVER))
assert_server_response(resp, op=OP_ERROR, payload=b'invalid mode flag in packet')

print("TEST (tcp): sending wrong op should fail")
resp = tcp_connection(Packet(op=0x4, flags=FL_CLIENT_TO_SERVER | FL_USER))
assert_server_response(resp, op=OP_ERROR, payload=b'invalid op value in packet')

print("TEST (tcp): sending HELLO should work and return a OK op")
resp = tcp_connection(Packet(op=OP_HELLO, flags=FL_CLIENT_TO_SERVER | FL_USER))
assert_server_response(resp, op=OP_OK, payload=b'')

print("TEST (unix): sending AUTH should work and return a OK op with 16 bytes payload")
resp = unix_connection(Packet(op=OP_AUTH, flags=FL_CLIENT_TO_SERVER | FL_USER))
assert_server_response(resp, op=OP_OK)
assert len(resp.payload) == 16, f"payload should be 16 bytes long: {resp}"
host_id = int.from_bytes(resp.payload, byteorder='little')

print("TEST (tcp): sending AUTH with id should return empty payload")
resp = tcp_connection(Packet(op=OP_AUTH, flags=FL_CLIENT_TO_SERVER | FL_USER, id=host_id))
assert_server_response(resp, op=OP_OK, payload=b'')

print("TEST (tcp): sending AUTH with wrong id should return ERROR: host not found")
resp = tcp_connection(Packet(op=OP_AUTH, flags=FL_CLIENT_TO_SERVER | FL_USER, id=0x12345))
assert_server_response(resp, op=OP_ERROR, payload=b'host not found')

print("TEST (tcp): sending AUTH should return a message after adding too many hosts")
for _ in range(HOST_MAX_COUNT - 1):  # one already added
    resp = tcp_connection(Packet(op=OP_AUTH, flags=FL_CLIENT_TO_SERVER | FL_USER))
    assert resp.op == OP_OK, f"expected OK: {resp}"
    host_id = int.from_bytes(resp.payload, byteorder='little')

resp = tcp_connection(Packet(op=OP_AUTH, flags=FL_CLIENT_TO_SERVER | FL_USER))
assert_server_response(resp, op=OP_ERROR, payload=b'host limit has been reached')

print("TEST (tcp): sending REGISTER with empty id should return unauthorized error")
resp = tcp_connection(Packet(op=OP_REGISTER, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=0))
assert_server_response(resp, op=OP_ERROR, payload=b'user is unauthorized to perform this request')

print("TEST (tcp): sending REGISTER with FL_USER flag should return unauthorized error")
resp = tcp_connection(Packet(op=OP_REGISTER, flags=FL_CLIENT_TO_SERVER | FL_USER, id=host_id))
assert_server_response(resp, op=OP_ERROR, payload=b'user is unauthorized to perform this request')

print("TEST (tcp): sending REGISTER with service payload should return service and OK")
service_name = "my-service"
svc = Service(name=service_name, type=0x00)
resp = tcp_connection(Packet(op=OP_REGISTER, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=svc.pack()))
assert_server_response(resp, op=OP_OK)
resp_svc = Service.unpack(resp.payload)
assert(resp_svc.id != 0x0),                             f"id not generated"
assert(resp_svc.name != service_name),                  f"wrong name: {resp_svc.name}"
assert(resp_svc.status == SERVICE_STATUS_REGISTERED),   f"wrong status: {resp_svc.status}"
service_id = resp_svc.id

print("TEST (tcp): sending REGISTER should return ERROR after registering too many services")
for _ in range(SERVICE_MAX_COUNT_PER_HOST - 1):  # one already registered
    resp = tcp_connection(Packet(op=OP_REGISTER, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=svc.pack()))
    assert resp.op == OP_OK, f"expected OK: {resp}"

resp = tcp_connection(Packet(op=OP_REGISTER, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=svc.pack()))
assert_server_response(resp, op=OP_ERROR, payload=b'service limit per host has been reached')

print("TEST (tcp): sending START with empty id should return unauthorized error")
resp = tcp_connection(Packet(op=OP_START, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=0))
assert_server_response(resp, op=OP_ERROR, payload=b'user is unauthorized to perform this request')

print("TEST (tcp): sending START with wrong server host id should return service not found")
wrong_host_id_payload = (0x12345).to_bytes(16, 'little') + service_id.to_bytes(16, 'little')
resp = tcp_connection(Packet(op=OP_START, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=wrong_host_id_payload))
assert_server_response(resp, op=OP_ERROR, payload=b'service not found')

print("TEST (tcp): sending START with correct host id but wrong service id should return service not found")
wrong_service_id_payload = host_id.to_bytes(16, 'little') + (0x12345).to_bytes(16, 'little')
resp = tcp_connection(Packet(op=OP_START, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=wrong_service_id_payload))
assert_server_response(resp, op=OP_ERROR, payload=b'service not found')


print("TEST (tcp): sending START with correct host id and correct service id should return OK")
payload = host_id.to_bytes(16, 'little') + service_id.to_bytes(16, 'little')
resp = tcp_connection(Packet(op=OP_START, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=payload))
assert_server_response(resp, op=OP_OK, payload=b'')

print("All tests passed!")
