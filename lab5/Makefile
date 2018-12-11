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

CSRCS1	= server.c db.c
CSRCS2	= client.c
TARGET1	= server
TARGET2	= client
OBJECTS1	= $(CSRCS1:.c=.o)
OBJECTS2	= $(CSRCS2:.c=.o)

all : $(TARGET1) $(TARGET2)

$(TARGET1) : $(OBJECTS1)
	$(CC) $(OBJECTS1) -o $@ -pthread

$(TARGET2) : $(OBJECTS2)
	$(CC) $(OBJECTS2) -o $@ -pthread

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS1) $(TARGET1) $(OBJECTS2) $(TARGET2)
