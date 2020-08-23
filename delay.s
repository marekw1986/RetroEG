; Delay subroutines

.export _delay_16us,_delay_ms

.segment  "CODE"

; ---------------------------------------------------------------------------
; Delay's based on 2MHz 65C02 CPU
;
; Delay 16us
; there's an additional 12us overhead for each call, so the
; minimum delay is 28us with increments of 16us. The formula is:
; duration = 16*a + 12, where "a" is the value in the accumulator
; note a value of zero in "a" is treated as 256, not zero!
; range is from 28us to 4108us

_delay_16us:      ; 6 jsr
   tax         ; 2
delay_16us_loop:
   pha         ; 3
   pla         ; 4
   nop         ; 2
   nop         ; 2
   dex         ; 2
   bne delay_16us_loop    ; 2/3
   rts         ; 6

; ---------------------------------------------------------------------------
; Delay ms
;
_delay_ms:
   tay
   lda #62                            ; 62*16 + 12 = 1004 
delay_ms_loop1:   
   jsr _delay_16us
   dey
   bne delay_ms_loop1
   rts
