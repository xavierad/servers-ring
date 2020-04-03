CC = gcc
CFLAGS = -Wall -g

dkt: main.c checks.c server.c
	$(CC) $(CFLAGS) -o dkt main.c checks.c server.c
