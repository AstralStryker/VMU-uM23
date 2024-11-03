


ui_main:
	MOV   #$92, OCR			;fast
	MOV   #$7F, SP
	MOV   #0, STAD
	CLR1  PSW, RAMBK0					;values we're writing are in bank 0
	MOV   #$20, CODE_START_MI
	MOV   #$00, CODE_START_HI
	JMPF  core_start

	
	
ui_draw_char:				;char in A, row in B, col in C; VRMAD2's bottom bit set means invert char, else not, chars are 4 wide and 6 tall, clobbers R2, B, C, TRH, TRL, 
	
	SUB   #$20
							;get pointer to char in TRH:TRL
	PUSH  ACC
	ADD   #<font_4x6
	ST    TRL
	AND   #$00
	ADDC  #>font_4x6
	ST    TRH
	POP   ACC
	CLR1  PSW, CY
	ROLC
	ADD   TRL
	ST    TRL
	AND   #$00
	ADDC  TRH
	ST    TRH
							;convert B from "char row" to "pixel row", then get pointer to said row into XBNK: R2
	LD    B
	ROL
	ADD   B					;A = B * 3. each 2 rows of LCD are 16 bytes, mul by 16
	MOV   #$80, RR2
	MOV   #0, XBNK
	CALLF ror4
	CALLF adjRR2			;now carry is our bank number and A is the pointer in bank to the row
							;get pointer to byte of the char we'll be using
	LD    C
	ROR
	AND   #$7F
	ADD   RR2
	ST    RR2				;guaranteed to nor overflow
							;prepare to draw
	MOV   #3, B
char_two_rows:
	MOV   #3, ACC
	SUB   B
	LDC
	BN    VRMAD2, 0, no_invert
	XOR   #$FF
no_invert:
	ST    TMPBYTE0
	AND   #$F0
	MOV   #2, TMPBYTE2
	ST    TMPBYTE1
char_row:
	PUSH  OCR
	MOV   #$81, OCR
	LD    @R2
	BN    C, 0, char_not_second_1
	CALLF ror4
char_not_second_1:
	AND   #$0F
	OR    TMPBYTE1
	BN    C, 0, char_not_second_2
	CALLF ror4
char_not_second_2:
	ST    @R2
	POP   OCR
	MOV   #6, ACC
	CALLF adjRR2
	LD    TMPBYTE0
	CALLF ror4
	AND   #$F0
	ST    TMPBYTE1
	
	DEC   TMPBYTE2
	BP    TMPBYTE2, 0, char_row
	MOV   #4, ACC
	CALLF adjRR2
	DBNZ  B, char_two_rows
	
	RET

ror4:
	ROR
	ROR
	ROR
	ROR
	RET

adjRR2:
	BP    XBNK, 0, adjRR2_bank0
	CLR1  RR2, 7
adjRR2_bank0:
	ADD   RR2
	MOV   #0, XBNK
	BN    ACC, 7, adjRR2_bank1
	INC   XBNK
adjRR2_bank1:
	OR    #$80
	ST    RR2
	RET


