# Contributing to Glyphcast

Thank you for helping improve Glyphcast. Small, focused pull requests are easiest to review.

## Workflow

1. Fork the repository and create a branch: `git checkout -b feature/short-description`.
2. Make your change and add tests where they apply.
3. Run the test suites for every implementation you touched.
4. Open a pull request that explains the change and why it is needed.

## Running the checks

```bash
# C++
cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure

# Python
cd python && python -m unittest -v

# JavaScript
node --check web/glyphcast.js
```

## Keeping the implementations in step

The C++, Python and browser renderers must produce the same output for the same input.
If you change the sampling, luminance or aspect logic, update all three and add a test
to the C++ and Python suites.

## Style

- **C++:** C++17, four-space indentation, no global mutable state, warnings enabled.
- **Python:** PEP 8, type hints on public functions, standard library only.
- **JavaScript:** modern ES2020+, no build step, no dependencies.

## Commit messages

Use a short imperative subject, for example `Add dense ramp to the web UI`.
