#!/usr/bin/env python3

import typing as t
import math
import os
import sys
from pathlib import Path
import tempfile
from subprocess import Popen
import queue
import struct
import multiprocessing as mp
import shutil


job_queue: queue.Queue[tuple[int, int, str, str]]

ASM_BEGIN = f'''\
BITS 64
is_even:
'''.encode()
ASM_END = f'''\
\tmov eax, 2
\tret
.even:
\tmov eax, 1
\tret
.odd:
\tmov eax, 0
\tret
'''.encode()


def gen_if_chain(wfd: int, cnt: int) -> None:
    for i in range(cnt):
        if i % 2 == 0:
            jmp = b'\tje .even\n'
        else:
            jmp = b'\tje .odd\n'
        stmt = b'\tcmp dword edi, 0xdeadbeef\n'
        os.write(wfd, stmt + jmp)


def gen_asm(wfd: int, cnt: int) -> None:
    os.write(wfd, ASM_BEGIN)
    gen_if_chain(wfd, cnt)
    os.write(wfd, ASM_END)


def gen_names(chunk: int) -> tuple[str, str]:
    chunk_dir = chunk >> 8
    hstr = f'{chunk_dir:06x}'
    hfrags: list[str] = []
    for i in range(0, len(hstr), 2):
        hfrags.append(hstr[i] + hstr[i+1])
    hpath = os.path.sep.join(hfrags)

    return hpath, f'{chunk:06x}'


def producer_thread(chunk_size: int, out_base: str) -> None:
    try:
        chunks_dec = 2**32 / chunk_size
        chunks = math.floor(chunks_dec)
        start = 0
        end = chunk_size

        print(f'Producing {math.ceil(chunks_dec)} chunks. This may take a while...', end='', flush=True)

        def queue_chunk_job(chunk: int) -> None:
            dir_name, file_name = gen_names(chunk)
            job_queue.put((start, end, os.path.join(out_base, dir_name), file_name))

        i = 0
        for i in range(chunks):
            queue_chunk_job(i)
            start = end
            if i < chunks-1:
                end += chunk_size
            else:
                end += 2**32 % chunk_size

        queue_chunk_job(i+1)
    except KeyboardInterrupt:
        pass


def make_binary_tmpl(cnt: int) -> bytes | None:
    fdin, namein = tempfile.mkstemp(text=True)
    fdout, nameout = tempfile.mkstemp(text=True)

    gen_asm(fdin, cnt)

    if not shutil.which('nasm'):
        print('nasm assembler not found.')
        return None

    p = Popen(['nasm', '-o', nameout, '-O0', '-fbin', namein])
    if (status := p.wait()) != 0:
        print(f'GCC exited with code {status}')
        return None

    tmpl = os.read(fdout, os.fstat(fdout).st_size)

    os.close(fdout)
    os.close(fdin)
    os.unlink(nameout)
    os.unlink(namein)

    return bytearray(tmpl)

def consumer_thread(chunk_size: int, tmpl: bytearray) -> None:
    last_dir: str | None = None
    done_cnt = 0

    def inc_done_cnt() -> None:
        nonlocal done_cnt
        done_cnt += 1
        if done_cnt % 100 == 0:
            print('.', end='', flush=True)

    while True:
        try:
            try:
                start, end, out_dir, out_file = job_queue.get(block=False, timeout=1)
            except queue.Empty:
                return

            out_path = os.path.join(out_dir, out_file)
            if os.path.exists(out_path):
                inc_done_cnt()
                continue

            if out_dir != last_dir:
                Path(out_dir).mkdir(parents=True, exist_ok=True)
                last_dir = out_dir

            if end - start == chunk_size:
                code = tmpl.copy()
            else:
                code = make_binary_tmpl(end - start)

            find_start = 0
            i = 0
            for i in range(start, end):
                if (idx := code.find(b'\xef\xbe\xad\xde', find_start)) == -1:
                    print(f'Too many substitutions!')
                    return

                for j, b in enumerate(struct.pack('=I', i)):
                    code[idx+j] = b

                find_start = idx+4

            if code.find(b'\xef\xbe\xad\xde', find_start) != -1:
                print(f'Not enough substitutions!')
                return

            with open(out_path, 'w+b') as fp:
                fp.write(code)

            inc_done_cnt()
        except KeyboardInterrupt:
            break


def usage_die() -> None:
    print(f'Usage: {sys.argv[0]} CHUNK_SIZE OUT_DIR')
    sys.exit(1)


def clamp(n: int, smallest: int, largest: int) -> int:
    if n < smallest:
        return smallest
    if n > largest:
        return largest
    return n


def main() -> None:
    if len(sys.argv) != 3:
        usage_die()

    try:
        chunk_size = int(sys.argv[1])
    except ValueError:
        usage_die()

    outdir = sys.argv[2]
    Path(outdir).mkdir(exist_ok=True)

    tmpl = make_binary_tmpl(chunk_size)

    global job_queue
    job_queue = mp.Queue(512)

    jobp = mp.Process(target=producer_thread, args=(chunk_size,outdir,))
    jobp.start()

    while job_queue.empty():
        pass

    nproc = clamp(mp.cpu_count(), 1, 8) # consumer is IO-bound. Too many threads will likely hurt performance
    processes: list[mp.Process] = []
    for _ in range(nproc):
        p = mp.Process(target=consumer_thread, args=(chunk_size,tmpl,))
        processes.append(p)
        p.start()

    [p.join() for p in processes]
    jobp.join()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass