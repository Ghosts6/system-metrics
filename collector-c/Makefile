CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
LDFLAGS = -lcurl -lm

TARGET = collector
SOURCES = src/main.c src/metrics.c src/http_client.c src/logger.c src/ini_parser.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
