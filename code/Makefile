# Adjust the run address to match the .org in the source code
all: ethergeiger.hex

ethergeiger.hex: a.out
	bin2hex.py a.out ethergeiger.hex

a.out: interrupt.o vectors.o mc6840.o mos6551.o hd44780.o delay.o m6242.o io.o parser.o cf.o diskio.o ff.o main.o 
	ld65 -C ./lib/ethergeiger.cfg -m main.map interrupt.o vectors.o mc6840.o mos6551.o hd44780.o delay.o m6242.o io.o parser.o cf.o diskio.o ff.o main.o ./lib/ethergeiger.lib	

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
	
parser.s: parser.c
	cc65 -W -unused-param -t none -O --cpu 65c02 parser.c	

parser.o: parser.s
	ca65 --cpu 65c02 parser.s		
	
io.s: io.c
	cc65 -t none -O --cpu 65c02 io.c	

io.o: io.s
	ca65 --cpu 65c02 io.s		
	
interrupt.o: interrupt.s
	ca65 --cpu 65c02 interrupt.s
	
vectors.o: vectors.s
	ca65 --cpu 65c02 vectors.s	
	
clean:
	$(RM) *.o *.lst *.map *.out *.hex a.out m6242.s ff.s diskio.s cf.s main.s hd44780.s mc6840.s mos6551.s io.s parser.s
