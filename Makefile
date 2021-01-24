
TARGET=show_school_data

SOURCES=$(wildcard *.c)
HEADERS=$(wildcard *.h)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

CFLAGS=-O2 -Wall
LIBS=-ljson-c

all: $(TARGET)

clean:
	-rm $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	gcc -o $@  $^  $(LIBS)

$(OBJECTS): %.o: %.c $(HEADERS)
	gcc -c $(CFLAGS) $< -o $@ 
