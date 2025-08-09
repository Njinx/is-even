PYTHON ?= python3

default: outdir is_even

.PHONY: outdir
outdir:
	$(PYTHON) gen_ifs.py $@

is_even: is_even.c
