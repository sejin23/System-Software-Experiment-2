#-----------------------------------------------------------
#
# SSE2033: System Software Experiment 2 (Fall 2018)
#
# Makefile for PA#3
#
# Nov 1, 2018.
# CSLab, Sungkyunkwan University
#
#-----------------------------------------------------------


CC		= gcc
CFLAGS 	= -g -O -Wall

CSRCS	= swsh.c
TARGET	= swsh.o head.o cat.o cp.o tail.o rm.o mv.o pwd.o
OBJECTS	= $(CSRCS:.c=.o)

all : $(TARGET)

head.o:
	$(CC) -o head head.c
cat.o:
	$(CC) -o cat cat.c
cp.o:
	$(CC) -o cp cp.c
tail.o:
	$(CC) -o tail tail.c
rm.o:
	$(CC) -o rm rm.c
mv.o:
	$(CC) -o mv mv.c
pwd.o:
	$(CC) -o pwd pwd.c
swsh.o:
	$(CC) -o swsh swsh.c
clean:
	rm -f $(OBJECTS) $(TARGET)
