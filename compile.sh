#/bin/bash
bear -- gcc src/*.c -I include -lncurses -lm -Wall -std=c99 -g -march=native -fopenmp -O3 -o game.o
