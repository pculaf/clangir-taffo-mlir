# ClangIR / TAFFO-MLIR Integration

A course project for **Code Transformation and Optimization**, developing a
C frontend for TAFFO-MLIR using ClangIR.

The project lowers a subset of ClangIR's CIR dialect to standard MLIR dialects
for integration with the TAFFO-MLIR pipeline.

[Project presentation (PDF)](docs/presentation.pdf)

## Scope

The standalone `clangir-taffo-opt` tool provides conversions for:

- **Arithmetic:** floating-point arithmetic, integer control expressions,
  comparisons, and selected numeric conversions.
- **Functions and control flow:** function definitions, direct calls, returns,
  and branches, including calls used to specify value ranges for TAFFO.
- **Counted loops:** normalization of lifted control flow to `scf.for`, reusing
  MLIR's existing loop conversion. Tests cover fixed and runtime bounds,
  positive strides, counting `while` loops, nesting, and conditional bodies.

## Project Structure

- [include/ClangIRTAFFO](include/ClangIRTAFFO): Pass declarations and definitions.
- [lib/Conversion](lib/Conversion): Conversion pass implementations.
- [tools/clangir-taffo-opt](tools/clangir-taffo-opt): Command-line tool.
- [test](test): MLIR regression tests and C examples.
- [llvm-taffo-pinned](llvm-taffo-pinned): LLVM revision and build configuration
  used for development.
- [llvm-top-of-tree](llvm-top-of-tree): LLVM revision and configuration from
  upstream investigation.

## Building

Requires CMake, Ninja, and an LLVM/MLIR/Clang installation with CIR enabled.
Use the revision and configuration in [llvm-taffo-pinned](llvm-taffo-pinned).

```sh
cmake -G Ninja -S . -B build \
  -DLLVM_INSTALL_DIR=<llvm-install-prefix> \
  -DLLVM_EXTERNAL_LIT=<llvm-source>/llvm/utils/lit/lit.py
cmake --build build --target clangir-taffo-opt
```

## Usage

Run from the repository root, with the LLVM installation's tools on `PATH`:

```sh
export PATH="<llvm-install-prefix>/bin:$PATH"

clang -fclangir -emit-cir test/Integration/add.c -o /tmp/add.cir
cir-opt --cir-flatten-cfg --mem2reg /tmp/add.cir \
  | build/bin/clangir-taffo-opt --convert-cir-to-standard \
  | mlir-opt --canonicalize -o /tmp/add.mlir
```

For counted loops, apply the loop conversion to the standard MLIR output:

```sh
mlir-opt input.mlir --canonicalize --lift-cf-to-scf --canonicalize \
  | build/bin/clangir-taffo-opt --convert-lifted-cf-loops-to-scf-for \
      -o output.mlir
```

Additional C examples are in [test/Integration](test/Integration).

## Tests

```sh
cmake --build build --target check-clangir-taffo
```
