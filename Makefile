CC = gcc
TARGET = mtk-fmradio
SRC = main.c fmradio.c
LDFLAGS = `pkg-config --libs gtk4`
CFLAGS = `pkg-config --cflags gtk4`

PREFIX ?= /usr

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)

install:
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/
