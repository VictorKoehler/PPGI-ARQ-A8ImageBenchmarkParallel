SHELL := /bin/bash
CC := mkdir -p output/bin && g++
CCO := -lm -Wall -lpthread -std=c++17
SRC := $(shell find src/ -type f -name '*.cpp')
OUTBIN := output/bin/conv

all:
	$(CC) $(CCO) -DNDEBUG -O0 $(SRC) -o $(OUTBIN)

optimize:
	$(CC) $(CCO) -DNDEBUG -O5 $(SRC) -o $(OUTBIN)

optdebug:
	$(CC) $(CCO) -DNDEBUG -O5 -g3 $(SRC) -o $(OUTBIN)

debug:
	$(CC) $(CCO) -O0 -g3 $(SRC) -o $(OUTBIN)

clean:
	rm -rf bin/
