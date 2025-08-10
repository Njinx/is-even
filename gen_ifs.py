#!/usr/bin/env python3

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


def gen_if_chain(cnt: int) -> str:
    stmts: list[str] = []
    for i in range(cnt):
        stmts.append(f'cmp dword edi, 0xdeadbeef\n\t{"je .even" if i % 2 == 0 else "je .odd"}')

    return '\n\t'.join(stmts)


def gen_asm(cnt: int) -> str:
    return f'''BITS 64
is_even_chunk:
\t{gen_if_chain(cnt)}
\tmov eax, 2
\tret
.even:
\tmov eax, 1
\tret
.odd:
\tmov eax, 0
\tret
'''


def gen_names(chunk: int) -> tuple[str, str]:
    chunk_dir = chunk >> 8
    hstr = f'{chunk_dir:06x}'
    hfrags: list[str] = []
    for i in range(0, len(hstr), 2):
        hfrags.append(hstr[i] + hstr[i+1])
    hpath = os.path.sep.join(hfrags)

    return hpath, f'{chunk:06x}'


def job_worker(out_base: str) -> None:
    try:
        chunks = math.floor(2**32 / 100_000)
        start = 0
        end = 100_000

        def queue_chunk_job(chunk: int) -> None:
            dir_name, file_name = gen_names(chunk)
            job_queue.put((start, end, os.path.join(out_base, dir_name), file_name))

        i = 0
        for i in range(chunks):
            queue_chunk_job(i)
            start = end
            if i < chunks-1:
                end += 100_000
            else:
                end += 2**32 % 100_000

        queue_chunk_job(i+1)
    except KeyboardInterrupt:
        pass


def make_binary_tmpl(cnt: int) -> bytes | None:
    fdin, namein = tempfile.mkstemp(text=True)
    fdout, nameout = tempfile.mkstemp(text=True)

    if os.write(fdin, gen_asm(cnt).encode()) == 0:
        print('0 bytes of source code written. Not good :(')
        return None

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

def compiler_worker(tmpl: bytearray) -> None:
    last_dir: str | None = None
    while True:
        try:
            try:
                start, end, out_dir, out_file = job_queue.get(block=False, timeout=1)
            except queue.Empty:
                return

            out_path = os.path.join(out_dir, out_file)
            if os.path.exists(out_path):
                continue

            print('.', end='', flush=True)

            if out_dir != last_dir:
                Path(out_dir).mkdir(parents=True, exist_ok=True)
                last_dir = out_dir

            if end - start == 100_000:
                code = tmpl.copy()
            else:
                code = make_binary_tmpl(end - start)

            find_start = 0
            i = 0
            for i in range(start, end):
                if (idx := code.find(b'\xef\xbe\xad\xde', find_start)) == -1:
                    print(f'Too many substitutions!')
                    break

                for j, b in enumerate(struct.pack('=I', i)):
                    code[idx+j] = b

                find_start = idx+4

            if code.find(b'\xef\xbe\xad\xde', find_start) != -1:
                print(f'Not enough substitutions!')
                return

            with open(out_path, 'w+b') as fp:
                fp.write(code)
        except KeyboardInterrupt:
            break


def main() -> None:
    outdir = sys.argv[1]
    Path(outdir).mkdir(exist_ok=True)

    tmpl = make_binary_tmpl(100_000)

    global job_queue
    job_queue = mp.Queue(512)

    jobp = mp.Process(target=job_worker, args=(outdir,))
    jobp.start()

    while job_queue.empty():
        pass

    processes: list[mp.Process] = []
    for _ in range(16):
        p = mp.Process(target=compiler_worker, args=(tmpl,))
        processes.append(p)
        p.start()

    [p.join() for p in processes]
    jobp.join()

if __name__ == '__main__':
    main()