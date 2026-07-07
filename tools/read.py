#!/usr/bin/env python3
from pymodbus.client import ModbusSerialClient
from pymodbus.exceptions import ModbusException
import time

SLAVE_ADDR = 0xAB

MODBUS_INPUT_CPM = 0
MODBUS_INPUT_SIVERT_INT = 1
MODBUS_INPUT_SIVERT_FRACT = 2
MODBUS_INPUT_DSTEMPH = 3
MODBUS_INPUT_DSTEMPL = 4
MODBUS_INPUT_DSTEMP_TIMEH = 5
MODBUS_INPUT_DSTEMP_TIMEL = 6
MODBUS_INPUT_TIMEH = 7
MODBUS_INPUT_TIMEL = 8
MODBUS_INPUT_UPTIMEH = 9
MODBUS_INPUT_UPTIMEL = 10
MODBUS_INPUT_LASTCFH = 11
MODBUS_INPUT_LASTCFL = 12
MODBUS_INPUT_CFRES = 13

def read_inputs(client):
    try:
        rr = client.read_input_registers(
            address=0,
            count=14,
            slave=SLAVE_ADDR
        )
    except ModbusException as exc:
        print("Modbus exception:", exc)
        return

    if rr is None or rr.isError():
        print("Read error:", rr)
        return

    regs = rr.registers
    cpm = regs[MODBUS_INPUT_CPM]
    siv_int = regs[MODBUS_INPUT_SIVERT_INT]
    siv_fract = regs[MODBUS_INPUT_SIVERT_FRACT]
    
    dsh = regs[MODBUS_INPUT_DSTEMPH]
    dsl = regs[MODBUS_INPUT_DSTEMPL]
    dstimeh = regs[MODBUS_INPUT_DSTEMP_TIMEH]
    dstimel = regs[MODBUS_INPUT_DSTEMP_TIMEL]
    
    timeh = regs[MODBUS_INPUT_TIMEH]
    timel = regs[MODBUS_INPUT_TIMEL]
    uptimeh = regs[MODBUS_INPUT_UPTIMEH]
    uptimel = regs[MODBUS_INPUT_UPTIMEL]
    cftimeh = regs[MODBUS_INPUT_LASTCFH]
    cftimel = regs[MODBUS_INPUT_LASTCFL]
    cfres = regs[MODBUS_INPUT_CFRES]

    temp_int = int16(dsh)
    temp_frac = dsl
    if temp_int < 0:
        temp_str = f"-{abs(temp_int)}.{temp_frac:03d}"
    else:
        temp_str = f"{temp_int}.{temp_frac:03d}"

    ds18b20_temp_timestamp = (dstimeh << 16) | dstimel
    timestamp = (timeh << 16) | timel
    uptime = (uptimeh << 16) | uptimel
    cftime = (cftimeh << 16) | cftimel

    print("CPM:", cpm)
    print("Siv:", f"{siv_int}.{siv_fract:04d} uSv/h")
    print("Temp:", temp_str, "°C")
    print("Temp timestamp: ", ds18b20_temp_timestamp)
    print("Time raw:", timestamp)
    print("Uptime:", uptime, "seconds")
    print("CF update: ", cftime)
    print("CF result:", hex(cfres))
    print("-" * 40)

def int16(v):
    """Convert unsigned Modbus register to signed int16."""
    return v - 0x10000 if v & 0x8000 else v

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
            time.sleep(10)
    except KeyboardInterrupt:
        print("Stopped")
    finally:
        client.close()

if __name__ == "__main__":
    main()
