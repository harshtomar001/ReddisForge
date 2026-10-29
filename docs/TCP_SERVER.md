# RedisForge TCP Server — Study Guide

## 1. Overview

RedisForge uses a TCP server to communicate with clients over a network.

The TCP server is responsible for:

* Listening for incoming TCP connections.
* Accepting client connections asynchronously.
* Receiving data from connected clients.
* Sending responses back to clients.
* Eventually passing received RESP2 requests to the command-processing layer.

**Technology:** C++20, Standalone Asio, TCP/IP, RESP2.

**Development port:** `6379`

> Current implementation status: The server can accept TCP connections and has a temporary handler that reads client data and returns a hardcoded `+PONG\r\n` response. The actual `CommandProcessor` has not yet been integrated into the TCP server.

---

## 2. TCP Server Architecture

The current architecture consists of the following components:

```text
                  TCP Client
                      |
                      | TCP Connection
                      v
              +------------------+
              |    TcpServer     |
              |                  |
              |  TCP Acceptor    |
              +------------------+
                      |
                      | Accept Connection
                      v
              +------------------+
              |  Client Socket   |
              +------------------+
                      |
                      | Receive Bytes
                      v
              +------------------+
              |  Receive Buffer  |
              +------------------+
                      |
                      | Temporary Response
                      v
              +------------------+
              |    TCP Client    |
              +------------------+
```

The eventual architecture will connect the TCP server to the existing protocol and command layers:

```text
TCP Client
    |
    | RESP2 Request
    v
TcpServer
    |
    v
Resp2Parser
    |
    v
CommandProcessor
    |
    v
CommandExecutor
    |
    v
StorageEngine
    |
    v
Resp2Encoder
    |
    v
TCP Client
```

The second diagram represents the intended architecture, not the current TCP implementation.

---

## 3. What Is TCP?

TCP stands for **Transmission Control Protocol**.

It provides a reliable, ordered byte stream between two applications.

For example, when a client connects to RedisForge:

```text
Client                         RedisForge
  |                                |
  |-------- TCP connection ------->|
  |                                |
  |<------- Connection ready ------|
  |                                |
  |---------- Send data ---------->|
  |                                |
  |<--------- Send data -----------|
```

TCP handles reliable delivery and ordering of bytes. It does not understand application-level commands such as `PING`, `SET`, or `GET`.

That interpretation belongs to the application protocol—in RedisForge's case, RESP2.

### Important: TCP is a byte stream

Suppose a client sends:

```text
PING
```

The server might receive all four bytes in one read, or receive the bytes across multiple reads.

Similarly, a single read might contain bytes from multiple requests.

Therefore:

**One TCP read does not necessarily equal one complete command.**

A production implementation must buffer incoming bytes, identify complete RESP2 requests, and handle incomplete requests correctly.

---

## 4. What Is Standalone Asio?

Standalone Asio is a C++ networking library that provides facilities for asynchronous network programming.

RedisForge uses Asio for TCP socket management and asynchronous operations.

The primary components used in this project are:

| Component                 | Responsibility                                  |
| ------------------------- | ----------------------------------------------- |
| `asio::io_context`        | Runs and coordinates asynchronous operations.   |
| `asio::ip::tcp::acceptor` | Accepts incoming TCP connections.               |
| `asio::ip::tcp::socket`   | Represents a TCP connection.                    |
| `async_accept()`          | Asynchronously accepts a connection.            |
| `async_read_some()`       | Asynchronously reads available bytes.           |
| `async_write()`           | Asynchronously writes data.                     |
| `asio::buffer()`          | Describes a memory region for an I/O operation. |

On Windows, the project also links the necessary system networking libraries, including `ws2_32` and `mswsock`.

---

## 5. The `io_context`

Example:

```cpp
asio::io_context io_context;
```

The `io_context` is the engine that executes asynchronous operations.

Consider this operation:

```cpp
acceptor_.async_accept(
    [](const asio::error_code& error,
       asio::ip::tcp::socket socket) {
        // Handle the connection.
    }
);
```

Calling `async_accept()` initiates an asynchronous operation and returns without waiting for a client to connect.

The callback executes when the operation completes, provided the `io_context` is running.

That is why `main.cpp` contains:

```cpp
io_context.run();
```

Without running the event loop, the registered asynchronous callbacks will not be processed.

### Execution flow

```text
Create io_context
       |
       v
Create TcpServer
       |
       v
Call server.start()
       |
       v
Register async_accept()
       |
       v
Call io_context.run()
       |
       v
Wait for asynchronous events
       |
       v
Execute callback when a client connects
```

