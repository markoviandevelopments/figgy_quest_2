# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic
LDFLAGS = -lraylib -lm -pthread

# Source files
SERVER_SRC = server.c
CLIENT_SRC = client.c draw_chessboard.c
COMMON_HEADER = game.h

# Output binaries
SERVER_BIN = server
CLIENT_BIN = client

# Default target
all: $(SERVER_BIN) $(CLIENT_BIN)

# Compile the server
$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile the client
$(CLIENT_BIN): $(CLIENT_SRC) $(COMMON_HEADER)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Clean up compiled files
clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)

# Phony targets
.PHONY: all clean
