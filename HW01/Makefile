CC=gcc
CFLAGS=-Wall -Werror
BIN=app1 app2 app3

all: $(BIN)

app1:
	$(CC) -o $@ $@.c $(CFLAGS)

app2:
	$(CC) -o $@ $@.c $(CFLAGS)

app3:
	$(CC) -o $@ $@.c $(CFLAGS)

clean:
	rm -f $(BIN)