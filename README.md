# GreenOS: User-Space Thread Scheduler and M:N Preemption

GreenOS is a lightweight, bare-metal multithreading operating system scheduler built entirely from scratch in C++ and x86-64 Assembly. 

This project bypasses standard OS-level threads (like `std::thread` or POSIX threads) to implement custom Green Threads in user space. Inspired by the concurrency models of Go (Goroutines) and Java (Virtual Threads), it demonstrates a deep understanding of CPU architecture, compiler engineering, and the bridge between User Space and Kernel Space.

## Overview and Architecture

Standard multithreading relies on the Operating System kernel to manage thread execution, which incurs heavy context-switching overhead. GreenOS shifts this responsibility to user space, allowing for blisteringly fast context switches managed entirely by the application.

### Key Technical Features

* **Bare-Metal Context Switching:** Contains a custom x86-64 Assembly subroutine to safely save and restore CPU state (callee-saved registers and Stack Pointers) directly in hardware, strictly adhering to the Windows x64 ABI calling convention.
* **M:N Preemption Model:** To prevent CPU starvation without relying on heavy OS-level thread switching, GreenOS spawns a single background OS "Monitor Thread". This acts as a hardware alarm clock, cleanly interrupting CPU-hogging user-space calculations every 50ms without sacrificing bare-metal execution speed.
* **Asynchronous I/O Wrappers:** Implements custom non-blocking system calls (e.g., `green_getchar`). When a thread waits for user input or I/O, it does not block the underlying OS thread. Instead, it yields the CPU to other Green Threads until the I/O is ready, maintaining total system responsiveness.
* **Compiler AST Engineering:** Includes a custom Clang/LLVM Abstract Syntax Tree (AST) manipulation tool. It parses C++ source code on the fly and automatically injects lightweight preemption checkpoints (`GREEN_CHECK()`) into heavy `for` and `while` loops.
* **Automated Garbage Collection:** Features a "Zombie Thread" graveyard. When threads complete execution, they implicitly fall into a trap function (`green_exit`). This safely yields the CPU permanently while allowing the next scheduled thread to asynchronously deallocate the previous thread's 64KB stack, preventing memory leaks and segmentation faults.

## Project Structure

* `main.cpp`: The core OS Scheduler, Preemption Monitor, Ready Queue, Non-blocking I/O wrappers, and Garbage Collector logic.
* `greenthread.h` / `greenthread.cpp`: The Thread struct, heap memory allocation, and stack-forging logic.
* `context_switch.S`: The raw x86-64 Assembly instructions that manipulate the hardware registers.
* `loop_injector.cpp`: The Clang/LLVM tooling script to parse the AST and inject loop checkpoints.
* `CMakeLists.txt`: Build configuration for the LLVM compiler tool.

## How It Works Under the Hood

1. **Stack Forging:** When a thread is spawned, the OS manually allocates a 64KB heap array and manipulates the memory addresses, pushing a trap exit function, the target function address, and 8 dummy registers to simulate a suspended execution state.
2. **The Swap:** The `context_switch` assembly function violently overwrites the physical CPU `rsp` (Stack Pointer) register with the forged stack, effectively teleporting execution to the new task.
3. **The Preemption Checkpoint:** The background OS thread flips an `std::atomic<bool>` every 50ms. The Clang-injected `GREEN_CHECK()` macro inside user loops checks this boolean (which costs virtually 0 clock cycles) and forces a `green_yield()` if the time slice has expired.

## Prerequisites and Setup

This project is configured for the Windows environment using MinGW-w64 and MSYS2. Because it utilizes bare-metal assembly and LLVM compiler libraries, a specific toolchain is required.

### Required Software
1. **MSYS2** (Provides the GCC compiler and Make tools for Windows).
2. **MinGW-w64 toolchain** (`gcc`, `g++`, `gdb`).
3. **LLVM and Clang development packages** (Required to build the AST injector).
4. **CMake** and **Ninja** build systems.

## Build and Run Instructions

### Step 1: Build the Clang AST Injector
The compiler tool must be built first to parse and modify the OS source code. Open your terminal in the root directory of the project.

    mkdir build
    cd build
    cmake -G Ninja ..
    ninja

### Step 2: Inject Preemption Checkpoints
Run the compiled LLVM tool against the main OS file. This will automatically locate all loops and inject the `GREEN_CHECK()` macro.

    ./loop_injector.exe ../main.cpp -- 

### Step 3: Compile and Run GreenOS
Navigate back to the root directory and compile the C++ files along with the Assembly context switcher. We include the `-pthread` flag to enable the background monitor thread.

    cd ..
    g++ main.cpp greenthread.cpp context_switch.S -o green_os.exe -pthread
    ./green_os.exe

## Limitations and Future Work

While this project successfully demonstrates a highly performant user-space scheduler, it is an educational build designed to highlight systems programming concepts.

1. **Single-Core Multiplexing:** Currently, all Green Threads are multiplexed onto a single OS worker thread (running on a single CPU core). Achieving true Symmetric Multiprocessing (SMP) would require implementing work-stealing queues across multiple OS worker threads.
2. **ABI Specificity:** The `context_switch.S` assembly code is strictly coupled to the Microsoft x64 Calling Convention (using `rcx` and `rdx`). To run this architecture on Linux or macOS (Intel), the assembly must be updated to adhere to the System V ABI (using `rdi` and `rsi`).
3. **Network I/O:** The current asynchronous wrapper handles console input (`conio.h`). A full production implementation would require wrapping standard socket libraries using `epoll` (Linux) or I/O Completion Ports (Windows) to handle non-blocking network requests.