---

## 6. The TCP Acceptor

The `TcpServer` class contains:

```cpp
asio::ip::tcp::acceptor acceptor_;
```

An acceptor listens for incoming connection requests.

Its constructor is:

```cpp
TcpServer::TcpServer(
    asio::io_context& io_context,
    unsigned short port
)
    : acceptor_(
          io_context,
          asio::ip::tcp::endpoint(
              asio::ip::tcp::v4(),
              port
          )
      ) {
}
```

### Constructor parameters

**`asio::io_context& io_context`**

A reference to an existing Asio event-loop object.

The acceptor uses this context to perform asynchronous operations.

**`unsigned short port`**

The local TCP port on which the server will listen.

For RedisForge:

```cpp
TcpServer server(io_context, 6379);
```

**`asio::ip::tcp::v4()`**

Specifies IPv4.

**`asio::ip::tcp::endpoint(...)`**

Constructs the local endpoint from the protocol and port.

In this example, using the IPv4 any-address means the server can listen on all local IPv4 interfaces.

### What does the acceptor constructor do?

The endpoint-taking acceptor constructor opens the acceptor, binds it to the endpoint, and places it into the listening state.

Conceptually, these operations are:

1. Open a TCP socket.
2. Bind it to a local address and port.
3. Start listening for connection requests.

If these operations fail, construction can throw an `asio::system_error`.

---

## 7. Starting the Server

The `start()` method prints the listening port and initiates the first asynchronous accept operation.

```cpp
void TcpServer::start() {
    std::cout << "RedisForge TCP server listening on port "
              << acceptor_.local_endpoint().port()
              << '\n';

    start_accept();
}
```

### `acceptor_.local_endpoint().port()`

Returns the local port associated with the acceptor.

This is useful for logging the actual listening port.

### `start_accept()`

Initiates the operation that waits for an incoming client connection.

The operation is asynchronous, so the server can process other events while waiting.

---

## 8. Accepting Connections Asynchronously

The accept loop is implemented using:

```cpp
void TcpServer::start_accept() {
    acceptor_.async_accept(
        [this](const asio::error_code& error,
               asio::ip::tcp::socket socket) {
            if (!error) {
                std::cout << "Client connected: "
                          << socket.remote_endpoint()
                          << '\n';

                handle_client(std::move(socket));
            } else {
                std::cerr << "Accept failed: "
                          << error.message()
                          << '\n';
            }

            start_accept();
        }
    );
}
```

### Understanding `async_accept()`

```cpp
acceptor_.async_accept(
    callback
);
```

This registers an asynchronous accept operation.

When a client connection is accepted, Asio invokes the callback with:

* An error code.
* A socket representing the accepted connection.

### The error code

```cpp
if (!error) {
    // Connection accepted successfully.
}
```

A zero error code indicates success.

If the operation fails, the error code describes the failure.

### `socket.remote_endpoint()`

```cpp
socket.remote_endpoint()
```

Returns the remote endpoint of the connected socket.

It can provide the client's IP address and port.

For example:

```text
Client connected: 127.0.0.1:54321
```

The client's port is usually an ephemeral port selected by the operating system.

### Why call `start_accept()` again?

```cpp
start_accept();
```

This registers another accept operation.

Without another accept operation, the server would not continue accepting new connections after the current one was handled.

This is the beginning of an asynchronous accept loop.

---

## 9. Why Use `std::move()`?

The accept callback receives a socket by value:

```cpp
asio::ip::tcp::socket socket
```

The accepted socket represents an established TCP connection.

We pass it to the client handler:

```cpp
handle_client(std::move(socket));
```

### Why not simply pass `socket`?

Asio TCP sockets are non-copyable.

Passing a socket by value without moving it would attempt to copy it, which is not permitted.

### What does `std::move()` do?

`std::move()` converts its argument into an expression that can be used for moving.

It enables the socket's move constructor to transfer its resources into the function parameter.

It does not itself transfer resources; the move operation does that.

```cpp
handle_client(std::move(socket));
```

The intention is to transfer ownership of the accepted connection to the handler.

After moving, the original socket object remains valid but has an unspecified state. Do not rely on it continuing to represent the original connection.

---

## 10. Why Use `std::shared_ptr`?

The client handler creates a shared pointer to the socket:

```cpp
auto client =
    std::make_shared<asio::ip::tcp::socket>(
        std::move(socket)
    );
```

The deduced type of `client` is:

```cpp
std::shared_ptr<asio::ip::tcp::socket>
```

### What does `std::make_shared()` do?

