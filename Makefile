CFLAGS = -g -Wall
CC = gcc
INCLUDE_DIR = 

%.o: %.c
	$(CC) -o $@ -c -I$(INCLUDE_DIR) $(CFLAGS) $<

all: diskstat2

diskstat2: hashtable.o main.o readrecords.o statistic.o 
	$(CC) hashtable.o main.o readrecords.o statistic.o -o diskstat2 -I$(INCLUDE_DIR) $(CFLAGS)

clean:
	rm -rf *.o diskstat2
