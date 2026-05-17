import struct
import socket
from dataclasses import dataclass

# Magic
MAGIC       = 0xCAFE
WRONG_MAGIC = 0xBEEF

# Request OP codes
OP_HELLO    = 0x00
OP_AUTH     = 0x01
OP_REGISTER = 0x02
OP_START    = 0x03

# Return OP codes
OP_OK    = 0x0
OP_ERROR = 0x1

# Flags
FL_CLIENT_TO_SERVER = 0b0000
FL_SERVER_TO_CLIENT = 0b0001
FL_USER             = 0b0010
FL_HOST             = 0b0100
FL_SERVER           = 0b1000

PAYLOAD_MAX_LEN = 0xFFFF

SOCKET_PATH = 'mishell.sock'
SOCKET_PORT = 7474

# Wire format: little-endian
# H  = uint16  magic       (2 bytes)
# B  = uint8   op          (1 byte)
# B  = uint8   flags       (1 byte)
# QQ = uint128 id          (16 bytes, two uint64s)
# H  = uint16  payload_len (2 bytes)
# Total header: 22 bytes
_HEADER_FMT  = '<HBBQQH'
_HEADER_SIZE = struct.calcsize(_HEADER_FMT)


@dataclass
class Packet:
    magic:   int   = MAGIC
    op:      int   = 0
    flags:   int   = 0
    id:      int   = 0      # 128-bit value stored as Python int
    payload: bytes = b''

    # Serialisation
    def pack(self) -> bytes:
        id_hi  = self.id >> 64
        id_lo  = self.id & 0xFFFFFFFFFFFFFFFF
        header = struct.pack(_HEADER_FMT,
                             self.magic, self.op, self.flags,
                             id_lo, id_hi,
                             len(self.payload))
        return header + self.payload

    # Deserialisation
    @classmethod
    def unpack(cls, data: bytes) -> 'Packet':
        """Parse wire bytes into a Packet.

        Raises ValueError on malformed input.
        """
        if len(data) < _HEADER_SIZE:
            raise ValueError(
                f"Buffer too short: need {_HEADER_SIZE} bytes, got {len(data)}"
            )
        magic, op, flags, id_lo, id_hi, payload_len = \
            struct.unpack_from(_HEADER_FMT, data)
        payload = data[_HEADER_SIZE : _HEADER_SIZE + payload_len]
        return cls(magic=magic, op=op, flags=flags,
                   id=(id_hi << 64) | id_lo, payload=payload)

    # Helpers
    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Packet):
            return NotImplemented
        return (self.magic   == other.magic   and
                self.op      == other.op      and
                self.flags   == other.flags   and
                self.id      == other.id      and
                self.payload == other.payload)

    def __repr__(self) -> str:
        flag_names = []
        if self.flags & FL_SERVER_TO_CLIENT: flag_names.append('S->C')
        else:                                flag_names.append('C->S')
        if self.flags & FL_USER:   flag_names.append('USER')
        if self.flags & FL_HOST:   flag_names.append('HOST')
        if self.flags & FL_SERVER: flag_names.append('SERVER')
        op_name = {
            OP_HELLO: 'HELLO',
            OP_AUTH: 'AUTH',
            OP_REGISTER: 'REGISTER',
            OP_OK: 'OK',
            OP_ERROR: 'ERROR'
        }.get(self.op, f'0x{self.op:02X}')

        return (f"Packet(magic=0x{self.magic:04X}, op={op_name}, "
                f"flags=[{' | '.join(flag_names)}], id={self.id}, "
                f"payload={self.payload!r})")

def recv_packet(sock: socket.socket) -> Packet:
    return Packet.unpack(sock.recv(4096))
