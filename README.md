GreenOS: User-Space Thread Scheduler from Scratch

A lightweight, cooperative multithreading operating system scheduler built entirely from scratch in C++ and x86-64 Assembly.

This project bypasses standard OS-level threads (like std::thread or POSIX threads) to implement custom Green Threads in user space. It demonstrates a deep, bare-metal understanding of CPU architecture, the Windows x64 Calling Convention (ABI), and dynamic memory management.

🚀 Features

Custom Thread Control Block (TCB): Manual allocation and management of 64KB stacks for individual tasks.

Bare-Metal Context Switching: x86-64 Assembly subroutine to safely save and restore CPU state (Callee-saved registers and Stack Pointers) directly in hardware.

Round-Robin Cooperative Scheduler: A custom C++ scheduler utilizing a Ready Queue to cooperatively multiplex multiple execution contexts onto a single OS thread.

Automated Garbage Collection: Implemented a "Zombie Thread" graveyard. Threads implicitly fall into a trap (green_exit) upon completion, yielding the CPU permanently while allowing the next scheduled thread to safely deallocate the previous thread's memory to prevent segmentation faults.

Windows x64 ABI Compliant: Perfectly aligns with the Microsoft calling convention (utilizing rcx/rdx and preserving 8 specific registers).

🧠 How It Works

Stack Forging: When a thread is spawned, the OS manually manipulates the thread's memory address array, pushing a trap exit function, the target function address, and 8 dummy registers to perfectly simulate a suspended execution state.

The Swap: The context_switch assembly function violently overwrites the physical CPU rsp (Stack Pointer) register with the forged stack, effectively teleporting execution to the new task.

Yielding: Tasks voluntarily call green_yield(), which saves their current hardware state, pushes them to the back of the Ready Queue, and pops the next task into the CPU.

🛠️ Build and Run

This project is configured for the Windows environment using the g++ compiler (MinGW/MSYS2).

# 1. Compile the C++ and Assembly files together
g++ main.cpp greenthread.cpp context_switch.S -o green_os.exe

# 2. Execute the scheduler
.\green_os.exe


📂 Project Structure

main.cpp: The core OS Scheduler, Ready Queue, and Garbage Collector logic.

greenthread.h / greenthread.cpp: The Thread struct, memory allocation, and stack-forging logic.

context_switch.S: The raw x86-64 Assembly instructions that manipulate the hardware registers.