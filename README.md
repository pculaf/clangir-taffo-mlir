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
!cir.double      -> f64
!cir.bool        -> i1
cir.func         -> func.func
cir.const(fp)    -> arith.constant
cir.unary(minus) -> arith.negf
cir.call(direct) -> func.call
cir.binop(add)   -> arith.addf
cir.binop(sub)   -> arith.subf
cir.binop(mul)   -> arith.mulf
cir.binop(div)   -> arith.divf
cir.br           -> cf.br
cir.brcond       -> cf.cond_br
cir.return       -> func.return
```

The initial pass converts complete floating-point functions using addition,
subtraction, multiplication, and division without leaving CIR operations in
the output. Unsupported CIR operations cause the conversion to fail. The tool
setup and conversions are covered by `lit` and `FileCheck` regression tests.
All four arithmetic paths have also been validated from C source through Clang
CIR generation and `mem2reg`. Range-annotated addition, multiplication, and
division have been validated through CIR conversion, TAFFO-MLIR raising and
optimization, and lowering back to `arith`. Boolean-controlled branching with
supported arithmetic in each branch has also been validated through the same
pipeline.

## Next Steps

The next step is to expand the supported CIR subset while continuing to
validate it through the TAFFO-MLIR pipeline.

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
