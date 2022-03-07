CC       = gcc
CXX      = g++
LANGFLAG = -x c++
CPPFLAGS += -I include/
CFLAGS   += -g -Wall -O2 -std=c++11
LDFLAGS  += $(LIBS) -lpthread -lz -rdynamic
BUILD_DIR = build

ifeq ($(zstd),1)
LDFLAGS		+= -lzstd
endif

BINARY = haru
OBJ = $(BUILD_DIR)/main.o \
      $(BUILD_DIR)/haru.o \

PREFIX = /usr/local
VERSION = `git describe --tags`

ifdef asan
	CFLAGS += -fsanitize=address -fno-omit-frame-pointer
	LDFLAGS += -fsanitize=address -fno-omit-frame-pointer
endif

.PHONY: clean distclean test

$(BUILD_DIR)/main.o: src/main.c
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(LANGFLAG) $< -c -o $@

clean:
	rm -rf $(BINARY) $(BUILD_DIR)/*.o
	make -C slow5lib clean

# Delete all gitignored files (but not directories)
distclean: clean
	git clean -f -X
	rm -rf $(BUILD_DIR)/* autom4te.cache

