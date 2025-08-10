# ⚡️🦀 is-even — *A Blazingly Fast Number Parity Analyzer™* 🚀✨

> Determining evenness — reimagined for web3

---

## ⚡️ Overview

**is-even** is an aggressively optimized, zero-runtime-overhead, multi-threaded number parity engine written in **C** — the language of **true performance**. While lesser systems rely on clunky "modulo" operations, **is-even** embraces a more scalable, systems-oriented solution: **a 4.2-billion-branch if-statement chain**.

Forget `%`. This is **parity detection done right**.

By leveraging the raw power of the C ecosystem, dynamic code loading techniques, and multi-core CPU scheduling, **is-even** delivers **blazingly fast** number analysis with surgical precision.

---

## 🧠 Architecture

Instead of interpreting the number at runtime, **is-even** pre-compiles **2<sup>32</sup>-1 specialized `if` statements** — one for every possible 32-bit unsigned integer — and then:

1. 📦 **Chunk Compiling**:
   Due to the massive codebase (roughly 50GB of machine code), the if-statement chain is **split into manageable chunks** of ~100,000 comparisons each, and compiled into individual object files.

2. 🧵 **Code as a Stream™ (CaaS™)**:
   At runtime, a pool of threads `mmap()` these object files on-demand, **streaming** them into memory only as needed. This reduces memory overhead to mere **tens-of-megabytes** — a **fraction** of what you'd use with non-streaming logic.

3. 🚀 **Blazing Concurrent Parity Resolution**:
   The number is routed through active chunks across threads until a match is found. Execution halts **immediately** upon identification, thus allowing **is-even** to **avoid unnecessary branch misses**.

---

## 🗿 Getting Started

### 💻 Requirements

- Linux (cross-platform support coming soon)
- Python >= 3.8
- C compiler
- `nasm`
- `make`

### 🦾 Compiling

```sh
> make
# OR
> make OPENAI_API_KEY=<KEY_GOES_HERE>
```

### 👨‍🦼‍➡️ Usage

```sh
> ./is_even 42
42 is even
> ./is_even 99999999
99999999 is odd
```
