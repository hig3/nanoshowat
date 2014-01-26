#CFLAGS=-Wall -g
CFLAGS=

nsa: nsa.o
	cc -o nsa $^
