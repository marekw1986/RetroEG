; Delay subroutines

.export _delay_4us,_delay_ms

.segment  "CODE"

; ---------------------------------------------------------------------------
; Delay's based on 4MHz 65C02 CPU
;
; Delay 4us
; there's an additional 3us overhead for each call, so the
; minimum delay is 7us with increments of 4us. The formula is:
; duration = 4*a + 3, where "a" is the value in the accumulator
; note a value of zero in "a" is treated as 256, not zero!
; range is from 7us to 1027us

_delay_4us:      ; 6 jsr
   tax         ; 2
delay_4us_loop:
   pha         ; 3
   pla         ; 4
   nop         ; 2
   nop         ; 2
   dex         ; 2
   bne delay_4us_loop    ; 2/3
   rts         ; 6

; ---------------------------------------------------------------------------
; Delay ms
;
_delay_ms:
   tay
   lda #249                            ; 249*4 + 3 = 999 
delay_ms_loop1:   
   jsr _delay_4us
   dey
   bne delay_ms_loop1
   rts
