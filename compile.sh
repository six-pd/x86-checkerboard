#!/bin/bash
cc -m32 -std=c99 -fpack-struct=1 -c ccomp.c
nasm -f elf32 acomp.s
cc -m32 -o checkerboard.out ccomp.o acomp.o
