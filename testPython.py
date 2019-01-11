import serial
import datetime
ser = serial.Serial('/dev/ttyACM1', 9600)

val = 0

cmds = ["MSv2Motors_60_speed_1_3F", "MSv2Motors_60_direction_1_1",
"MSv2Motors_60_speed_2_3F", "MSv2Motors_60_direction_2_1",
"MSv2Motors_60_speed_3_3F", "MSv2Motors_60_direction_3_1"]
#ser.write(cmds[0])
while True:
    #ser.write(cmds[val])
    #val -=1
    if "ready" in str(ser.readline()):
        print("here")
        if val < 2:
            print("current time: ", str(datetime.datetime.now()))
            ser.write(cmds[val].encode("utf-8"))
            val -= 1