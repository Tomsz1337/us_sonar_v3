import struct
import app.protocol as protocol

def test_parse_packet():
    header = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 1, 3, 3)
    payload = b"ABC"
    packet = header + payload
    result = protocol.parse_packet(packet)
    assert result is not None
    assert result[0] == 6
    assert result[1] == 10
    assert result[5] == b"ABC"

def test_parse_packet_too_short():
    packet = b"\x01\x02\x03"

    result = protocol.parse_packet(packet)

    assert result is None

def test_parse_packet_wrong_length():
    header = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 1, 3, 3)
    payload = b"AB"
    packet = header + payload

    result = protocol.parse_packet(packet)

    assert result is None

def test_reassembler_single_fragment():
    reassembler = protocol.Reassembler()

    header = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 1, 3, 3)
    payload = b"ABC"
    packet = header + payload

    result = reassembler.add_fragment(packet)

    assert result == b"ABC"

def test_reassembler_two_fragments():
    reassembler = protocol.Reassembler()

    header1 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 2, 6, 3)
    payload1 = b"ABC"
    packet1 = header1 + payload1

    header2 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 1, 2, 6, 3)
    payload2 = b"DEF"
    packet2 = header2 + payload2

    result = reassembler.add_fragment(packet2)
    assert result is None

    result = reassembler.add_fragment(packet1)
    assert result == b"ABCDEF"

def test_reassembler_duplicate_fragment():
    reassembler = protocol.Reassembler()

    header1 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 2, 6, 3)
    payload1 = b"ABC"
    packet1 = header1 + payload1

    header2 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 1, 2, 6, 3)
    payload2 = b"DEF"
    packet2 = header2 + payload2

    result = reassembler.add_fragment(packet2)
    assert result is None

    result = reassembler.add_fragment(packet2)
    assert result is None

    result = reassembler.add_fragment(packet1)
    assert result == b"ABCDEF"

def test_reassembler_conflicting_duplicate():
    reassembler = protocol.Reassembler()

    header1 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 2, 6, 3)
    payload1 = b"ABC"
    packet1 = header1 + payload1

    result = reassembler.add_fragment(packet1)
    assert result is None

    payload2 = b"XYZ"
    packet2 = header1 + payload2
    result = reassembler.add_fragment(packet2)
    assert result is False
    assert reassembler.active is False

def test_reassembler_new_sequence():
    reassembler = protocol.Reassembler()

    header1 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 2, 6, 3)
    packet1 = header1 + b"ABC"

    header2 = struct.pack(protocol.HEADER_FORMAT, 6, 11, 1, 2, 6, 3)
    packet2 = header2 + b"DEF"

    assert reassembler.add_fragment(packet1) is None
    assert reassembler.active is True
    assert reassembler.seq == 10
    assert reassembler.add_fragment(packet2) is None
    assert reassembler.active is True
    assert reassembler.seq == 11
    assert reassembler.fragments_received == 1
    assert reassembler.fragments_payload == {1: b"DEF"}

def test_reassembler_wrong_total_length():
    reassembler = protocol.Reassembler()

    header1 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 2, 6, 3)
    packet1 = header1 + b"ABC"

    header2 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 1, 2, 9, 3)
    packet2 = header2 + b"DEF"

    assert reassembler.add_fragment(packet1) is None
    assert reassembler.add_fragment(packet2) is False
    assert reassembler.active is False

def test_reassembler_wrong_fragment_count():
    reassembler = protocol.Reassembler()

    header1 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 2, 6, 3)
    packet1 = header1 + b"ABC"

    header2 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 1, 3, 6, 3)
    packet2 = header2 + b"DEF"

    assert reassembler.add_fragment(packet1) is None
    assert reassembler.add_fragment(packet2) is False
    assert reassembler.active is False

def test_reassembler_wrong_packet_type():
    reassembler = protocol.Reassembler()

    header1 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 2, 6, 3)
    packet1 = header1 + b"ABC"

    header2 = struct.pack(protocol.HEADER_FORMAT, 5, 10, 1, 2, 6, 3)
    packet2 = header2 + b"DEF"

    assert reassembler.add_fragment(packet1) is None
    assert reassembler.add_fragment(packet2) is False
    assert reassembler.active is False

