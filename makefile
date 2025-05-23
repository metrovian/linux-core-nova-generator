NAME := NovaGenerator

CC := gcc
CFLAGS := -Iinclude -Wall -Wno-pointer-sign -Wno-incompatible-pointer-types
LDFLAGS := -lasound -lfdk-aac -lopus -lcurl -lmicrohttpd -lrdkafka -lzookeeper_mt -lm -pthread

COMMON_SRCS := $(wildcard source/*.c)
COMMON_OBJS := $(COMMON_SRCS:.c=.o)

MODULE_SRCS := $(wildcard module/*.c)
MODULE_OBJS := $(MODULE_SRCS:.c=.o)

MODULE := $(MODULE_SRCS:module/main_%.c=%)

default: $(MODULE)

$(MODULE): %: $(COMMON_OBJS) module/main_%.o
	@$(CC) $(COMMON_OBJS) module/main_$*.o -o $@ $(LDFLAGS)
	@mkdir -pv bin/
	@mv $@ bin/
	@echo '$@ build success'

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo '$@ compile success'

list:
	@ls module | grep '^main_.*\.c$$' | sed 's/^main_//; s/\.c//' | sort

clean:
	@rm -fv $(COMMON_OBJS) $(MODULE_OBJS)
	@rm -fv $(NAME)
