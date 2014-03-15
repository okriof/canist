    title	"canlistener"
    list        p=16f628A
    include     <p16f628A.inc>

;    __config	_MCLRE_ON & _PWRTE_ON & _WDT_OFF & _INTRC_OSC_NOCLKOUT & _BODEN_OFF & _LVP_ON

; LVP problem sol test    __config _MCLRE_OFF & _WDT_OFF & _PWRTE_ON & _BODEN_ON & _LVP_ON & _FOSC_HS
    __config _MCLRE_OFF & _WDT_OFF & _PWRTE_ON & _BODEN_OFF & _LVP_OFF & _FOSC_HS
; 2012-05-31 brown out reset on..


;=====================
; register definitions
;=====================

; 2012-05-20: Moved buffers to after variables. Check RX_BYTES_COUNT for overflow


TX_NEXTSENDp    equ     0x20    ; adress of next byte to send
TX_BYTESTOSEND  equ     0x21    ; bytes left to send

RX_BUFp         equ     0x22    ; pointer to where to receive data
RX_EQ_CHECK     equ     0x23    ; circular buffer to check for 5 equal bits i a row
RX_BIT_COUNTER  equ     0x24    ; count 8 bits
RX_BYTES_COUNT  equ     0x25    ; number of bytes received

BUFFER_SETUP    equ     0x26    ; buffer setup (0 = send from 1, receive to 2) (1 = opposite)

WAIT_COUNTER    equ     0x27    ; counter waiting for interframe pause.


TX_BUF1_PRE     equ     0x30    ; first tx buffer first byte
TX_BUF1_PRE2    equ     0x31    ; first tx buffer
TX_BUF1_LENBYTE equ     0x32    ; length
TX_BUF1_DATA    equ     0x33    ; first tx buffer data start
TX_BUF1_DATA_E  equ     0x4f    ; end of buffer (32 bytes)

TX_BUF2_PRE     equ     0x50    ; second tx buffer first byte
TX_BUF2_PRE2    equ     0x51    ; second tx buffer
TX_BUF2_LENBYTE equ     0x52    ; length
TX_BUF2_DATA    equ     0x53    ; second tx buffer data start
TX_BUF2_DATA_E  equ     0x6f    ; end of buffer (32 bytes)





;=======================================================================
;------------------------------==MACROS==-------------------------------
;=======================================================================

;------------------ level check macros, internal comparator ------------
;btlsa macro                         ; bit test level, skip if active
;    btfsc       CMCON,C2OUT
;    endm
;btlsr macro                         ; bit test level, skip if recessive
;    btfss       CMCON,C2OUT
;    endm

;------------------ level check macros, external comparator ------------
btlsa macro                         ; bit test level, skip if active
    btfsc       PORTA,RA1
    endm
btlsr macro                         ; bit test level, skip if recessive
    btfss       PORTA,RA1
    endm


;------------------------------- getbit -------------------------------
; get one bit and check for 6 equal bits, jumps to handle6equal if found.
; 16 cycles including timing check output, 4th cycle is the active cycle

getbit macro                    ; get one bit
    movf        RX_BUFp,w           ; point to buffer
    movwf       FSR

    bcf         STATUS,C            ; set carry to input level
    ;btfsc       CMCON,C2OUT
    btlsa
    bsf         STATUS,C
    bcf         PORTB,RB0           ; timing check output

    rlf         INDF,f              ; rotate carry into buffer
    rrf         INDF,w              ; rotate carry back to carry (dont touch buffer)
    rlf         RX_EQ_CHECK,f       ; rotate carry into equal bits check

    ; check for 5 equal bits
    movf        RX_EQ_CHECK,w       ; get eq check
    andlw       b'00111111'         ; mask
    btfsc       STATUS,Z            ; check zero flag
    goto        handleErrorFrame    ; if set, 6 zero bits have been seen

    xorlw       b'00111111'         ; toggle six bits
    btfsc       STATUS,Z            ; check zero flag
    goto        handleEndOfFrame    ; if set, 6 ones bits have been seen

    endm






;=======================================================================
;--------------------------------==MAIN==-------------------------------
;=======================================================================


	org 	0x0000			; RESET VECTOR
	goto	start

;	org 0x0004			; interrupt vector
;	goto	interruptHandler

