# RedisForge Architecture

## 1. Project Overview

RedisForge is an educational, Redis-inspired in-memory database implemented in C++20.

The project aims to demonstrate practical systems engineering concepts, including data structures, networking, protocol parsing, persistence, concurrency, testing, and performance measurement.

RedisForge is an independent project and does not claim full Redis compatibility. Compatibility will be established incrementally through documented behavior and automated tests.

## 2. High-Level Architecture

The system is divided into the following components:

### 2.1 Storage Engine

Responsible for storing and retrieving key-value data, deleting keys, checking key existence, and supporting additional data types in later phases.

### 2.2 Command Execution

Validates commands and their arguments, executes supported operations, and returns structured results or errors.

### 2.3 Protocol Layer

Parses incoming RESP messages and encodes responses. Protocol handling will be independent of the storage engine.

### 2.4 Network Layer

Accepts client connections over TCP and transfers protocol messages between clients and the command-execution layer.

### 2.5 Persistence

A later phase will introduce append-only-file persistence and recovery.

### 2.6 Testing and Benchmarking

Automated tests will verify correctness. Benchmarks will measure performance independently of functional tests.

## 3. Initial Supported Commands

The initial storage-engine milestone targets:

* PING
* SET
* GET
* DEL
* EXISTS
* INCR
* DECR
* MGET
* MSET
* DBSIZE
* FLUSHDB

The exact command semantics, argument validation, error behavior, and return values will be documented and tested before implementation.

## 4. Initial Engineering Principles

* Use C++20.
* Prefer clear ownership and well-defined interfaces.
* Separate storage, command execution, protocol handling, and networking.
* Write automated tests for expected behavior and edge cases.
* Keep platform-specific networking code isolated behind suitable interfaces.
* Use CMake for configuration and builds.
* Use GoogleTest for automated unit tests.
* Use Git for version control and GitHub Actions for continuous integration.
* Measure performance rather than making unsupported performance claims.

## 5. Development Strategy

Development will proceed in small, verifiable milestones.

Each milestone must have defined acceptance criteria and passing tests before it is considered complete.

The first implementation milestone will focus on the storage engine, independently of TCP networking and RESP parsing.
