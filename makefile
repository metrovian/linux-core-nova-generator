CC := gcc
CFLAGS := -Iinclude -Wall -Wno-pointer-sign -Wno-incompatible-pointer-types
LDFLAGS = -lasound -lfdk-aac -lopus -pthread

SRC := $(wildcard source/*.c)
OBJ := $(SRC:.c=.o)

TARGET = NovaGenerator

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
