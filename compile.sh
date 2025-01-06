#/bin/bash
bear -- gcc src/*.c -I include -lncurses -lm -Wall -std=c99 -g -o game.o
