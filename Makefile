CC = gcc
TARGET = fmradio
SRCS = main.c fmradio.c

.PHONY: all clean

all: $(TARGET)

$(TARGET):
	$(CC) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
