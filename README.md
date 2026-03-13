# ThreadSafeQueueLib

Minimal header-only single-producer/single-consumer ring queue.

## Namespace

`tsq`

## Layout

```
ThreadSafeQueueLib/
├── include/
│   └── spsc_ring_queue.hpp
├── tests/
│   └── test_spsc.cpp
├── benchmarks/
│   └── bench_spsc.cpp
├── README.md
├── Makefile
└── .gitignore
```

## Build

```bash
make test
make bench
```

## Run

```bash
./build/test_spsc
./build/bench_spsc
```