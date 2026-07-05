#!/usr/bin/env python3

import datetime
import subprocess
import sys

PORT = "/dev/ttyUSB0"
SLAVE = "0xAB"
BAUD = "9600"

#now = datetime.datetime.now()
now = datetime.datetime.utcnow()

# Pack two 8-bit values into one Modbus holding register
H1 = (now.hour << 8) | now.minute
H2 = (now.second << 8) | now.day
H3 = (now.month << 8) | (now.year % 100)

CMD = 0x03      # MODBUS_CMD_SET_DATETIME

cmd = [
    "mbpoll",
    "-m", "rtu",
    "-b", BAUD,
    "-P", "none",
    "-a", SLAVE,
    "-t", "4",
    "-r", "1",          # Holding register 0
    PORT,
    str(CMD),
    str(H1),
    str(H2),
    str(H3)
]

print("Writing:")
print(f" Command = {CMD}")
print(f" H1 = 0x{H1:04X} ({now.hour:02}:{now.minute:02})")
print(f" H2 = 0x{H2:04X} ({now.second:02}s, {now.day:02} day)")
print(f" H3 = 0x{H3:04X} ({now.month:02}/{now.year % 100:02})")
print()

result = subprocess.run(cmd)

sys.exit(result.returncode)
