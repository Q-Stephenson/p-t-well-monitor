import serial
import sys

print("Serial Listener")

if len(sys.argv) < 2:
    print("Usage: serial_listener.py COMx")
    sys.exit(1)

com_port = sys.argv[1]

try:
    ser = serial.Serial(com_port, 115200)
    print(f"Listening on {com_port}...")
    while True:
        if ser.in_waiting:
            line = ser.readline().decode().strip()
            print(f"[{com_port}] {line}")
except Exception as e:
    print(f"Error: {e}")
