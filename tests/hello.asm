; Sample Assembly source code for chip8emu

%define SCREEN_W 64
%define SCREEN_H 32
%define EMPTY_DEF

main:
    xor v0, v0      ; v0 = x position
    xor v1, v1      ; v1 = y position
    ld ve, 1        ; ve = 1 (const)
    ld i, bitmap    ; Sprite byte index

y_loop:
    xor v0, v0

x_loop:
    drw v0, v1, 1
    add i, ve
    add v0, 8

    se v0, SCREEN_W
    jp x_loop

    add v1, 1

    se v1, SCREEN_H
    jp y_loop

beep:
    ld st, ve

end:
    jp end

bitmap:
    db 0b11111111,0b11111111,0b11111111,0b11111111,0b11111111,0b11111111,0b11111111,0b11111111,
    db 0b10000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000001,
    db 0b10000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000001,
    db 0b10000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000001,
    db 0b10000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000001,
    db 0b10000010,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000001,
    db 0b10000010,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000001,
    db 0b10000110,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000001,
    db 0b10000100,0b00000000,0b00000000,0b00100000,0b00000000,0b00000000,0b00000000,0b00000001,
    db 0b10000100,0b00000000,0b00000000,0b01111111,0b00000000,0b00000000,0b00000000,0b00000001,
    db 0b10001000,0b00000000,0b00100000,0b01000000,0b00000110,0b00000000,0b00000111,0b11100001,
    db 0b10001000,0b00000000,0b01000000,0b01000000,0b00000010,0b00100000,0b00001100,0b00010001,
    db 0b10001000,0b00000000,0b01000000,0b01000000,0b00000010,0b00100000,0b00010000,0b00010001,
    db 0b10001000,0b00000000,0b01000000,0b01000000,0b00000010,0b00100000,0b00110000,0b00001101,
    db 0b10001000,0b00000000,0b10000000,0b01000000,0b00000010,0b00100000,0b00100000,0b00001101,
    db 0b10011111,0b11111111,0b10000000,0b01000000,0b00000010,0b00110000,0b00100000,0b00000101,
    db 0b10001000,0b00000001,0b11000000,0b01111110,0b00000010,0b00010000,0b01000000,0b00000101,
    db 0b10001000,0b00000000,0b10000000,0b01000000,0b00000010,0b00010000,0b01000000,0b00000101,
    db 0b10001000,0b00000000,0b10000000,0b01000000,0b00000010,0b00010000,0b01000000,0b00001001,
    db 0b10001000,0b00000000,0b10000000,0b01000000,0b00000010,0b00010000,0b01000000,0b00001001,
    db 0b10001000,0b00000001,0b00000000,0b01000000,0b00000010,0b00010000,0b01000000,0b00001001,
    db 0b10001000,0b00000001,0b00000000,0b01000000,0b00000010,0b00010000,0b01000000,0b00010001,
    db 0b10001000,0b00000001,0b00000000,0b01000000,0b00000010,0b00010000,0b00100000,0b00010001,
    db 0b10001000,0b00000001,0b00000000,0b01100000,0b00000010,0b00010000,0b00110000,0b00100001,
    db 0b10001000,0b00000001,0b00000000,0b01111111,0b10000010,0b00011000,0b00011000,0b11000001,
    db 0b10000000,0b00000000,0b00000000,0b00000000,0b00000001,0b00001000,0b00001111,0b10000001,
    db 0b10000000,0b00000000,0b00000000,0b00000000,0b00000001,0b00000000,0b00000000,0b00000001,
    db 0b10000000,0b00000000,0b00000000,0b00000000,0b00000001,0b00000000,0b00000000,0b00000001,
    db 0b10000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000001,
    db 0b10000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000001,
    db 0b10000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000001,
    db 0b11111111,0b11111111,0b11111111,0b11111111,0b11111111,0b11111111,0b11111111,0b11111111

