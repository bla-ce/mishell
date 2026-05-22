from helpers import *

def run(host_id):
    with test("TEST (tcp): sending REGISTER with empty id should return unauthorized error"):
        resp = tcp_connection(Packet(op=OP_REGISTER, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=0))
        assert_server_response(resp, op=OP_ERROR, payload=b'user is unauthorized to perform this request')

    with test("TEST (tcp): sending REGISTER with FL_USER flag should return unauthorized error"):
        resp = tcp_connection(Packet(op=OP_REGISTER, flags=FL_CLIENT_TO_SERVER | FL_USER, id=host_id))
        assert_server_response(resp, op=OP_ERROR, payload=b'user is unauthorized to perform this request')

    with test("TEST (tcp): sending REGISTER with service payload and wrong type should return error"):
        svc = Service(name="ping", type=0x03)
        resp = tcp_connection(Packet(op=OP_REGISTER, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=svc.pack()))
        assert_server_response(resp, op=OP_ERROR, payload=b'invalid service type')

    with test("TEST (tcp): sending REGISTER with service payload with invalid name should return error"):
        svc = Service(name='a' * SERVICE_NAME_MAX_LEN, type=SERVICE_TYPE_PING)
        resp = tcp_connection(Packet(op=OP_REGISTER, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=svc.pack()))
        assert_server_response(resp, op=OP_ERROR, payload=b'invalid service name')

    with test("TEST (tcp): sending REGISTER with service payload should return service and OK"):
        service_name = "ping"
        svc = Service(name=service_name, type=SERVICE_TYPE_PING)
        resp = tcp_connection(Packet(op=OP_REGISTER, flags=FL_CLIENT_TO_SERVER | FL_HOST, id=host_id, payload=svc.pack()))
        assert_server_response(resp, op=OP_OK)
        resp_svc = Service.unpack(resp.payload)
        assert(resp_svc.id != 0x0),                             f"id not generated"
        assert(resp_svc.name == service_name),                  f"wrong name: {resp_svc.name}"
        assert(resp_svc.status == SERVICE_STATUS_REGISTERED),   f"wrong status: {resp_svc.status}"

    return resp_svc.id