start:
    ; status view
    bcf         STATUS,RP1
    bcf         STATUS,RP0
    bcf         PORTB,RB0
    bsf         STATUS,RP0
    bcf         TRISB,RB0
    bcf         STATUS,RP0
    bsf         PORTB,RB0


    ; setup uart for tx
    bcf         STATUS,RP1
    movlw       b'10010000'         ; enable serial port
    movwf       RCSTA

    bsf         STATUS,RP0          ; bank 1
    movlw       b'00100100'         ; 8 bits, TX enabled, Async, high baud rate
    movwf       TXSTA
    bsf         TRISB,TRISB2        ; TX pin, tristate on?

    clrf        SPBRG               ; full tx speed 1250kbaud

    bcf         STATUS,RP0          ; bank 0

    ; setup voltage reference module
    bsf         STATUS,RP0          ; bank 1
    ;movlw       b'11001101'         ; vref enabled, output on RA2, high range, 13 = 3,28 V
    ;movlw       b'11000100'         ; vref enabled, output on RA2, high range, 4 = 1,88 V
    movlw       b'00000000'         ; vref disabled, disconnected from RA2
    movwf       VRCON
    bcf         STATUS,RP0          ; bank 0

    ; setup comparator module
    bcf         STATUS,RP0          ; bank 0
    ;movlw       b'00110101'         ; comparator invert input (vref på in+) ; before differential amplifier
    ;movlw       b'00000101'         ; non-inverting input, comparator 1 off, comparator 2 on AN1- AN2+
    movlw       b'00000111'         ; comparators off, using RA1 for digital input, external comparator.
    movwf       CMCON



    ; setup sync bytes
    movlw       0xfa
    movwf       TX_BUF1_PRE
    movwf       TX_BUF2_PRE
    movlw       0x9f
    movwf       TX_BUF1_PRE2
    movwf       TX_BUF2_PRE2

    clrf        BUFFER_SETUP        ; using buffer setup zero

    movlw       TX_BUF1_PRE         ; set sending from buffer 1
    movwf       TX_NEXTSENDp
    clrf        TX_BYTESTOSEND      ; no bytes to send..

    movlw       TX_BUF2_DATA        ; set writing to buffer 2
    movwf       RX_BUFp





    



; TODO: sync
waitForInterframePause:
    ; check for corresponding 6 bits of recessive signal (ones),
    ; need only check a short bit into the last bit, marginal for
    ; detecting and syncing on the falling edge.
    ; 5*40+15 = 215 consecutive cycles of ones
    movlw       0x2b                    ; 43 (42*5+4 = 214 cycles of ones)
    movwf       WAIT_COUNTER
waitForInterframePause_waitLoop:        ; 5 cycles per loop iteration
    ;btfss       CMCON,C2OUT
    btlsr
    goto        waitForInterframePause  ; if input zero, start wait again...
    decfsz      WAIT_COUNTER,f
    goto        waitForInterframePause_waitLoop

; get frame:
waitForStartOfFrame:
    bcf         PORTB,RB0           ; timing check output
    ; RX_BUFp should already be set..

waitForStartOfFrameLoop:            ; wait for frame start edge (sync)
    ;btfsc       CMCON,C2OUT         ; wait for an1 to go low (high w diff-amp)
    btlsa
    goto        waitForStartOfFrameLoop
    ; now: have 18-4 = 14 cycles until first getbit
    bsf         PORTB,RB0           ; timing check output
    clrf        RX_BYTES_COUNT      ; no bytes received yet
    movlw       0xFF                ; set eq-check buf to 0xFF, (first (start) bit is zero)
    movwf       RX_EQ_CHECK
    ; in the begining of each frame, make sure 6 eq bits detect is not triggered
    ; first bit in each frame is a zero (dominant), setting RX_EQ_CHECK to 0xff
    ; should fix it

    ;call        tryTXbyte           ; uses 16 cycles, too long time, sending to computer is fast anyway...

    ; 8 = 18-6-4  (-4 for port read is 4 cycles into getbit)
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop


    ;  read byte
getbyteloop:
 
    ; read bit
    movlw       0x07
    movwf       RX_BIT_COUNTER      ; bit counter loop variable

getbitloop: ; -----------------------------------------------
    getbit                          ; get bit, 16 cycles
    call        tryTXbyte           ; transmit from previous buffer, 16 cycles
    bsf         PORTB,RB0           ; timing check output
    nop
    nop         ; nop x 40-16-16-4 = 4
    nop
    nop
    decfsz      RX_BIT_COUNTER,f    ; loop,
    goto        getbitloop
; end of getbitloop, RX_BIT_COUNTER == 0
    nop         ; for correct delay between getbits
    getbit                          ; get bit, 16 cycles
    call        tryTXbyte           ; transmit from previous buffer, 16 cycles

    ; TODO: remove tryTXbyte and add RX_BUFp overflow check instead.
    ; can be masked, if LSbits == 0 something is wrong..

    incf        RX_BUFp,f           ; receive byte to next byte in buffer
    incf        RX_BYTES_COUNT,f    ; one more byte received

    bsf         PORTB,RB0           ; todo: remove debug, nop instead
    nop        ;nop x 40-16-16-7 = 1
    goto        getbyteloop         ; next byte loop


