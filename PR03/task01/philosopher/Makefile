CC=gcc
CFLAGS=-Wall -Werror -g
LDFLAGS=-lpthread
BIN=dining_philosopher

all: $(BIN)

$(BIN):
	$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(BIN)
