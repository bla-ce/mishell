import struct
from dataclasses import dataclass

# Lengths / limits
ID_LEN                     = 0x10  # 16 bytes (128-bit, matches Packet.id)
SERVICE_NAME_MAX_LEN       = 0xF   # 15 bytes
SERVICE_MAX_COUNT_PER_HOST = 0x5

# Define type / status values and name maps here:
#   SERVICE_TYPE_TCP      = 0x01
#   SERVICE_TYPE_NAMES    = {SERVICE_TYPE_TCP: 'TCP'}
#   SERVICE_STATUS_UP     = 0x01
#   SERVICE_STATUS_NAMES  = {SERVICE_STATUS_UP: 'UP'}

SERVICE_STATUS_REGISTERED = 0x0

SERVICE_TYPE_NAMES   = {
}
SERVICE_STATUS_NAMES = {
    SERVICE_STATUS_REGISTERED: 'REGISTERED'
}

# Wire format: little-endian
# Q   = uint64   id_lo   (8 bytes)
# Q   = uint64   id_hi   (8 bytes)
# 15s = char[15] name    (15 bytes)
# B   = uint8    type    (1 byte)
# B   = uint8    status  (1 byte)
# 3x  = pad              (3 bytes)
_SERVICE_FMT  = '<QQ15sBB3x'
_SERVICE_SIZE = struct.calcsize(_SERVICE_FMT)  # 36


@dataclass
class Service:
    id:     int = 0    # 128-bit value stored as Python int (matches Packet.id)
    name:   str = ''   # max SERVICE_NAME_MAX_LEN (15) chars
    type:   int = 0
    status: int = 0

    def pack(self) -> bytes:
        name_bytes = self.name.encode('utf-8')[:SERVICE_NAME_MAX_LEN]
        name_bytes = name_bytes.ljust(SERVICE_NAME_MAX_LEN, b'\x00')
        id_lo = self.id & 0xFFFFFFFFFFFFFFFF
        id_hi = self.id >> 64
        return struct.pack(_SERVICE_FMT,
                           id_lo, id_hi,
                           name_bytes,
                           self.type,
                           self.status)

    @classmethod
    def unpack(cls, data: bytes) -> 'Service':
        """Parse wire bytes into a Service.
        Raises ValueError on malformed input.
        """
        if len(data) < _SERVICE_SIZE:
            raise ValueError(
                f"Buffer too short: need {_SERVICE_SIZE} bytes, got {len(data)}"
            )
        id_lo, id_hi, name_raw, svc_type, svc_status = \
            struct.unpack_from(_SERVICE_FMT, data)
        name = name_raw.rstrip(b'\x00').decode('utf-8', errors='replace')
        return cls(
            id     = (id_hi << 64) | id_lo,
            name   = name,
            type   = svc_type,
            status = svc_status,
        )

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Service):
            return NotImplemented
        return (self.id     == other.id     and
                self.name   == other.name   and
                self.type   == other.type   and
                self.status == other.status)

    def __repr__(self) -> str:
        type_str   = SERVICE_TYPE_NAMES.get(self.type,     f'0x{self.type:02X}')
        status_str = SERVICE_STATUS_NAMES.get(self.status, f'0x{self.status:02X}')
        return (f"Service(id={self.id}, name={self.name!r}, "
                f"type={type_str}, status={status_str})")

    def __str__(self, type_names: dict = SERVICE_TYPE_NAMES,
                       status_names: dict = SERVICE_STATUS_NAMES) -> str:
        """Key=value pairs separated by newlines."""
        type_str   = type_names.get(self.type,     f'0x{self.type:02X}')
        status_str = status_names.get(self.status, f'0x{self.status:02X}')
        return (
            f"id={self.id}\n"
            f"name={self.name}\n"
            f"type={type_str}\n"
            f"status={status_str}"
        )
