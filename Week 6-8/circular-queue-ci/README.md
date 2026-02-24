# Circular Queue CI Assignment

![C++ CI](https://github.com/besarthasani89-byte/ExamCI/actions/workflows/ci.yml/badge.svg)

Template circular queue implemented with a circular linked list and tested with GoogleTest.

## Requirements covered

- Makefile builds tests into `build/`
- `clean` rule removes generated files
- default target builds the test binary
- `test` rule runs the test binary
- queue is template-based (`T`)
- queue is non-copyable and movable
- runtime queue size in constructor (must be `> 2`)
- nodes are created and linked in the constructor
- `resize()` supports grow/shrink and drops oldest data first when shrinking
- `make_empty()`, `read()`, and `write()` do not create/delete nodes
- `write()` overwrites oldest data when full
- `size()`, `count()`, and `is_full()` are implemented
- `average()` exists only for arithmetic `T` and returns `double`
- tests cover `int`, `float`, and `std::string`
- GitHub Actions runs on push to any branch and PR to `main`

## Build and run

```bash
make
make test
make clean
```

## GitHub Flow

1. Create a private repository and push this project.
2. Create feature branches for each change.
3. Open pull requests into `main`.
4. Merge only when CI is passing.

## Badge setup

![C++ CI](https://github.com/besarthasani89-byte/ExamCI/actions/workflows/ci.yml/badge.svg)
