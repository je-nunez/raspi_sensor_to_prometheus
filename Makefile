
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


compile: rasppi_dht22_sampler.c
	$(CC) -c  rasppi_dht22_sampler.c   $(CFLAGS)
	$(CC) -c  common_dht_read.c   $(CFLAGS)
	$(CC) -c  Raspberry_Pi_2/pi_2_mmio.c   $(CFLAGS)
	$(CC) -c  Raspberry_Pi_2/pi_2_dht_read.c   $(CFLAGS)
	$(CC) rasppi_dht22_sampler.o  pi_2_dht_read.o  pi_2_mmio.o  common_dht_read.o  $(LIBFLAGS)  -o rasppi_dht22_sampler


.PHONY : clean


clean:
	-rm -f rasppi_dht22_sampler.o  pi_2_dht_read.o  common_dht_read.o  pi_2_mmio.o  rasppi_dht22_sampler

