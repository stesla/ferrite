PROGRAM=fe

CC=gcc

OBJS=main.o gc.o eval.o read.o error.o vm.o object.o

CFLAGS=-g -Wall

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -rf *.o $(PROGRAM)

test: $(PROGRAM)
	prove *.test