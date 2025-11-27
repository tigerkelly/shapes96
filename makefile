CFLAGS=-c -Wall -O2
LIBS = -lm -lpthread

all: libshapes96.a sample

libshapes96.a: fonts.o shapes96.o
	ar -rc libshapes96.a shapes96.o fonts.o
	sudo cp libshapes96.a /usr/local/lib
	sudo cp shapes96.h /usr/local/include

sample: sample.o
	$(CC) -o sample sample.c -lshapes96 -lpthread -lm

sample.o: sample.c
	$(CC) $(CFLAGS) sample.c

shapes96.o: shapes96.c
	$(CC) $(CFLAGS) shapes96.c

fonts.o: fonts.c
	$(CC) $(CFLAGS) fonts.c

clean:
	rm -rf *.o libshapes96.a sample
