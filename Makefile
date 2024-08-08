CC = gcc
TARGET = fmradio
SRC = main.c fmradio.c
LDFLAGS = $(shell pkg-config --libs gtk4)
CFLAGS = $(shell pkg-config --cflags gtk4)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)
