SRC=$(wildcard src/*.c)
OBJS=$(addprefix obj/, $(SRC:.c=.o))

CC=gcc
CFLAGS=-std=gnu17 -ggdb -Wall -Werror
INC=
LIBS=-lm -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

CFLAGS+=$(FLAGS)
INC+=$(INCLUDE)
LIBS+=$(LIBRARIES)

all:
	mkdir -p obj/src/
	$(MAKE) raycast

raycast: $(OBJS)
	$(CC) $(CFLAGS) $(INC) $(OBJS) $(LIBS)

obj/src/%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) $(INC) -c $< -o $@ $(LIBS)

clean:
	-rm -rf obj a.out

