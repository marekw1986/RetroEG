; ---------------------------------------------------------------------------
; interrupt.s
; ---------------------------------------------------------------------------
;
; Interrupt handler.
;
; Checks for a BRK instruction and returns from all valid interrupts.

.import   _init, _mos6551_rxrb, _mos6551_rxrb_head, _milliseconds, _uptime_value
.import   _update_geiger_pulses
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
           LDA M6242_STA	       ; Load RTC status register
           AND #04                ; Check if IRQ flag is set
           BEQ irq_ret            ; If not, this is not RTC interrupt, co continue
           LDA #$00               ; Otherwise clear flag
           STA M6242_STA
           JSR _update_geiger_pulses ; goto the subroutine updating giger pulses
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
