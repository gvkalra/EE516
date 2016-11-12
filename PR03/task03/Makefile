CC=gcc
CFLAGS=-Wall -Werror -g
LDFLAGS=-lpthread
BIN=semaphore_test

all: $(BIN)

$(BIN):
	$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(BIN)
