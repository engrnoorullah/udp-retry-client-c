# UDP timeout and retry example in C

A POSIX UDP echo client with finite retries, based on Task 5 of my MSc network-programming coursework at UET Peshawar. The exercise demonstrates what an application must do when a datagram or its reply is lost: wait for a matching reply, retry within a limit and report failure if the limit is reached.

The original client and server are preserved in `original/`. This runnable edition was cleaned and partly reimplemented in 2026. It is a small teaching example, not a reliable-transport implementation or a research claim about UDP reliability.

## Build and run

Requirements: a POSIX environment, a C11 compiler and Make. Python 3 is used for tests. Tested with GCC on Linux.

```sh
make
./build/udp_server 5001
```

In another terminal:

```sh
./build/udp_client 127.0.0.1 5001 'hello'
```

The client prints `hello` to stdout and reports the number of attempts on stderr. Defaults are a 250 ms timeout per attempt and at most four attempts. Override them with:

```sh
./build/udp_client 127.0.0.1 5001 'hello' 100 5
```

To demonstrate loss, restart the server with a count of initial requests to discard:

```sh
./build/udp_server 5001 2
./build/udp_client 127.0.0.1 5001 'hello' 100 4
# received reply after 3 attempt(s)
```

The server's discard count is global across clients; use one client for that example. Ctrl-C stops the server. `make test` checks echo responses, dropped packets, retry exhaustion, malformed replies, deadlines, payload limits and invalid CLI arguments. No external server is used.

## Behaviour and limits

Each datagram contains a four-byte identifier in network byte order followed by at most 1000 payload bytes. The same packet is retransmitted. The client accepts only a reply matching its identifier and entire payload. A connected UDP socket filters replies from other endpoints, and a monotonic deadline prevents invalid replies from resetting the timer.

Exit status is `0` for a matching reply, `2` for exhausted attempts and `1` for invalid input or a transport error. An ICMP error such as “connection refused” can produce an immediate transport error rather than a timeout. The server binds to loopback. Numeric IPv4 addresses are accepted; DNS lookup is outside the example.

UDP still offers no delivery, ordering or duplicate-suppression guarantee. Echo is idempotent, so repeated requests have no application side effect. Do not use this retry scheme unchanged for transactions or file transfer. There is no congestion control, adaptive retransmission timer, authentication, encryption or cryptographic request identifier. The timeout is an application deadline on a general-purpose OS, not a hard real-time bound.

## Source history

The original `udpsrv.c` and `udpclireliable.c` came from Task 5 in `Noorullah_All9TasksComplete.zip`. Their headers identify Noor Ullah. The original client used `SIGALRM`, `setjmp()` and `longjmp()` to retry, without a finite retry bound. The current client uses `poll()` and `CLOCK_MONOTONIC`, adds validation and limits, and replaces the course support header with standard headers. These are 2026 changes, not retrospective claims about the coursework. Checksums are recorded in `original/SHA256.json`.

The coursework follows conventional UNIX networking patterns. Third-party books, slides, support archives and compiled binaries from the supplied collection are excluded. Original coursework author: Noor Ullah. No redistribution licence has been selected for this repository.