font_4x6:
	;top nibble is higher row. LSB is left. 0x20..0x7f. missing char 0x7f
	;"tom thumb" font 
	.byte $00
	.byte $00
	.byte $00
	.byte $44
	.byte $40
	.byte $40
	.byte $AA
	.byte $00
	.byte $00
	.byte $AE
	.byte $AE
	.byte $A0
	.byte $6C
	.byte $6C
	.byte $40
	.byte $82
	.byte $48
	.byte $20
	.byte $CC
	.byte $EA
	.byte $60
	.byte $44
	.byte $00
	.byte $00
	.byte $24
	.byte $44
	.byte $20
	.byte $42
	.byte $22
	.byte $40
	.byte $A4
	.byte $A0
	.byte $00
	.byte $04
	.byte $E4
	.byte $00
	.byte $00
	.byte $04
	.byte $80
	.byte $00
	.byte $E0
	.byte $00
	.byte $00
	.byte $00
	.byte $40
	.byte $22
	.byte $48
	.byte $80
	.byte $6A
	.byte $AA
	.byte $C0
	.byte $4C
	.byte $44
	.byte $40
	.byte $C2
	.byte $48
	.byte $E0
	.byte $C2
	.byte $42
	.byte $C0
	.byte $AA
	.byte $E2
	.byte $20
	.byte $E8
	.byte $C2
	.byte $C0
	.byte $68
	.byte $EA
	.byte $E0
	.byte $E2
	.byte $48
	.byte $80
	.byte $EA
	.byte $EA
	.byte $E0
	.byte $EA
	.byte $E2
	.byte $C0
	.byte $04
	.byte $04
	.byte $00
	.byte $04
	.byte $04
	.byte $80
	.byte $24
	.byte $84
	.byte $20
	.byte $0E
	.byte $0E
	.byte $00
	.byte $84
	.byte $24
	.byte $80
	.byte $E2
	.byte $40
	.byte $40
	.byte $4A
	.byte $E8
	.byte $60
	.byte $4A
	.byte $EA
	.byte $A0
	.byte $CA
	.byte $CA
	.byte $C0
	.byte $68
	.byte $88
	.byte $60
	.byte $CA
	.byte $AA
	.byte $C0
	.byte $E8
	.byte $E8
	.byte $E0
	.byte $E8
	.byte $E8
	.byte $80
	.byte $68
	.byte $EA
	.byte $60
	.byte $AA
	.byte $EA
	.byte $A0
	.byte $E4
	.byte $44
	.byte $E0
	.byte $22
	.byte $2A
	.byte $40
	.byte $AA
	.byte $CA
	.byte $A0
	.byte $88
	.byte $88
	.byte $E0
	.byte $AE
	.byte $EA
	.byte $A0
	.byte $AE
	.byte $EE
	.byte $A0
	.byte $4A
	.byte $AA
	.byte $40
	.byte $CA
	.byte $C8
	.byte $80
	.byte $4A
	.byte $AE
	.byte $60
	.byte $CA
	.byte $EC
	.byte $A0
	.byte $68
	.byte $42
	.byte $C0
	.byte $E4
	.byte $44
	.byte $40
	.byte $AA
	.byte $AA
	.byte $60
	.byte $AA
	.byte $A4
	.byte $40
	.byte $AA
	.byte $EE
	.byte $A0
	.byte $AA
	.byte $4A
	.byte $A0
	.byte $AA
	.byte $44
	.byte $40
	.byte $E2
	.byte $48
	.byte $E0
	.byte $E8
	.byte $88
	.byte $E0
	.byte $08
	.byte $42
	.byte $00
	.byte $E2
	.byte $22
	.byte $E0
	.byte $4A
	.byte $00
	.byte $00
	.byte $00
	.byte $00
	.byte $E0
	.byte $84
	.byte $00
	.byte $00
	.byte $0C
	.byte $6A
	.byte $E0
	.byte $8C
	.byte $AA
	.byte $C0
	.byte $06
	.byte $88
	.byte $60
	.byte $26
	.byte $AA
	.byte $60
	.byte $06
	.byte $AC
	.byte $60
	.byte $24
	.byte $E4
	.byte $40
	.byte $06
	.byte $AE
	.byte $24
	.byte $8C
	.byte $AA
	.byte $A0
	.byte $40
	.byte $44
	.byte $40
	.byte $20
	.byte $22
	.byte $A4
	.byte $8A
	.byte $CC
	.byte $A0
	.byte $C4
	.byte $44
	.byte $E0
	.byte $0E
	.byte $EE
	.byte $A0
	.byte $0C
	.byte $AA
	.byte $A0
	.byte $04
	.byte $AA
	.byte $40
	.byte $0C
	.byte $AA
	.byte $C8
	.byte $06
	.byte $AA
	.byte $62
	.byte $06
	.byte $88
	.byte $80
	.byte $06
	.byte $C6
	.byte $C0
	.byte $4E
	.byte $44
	.byte $60
	.byte $0A
	.byte $AA
	.byte $60
	.byte $0A
	.byte $AE
	.byte $40
	.byte $0A
	.byte $EE
	.byte $E0
	.byte $0A
	.byte $44
	.byte $A0
	.byte $0A
	.byte $A6
	.byte $24
	.byte $0E
	.byte $6C
	.byte $E0
	.byte $64
	.byte $84
	.byte $60
	.byte $44
	.byte $04
	.byte $40
	.byte $C4
	.byte $24
	.byte $C0
	.byte $6C
	.byte $00
	.byte $00
	.byte $EE
	.byte $EE
	.byte $E0
	;up arrow - 0x80
	.byte $4E
	.byte $44
	.byte $44
	;down arrow - 0x81
	.byte $44
	.byte $44
	.byte $E4
	
