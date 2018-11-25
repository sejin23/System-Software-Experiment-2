#-----------------------------------------------------------
#
# SWE2007: Software Experiment 2 (Fall 2017)
#
# Makefile for PA#1
#
# Sep 18, 2017.
# CSLab, Sungkyunkwan University
#
#-----------------------------------------------------------


CC		= gcc
CFLAGS 	= -g -O -Wall

CSRCS	= main.c db.c
TARGET	= wordcount
OBJECTS	= $(CSRCS:.c=.o)

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
