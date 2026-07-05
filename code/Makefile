# Adjust the run address to match the .org in the source code
all: ethergeiger.hex

ethergeiger.hex: a.out
	bin2hex.py a.out ethergeiger.hex

a.out: interrupt.o vectors.o mc6840.o mos6551.o hd44780.o delay.o m6242.o io.o modbus.o cf.o diskio.o ff.o bus8255.o onewire.o i2c.o main.o 
	ld65 -C ./lib/ethergeiger.cfg -m main.map interrupt.o vectors.o mc6840.o mos6551.o hd44780.o delay.o m6242.o io.o modbus.o cf.o diskio.o ff.o bus8255.o onewire.o i2c.o main.o ./lib/ethergeiger.lib	

main.s: main.c
	cc65 -t none -O --cpu 65c02 main.c
main.o: main.s
	ca65 --cpu 65c02 main.s
	
cf.s: cf.c
	cc65 -t none -O --cpu 65c02 cf.c	
cf.o: cf.s
	ca65 --cpu 65c02 cf.s
	
diskio.s: diskio.c
	cc65 -t none -O --cpu 65c02 diskio.c	
diskio.o: diskio.s
	ca65 --cpu 65c02 diskio.s

ff.s: ff.c
	cc65 -t none -O --cpu 65c02 ff.c	
ff.o: ff.s
	ca65 --cpu 65c02 ff.s

m6242.s: m6242.c
	cc65 -t none -O --cpu 65c02 m6242.c	
m6242.o: m6242.s
	ca65 --cpu 65c02 m6242.s	
	
delay.o: delay.s
	ca65 --cpu 65c02 delay.s
	
mc6840.s: mc6840.c
	cc65 -t none -O --cpu 65c02 mc6840.c	
mc6840.o: mc6840.s
	ca65 --cpu 65c02 mc6840.s

mos6551.s: mos6551.c
	cc65 -t none -O --cpu 65c02 mos6551.c	
mos6551.o: mos6551.s
	ca65 --cpu 65c02 mos6551.s
	
hd44780.s: hd44780.c
	cc65 -t none -O --cpu 65c02 hd44780.c	
hd44780.o: hd44780.s
	ca65 --cpu 65c02 hd44780.s
	
modbus.s: modbus.c
	cc65 -W -unused-param -t none -O --cpu 65c02 modbus.c	
modbus.o: modbus.s
	ca65 --cpu 65c02 modbus.s		
	
io.s: io.c
	cc65 -t none -O --cpu 65c02 io.c	
io.o: io.s
	ca65 --cpu 65c02 io.s
	
bus8255.s: bus8255.c
	cc65 -t none -O --cpu 65c02 bus8255.c	
bus8255.o: bus8255.s
	ca65 --cpu 65c02 bus8255.s
	
onewire.s: onewire.c
	cc65 -t none -O --cpu 65c02 onewire.c	
onewire.o: onewire.s
	ca65 --cpu 65c02 onewire.s		
	
i2c.s: i2c.c
	cc65 -t none -O --cpu 65c02 i2c.c	
i2c.o: i2c.s
	ca65 --cpu 65c02 i2c.s	
	
interrupt.o: interrupt.s
	ca65 --cpu 65c02 interrupt.s
	
vectors.o: vectors.s
	ca65 --cpu 65c02 vectors.s	
	
clean:
	$(RM) *.o *.lst *.map *.out *.hex a.out m6242.s ff.s diskio.s cf.s main.s hd44780.s mc6840.s mos6551.s io.s modbus.s bus8255.s onewire.s i2c.s
