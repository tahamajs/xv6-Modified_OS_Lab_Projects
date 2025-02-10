
# Operating Systems Lab Projects

This repository contains a series of Operating Systems lab assignments implemented using **xv6**, an educational Unix-like OS.

## Table of Contents
1. [Lab 1: Introduction to xv6](#lab-1-introduction-to-xv6)
2. [Lab 2: System Calls](#lab-2-system-calls)
3. [Lab 3: Process Scheduling](#lab-3-process-scheduling)
4. [Lab 4: Synchronization](#lab-4-synchronization)
5. [Lab 5: Memory Management](#lab-5-memory-management)

## Lab 1: Introduction to xv6
**Description:**  
This lab introduces students to **xv6**, a minimalistic OS developed at MIT for educational purposes.  
Tasks include:
- Setting up **xv6** on **QEMU**.
- Implementing console improvements like cursor movement and command history.
- Adding support for **clipboard operations** in the terminal.

**Resources:**  
- Official xv6 repository: [xv6-public](https://github.com/mit-pdos/xv6-public)  
- Compilation & Execution:  
  ```sh
  make qemu
  ```


## Lab 2: System Calls
**Description:**  
This lab focuses on **system calls** in xv6.  
Tasks include:
- Understanding the **trap mechanism** for system calls.
- Implementing new system calls.
- Exploring **how user programs interact with the kernel**.

**Key Concepts:**  
- System Call Handling  
- Trap Gate in xv6  
- Wrappers in **usys.S**  

---

## Lab 3: Process Scheduling
**Description:**  
This lab covers **CPU scheduling mechanisms** in xv6.  
Tasks include:
- Studying **Round Robin (RR) scheduling** used in xv6.
- Implementing a **Multi-Level Feedback Queue (MLFQ)** scheduler.
- Ensuring correct **context switching** between processes.

**Key Concepts:**  
- **Context Switching**
- **Scheduler Implementations**
- **Process Life Cycle in xv6**

---

## Lab 4: Synchronization
**Description:**  
This lab explores **concurrency control** and synchronization mechanisms.  
Tasks include:
- Understanding **spinlocks**, **sleep locks**, and **semaphores** in xv6.
- Implementing **mutex-based synchronization**.
- Debugging **race conditions** in multi-threaded applications.

**Key Concepts:**  
- **Spinlocks vs. Sleep Locks**
- **Race Conditions**
- **Process Synchronization in xv6**

---

## Lab 5: Memory Management
**Description:**  
This lab covers **memory management** in xv6.  
Tasks include:
- Understanding **paging and segmentation**.
- Exploring **virtual memory translation** in xv6.
- Implementing **memory allocation mechanisms**.

**Key Concepts:**  
- **Page Tables & Address Translation**
- **Physical vs. Virtual Memory**
- **Swapping & Memory Allocation in xv6**

---

## Setup & Usage
### Prerequisites:
- GCC and Make  
- QEMU Emulator  
- Clone the repository:
  ```sh
  git clone https://github.com/tahamajs/xv6-Modified_OS_Lab_Projects.git
  cd OS-Labs
  ```

### Running xv6:
```sh
make qemu
```

---

## Contributors
- **Ali Ghanbari & Nesa Abbasi** (Lab 1)
- **Mehdi Noori & Golrashidi** (Lab 2)
- **Mohammadreza Nemati & Behrad Elmi** (Lab 3)
- **Ali Ataollahi & Mehdi Haji** (Lab 4)
- **Arian Firoozi & Puriya Tajmehrabi** (Lab 5)

---

## License
This project is for educational purposes only. Feel free to use and modify it.

