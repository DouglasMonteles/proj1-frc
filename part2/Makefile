# Executavel
BINFOLDER := bin/
# .h
INCFOLDER := inc/
# .c
SRCFOLDER := src/
# .o
# OBJFOLDER := obj/

CC := gcc
CFLAGS := -std=c99
LINKFLAGS := -lrt -lpthread
SRCFILES := $(wildcard src/*.c)

all: $(SRCFILES:src/%.c=obj/%.o)
	$(CC) $(CFLAGS) obj/*.o -o bin/projeto1_frc $(LINKFLAGS)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I./inc $(LINKFLAGS)

run-s: bin/projeto1_frc
	bin/projeto1_frc sender

run-r: bin/projeto1_frc
	bin/projeto1_frc sender

.PHONY: clean
clean:
	rm -rf obj/*
	rm -rf bin/*
	touch obj/.gitkeep
	touch bin/.gitkeep