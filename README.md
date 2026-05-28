# TAFFO-MLIR / ClangIR Integration

This project investigates how TAFFO-MLIR could be connected to a ClangIR-based
C/C++ frontend.

The current work focuses on validating the relevant LLVM/ClangIR and TAFFO-MLIR
toolchains, documenting the available lowering paths, and identifying the first
practical integration point.

## Current Status

The current work has established a working TAFFO-MLIR setup and evaluated the
ClangIR/CIR tools available in both the TAFFO-pinned LLVM snapshot and a current
top-of-tree LLVM build.

The main result so far is that current ClangIR tools can emit CIR and lower CIR
to LLVM-level representations, but do not expose a path from CIR to the
high-level MLIR dialects needed for the planned TAFFO-MLIR integration work.

## Toolchain State

TAFFO-MLIR was built successfully against the LLVM snapshot requested by the
TAFFO-MLIR repository:

- commit: `1f5b6ae89fbc88d22c323fa56d8bdad9f7b695c3`
- version: `21.0.0git`
- ClangIR/CIR support enabled with `CLANG_ENABLE_CIR=ON`

A separate top-of-tree LLVM/ClangIR build was also used to inspect the current
upstream ClangIR/CIR state:

- branch: `main`
- commit: `0f79ba29f371a1acc0b592fdaf58a58a77f0496a`
- version: `23.0.0git`
- ClangIR/CIR support enabled with `CLANG_ENABLE_CIR=ON`

Observed ClangIR/CIR capabilities:

- `clang -fclangir -emit-cir` emits CIR MLIR from C/C++ input.
- `clang -fclangir -emit-llvm` emits textual LLVM IR through ClangIR.
- `cir-opt --cir-to-llvm` lowers CIR to LLVM dialect MLIR.
- `cir-translate --cir-to-llvmir` translates CIR to textual LLVM IR.

The currently exposed pipeline is:

```text
C/C++ -> CIR
CIR -> LLVM dialect MLIR
CIR -> LLVM IR
```

No exposed tool path was found for:

```text
CIR -> func/arith/scf/cf/memref
```

An internal pass-dump check also did not show a hidden high-level MLIR lowering
stage in the tested top-of-tree build.

## Current Conclusion

Both checked ClangIR toolchains can produce CIR and can lower CIR to
LLVM-level representations.

No exposed tool path was found for lowering CIR to high-level MLIR dialects
such as `func`, `arith`, `scf`, `cf`, and `memref`.

The incubator `llvm/clangir` repository appears to contain through-MLIR lowering
infrastructure, but that path was not present or usable in the official
top-of-tree LLVM build tested locally. It should be treated as reference
material unless the project explicitly decides to build and evaluate that
repository separately.

## Next Steps

The next implementation step is to define an initial CIR subset and lower it to
standard MLIR dialects, starting with arithmetic operations.

Planned work:

- define the first supported CIR subset
- generate representative CIR inputs with ClangIR
- implement an initial CIR-to-standard-MLIR lowering experiment
- validate the produced MLIR with MLIR and TAFFO-MLIR tools
- document supported operations and current limitations

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
