.PHONY: all clean

CC := cc
CFLAGS := -Wextra -Wall -g -lgc $(shell pkg-config fuse --cflags --libs)

TARGET = kfs
SRCS = \
	$(shell find ./ -maxdepth 1 -name "*.c") \
	$(shell find ./sds -name "*.c")
OBJS = $(shell find ./ -name "*.o")

GENERATED = generated

TEST_TARGET = kfs_test
TEST_SRCS = \
	$(shell find ./ ! -name "kfsmain.c" -name "*.c") \
	$(shell find ./sds -name "*.c") \
	$(shell find ./tests -name "*.c")

all: $(TARGET)

test: build_test run_test

build_test: $(TEST_TARGET)

run_test:
	$(GENERATED)/$(TEST_TARGET)

$(TARGET): $(SRCS) | $(GENERATED)
	$(CC) -o $(addprefix $(GENERATED)/, $@) $^ $(CFLAGS)

$(TEST_TARGET): $(TEST_SRCS) | $(GENERATED)
	$(CC) -o $(addprefix $(GENERATED)/, $@) $^ $(CFLAGS)  -I ./

$(GENERATED):
	@mkdir -p $(GENERATED)

clean:
	$(RM) $(OBJS) $(addprefix $(GENERATED)/, $(TARGET) $(TEST_TARGET))
