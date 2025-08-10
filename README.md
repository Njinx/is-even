# ⚡️🦀 is-even — *A Blazingly Fast Number Parity Analyzer™* 🚀✨

> Determining evenness — reimagined for web3

---

## ⚡️ Overview

**is-even** is an aggressively optimized, zero-runtime-overhead, multi-threaded number parity engine written in **C** — the language of **true performance**. While lesser systems rely on clunky "modulo" operations, **is-even** embraces a more scalable, systems-oriented solution: **a 4.2-billion-branch if-statement chain**.

Forget `%`. This is **parity detection done right**.

By leveraging the raw power of the C toolchain, advanced memory mapping techniques, and multi-core CPU scheduling, **is-even** delivers **blazingly fast** number analysis with surgical precision.

---

## 🧠 Architecture

Instead of interpreting the number at runtime, **is-even** pre-compiles **2<sup>32</sup>-1 specialized `if` statements** — one for every possible 32-bit unsigned integer — and then:

1. 📦 **Chunk Compiling**
   Due to the massive codebase (roughly 50GB of machine code), the if-statement chain is **split into manageable chunks** of ~100,000 comparisons each, and compiled into individual object files.

2. 🧵 **Threaded mmap Streaming™**
   At runtime, a pool of threads `mmap()` these object files on-demand, **streaming** them into memory only as needed. This reduces memory overhead to mere tens-of-megabytes — a **fraction** of what you'd use with non-streaming logic.

3. 🚀 **Blazing Fast Parity Resolution**
   The number is routed through active chunks across threads until a match is found. Execution halts **immediately** upon identification, avoiding unnecessary comparison cycles. This allows **is-even** to hit **near-L1 cache speeds**, in theory.

---

## 🗿 Usage

### 💻 Requirements

- Python >= 3.8
- C compilers

```
> make
...
> ./is_even 42
42 is even
```
