PYTHON ?= python3
ASSETS_DIR ?= assets
CHUNK_SIZE ?= 100000

CFLAGS := -Wall -Wextra -Wundef -Wshadow -Wpointer-arith -Wcast-align \
	-Wstrict-overflow=5 -Wwrite-strings -Waggregate-return -Wcast-qual \
	-Wswitch-enum -Wunreachable-code -Wno-unused-parameter -pthread \
	-D_DEFAULT_ASSETS_PATH="\"$(ASSETS_DIR)\"" -D_CHUNK_SIZE="$(CHUNK_SIZE)"

default: assets is_even

.PHONY: assets
assets:
	$(PYTHON) gen_ifs.py $(CHUNK_SIZE) $(ASSETS_DIR)

is_even: is_even.c queue.c queue.h config.h
	$(CC) -o $@ $(CFLAGS) is_even.c queue.c