def test_reassembler_wrong_assembled_length():
    reassembler = protocol.Reassembler()

    header1 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 2, 7, 3)
    packet1 = header1 + b"ABC"

    header2 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 1, 2, 7, 3)
    packet2 = header2 + b"DEF"

    assert reassembler.add_fragment(packet1) is None
    assert reassembler.add_fragment(packet2) is False
    assert reassembler.active is False

def test_parse_packet_zero_fragments():
    header = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 0, 3, 3)
    packet = header + b"ABC"

    assert protocol.parse_packet(packet) is None

def test_parse_packet_invalid_fragment_index():
    header = struct.pack(protocol.HEADER_FORMAT, 6, 10, 2, 2, 6, 3)
    packet = header + b"ABC"

    assert protocol.parse_packet(packet) is None

def test_parse_packet_payload_too_large():
    header = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 1, 2, 3)
    packet = header + b"ABC"

    assert protocol.parse_packet(packet) is None

def test_reassembler_empty_payload():
    reassembler = protocol.Reassembler()

    header = struct.pack(protocol.HEADER_FORMAT, 2, 10, 0, 1, 0, 0)
    packet = header

    result = reassembler.add_fragment(packet)

    assert result == b""
    assert reassembler.active is False

def test_parse_packet_max_message_size():
    payload = b"A" * 65535

    header = struct.pack(
        protocol.HEADER_FORMAT,
        6, 10, 0, 1, 65535, 65535
    )

    packet = header + payload

    result = protocol.parse_packet(packet)

    assert result is not None
    assert len(result[5]) == 65535

def test_parse_packet_unknown_type():
    header = struct.pack(protocol.HEADER_FORMAT, 200, 10, 0, 1, 3, 3)
    packet = header + b"ABC"

    result = protocol.parse_packet(packet)

    assert result is None

def test_reassembler_new_message_after_reset():
    reassembler = protocol.Reassembler()

    header1 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 2, 6, 3)
    packet1 = header1 + b"ABC"

    header2 = struct.pack(protocol.HEADER_FORMAT, 6, 11, 0, 1, 3, 3)
    packet2 = header2 + b"XYZ"

    assert reassembler.add_fragment(packet1) is None
    assert reassembler.active is True

    assert reassembler.add_fragment(packet2) == b"XYZ"
    assert reassembler.active is False

def test_reassembler_repeated_message():
    reassembler = protocol.Reassembler()

    header = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 1, 3, 3)
    packet = header + b"ABC"

    result1 = reassembler.add_fragment(packet)
    result2 = reassembler.add_fragment(packet)

    assert result1 == b"ABC"
    assert result2 == b"ABC"

def test_reassembler_new_sequence_complete_message():
    reassembler = protocol.Reassembler()

    # Pierwsza wiadomość: seq=10, fragment 0/2
    header1 = struct.pack(protocol.HEADER_FORMAT, 6, 10, 0, 2, 6, 3)
    packet1 = header1 + b"ABC"

    # Nowa wiadomość: seq=11, fragment 0/2
    header2 = struct.pack(protocol.HEADER_FORMAT, 6, 11, 0, 2, 6, 3)
    packet2 = header2 + b"DEF"

    # Drugi fragment nowej wiadomości
    header3 = struct.pack(protocol.HEADER_FORMAT, 6, 11, 1, 2, 6, 3)
    packet3 = header3 + b"GHI"

    # Rozpoczynamy składanie pierwszej wiadomości
    assert reassembler.add_fragment(packet1) is None
    assert reassembler.active is True
    assert reassembler.seq == 10

    # Nowa wiadomość przerywa poprzednią
    assert reassembler.add_fragment(packet2) is None
    assert reassembler.active is True
    assert reassembler.seq == 11

    # Odbieramy drugi fragment i składamy całość
    result = reassembler.add_fragment(packet3)

    assert result == b"DEFGHI"
    assert reassembler.active is False
    assert reassembler.fragments_received == 0