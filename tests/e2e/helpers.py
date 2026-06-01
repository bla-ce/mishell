import socket
from contextlib import contextmanager
from packet import *
from service import *

HOST_MAX_COUNT = 5

GREEN = "\033[32m"
RED   = "\033[31m"
RESET = "\033[0m"

@contextmanager
def test(description):
    print(description, end=" ... ", flush=True)
    try:
        yield
        print(f"{GREEN}PASSED{RESET}")
    except Exception:
        print(f"{RED}FAILED{RESET}")
        raise

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
    assert resp.flags == (FL_PEER_TO_PEER | FL_HOST) or (FL_SERVICE | FL_SERVICE_TO_CLIENT), f"wrong flags: {resp}"
    assert resp.id == id,                                f"wrong id: {resp}"
    if payload is not None:
        assert resp.payload == payload,                  f"wrong payload: {resp}"
