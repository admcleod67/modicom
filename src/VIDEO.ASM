;---------------------------------------------------------------
; Video support routines for ModiCom editor
; Copyright (c) 1994-2026 Allan McLeod
;---------------------------------------------------------------


	IDEAL
	DOSSEG
	MODEL small, c


MonoBASE        EQU     0b000h          ; Monochrome RAM segment address
ColourBASE      EQU     0b800h          ; Colour RAM segment address


	DATASEG

VideoBASE       DW      00000h          ; Video RAM segment address

	CODESEG

	PUBLIC  showline, showpage


;---------------------------------------------------------------
; VideoInit	Initialise VIDEO module
;---------------------------------------------------------------
; Input:
;       none
; Output:
;	es = video segment address
;       VideoBASE initialised
; Registers:
;       ax, es
;---------------------------------------------------------------

PROC    VideoInit

	push    bx
	mov     ah, 15			; BIOS function number
	int     10h			; Get video mode in al
	cmp     al, 7                   ; Is mode monochrome?
	jne     @@10                    ; If no, jump
	mov     ax, MonoBASE            ; Set monochrome segment
	jmp	@@20
@@10:
	mov	ax, ColourBASE          ; Set colour segment
@@20:
	mov	[VideoBASE], ax         ; Store segment to VideoBASE
	mov	es, ax                  ; and assign it to es
	pop     bx
	ret

ENDP    VideoInit


;---------------------------------------------------------------
; SetVidAddr    Calculate video RAM address
;---------------------------------------------------------------
; Input:
;	none
; Output:
;       es:di = video RAM buffer address for (row, column)
;       Note: dh and dl are not checked!!
; Registers:
;       ax, bx, dx, di, es changed
;---------------------------------------------------------------

PROC    SetVidAddr

	mov     es, [VideoBASE]         ; Retrieve video segment
	mov	ax, es                  ; Copy it to ax
	or	ax, ax                  ; Is it zero
	jnz	@@10                    ; If not, jump
	call	VideoInit               ; Otherwise call VideoInit
@@10:
	mov     ah, 15          	; Get display page in bh
	int     10h             	; Call BIOS video service
	mov     ah, 3           	; Get cursor position in dx
	int     10h             	; Call BIOS video service
	mov     al, dh                  ; Get row number
	mov     bl, 80                  ; Multiply by 80 line length
	mul     bl                      ; ax = row * 80
	add     al, dl                  ; Add column
	adc     ah, 0                   ; ax = row * 80 + column
	shl     ax, 1                   ; Convert to byte offset
	mov     di, ax                  ; Assign offset to di
	ret

ENDP    SetVidAddr


;---------------------------------------------------------------
; ScPokeLine	Poke a line of text into video RAM
;---------------------------------------------------------------
; Input:
;	cx = maximum line length of text
;       ds:si = address of ASCIIZ string in text buffer
;	ds:bx = address of physical text buffer end
;	es:di = address of cursor in video RAM
; Output:
;	ds:si = address of next string in text buffer
;	es:di = address of next screen character
;	Line of text output to video RAM
; Registers:
;       ax, cx, dx, di, si changed
;---------------------------------------------------------------

PROC    ScPokeLine

	mov     ah, 07h			; Assign attribute to ah
	cld
	jcxz	@@25			; No room: peek, then marker if needed
@@10:
	lodsb                           ; Get next character
	or	al, al                  ; NULL terminator?
	jnz	@@20
	mov	al, '<'
	jmp	@@50
@@20:
	cmp	al, 0dh
	je	@@10
	cmp	al, 8dh
	je	@@40
	stosw                           ; Otherwise display it
	loop	@@10                    ; Until max line length
@@25:
	lodsb				; Peek next byte, skipping CRs
	cmp	al, 0dh
	je	@@25
	dec	si			; Unread it (DF clear)
	or	al, al
	jz	@@10			; Exact end: normal terminator
	cmp	al, 8dh
	je	@@10			; Soft break: exact end of this row
	mov	al, '+'                 ; Displayable byte hidden: marker
	stosw                           ; and display it
	mov	cx, 0ffffh              ; Set max search length
@@30:
	lodsb                           ; Get next character
	or	al, al                  ; NULL terminator?
	jz      @@70                    ; If found, jump
	loop	@@30                    ; Otherwise repeat
	jmp	@@70
@@40:
	inc	si
	mov	al, ' '
@@50:
	push	ax
	mov	al, ' '                 ; Set al to space
	rep stosw                       ; Clear to eol
	pop	ax
	cmp     si, bx                  ; End of buffer?
	jna	@@60
	mov	al, ' '                 ; Set al to space
@@60:
	stosw                           ; and display it
@@70:
	ret

ENDP    ScPokeLine


;---------------------------------------------------------------
; ScPokePage	Poke a page of text into video RAM
;---------------------------------------------------------------
; Input:
;       dh = row (0 is top line)
;       dl = column (0 is at far left)
;       ds:si = address of ASCIIZ string in text buffer
;	ds:bx = address of physical text buffer end
;	es:di = address of cursor in video RAM
; Output:
;	Page of text output to video RAM
; Registers:
;       ax, cx, dx, di, si changed
;---------------------------------------------------------------

PROC	ScPokePage

	mov	cx, 79                  ; Set maximum line length
	sub	cl, dl                  ; Subtract column offset
@@10:
	call	ScPokeLine              ; Display current line
	cmp	dh, 24                  ; Bottom row already drawn?
	jae	@@30                    ; Done (do not draw/clear below)
	cmp	si, bx                  ; End of buffer?
	ja	@@20                    ; If so, clear the rest
	inc	dh                      ; Otherwise, move to
	mov	cx, 79                  ; Set maximum line length
	jmp	@@10                    ; next line and repeat
@@20:
	cmp	dh, 24                  ; Cleared through bottom row?
	jae	@@30                    ; If so, done
	inc	dh                      ; Next row to clear (di already there)
	mov	cx, 80                  ; Set cx to line length
	mov	al, 32                  ; Set al to space
	rep stosw			; Clear that row
	jmp	@@20
@@30:
	ret

ENDP	ScPokePage


;---------------------------------------------------------------
; showline	C interface routine for ScPokeLine
;---------------------------------------------------------------
; Prototype:
; 	void showline(char *string, char *endptr)
; Input:
;	string = pointer to ASCIIZ string in text buffer
;	endptr = pointer to last physical character in buffer
; Output:
;       Line of text output to video RAM
;---------------------------------------------------------------

PROC    showline

	ARG	string: PTR BYTE, endptr: PTR BYTE
	USES	di, es, si

	mov	si, [string]            ; Get string address
	call    SetVidAddr              ; Calculate cursor position
	mov	bx, [endptr]            ; Get end of buffer
	mov	cx, 79                  ; Set maximum line length
	sub	cl, dl                  ; Subtract column offset
	call	ScPokeLine              ; Display line of text
	ret

ENDP	showline


;---------------------------------------------------------------
; showpage	C interface routine for ScPokePage
;---------------------------------------------------------------
; Prototype:
; 	void showpage(char *string, char *endptr)
; Input:
;	string = pointer to ASCIIZ string in text buffer
;	endptr = pointer to last physical character in buffer
; Output:
;       Page of text output to video RAM
;---------------------------------------------------------------

PROC    showpage

	ARG	string: PTR BYTE, endptr: PTR BYTE
	USES	di, es, si

	mov	si, [string]            ; Get string address
	call    SetVidAddr              ; Calculate cursor position
	mov	bx, [endptr]            ; Get end of buffer
	call	ScPokePage              ; Display page of text
	ret

ENDP	showpage


	END                             ; End of module
