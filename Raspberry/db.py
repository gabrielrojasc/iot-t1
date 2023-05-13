import sqlite3 as sql
import datetime

# Documentación https://docs.python.org/3/library/sqlite3.html


def data_save(header, data):
    now = datetime.datetime.now()

    with sql.connect("DB.sqlite") as con:
        cur = con.cursor()

        # Obtenemos el Timestamp de la última fila, para poder calcular el Timedelay
        cursor = con.execute(
            "SELECT Timestamp FROM Datos ORDER BY id_datos DESC LIMIT 1"
        )
        row = cursor.fetchone()
        previous_timestamp = row[0] if row else None

        # Datos comunes para todos los protocolos
        cur.execute(
            """insert into Datos (id_device, MAC, transport_layer, protocol, length, Val1, Batt_level, Timestamp) values (?, ?, ?, ?, ?, ?, ?, ?)""",
            (
                header["id_device"],
                header["MAC"],
                header["transport_layer"],
                header["protocol"],
                header["length"],
                header["protocol"],
                data["Val: 1"],
                data["Batt_level"],
                now,
            ),
        )

        row_id = cur.lastrowid

        # Updates de Datos para protocolos específicos
        if header["protocol"] in [1, 2, 3, 4]:
            cur.execute(
                """UPDATE Datos SET Temp = ?, Press = ?, Hum = ?, Co = ? WHERE id_datos = ?""",
                (data["Temp"], data["Press"], data["Hum"], data["Co"], row_id),
            )

        if header["protocol"] in [2, 3]:
            cur.execute(
                """UPDATE Datos SET RMS = ? WHERE id_datos = ?""", (data["RMS"], row_id)
            )

        if header["protocol"] in [3]:
            cur.execute(
                """UPDATE Datos SET Amp_x = ?, Frec_x = ?, Amp_y = ?, Frec_y = ?, Amp_z = ?, Frec_z = ? WHERE id_datos = ?""",
                (
                    data["Amp_x"],
                    data["Frec_x"],
                    data["Amp_y"],
                    data["Frec_y"],
                    data["Amp_z"],
                    data["Frec_z"],
                    row_id,
                ),
            )

        # Insert de Logs
        cur.execute(
            """insert into Logs (datos, id_device, transport_layer, protocol, Timestamp) values (?, ?, ?, ?, ?)""",
            (
                row_id,
                header["id_device"],
                header["transport_layer"],
                header["protocol"],
                now,
            ),
        )

        # Insert de Configuracion
        cur.execute(
            """insert into Configuracion (protocol, transport_layer) values (?, ?)""",
            (header["protocol"], header["transport_layer"]),
        )  # No sé realmente si así haya que trabajar con esta tabla

        # Insert de Loss
        if previous_timestamp is not None:
            delay = now - previous_timestamp
        else:
            delay = None

        cur.execute(
            """insert into Loss (datos, Timedelay, Packet_loss) values (?, ?, ?)""",
            (row_id, delay, 0),
        )  # Cambiar para guardar el Packet_loss
