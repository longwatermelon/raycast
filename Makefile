SRC=$(wildcard src/*.c)
OBJS=$(addprefix obj/, $(SRC:.c=.o))

CC=gcc
CFLAGS=-std=gnu17 -ggdb\
	   # Uncomment for cheats
	   # -DCHEATS_ON\
	   # Uncomment to randomize map
	   # -DRANDOMIZE_MAP
LIBS=-lm -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

all:
	mkdir -p obj/src/
	$(MAKE) raycast

raycast: $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJS) -o $@

obj/src/%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@

clean:
	rm -rf obj/*

