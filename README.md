# CacheSavior
A high-performance HTTP forward proxy server built in C using POSIX sockets, pthreads, and semaphores. The server handles multiple client connections concurrently, used proxy_parse -- a HTTP Request Parsing Library to parse HTTP GET requests, forwards them to destination servers, and caches responses to reduce latency and bandwidth usage.

