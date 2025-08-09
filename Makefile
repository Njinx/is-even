PYTHON ?= python3

default: outdir main

.PHONY: outdir
outdir:
	$(PYTHON) gen_ifs.py $@

main: main.c
