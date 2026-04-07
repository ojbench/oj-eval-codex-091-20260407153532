CC=g++-13
CXXFLAGS=-std=gnu++17 -O3 -pipe -march=native -mtune=native -fno-omit-frame-pointer
LDFLAGS=

SRC=src/main.cpp
BIN=code

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(BIN) *.o

