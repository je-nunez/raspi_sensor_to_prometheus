
CC = gcc
CFLAGS = -g -fpic -Wall -I . -I Raspberry_Pi_2/ 
LIBFLAGS =-lrt -L.


.SILENT:  help


help:
	echo "Makefile help"	
	echo -e "Possible make targets:\n"	
	echo "    make compile"	
	echo -e "         Compile the program.\n"	
	echo "    make clean"	
	echo -e "         Remove compiled and binary-object files.\n"	


compile: prg.c
	$(CC) -c  prg.c   $(CFLAGS)
	$(CC) -c  common_dht_read.c   $(CFLAGS)
	$(CC) -c  Raspberry_Pi_2/pi_2_mmio.c   $(CFLAGS)
	$(CC) -c  Raspberry_Pi_2/pi_2_dht_read.c   $(CFLAGS)
	$(CC) prg.o  pi_2_dht_read.o  pi_2_mmio.o  common_dht_read.o  $(LIBFLAGS)  -o prg


.PHONY : clean


clean:
	-rm -f prg.o  pi_2_dht_read.o  common_dht_read.o  pi_2_mmio.o

