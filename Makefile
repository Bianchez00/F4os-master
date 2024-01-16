SERVER=bin/F4Server
CLIENT=bin/F4Client
CFLAGS=-Wall -std=c99
INCLUDES=-I./lib

SRCS_SERVER=src/errExit.c src/F4Server.c src/semaphore.c
SRCS_CLIENT=src/errExit.c src/F4Client.c src/semaphore.c

all: $(SERVER) $(CLIENT)

server: $(SERVER)

client: $(CLIENT)

$(SERVER): $(SRCS_SERVER)
	@echo "Compiling F4Server..."
	gcc $(CFLAGS) $(INCLUDES) $(SRCS_SERVER) -o $(SERVER)
	@echo "Done."

$(CLIENT): $(SRCS_CLIENT)
	@echo "Compiling F4Client..."
	gcc $(CFLAGS) $(INCLUDES) $(SRCS_CLIENT) -o $(CLIENT)
	@echo "Done."

.PHONY: clean

clean:
	@echo "Removing executable file(s)..."
	@rm -f bin/*
	@echo "Done."

