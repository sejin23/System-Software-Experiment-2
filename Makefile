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
TARGET	= swsh
OBJECTS	= $(CSRCS:.c=.o)

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
