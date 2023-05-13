#!/usr/bin/env python3

import sqlite3 as sql

createDatos = """CREATE TABLE Datos (
    id_datos INTEGER PRIMARY KEY AUTOINCREMENT,

    id_device CHAR(2) NOT NULL,
    MAC BINARY(6) NOT NULL,
    transport_layer TINYINT NOT NULL,
    protocol TINYINT NOT NULL,
    length SMALLINT NOT NULL,

    Val1 TINYINT NOT NULL,
    Batt_level TINYINT NOT NULL,
    Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    Temp TINYINT,
    Press FLOAT,
    Hum TINYINT,
    Co FLOAT,
    RMS FLOAT,
    Amp_x FLOAT,
    Frec_x FLOAT,
    Amp_y FLOAT,
    Frec_y FLOAT,
    Amp_z FLOAT,
    Frec_z FLOAT
);"""

createLogs = """CREATE TABLE Logs (
    id_log INTEGER PRIMARY KEY AUTOINCREMENT,
    datos INTEGER NOT NULL,

    id_device CHAR(2) NOT NULL,
    transport_layer TINYINT NOT NULL,
    protocol TINYINT NOT NULL,
    Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY(datos) REFERENCES Datos(id_datos)
);"""

createConfiguracion = """CREATE TABLE Configuracion (
    id_conf INTEGER PRIMARY KEY AUTOINCREMENT,
    protocol TINYINT NOT NULL,
    transport_layer TINYINT NOT NULL
);"""

createLoss = """CREATE TABLE Loss (
    id_loss INTEGER PRIMARY KEY AUTOINCREMENT,
    datos INTEGER NOT NULL,

    Timedelay DATETIME DEFAULT CURRENT_TIMESTAMP,
    Packet_loss INTEGER,

    FOREIGN KEY(datos) REFERENCES Datos(id_datos)
);"""


def create_config(cur):
    for protocol in range(5):
        for transport_layer in range(2):
            print(
                f"Creando configuración: protocolo {protocol}, capa de transporte {transport_layer}"
            )
            cur.execute(
                "INSERT INTO Configuracion (protocol, transport_layer) VALUES (?, ?);",
                (protocol, transport_layer),
            )


# inicializa la BDD
conn = sql.connect("DB.sqlite")
cur = conn.cursor()
print("Creando tablas...")
cur.execute(createDatos)
cur.execute(createLogs)
cur.execute(createConfiguracion)
cur.execute(createLoss)
print("Creando configuraciones...")
create_config(cur)
conn.commit()
conn.close()
