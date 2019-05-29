# Adjust the run address to match the .org in the source code
all: main.hex

main.hex: a.out
	bin2hex.py --offset=0xc000 a.out main.hex

# 
a.out: interrupt.o vectors.o geiger.o time.o hd44780.o delay.o m6242.o ff.o diskio.o cf.o main.o 
	ld65 -C ./lib/ethergeiger.cfg -m main.map interrupt.o vectors.o geiger.o time.o hd44780.o delay.o m6242.o ff.o diskio.o cf.o main.o ./lib/ethergeiger.lib	

main.s: main.c
	cc65 -t none -O --cpu 6502 main.c
	
main.o: main.s
	ca65 --cpu 6502 main.s
	
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
	ca65 --cpu 6502 delay.s
	
time.s: time.c
	cc65 -t none -O --cpu 6502 time.c	
	
time.o: time.s
	ca65 --cpu 6502 time.s
	
geiger.s: geiger.c
	cc65 -t none -O --cpu 6502 geiger.c	
	
geiger.o: geiger.s
	ca65 --cpu 6502 geiger.s	
	
hd44780.s: hd44780.c
	cc65 -t none -O --cpu 6502 hd44780.c	
	
hd44780.o: hd44780.s
	ca65 --cpu 6502 hd44780.s
	
interrupt.o: interrupt.s
	ca65 --cpu 6502 interrupt.s
	
vectors.o: vectors.s
	ca65 --cpu 6502 vectors.s	
	
clean:
	$(RM) *.o *.lst *.map, *.out, *.hex a.out m6242.s ff.s diskio.s cf.s main.s hd44780.s
