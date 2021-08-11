; Test assembly file for chip8emu
; Test comment

%define VAL_TO_LOAD 73
%define MY_MACRO

;%foobar

ld v1, 0xff
LD v2, 13

ld v8, VAL_TO_LOAD

cls

function:
    cls
    ret

label1:
    call function ; Test comment

label2:
    ld i, 0x8
    ld v8, 0x12
    ld v3, 0b11
    ld v2, 'a'

data:
    db 0xde 0xad 0xbe 0xef ; Store some data in the binary
    db 0b10011011, 1
    db
    db 'f','o','o','b','a','r','\0'
    dw 'f','o','o','b','a','r','\0'
    db '\n'
    db '\\'
    db ' '
    db '"'
    db '\''
    db "Hello world",0

