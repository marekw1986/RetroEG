; ---------------------------------------------------------------------------
; interrupt.s
; ---------------------------------------------------------------------------
;
; Interrupt handler.
;
; Checks for a BRK instruction and returns from all valid interrupts.

.import   _init, _mos6551_rxrb, _mos6551_rxrb_head, _milliseconds, _uptime_value
.import   _update_geiger_pulses, _geiger_pulses, _tmp16, _geiger_ind
.export   _irq_int, _nmi_int

MC6840_STA	  = $6481
M6242_STA     = $640D
MC6840_TIMER1 = $6482
MC6840_TIMER3 = $6486
ACIA_RXD = $6500
ACIA_STS = $6501

.segment  "CODE"

.PC02                             ; Force 65C02 assembly mode

; ---------------------------------------------------------------------------
; Non-maskable interrupt (NMI) service routine

_nmi_int:  RTI                    ; Return from all NMI interrupts

; ---------------------------------------------------------------------------
; Maskable interrupt (IRQ) service routine

_irq_int:  PHX                    ; Save X register contents to stack
           TSX                    ; Transfer stack pointer to X
           PHA                    ; Save accumulator contents to stack
           INX                    ; Increment X so it points to the status
           INX                    ;   register value saved on the stack
           LDA $100,X             ; Load status register contents
           AND #$10               ; Isolate B status bit
           BNE break              ; If B = 1, BRK detected

; ---------------------------------------------------------------------------
; IRQ detected

irq_chk_acia_rx:
           LDA ACIA_STS
           AND #$80
           BEQ irq_chk_t3
           LDA ACIA_RXD
           LDX _mos6551_rxrb_head
           STA _mos6551_rxrb, X
           INC _mos6551_rxrb_head
irq_chk_t3:
		   LDA MC6840_STA	      ; Load MC6840 status register
           AND #$04               ; Check if T3 interrupt flag is set
           BEQ irq_chk_rtc        ; If flag is cleared, go to the next stage
           LDA MC6840_TIMER3      ; You must read T3 to clear interrupt flag
           LDA MC6840_TIMER3+1
           INC _milliseconds      ; Increment milliseconds variable
irq_chk_rtc:
           LDA M6242_STA	      ; Load RTC status register
           AND #04                ; Check if IRQ flag is set
           BEQ irq_ret            ; If not, this is not RTC interrupt, co continue
           LDA #$00               ; Otherwise clear flag
           STA M6242_STA		  ; We can do it here. We are still a second ahead next RTC int
           
           LDA MC6840_TIMER1	  ; Load value from TIMER1
		   STA _tmp16+1			  ; Then save it to tmp16
		   LDA MC6840_TIMER1+1
		   STA _tmp16
		   LDA #$FF				  ; Write 0xFFFF to the TIMER1
		   STA MC6840_TIMER1
		   STA MC6840_TIMER1+1
		   LDX _geiger_ind		  ; Load the value of index to the X
           SEC					  ; Set carry out for borrow puropose
		   ; We need to subtract tmp from 0xFFFF to get real anount of pulses, A is still loaded with 0xFF
           SBC _tmp16			  ; We subtract LSB first
           STA _geiger_pulses, X  ; Then we store the result, LSB first using X value as an index
           LDA #$FF
           SBC _tmp16+1
           STA _geiger_pulses+1, X
           INX					  ; Increment X
           INX					  ; Increment X
           CMP #$3C*2			  ; Check if A<=60*2
           BCC irq_chk_rtc_svind  ; If not, just save current, incremented value from X
           LDX #$00				  ; If yes, zero it out
irq_chk_rtc_svind:
           STX _geiger_ind
irq_chk_rtc_inc_uptime:           
           INC _uptime_value      ; And increment uptime variable
           BNE irq_ret
           INC _uptime_value+1
           BNE irq_ret
           INC _uptime_value+2
           BNE irq_ret
           INC _uptime_value+3
irq_ret:   PLA                    ; Restore accumulator contents
           PLX                    ; Restore X register contents
           RTI                    ; Return from all IRQ interrupts

; ---------------------------------------------------------------------------
; BRK detected, stop

break:     JMP _init              ; If BRK is detected, something very bad
                                   ; has happened, restart
