CC := gcc
CFLAGS := -Iinclude -Wall

SRC := $(wildcard source/*.c)
OBJ := $(SRC:.c=.o)

TARGET = NovaGenerator

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
