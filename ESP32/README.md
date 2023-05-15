# ESP32 Server

Estructura:

```
esp32
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   ├── conexiones.c
│   ├── main.c
│   ├── packeting.c
│   └── sensors.c
└── sdkconfig
```

To run (macos):

```bash
idf.py build && idf.py -p /dev/cu.usbserial-001 flash monitor
```
