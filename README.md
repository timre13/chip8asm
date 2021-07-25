# chip8asm
A modern CHIP-8 assembler written in C++.

## Dependencies:
* A C++ compiler supporting C++17 (gcc, clang)
* CMake
* Make

## Building
~~~sh
mkdir build
cd build
cmake ..
make
~~~

## Running
To assemble the file `source.asm` to `test.ch8` run `./chip8asm source.asm -o test.ch8`.
Use `./chip8asm -h` to get help.

