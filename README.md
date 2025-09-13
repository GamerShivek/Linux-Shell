# Linux Shell

Linux Shell is a C-based Unix-like shell project that demonstrates core shell features such as command parsing, execution, piping, background jobs, and a small set of built-in commands. The code is organized as a teaching and experimentation project rather than a general-purpose production shell.

## Overview

The shell accepts user input, splits it into commands, and routes each command through the correct execution path. It supports ordinary external programs as well as project-specific built-ins such as `reveal`, `hop`, and `log`.

The implementation is split into small modules so that parsing, execution, process management, and built-in behavior stay separated and easy to inspect.

## Features

- Command parsing with arguments
- Sequential command handling with `;`
- Background execution with `&`
- Piping between commands
- Built-in commands for navigation, listing, and log inspection
- Process and job tracking for foreground and background work
- File and directory handling helpers for shell-specific behavior

## Repository Layout

- `shell/Makefile` - build rules for the shell executable
- `shell/include/` - headers for parsing, execution, jobs, piping, and built-ins
- `shell/src/` - implementation files for the shell

## Build

Build the shell from the `shell/` directory:

```bash
cd shell
make
```

This produces `shell.out`.

For a debug build:

```bash
cd shell
make debug
```

For an optimized build:

```bash
cd shell
make release
```

## Run

After building, start the shell with:

```bash
cd shell
./shell.out
```

Example commands inside the shell:

```bash
ls -la
hop ..
reveal
reveal -a
sleep 5 &
cat input.txt | grep hello
echo first; echo second
```

## Command Behavior

The shell focuses on a compact command set that covers the typical features expected in an operating-systems project:

- `reveal` - directory and file listing behavior similar to `ls`
- `hop` - directory navigation behavior similar to `cd`
- `log` - inspect shell history or recent command activity
- `&` - run a command in the background
- `|` - connect the output of one command to the input of another
- `;` - execute multiple commands in sequence

## Implementation Notes

The project is divided into modules for background jobs, command parsing, piping, and built-in handling. That structure keeps most of the behavior local to the component that owns it and makes the code easier to modify for classwork or experimentation.

## Cleaning Up

Remove build artifacts with:

```bash
cd shell
make clean
```
