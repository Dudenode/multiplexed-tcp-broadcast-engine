# Multiplexed IPv4 TCP Broadcast Engine

## 1. Abstract
This repository contains the architectural specification and source code for a custom IPv4 Transmission Control Protocol (TCP) broadcast engine implemented in C. The software functions as a single-threaded, non-blocking network router, designed to ingest raw byte streams from multiple concurrent clients and multiplex those payloads across an active peer network. By leveraging POSIX socket APIs and event-driven I/O polling, the engine bypasses the execution overhead of thread-per-client concurrency models, ensuring predictable memory utilization and low-latency packet routing.

## 2. Architectural Design & Memory Management
The core architecture relies on an asynchronous, single-threaded event loop. Rather than spawning independent operating system threads or processes for each incoming connection, the server manages all network connections concurrently within a single execution thread.

State management is delegated to the operating system kernel via the `poll()` system call. The kernel tracks network state changes across an array of registered file descriptors. The application execution halts until a kernel interrupt signals an active state change, at which point the engine sequentially processes only the file descriptors flagged with `POLLIN` events.

To eliminate the risk of memory leaks and heap fragmentation, the engine strictly avoids dynamic heap allocation (`malloc`/`free`) during runtime. Network state arrays are statically allocated. I/O buffers are allocated on the application stack (1,024 bytes per event) and decay upon block exit.

## 3. Network I/O Model
*   **Infrastructure Initialization:** The network engine requests an `AF_INET` streaming socket (`SOCK_STREAM`) bound to `INADDR_ANY`.
*   **Asynchronous Processing:** The primary listening socket is modified via `fcntl()` to enforce `O_NONBLOCK` operations. System calls like `accept()` or `read()` return immediately with `EWOULDBLOCK` rather than suspending the application if data is not instantly available.
*   **Broadcast Routing Protocol:** The engine strictly routes data without echoing. Upon reading a payload from origin file descriptor *N*, the engine iterates over the active file descriptor registry and executes a `send()` system call to all active descriptors where target index *j ≠ N*.

## 4. Process Lifecycle & Graceful Termination
The engine implements a strict user-space graceful shutdown mechanism to ensure all file descriptors are manually closed before termination, preventing port exhaustion.
*   **Signal Interception:** The POSIX `signal()` API intercepts `SIGINT`.
*   **Atomic State Management:** A global execution flag (`volatile sig_atomic_t`) prevents memory tearing when the signal handler modifies the loop condition asynchronously.
*   **System Call Interruption (EINTR):** When `SIGINT` aborts a blocking `poll()` call, the event loop explicitly filters the `EINTR` error code, bypassing fatal termination to evaluate the modified state flag.
*   **Socket Teardown:** A dedicated cleanup routine linearly traverses the active file descriptor array ($O(N)$) and executes `close()` on every valid socket before exiting cleanly with status 0.

## 5. Diagnostic & Error Management Subsystem
A centralized, variadic error-handling module physically separates error reporting from the core event loop logic and standardizes output to `stderr`.
*   **Fatal Error Wrapper (`die`):** Captures the global `errno`, formats a variadic string, appends the system's exact error description via `strerror`, and forcefully terminates the process (`exit(EXIT_FAILURE)`). Utilized for unrecoverable system failures.
*   **Non-Fatal Diagnostics (`sys_warn`):** Executes identical variadic formatting and `errno` extraction, but returns control to the execution thread to drop faulty connections without halting the primary event loop.

## 6. Repository Structure
The source tree separates implementation, header interfaces, build artifacts, and automated testing suites.
```text
.
├── bin/                 # Compiled binaries and object files (Git ignored)
├── include/             # Header interfaces
│   ├── error.h
│   └── server.h
├── src/                 # Source implementations
│   ├── error.c
│   ├── main.c
│   └── server.c
├── tests/               # Integration testing suite
│   └── test_engine.py
├── Makefile             # GNU Make build automation graph
└── README.md