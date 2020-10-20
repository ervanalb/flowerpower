import serial
with serial.Serial("/dev/ttyACM2") as ser:
    ser.write(b"/time/set 2020-10-19T22:59:50\n")
    ser.write(b"/relay/1/on/hour/set 22\n")
    ser.write(b"/relay/1/off/hour/set 23\n")
    ser.write(b"/relay/1/mode/set SCHEDULE\n")
    while True:
        print(ser.readline())
