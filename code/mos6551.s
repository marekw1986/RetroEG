;
; File generated by cc65 v 2.17 - Git 535088fe
;
	.fopt		compiler,"cc65 v 2.17 - Git 535088fe"
	.setcpu		"65C02"
	.smart		on
	.autoimport	on
	.case		on
	.debuginfo	off
	.importzp	sp, sreg, regsave, regbank
	.importzp	tmp1, tmp2, tmp3, tmp4, ptr1, ptr2, ptr3, ptr4
	.macpack	longbranch
	.export		_mos6551_init
	.export		_mos6551_putc
	.export		_mos6551_puts
	.export		_mos6551_handle_rx
	.import		_parse_cmd
	.import		_port_set
	.import		_port_clr
	.export		_mos6551_RxChar
	.export		_mos6551_rxrb
	.export		_mos6551_rxrb_head
	.export		_mos6551_rxrb_tail
	.export		_mos6551_line
	.export		_mos6551_line_ind

.segment	"DATA"

_mos6551_rxrb_head:
	.byte	$00
_mos6551_rxrb_tail:
	.byte	$00
_mos6551_line_ind:
	.byte	$00

.segment	"BSS"

_mos6551_RxChar:
	.res	1,$00
_mos6551_rxrb:
	.res	256,$00
_mos6551_line:
	.res	256,$00

; ---------------------------------------------------------------
; void __near__ __fastcall__ mos6551_init (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_mos6551_init: near

.segment	"CODE"

	lda     #$FF
	sta     $6501
	lda     #$09
	sta     $6502
	lda     #$1E
	sta     $6503
	lda     #$20
	jmp     _port_clr

.endproc

; ---------------------------------------------------------------
; void __near__ __fastcall__ mos6551_putc (unsigned char)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_mos6551_putc: near

.segment	"CODE"

	jsr     pusha
L004B:	lda     $6501
	and     #$10
	beq     L004B
	lda     (sp)
	sta     $6500
	jmp     incsp1

.endproc

; ---------------------------------------------------------------
; void __near__ __fastcall__ mos6551_puts (__near__ const unsigned char *)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_mos6551_puts: near

.segment	"CODE"

	jsr     pushax
	lda     #$20
	jsr     _port_set
	bra     L0022
L0020:	jsr     ldax0sp
	sta     ptr1
	stx     ptr1+1
	lda     (ptr1)
	jsr     _mos6551_putc
	jsr     ldax0sp
	ina
	bne     L0027
	inx
L0027:	jsr     stax0sp
L0022:	jsr     ldax0sp
	sta     ptr1
	stx     ptr1+1
	lda     (ptr1)
	bne     L0020
	lda     #$20
	jsr     _port_clr
	jmp     incsp2

.endproc

; ---------------------------------------------------------------
; void __near__ __fastcall__ mos6551_handle_rx (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_mos6551_handle_rx: near

.segment	"CODE"

	bra     L004D
L002B:	ldy     _mos6551_rxrb_tail
	lda     _mos6551_rxrb,y
	sta     _mos6551_RxChar
	inc     _mos6551_rxrb_tail
	lda     _mos6551_RxChar
	beq     L004D
	cmp     #$0A
	beq     L003C
	cmp     #$0D
	beq     L004D
	bra     L0045
L003C:	ldy     _mos6551_line_ind
	lda     #$00
	sta     _mos6551_line,y
	stz     _mos6551_line_ind
	lda     #<(_mos6551_line)
	ldx     #>(_mos6551_line)
	jsr     _parse_cmd
	bra     L004D
L0045:	ldy     _mos6551_line_ind
	lda     _mos6551_RxChar
	sta     _mos6551_line,y
	inc     _mos6551_line_ind
L004D:	lda     _mos6551_rxrb_tail
	cmp     _mos6551_rxrb_head
	bne     L002B
	rts

.endproc

