PYTHON ?= python3
ASSETS_DIR ?= assets

CFLAGS := -Wall -Wextra -Wundef -Wshadow -Wpointer-arith -Wcast-align \
	-Wstrict-overflow=5 -Wwrite-strings -Waggregate-return -Wcast-qual \
	-Wswitch-enum -Wunreachable-code -Wno-unused-parameter -pthread \
	-D_DEFAULT_ASSETS_PATH="\"$(ASSETS_DIR)\""

default: assets is_even

.PHONY: assets
assets:
	$(PYTHON) gen_ifs.py $(ASSETS_DIR)

is_even: is_even.c queue.c queue.h config.h
	$(CC) -o $@ $(CFLAGS) is_even.c queue.c
