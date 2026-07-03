#!/usr/bin/env python3

from pymodbus.client import ModbusSerialClient
import time

SLAVE_ADDR = 0xAB

# Register mapping (your firmware layout)
MODBUS_INPUT_CPM = 0
MODBUS_INPUT_SIVERT_INT = 1
MODBUS_INPUT_SIVERT_FRACT = 2
MODBUS_INPUT_TIMEH = 3
MODBUS_INPUT_TIMEL = 4
MODBUS_INPUT_UPTIMEH = 5
MODBUS_INPUT_UPTIMEL = 6

def read_inputs(client):
    rr = client.read_input_registers(
        address=0,
        count=7,
        slave=SLAVE_ADDR
    )

    if rr.isError():
        print("Read error:", rr)
        return

    regs = rr.registers

    cpm = regs[MODBUS_INPUT_CPM]
    siv_int = regs[MODBUS_INPUT_SIVERT_INT]
    siv_fract = regs[MODBUS_INPUT_SIVERT_FRACT]

    timeh = regs[MODBUS_INPUT_TIMEH]
    timel = regs[MODBUS_INPUT_TIMEL]

    uptimeh = regs[MODBUS_INPUT_UPTIMEH]
    uptimel = regs[MODBUS_INPUT_UPTIMEL]

    # Combine 32-bit values
    timestamp = (timeh << 16) | timel
    uptime = (uptimeh << 16) | uptimel

    print("CPM:", cpm)
    print("Dose:", f"{siv_int}.{siv_fract:04d} uSv")
    print("Time raw:", hex(timestamp))
    print("Uptime:", uptime, "seconds")
    print("-" * 40)


def main():
    client = ModbusSerialClient(
        port="/dev/ttyUSB0",
        baudrate=9600,
        parity="N",
        stopbits=1,
        bytesize=8,
        timeout=1
    )

    if not client.connect():
        print("Failed to connect to serial port")
        return

    try:
        while True:
            read_inputs(client)
            time.sleep(1)

    except KeyboardInterrupt:
        print("Stopped")

    finally:
        client.close()


if __name__ == "__main__":
    main()
