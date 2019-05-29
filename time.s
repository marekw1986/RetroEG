;
; File generated by cc65 v 2.17 - Git 535088f
;
	.fopt		compiler,"cc65 v 2.17 - Git 535088f"
	.setcpu		"6502"
	.smart		on
	.autoimport	on
	.case		on
	.debuginfo	off
	.importzp	sp, sreg, regsave, regbank
	.importzp	tmp1, tmp2, tmp3, tmp4, ptr1, ptr2, ptr3, ptr4
	.macpack	longbranch
	.export		_millis
	.export		_uptime
	.export		_get_fattime
	.export		_milliseconds
	.export		_uptime_value

.segment	"DATA"

_milliseconds:
	.dword	$00000000
_uptime_value:
	.dword	$00000000

; ---------------------------------------------------------------
; unsigned long __near__ millis (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_millis: near

.segment	"CODE"

	lda     _milliseconds+3
	sta     sreg+1
	lda     _milliseconds+2
	sta     sreg
	ldx     _milliseconds+1
	lda     _milliseconds
	rts

.endproc

; ---------------------------------------------------------------
; unsigned long __near__ uptime (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_uptime: near

.segment	"CODE"

	lda     _uptime_value+3
	sta     sreg+1
	lda     _uptime_value+2
	sta     sreg
	ldx     _uptime_value+1
	lda     _uptime_value
	rts

.endproc

; ---------------------------------------------------------------
; unsigned long __near__ get_fattime (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_get_fattime: near

.segment	"CODE"

	ldx     #$00
	stx     sreg
	stx     sreg+1
	txa
	rts

.endproc
