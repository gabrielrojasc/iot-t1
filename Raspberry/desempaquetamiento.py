from struct import unpack, pack
import traceback
from DatabaseWork import *

# Documentación struct unpack,pack :https://docs.python.org/3/library/struct.html#
"""
Estas funciones se encargan de parsear y guardar los datos recibidos.
Usamos struct para pasar de un array de bytes a una lista de numeros/strings. (https://docs.python.org/3/library/struct.html)
(La ESP32 manda los bytes en formato little-endian, por lo que los format strings deben empezar con <)

-dataSave: Guarda los datos en la BDD
-response: genera un OK para mandar de vuelta cuando se recibe un mensaje, con posibilidad de pedir que se cambie el status/protocol
-protUnpack: desempaca un byte array con los datos de un mensaje (sin el header)
-headerDict: Transforma el byte array de header (los primeros 10 bytes de cada mensaje) en un diccionario con la info del header
-dataDict: Transforma el byta array de datos (los bytes luego de los primeros 10) en un diccionario con los datos edl mensaje

"""


def response(change: bool = False, status: int = 255, protocol: int = 255):
    OK = 1
    CHANGE = 1 if change else 0
    return pack("<BBBB", OK, CHANGE, status, protocol)


def parse_data(packet):
    header = packet[:12]
    data = packet[12:]
    header = header_dict(header)
    dataD = data_dict(header["protocol"], data)
    # if dataD is not None:
    #    dataSave(header, dataD)

    return None if dataD is None else {**header, **dataD}


def prot_unpack(protocol: int, data):
    protocol_unpack = ["<2Bl", "<2BlBfBf", "<2BlBfBff", "<2BlBfB8f", "<2BlBfBf6000f"]
    array = unpack(protocol_unpack[protocol], data)
    if protocol == 4:
        pad = 7
        l_arr = 2000
        array = (
            *array[:pad],
            array[pad : pad + l_arr],
            array[pad + l_arr : pad + 2 * l_arr],
            array[pad + 2 * l_arr :],
        )
    return array


def header_dict(data):
    (
        id_device,
        M1,
        M2,
        M3,
        M4,
        M5,
        M6,
        transport_layer,
        protocol,
        leng_msg,
    ) = unpack("<2s6BccH", data)
    MAC = ":".join([hex(x)[2:] for x in [M1, M2, M3, M4, M5, M6]])
    return {
        "id_device": id_device,
        "MAC": MAC,
        "transport_layer": int(transport_layer),
        "protocol": int(protocol),
        "length": leng_msg,
    }


def data_dict(protocol: int, data):
    if protocol not in [0, 1, 2, 3, 4, 5]:
        print("Error: protocol doesnt exist")
        return None

    p0 = ("Val: 1", "Batt_level", "Timestamp")
    p1 = (*p0, "Temp", "Press", "Hum", "Co")
    p2 = (*p1, "RMS")
    p3 = (*p2, "Amp_x", "Frec_x", "Amp_y", "Frec_y", "Amp_z", "Frec_z")
    p4 = (*p1, "Acc_x", "Acc_y", "Acc_z")
    p = (p0, p1, p2, p3, p4)

    try:
        unp = prot_unpack(protocol, data)
        keys = p[protocol]
        return {key: val for (key, val) in zip(keys, unp)}
    except Exception:
        print("Data unpacking Error:", traceback.format_exc())
        return None
