CC=gcc
CFLAGS=-Wall -Werror -g
LDFLAGS=-lpthread
BIN=training_monkey

all: $(BIN)

$(BIN):
	$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(BIN)