; end of getbyte loop --------------------------------------------------
; ----------------------------------------------------------------------


    ; called after 6 recessive bits in a row
    ; jump out, switch buffers and wait for next start of frame
    ; no need for nopping, just do it quickly and get ready for the next frame start

handleEndOfFrame:                   ; switch buffers and stuff,
    movf        RX_BUFp,w
    movwf       FSR                 ; point to buffer
    movf        RX_BIT_COUNTER,f
    btfsc       STATUS,Z            ; if bit counter == 0, en byte just fylld
    goto        handleEndOfFrame_Continue   ; hoppa över skiftande

handleEndOfFrame_FillLastByte:      ; fill remainder of byte with ones
    bsf         STATUS,C
    rlf         INDF,f
    decfsz      RX_BIT_COUNTER,f
    goto        handleEndOfFrame_FillLastByte

handleEndOfFrame_Continue:
    ; TODO: check if TX_BYTESTOSEND is zero, else the old frame is not transmitted yet
    movf        RX_BYTES_COUNT,w
    addlw       0x4                 ; 2 sync bytes, 1 lenght byte + the last byte received
    movwf       TX_BYTESTOSEND      ; save in bytes to send.

    ; overflow check,  TX_BYTESTOSEND in w
    andlw       b'11100000'         ; 32 byte buffer, w should be 0 unless overflow
    btfss       STATUS,Z
    goto        start               ; if overflow, restart

; check buffer order
    movf        BUFFER_SETUP,f      ;
    btfsc       STATUS,Z
    goto        handleEndOfFrame_BufferSetupZero

; buffer setup was one, should now be zero (send from 1, receive to 2)
    clrf        BUFFER_SETUP        ; set buffer setup zero

    movf        TX_BYTESTOSEND,w
    movwf       TX_BUF1_LENBYTE     ; set sending buffer lenght byte
    movlw       TX_BUF1_PRE
    movwf       TX_NEXTSENDp      ; next byte to send point to buf2

    movlw       TX_BUF2_DATA
    movwf       RX_BUFp             ; next byte to receive point to data of buf1

    goto        waitForStartOfFrame

handleEndOfFrame_BufferSetupZero:
    ; buffer setup was zero, should now be one (send from 2, receive to 1)
    incf        BUFFER_SETUP,f      ; set buffer setup one

    movf        TX_BYTESTOSEND,w
    movwf       TX_BUF2_LENBYTE     ; set sending buffer lenght byte
    movlw       TX_BUF2_PRE
    movwf       TX_NEXTSENDp      ; next byte to send point to buf2

    movlw       TX_BUF1_DATA
    movwf       RX_BUFp             ; next byte to receive point to data of buf1

    goto        waitForStartOfFrame



; ---------------------------------------------------------------------------


    ; called after 6 dominant bits in a row (stuffing error or error frame)
handleErrorFrame:                   ; error frame (6 consecutive dominant bits, zeros)
        ; set rx buffer pointer back to beginning and wait for interframe pause.
    movlw       TX_BUF1_DATA        ; pointer to first buffer
    movf        BUFFER_SETUP,f      ; check buffer setup
    btfsc       STATUS,Z            ; if buffer setup 0, Z is set and buffer 2 should be written to
    movlw       TX_BUF2_DATA        ; load buf2 pointer instead
    movwf       RX_BUFp
;    goto        waitForInterframePause  ; resync to frames
    goto        start               ; test: reinitialize after frame error.





;=======================================================================
;--------------------------------==SUBS==-------------------------------
;=======================================================================
;   tryTXbyte
;   Sends byte from buffer starting at by TX_NEXTSENDp, multiple calls
;   sends TX_BYTESTOSEND bytes.
;   16 cycles to call, = 14 for routine, 2 for call

tryTXbyte:
    btfss       PIR1, TXIF
    goto        tryTXbyte_notReady      ; if not set, wait more..

    movf        TX_BYTESTOSEND,w
    btfsc       STATUS,Z                ; if no bytes, z is set, go to wait
    goto        tryTXbyte_nobytes

    movwf       TX_BYTESTOSEND          ; one less byte to send
    decf        TX_BYTESTOSEND,f

    movf        TX_NEXTSENDp,w          ; send next byte
    movwf       FSR
    movf        INDF,w
    movwf       TXREG

    incf        TX_NEXTSENDp,f          ; step pointer
    return      ; 14 cykler t.o.m return


tryTXbyte_notReady: ; 14-2-3 = 9 nops
    nop
    nop
    nop
tryTXbyte_nobytes:  ; 14-2-6 = 6 nops
    nop
    nop
    nop
    nop
    nop
    nop
    return






    end


