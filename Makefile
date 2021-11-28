SRC=$(wildcard src/*.c)
OBJS=$(addprefix obj/, $(SRC:.c=.o))

CC=gcc
CFLAGS=-std=gnu17 -ggdb -Wall -Werror -DCHEATS_ON
INC=
LIBS=-lm -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

all:
	mkdir -p obj/src/
	$(MAKE) raycast

raycast: $(OBJS)
	$(CC) $(CFLAGS) $(INC) $(LIBS) $(OBJS) -o $@

obj/src/%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) $(INC) $(LIBS) -c $< -o $@

clean:
	rm -rf obj
	rm raycast

