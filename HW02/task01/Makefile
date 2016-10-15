CC=gcc
CFLAGS=-Wall -Werror -g
LDFLAGS=-lpthread
BIN=proc_con

all: $(BIN)

$(BIN):
	$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(BIN)