It creates an object managed by a `std::shared_ptr`.

The socket is move-constructed from the function's socket parameter.

### Why is shared ownership useful?

Asynchronous operations can complete after the function that initiated them has returned.

The socket must remain alive until those operations no longer depend on it.

Capturing the shared pointer by value in a callback retains shared ownership:

```cpp
[client](const asio::error_code& error,
         std::size_t bytes_received) {
    // Use client here.
}
```

Each captured `shared_ptr` keeps the socket object alive while that ownership remains.

When the final owning `shared_ptr` is destroyed, the socket object is destroyed.

### Important distinction

`std::shared_ptr` manages the socket object's lifetime. It does not automatically make every operation on the socket safe, nor does it automatically coordinate concurrent reads and writes.

The server must still manage connection state, errors, and asynchronous operation lifetimes correctly.

---

## 11. Receiving Data with `async_read_some()`

The client handler creates a socket managed by a shared pointer and a shared receive buffer:

```cpp
void TcpServer::handle_client(asio::ip::tcp::socket socket) {
    auto client =
        std::make_shared<asio::ip::tcp::socket>(
            std::move(socket)
        );

    auto buffer =
        std::make_shared<std::array<char, 1024>>();

    // Initiate asynchronous read.
}
```

### The receive buffer

```cpp
std::array<char, 1024>
```

This is a fixed-size array containing 1,024 characters.

It can be used to receive up to 1,024 bytes in a single read operation.

### Why use a shared pointer for the buffer?

The asynchronous read operation may outlive the function call that starts it.

Capturing the buffer in the callback helps keep it alive for as long as the callback owns it.

### Initiating the read

```cpp
client->async_read_some(
    asio::buffer(*buffer),
    [client, buffer](
        const asio::error_code& error,
        std::size_t bytes_received
    ) {
        // Handle the read result.
    }
);
```

**`asio::buffer(*buffer)`**

The `*` dereferences the shared pointer to access the actual array.

`asio::buffer()` creates a buffer descriptor that tells Asio which memory region to use.

**`bytes_received`**

The number of bytes received by this operation.

It may be smaller than 1,024.

**`error`**

Indicates whether the read completed successfully.

A robust implementation must handle normal disconnection, other errors, and any additional data still required to complete a RESP2 request.

---

## 12. Sending Data with `async_write()`

The current temporary handler sends:

```cpp
const std::string response = "+PONG\r\n";

asio::async_write(
    *client,
    asio::buffer(response),
    [client, response](
        const asio::error_code& write_error,
        std::size_t bytes_sent
    ) {
        if (write_error) {
            std::cerr << "Write failed: "
                      << write_error.message()
                      << '\n';
        }
    }
);
```

### Why use `async_write()`?

It asynchronously writes the complete supplied buffer, unless an error occurs.

This is useful because a lower-level asynchronous write operation may transfer fewer bytes than requested.

### Why capture `response`?

```cpp
[client, response]
```

The response string is captured by value so its contents remain available while the asynchronous write uses the associated buffer.

This is important because the write operation can finish after the surrounding function has returned.

### Why capture `client`?

The callback retains shared ownership of the socket while the write operation is pending.

### Current limitation

The handler always sends:

```text
+PONG\r\n
```

It does not check whether the request was actually `PING`.

This is only a temporary connectivity test.

---

## 13. Understanding RESP2 PING

RedisForge's protocol parser expects a RESP2 request.

A PING request is represented as:

```text
*1\r\n
$4\r\n
PING\r\n
```

As a single line of escaped text:

```text
*1\r\n$4\r\nPING\r\n
```

### Breakdown

| Component  | Meaning                                                    |
| ---------- | ---------------------------------------------------------- |
| `*1\r\n`   | An array containing one element.                           |
| `$4\r\n`   | A bulk string containing four bytes.                       |
| `PING\r\n` | The four-byte command followed by the required terminator. |

The request is 14 bytes long.

A successful simple-string response is:

```text
+PONG\r\n
```

The response consists of seven bytes.

The TCP layer transports these bytes. The RESP2 parser and encoder are responsible for interpreting and constructing the protocol messages.

---

## 14. Testing with PowerShell

The following script connects to RedisForge, sends a RESP2 PING request, reads a response, and closes the connection.

```powershell
# Create a TCP client
$client = [System.Net.Sockets.TcpClient]::new()

# Connect to RedisForge
$client.Connect("127.0.0.1", 6379)

# Get the network stream
$stream = $client.GetStream()

# Construct a RESP2 PING request
$bytes = [System.Text.Encoding]::ASCII.GetBytes("*1`r`n`$4`r`nPING`r`n")

