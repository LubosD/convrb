CC=g++
CFLAGS=-O2

all: convrb

convrb: convrb.o util.o
	$(CC) $(CFLAGS) convrb.o util.o -o convrb

util.o: util.cpp
	$(CC) $(CFLAGS) -c util.cpp -o util.o

convrb.o: convrb.cpp
	$(CC) $(CFLAGS) -c convrb.cpp -o convrb.o

clean:
	rm -f convrb *.o


