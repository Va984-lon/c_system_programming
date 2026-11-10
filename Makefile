	CC = gcc
CFLAGS = -pthread -Wall

all: simulator processor

simulator: src/simulator.c src/common.c src/logger.c src/data_loader.c
	$(CC) $(CFLAGS) src/simulator.c src/common.c src/logger.c src/data_loader.c -o simulator.exe

processor: src/main.c src/common.c src/logger.c src/data_loader.c src/report.c
	$(CC) $(CFLAGS) src/main.c src/common.c src/logger.c src/data_loader.c src/report.c -o processor.exe