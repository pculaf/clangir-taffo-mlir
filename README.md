# TAFFO-MLIR / ClangIR Integration

This project develops an integration path between a ClangIR-based C/C++
frontend and TAFFO-MLIR.

The current work focuses on lowering a supported CIR subset to standard MLIR
dialects that can be consumed by the TAFFO-MLIR pipeline.

## Current Status

TAFFO-MLIR has been built successfully against its pinned LLVM/MLIR snapshot,
with ClangIR/CIR support enabled. A separate top-of-tree LLVM build was also
used to check the current upstream ClangIR state.

At the current checked LLVM revisions, ClangIR can emit CIR and lower CIR to
LLVM-level representations. Lowering CIR to high-level MLIR dialects such as
`func`, `arith`, `scf`, `cf`, and `memref` does not appear to be available as an
exposed tool path in the tested tools.

The incubator `llvm/clangir` repository appears to contain through-MLIR lowering
infrastructure, but that path does not appear to be available in the official
top-of-tree LLVM build checked here.

The currently exposed ClangIR path is:

```text
C/C++ -> CIR
CIR -> LLVM dialect MLIR
CIR -> LLVM IR
```

This repository now provides the standalone `clangir-taffo-opt` tool and an
initial `--convert-cir-to-standard` pass implemented with MLIR's dialect
conversion infrastructure. The currently supported conversion is:

```text
!cir.float       -> f32
cir.func         -> func.func
cir.binop(add)   -> arith.addf
cir.return       -> func.return
```

The initial pass converts a complete floating-point addition function without
leaving CIR operations in the output. Unsupported CIR operations cause the
conversion to fail. The tool setup and addition conversion are covered by
`lit` and `FileCheck` regression tests.

## Next Steps

The next step is to validate the initial conversion with CIR emitted by Clang,
then extend the supported arithmetic subset and test the resulting standard
MLIR with the TAFFO-MLIR pipeline.

## Building

The project is configured as a standalone MLIR project and requires an
LLVM/MLIR/Clang installation that includes CIR headers and libraries.

```sh
cmake -G Ninja -S . -B build \
  -DLLVM_INSTALL_DIR=<llvm-install-prefix> \
  -DLLVM_EXTERNAL_LIT=<llvm-source>/llvm/utils/lit/lit.py
cmake --build build --target clangir-taffo-opt
```

Run the regression tests with:

```sh
cmake --build build --target check-clangir-taffo
```

## Build References

The raw LLVM version and CMake configuration files are kept separately so they
can be referenced from this README or reused in future build instructions.

TAFFO-pinned LLVM:

- `llvm-taffo-pinned/llvm_commit.txt`
- `llvm-taffo-pinned/llvm_compile_flags.txt`
- `llvm-taffo-pinned/llvm_version.txt`

Top-of-tree LLVM:

- `llvm-top-of-tree/llvm_commit.txt`
- `llvm-top-of-tree/llvm_compile_flags.txt`
- `llvm-top-of-tree/llvm_version.txt`
