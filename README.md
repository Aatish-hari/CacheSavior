# Multithreaded HTTP Proxy Server with Caching

A lightweight forward proxy server written in C, built with raw POSIX sockets, `pthread`-based concurrency, and an in-memory LFU-style cache for repeated requests.

## Overview

This project implements a **forward HTTP proxy**: a server that sits between a client (browser, `curl`, etc.) and the destination web server, forwarding requests on the client's behalf and relaying the response back.

When a client is configured to use this proxy, requests flow like this:

Client  ──────►  Proxy Server  ──────►  Destination Web Server

Client  ◄──────  Proxy Server  ◄──────  Destination Web Server

## Features

- **Concurrent client handling** using POSIX threads (`pthread`), with a counting semaphore capping the number of clients served at once (`MAX_CLIENTS`)
- **HTTP request parsing** via the `proxy_parse` library, extracting method, host, port, path, and headers from raw request text
- **In-memory caching** of responses, keyed by request, with:
  - A usage counter per cached entry
  - Least-frequently-used eviction (`remove_cache_element`) once the cache exceeds `MAX_CACHE_SIZE`
  - Per-element size limits (`MAX_ELEMENT_SIZE`) to prevent any single response from dominating the cache
- **Custom HTTP error responses** for common status codes (400, 403, 404, 500, 501, 505)
- **Thread-safe cache access** guarded by a mutex


## Benchmark Results

Tested using `curl` against `www.google.com` — first request (cache miss) vs. second request (cache hit):

![Benchmark screenshot](benchmark/Screenshot%202026-07-16%20135444.png)
![serverside screenshot](benchmark/Screenshot%202026-07-16%20140009.png)


Cache hits reduced response time from ~520ms to ~0.5ms — a ~99.9% latency reduction.