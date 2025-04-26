CC := gcc
CFLAGS := -Iinclude -Wall -Wno-pointer-sign -Wno-incompatible-pointer-types
LDFLAGS := -lasound -lfdk-aac -lopus -pthread

COMMON_SRCS := $(wildcard source/*.c)
COMMON_OBJS := $(COMMON_SRCS:.c=.o)

MAIN ?= app
MAIN_SRC := module/main_$(MAIN).c
MAIN_OBJ := $(MAIN_SRC:.c=.o)

OBJS := $(COMMON_OBJS) $(MAIN_OBJ)

TARGET = NovaGenerator

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

list:
	@ls module | grep '^main_.*\.c$$' | sed 's/^main_//; s/\.c//' | sort

clean:
	@rm -f $(OBJS) $(TARGET)
