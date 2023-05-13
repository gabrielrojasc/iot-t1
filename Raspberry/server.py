import socket
import sys
import select
from struct import pack

from unpacking import parse_data
from db import get_configs


def TCP_frag_recv(conn):
    doc = b""
    while True:
        try:
            conn.settimeout(5)
            data = conn.recv(1024)
            if data == b"\0":
                break
            else:
                doc += data
        except socket.timeout:
            conn.send(b"\0")
            break
        except Exception:
            conn.send(b"\0")
            raise
        conn.send(b"\1")
    return doc


def UDP_frag_recv(s):
    doc = b""
    addr = None
    while True:
        try:
            data, addr = s.recvfrom(1024)
            if data == b"\0":
                break
            else:
                doc += data
        except socket.timeout:
            break
        except Exception:
            raise
        s.sendto(b"\1", addr)
    return (doc, addr)


def send_config(socket, protocol, transport_layer):
    packet = pack("<2c", str(protocol).encode(), str(transport_layer).encode())
    socket.sendall(packet)
    print(f"Enviado: {packet}")


def create_socket_UDP():
    sUDP = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # Internet  # UDP
    sUDP.bind((HOST, UDP_PORT))
    sUDP.settimeout(5)
    return sUDP


# TCP SOCKET
HOST = "0.0.0.0"
PORT = 8000


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # internet  # TCP
s.bind((HOST, PORT))
s.listen(5)
print(f"Listening on {HOST}:{PORT}")

# UDP SOCKET
UDP_PORT = 8001
sUDP = create_socket_UDP()

# TCP: 1; UDP: 0
transport_layer = 1
buffer = 1024


for protocol, transport_layer in get_configs():
    conn, addr = s.accept()
    print(f"Conectado por alguien ({addr[0]}) desde el puerto {addr[1]}")
    send_config(conn, protocol, transport_layer)
    data = b""
    for _ in range(5):
        if select.select([sys.stdin], [], [], 0) == ([sys.stdin], [], []):
            key = sys.stdin.read(1)
            if key == "n":
                break
            else:
                print(f"Key '{key}' pressed!")

        try:
            if transport_layer == 1:  # TCP
                data = TCP_frag_recv(conn)
            else:  # UDP
                data, _ = UDP_frag_recv(sUDP)
        except ConnectionResetError:
            break

        if data == b"":
            continue

        print(f"Recibido raw:\n{data}")
        parsed_data = parse_data(data)

        if parse_data is not None:
            print(f"Recibido:\n{parsed_data}")
            transport_layer = parsed_data.get("transport_layer")

    conn.sendall(b"\2")
    conn.close()
    if transport_layer == 0:
        sUDP.sendall(b"\2")
        sUDP.close()
        sUDP = create_socket_UDP()

    print("Desconectado")
