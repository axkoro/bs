# Operating Systems Projects

This repository contains several projects developed during the "Betriebssysteme I" (_Operating Systems I_) course in my Computer Science studies.

## The Projects

- **Bootloader:**  
  A demonstration "bootloader" written in C with inline assembly that simulates the boot process. It briefly switches to 16-bit mode, displays a greeting (“Hello!”), and accepts basic user input. Although it isn’t really a fully functional bootloader, it helped understand key concepts of bootstrapping and BIOS interrupt handling.

- **Memory Allocator:**  
  A custom dynamic memory manager implemented with a buddy allocation strategy on a fixed-size heap. It allocates, reallocates, and frees memory blocks by tracking usage via a bitset, and provides a function to dump the current memory state for debugging purposes.

- **Shell:**  
  A custom shell that supports built-in commands (`exit`, `cd`, and `wait`) and the execution of external programs. It handles asynchronous execution using the `&` operator and correctly forwards SIGINT to running processes.

- **Network Shell:**  
  A network-based shell that creates a remote command-line interface using TCP sockets. It supports file transfers via put (and an incomplete get command) and uses POSIX threads to manage multiple clients. Unfortunately, I didn't fully finish that project.

## Setup and Execution

Each project includes its own Makefile. To run any project, navigate to the respective project directory and execute:

```bash
make run
```

The client and server instances of the network shell should be run in two different terminal instances. You can do this by first compiling via:

```bash
make all
```

And then running manually:

```bash
./client # or ./server, respectively
```