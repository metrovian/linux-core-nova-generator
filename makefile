CC := gcc
CFLAGS := -Iinclude -Wall -Wno-pointer-sign -Wno-incompatible-pointer-types
LDFLAGS := -lasound -lfdk-aac -lopus -lcurl -lmicrohttpd -lrdkafka -lzookeeper_mt -lspdlog -lfmt -lm -pthread

CXX := g++
CXXFLAGS := -Iinclude -Wall

WRAPPER_SRCS := $(wildcard source/*.cpp)
WRAPPER_OBJS := $(WRAPPER_SRCS:.cpp=.o)

COMMON_SRCS := $(wildcard source/*.c)
COMMON_OBJS := $(COMMON_SRCS:.c=.o)

MODULE_SRCS := $(wildcard module/*.c)
MODULE_OBJS := $(MODULE_SRCS:.c=.o)

MODULE := $(MODULE_SRCS:module/main_%.c=%)

default: $(MODULE)

$(MODULE): %: $(WRAPPER_OBJS) $(COMMON_OBJS) module/main_%.o
	@$(CXX) $(WRAPPER_OBJS) $(COMMON_OBJS) module/main_$*.o -o $@ $(LDFLAGS)
	@mkdir -pv bin/
	@mv $@ bin/
	@echo '$@ build success'

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo '$@ compile success'

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "$@ compile success"

list:
	@ls module | grep '^main_.*\.c$$' | sed 's/^main_//; s/\.c//' | sort

clean:
	@rm -fv $(WRAPPER_OBJS) $(COMMON_OBJS) $(MODULE_OBJS)
	@rm -fv bin/*
