from helpers import *

def run(host_id):
    print("SETUP: registering ping service")
    svc = Service(name="ping", type=SERVICE_TYPE_PING)
    resp = tcp_connection(Packet(op=OP_REGISTER, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=svc.pack()))
    assert_server_response(resp, op=OP_OK)
    resp_svc = Service.unpack(resp.payload)
    assert(resp_svc.id != 0x0),                             f"id not generated"
    assert(resp_svc.name == "ping"),                        f"wrong name: {resp_svc.name}"
    assert(resp_svc.status == SERVICE_STATUS_REGISTERED),   f"wrong status: {resp_svc.status}"
    ping_service_id = resp_svc.id

    print("TEST (tcp): sending PONG on stopped service should return service not started")
    resp = tcp_connection(Packet(op=0x0, flags=FL_CLIENT_TO_SERVICE | FL_HOST, id=host_id, dest_host=host_id, dest_service=ping_service_id))
    assert_server_response(resp, op=OP_ERROR, payload=b'service not started')

    print("SETUP: starting ping service")
    payload = host_id.to_bytes(16, 'little') + ping_service_id.to_bytes(16, 'little')
    resp = tcp_connection(Packet(op=OP_START, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=payload))
    assert_server_response(resp, op=OP_OK, payload=b'')

    print("TEST (tcp): sending PONG commmand to service should return OK")
    resp = tcp_connection(Packet(op=0x0, flags=FL_CLIENT_TO_SERVICE | FL_HOST, id=host_id, dest_host=host_id, dest_service=ping_service_id))
    assert_server_response(resp, op=OP_OK, payload=b'pong')

    print("SETUP: registering storage service")
    svc = Service(name="storage", type=SERVICE_TYPE_STORAGE)
    resp = tcp_connection(Packet(op=OP_REGISTER, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=svc.pack()))
    assert_server_response(resp, op=OP_OK)
    resp_svc = Service.unpack(resp.payload)
    assert(resp_svc.id != 0x0),                             f"id not generated"
    assert(resp_svc.name == "storage"),                     f"wrong name: {resp_svc.name}"
    assert(resp_svc.status == SERVICE_STATUS_REGISTERED),   f"wrong status: {resp_svc.status}"
    storage_service_id = resp_svc.id

    print("SETUP: starting storage service")
    payload = host_id.to_bytes(16, 'little') + storage_service_id.to_bytes(16, 'little')
    resp = tcp_connection(Packet(op=OP_START, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=payload))
    assert_server_response(resp, op=OP_OK, payload=b'')

    print("TEST (tcp): sending TEST commmand to STORAGE service should return test")
    resp = tcp_connection(Packet(op=0x0, flags=FL_CLIENT_TO_SERVICE | FL_HOST, id=host_id, dest_host=host_id, dest_service=storage_service_id))
    assert_server_response(resp, op=OP_OK, payload=b'test')

    print("TEST (tcp): sending REGISTER should return ERROR after registering too many services")
    for _ in range(SERVICE_MAX_COUNT_PER_HOST - 2):  # two already registered
        resp = tcp_connection(Packet(op=OP_REGISTER, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=svc.pack()))
        assert resp.op == OP_OK, f"expected OK: {resp}"

    resp = tcp_connection(Packet(op=OP_REGISTER, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=svc.pack()))
    assert_server_response(resp, op=OP_ERROR, payload=b'service limit per host has been reached')
