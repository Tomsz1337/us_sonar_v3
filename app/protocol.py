import struct

# =========================
# PROTOCOL CONSTANTS
# =========================
HEADER_FORMAT = "<BHBBHH"
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)

MAX_MESSAGE_SIZE = 65535
MAX_FRAGMENT_COUNT = 255

COMMAND     = 1
ACK         = 2
NACK        = 3
EVENT       = 4
TELEMETRY   = 5
SONAR       = 6
GPS         = 7
PING        = 8
PONG        = 9


CMD_NONE                = 0
CMD_HELLO               = 1
CMD_START_SONAR         = 2
CMD_STOP_SONAR          = 3
CMD_REQUEST_TELEMETRY   = 4
CMD_REINITIALIZE        = 5


PKT_NAMES = {
    COMMAND: "COMMAND",
    ACK: "ACK",
    NACK: "NACK",
    EVENT: "EVENT",
    TELEMETRY: "TELEMETRY",
    SONAR: "SONAR",
    GPS: "GPS",
    PING: "PING",
    PONG: "PONG"
}

def parse_packet(data_bytes: bytes):
    if len(data_bytes) < HEADER_SIZE:
        return None
    ptype, seq, fragment_index, fragment_count, total_length, payload_length = struct.unpack_from(HEADER_FORMAT, data_bytes, 0)
    if ptype not in PKT_NAMES:
        return None
    if len(data_bytes) != HEADER_SIZE + payload_length:
        return None
    if (fragment_count == 0) or (fragment_index >= fragment_count ) or (payload_length > total_length) or (total_length > MAX_MESSAGE_SIZE):
        return None
    if fragment_count == 1 and (fragment_index != 0 or payload_length != total_length):
        return None
    if fragment_count > 1 and payload_length == 0:
        return None
    
    payload = data_bytes[HEADER_SIZE:HEADER_SIZE + payload_length]

    return ptype, seq, fragment_index, fragment_count, total_length, payload

class Reassembler:
    def __init__(self):
        self.ptype = 0
        self.seq = 0
        self.fragment_count = 0
        self.total_length = 0
        self.fragments_payload = {}
        self.fragments_received = 0
        self.active = False
    def reset(self):
        self.ptype = 0
        self.seq = 0
        self.fragment_count = 0
        self.total_length = 0
        self.fragments_payload.clear()
        self.fragments_received = 0
        self.active = False
    def add_fragment(self, data_bytes):
        packet = parse_packet(data_bytes)
        if packet is None:
            return False
        ptype, seq, fragment_index, fragment_count, total_length, payload = packet
        if not self.active: 
            self.ptype = ptype
            self.seq = seq
            self.fragment_count = fragment_count
            self.total_length = total_length
            self.fragments_payload[fragment_index] = payload
            self.fragments_received += 1
            self.active = True
        else:
            if self.ptype != ptype or self.seq != seq or self.fragment_count != fragment_count or self.total_length != total_length:
                self.reset()
                return False
            if fragment_index in self.fragments_payload:
                if self.fragments_payload[fragment_index] == payload:
                    return None
                else:
                    self.reset()
                    return False
            
            self.fragments_payload[fragment_index] = payload
            self.fragments_received += 1

        if self.fragments_received == fragment_count:
            complete_payload = b''.join(self.fragments_payload[i] for i in range(self.fragment_count))

            if len(complete_payload) != self.total_length:
                self.reset()
                return False

            self.reset()
            return complete_payload
        return None
