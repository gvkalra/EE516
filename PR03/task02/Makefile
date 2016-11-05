CC=gcc
CFLAGS=-Wall -Werror
BIN=syscall_test

all: $(BIN)

$(BIN):
	$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(BIN)
