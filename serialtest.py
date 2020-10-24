import serial
with serial.Serial("/dev/ttyACM0") as ser:
    ser.write(b"/time/set 2020-10-19T22:59:50\n")
    ser.write(b"/pot/1/start_hour/set 21\n")
    ser.write(b"/pot/1/end_hour/set 2\n")
    ser.write(b"/pot/1/period_hours/set 2\n")
    ser.write(b"/pot/1/start_minute/set 0\n")
    ser.write(b"/pot/1/stop_minute/set 1\n")
    ser.write(b"/pot/1/mode/set LEVEL_SCHEDULE\n")
    #ser.write(b"/relay/1/on_hour/set 22\n")
    #ser.write(b"/relay/1/off_hour/set 23\n")
    #ser.write(b"/relay/1/mode/set SCHEDULE\n")
    #ser.write(b"/pot/1/pump/set 1\n")
    #ser.write(b"/pot/1/mode/set LEVEL_CONTROL\n")
    #ser.write(b"/pot/1/level_setpoint/set 0\n")
    #ser.write(b"/pot/2/mode/set LEVEL_CONTROL\n")
    #ser.write(b"/pot/2/level_setpoint/set 0\n")
    while True:
        l = ser.readline().decode()
        if not l.startswith("/pot") or l.startswith("/pot/1"):
            print(l.strip())
