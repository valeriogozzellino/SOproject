CC = gcc
all: bin/master bin/port bin/ship bin/storm_duration bin/swell_duration bin/maelstorm bin/dump
prova: bin/file-prova


CFLAGS = -g #-std=c89 -Wpedantic

INCLUDES = src/*.h

COMMON_DEPS = $(INCLUDES) Makefile

build/%.o: src/%.c $(COMMON_DEPS) 
	$(CC) $(CFLAGS) -c $< -o $@

bin/master: build/master.o build/configuration.o $(COMMON_DEPS)
	$(CC) -o bin/master build/configuration.o build/master.o -lm

bin/ship: build/ship.o build/configuration.o build/headership.o $(COMMON_DEPS)
	$(CC) -o bin/ship build/configuration.o build/headership.o build/ship.o -lm

bin/port: build/port.o build/configuration.o build/headerport.o $(COMMON_DEPS)
	$(CC) -o bin/port build/configuration.o build/port.o build/headerport.o -lm

bin/storm_duration: build/storm_duration.o build/configuration.o $(COMMON_DEPS)
	$(CC) -o bin/storm_duration build/configuration.o build/storm_duration.o -lm

bin/maelstorm: build/maelstorm.o build/configuration.o $(COMMON_DEPS)
	$(CC) -o bin/maelstorm build/configuration.o build/maelstorm.o -lm

bin/swell_duration: build/swell_duration.o build/configuration.o $(COMMON_DEPS)
	$(CC) -o bin/swell_duration build/configuration.o build/swell_duration.o -lm

bin/dump: build/dump.o build/configuration.o $(COMMON_DEPS)
	$(CC) -o bin/dump build/configuration.o build/dump.o -lm

bin/file-prova: build/file-prova.o build/configuration.o build/headerport.o $(COMMON_DEPS)
	$(CC) -o bin/file-prova build/configuration.o build/file-prova.o build/headerport.o -lm 
clean: 
	rm -f build/* bin/*
