#  Pepper — A Bytecode Interpreter

**Pepper** is a lightweight, fast, and modular **bytecode interpreter** designed to execute custom virtual machine instructions efficiently.  
It supports both an **interactive REPL** mode and **file execution** mode, making it ideal for testing, debugging, and experimenting with your own programming language or VM design.

---

## 🚀 Quick Start

### Build

To build the Pepper interpreter, simply run:

```bash
make

```
To remove build artifacts and binaries:

```bash
make clean
```
###  Run

REPL:

```bash

./pepper

> var count = 3;
> print count;
3
```

File Execution Mode:

Run a Pepper source or bytecode file directly:

```bash
./pepper <filename>
```


Example:
```bash
./pepper test_programs/morelocals.txt
```

Darrel Wihandi
Software Engineering @ University of Waterloo
Interests: Compilers, Virtual Machines, AI/ML Infrastructure

License

This project is distributed under the MIT License.
See LICENSE for more information.


