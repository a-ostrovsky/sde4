# sde4

A C++ XML parser designed to tolerate malformed input.

## Build and test

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
