CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11
LDFLAGS = -lglfw -lGLEW -lGL -lm

SRC = main.c game.c render.c menu.c highscores.c
OBJ = $(SRC:.c=.o)
BIN = pacman

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)