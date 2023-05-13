CC = gcc
all: bin/master bin/port bin/ship

CFLAGS = -g

INCLUDES = src/*.h

COMMON_DEPS = $(INCLUDES) Makefile

build/%.o: src/%.c $(COMMON_DEPS) 
	$(CC) $(CFLAGS) -c $< -o $@

bin/master: build/master.o build/configuration.o $(COMMON_DEPS)
	$(CC) -o bin/master build/configuration.o build/master.o -lm

bin/ship: build/ship.o build/configuration.o $(COMMON_DEPS)
	$(CC) -o bin/ship build/configuration.o build/ship.o -lm

bin/port: build/port.o build/configuration.o $(COMMON_DEPS)
	$(CC) -o bin/port build/configuration.o build/port.o -lm

clean: 
	rm -f build/* bin/*