# Send the request
$stream.Write($bytes, 0, $bytes.Length)

# Create a receive buffer
$buffer = New-Object byte[] 128

# Read the response
$count = $stream.Read($buffer, 0, $buffer.Length)

# Display the response
[System.Text.Encoding]::ASCII.GetString($buffer, 0, $count)

# Close the client
$client.Close()
```

Expected response:

```text
+PONG
```

The response includes the RESP2 `\r\n` terminator.

### Why escape `$4`?

In a PowerShell double-quoted string, `$` introduces variable interpolation.

Writing `` `$4 `` preserves the literal dollar sign required by RESP2.

Without that escape, the request may be malformed.

### What does this test prove?

If it succeeds, the test demonstrates that:

1. The server is listening on the expected port.
2. A client can establish a TCP connection.
3. The server can receive data.
4. The server can send a response.
5. The client can read the response.

It does not yet prove that RESP2 parsing or Redis command execution works through the TCP layer.

---

## 15. TCP Server Lifecycle

The lifecycle of a connection is:

```text
1. Create io_context
          |
          v
2. Construct TcpServer
          |
          v
3. Start listening
          |
          v
4. Register async_accept()
          |
          v
5. Run io_context
          |
          v
6. Accept a client connection
          |
          v
7. Transfer socket to handle_client()
          |
          v
8. Initiate asynchronous read
          |
          v
9. Process received bytes
          |
          v
10. Send response asynchronously
          |
          v
11. Handle completion or errors
```

The current implementation handles only one read and one response for each accepted connection. It does not yet implement a persistent client session or a full request-processing loop.

---

## 16. Current Implementation vs. Future Improvements

### Implemented

* Standalone Asio configured for the Windows build.
* TCP acceptor listening on port `6379`.
* Asynchronous client acceptance.
* Client socket ownership transfer using `std::move()`.
* Shared socket and buffer lifetimes using `std::shared_ptr`.
* Asynchronous reading of client bytes.
* Asynchronous writing of a temporary `+PONG\r\n` response.
* PowerShell connectivity test.

### Not yet implemented in the TCP layer

* Integration with `CommandProcessor`.
* RESP2 request buffering and framing across multiple reads.
* Processing multiple requests over a persistent connection.
* Proper handling of incomplete requests.
* Per-client session management.
* Graceful server shutdown and comprehensive error handling.
* Limits on per-client memory use and queued output.

These are natural next steps as RedisForge evolves.

---

## 17. Key Interview Questions

### Q1. What is TCP?

TCP is a connection-oriented transport protocol that provides a reliable, ordered byte stream between applications.

### Q2. What is the difference between a TCP socket and an acceptor?

The acceptor listens for and accepts incoming connections. A socket represents an individual connection used for communication.

### Q3. What is `asio::io_context`?

It is the execution context used by Asio to process asynchronous operations and their completion handlers.

### Q4. What is the difference between `async_read_some()` and `async_write()`?

`async_read_some()` initiates a read of some available data. `async_write()` is a composed operation that attempts to write the entire supplied buffer unless an error occurs.

### Q5. Why is `std::move()` used with the socket?

Asio TCP sockets are non-copyable. Moving allows ownership of the socket's resources to transfer to another socket object.

### Q6. Why use `std::shared_ptr` for the client socket?

It allows asynchronous callbacks to retain shared ownership, keeping the socket alive while those callbacks need it.

### Q7. Why must the receive buffer remain alive?

The asynchronous read operation uses the buffer's memory. That memory must remain valid until the operation completes.

### Q8. Why doesn't one TCP read necessarily contain one command?

TCP transports a byte stream and does not preserve application-level message boundaries. A request can arrive across multiple reads, or multiple requests can arrive together.

### Q9. What is RESP2?

RESP2 is the protocol Redis clients use to encode commands and responses. It defines types such as arrays, bulk strings, integers, simple strings, and errors.

### Q10. What does the current PING test prove?

It proves that the TCP communication path works for the tested request and response. It does not prove that the server correctly parses and executes Redis commands.

---

## 18. Final Summary

The TCP layer is the networking foundation of RedisForge.

It accepts connections, receives bytes, and sends bytes. Asio handles asynchronous I/O, while C++ ownership tools help ensure that sockets and buffers remain alive during pending operations.

The next major milestone is to connect the TCP layer to `CommandProcessor`, allowing RedisForge to parse RESP2 requests, execute commands against `StorageEngine`, and encode protocol-compliant responses.
