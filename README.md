# TAFFO-MLIR / ClangIR Integration

This project investigates how TAFFO-MLIR could be connected to a ClangIR-based
C/C++ frontend.

The current work focuses on validating the relevant LLVM/ClangIR and TAFFO-MLIR
toolchains, documenting the available lowering paths, and identifying the first
practical integration point.

## Current Status

TAFFO-MLIR has been built successfully against its pinned LLVM/MLIR snapshot,
with ClangIR/CIR support enabled. A separate top-of-tree LLVM build was also
used to check the current upstream ClangIR state.

At the current checked LLVM revisions, ClangIR can emit CIR and lower CIR to
LLVM-level representations. A direct path from CIR to high-level MLIR dialects
does not appear to be exposed by the tested tools.

The currently exposed ClangIR path is:

```text
C/C++ -> CIR
CIR -> LLVM dialect MLIR
CIR -> LLVM IR
```

## Current Conclusion

In the currently tested toolchains, lowering CIR to high-level MLIR dialects
such as `func`, `arith`, `scf`, `cf`, and `memref` does not appear to be
available as an exposed tool path.

The incubator `llvm/clangir` repository appears to contain through-MLIR lowering
infrastructure. In the official top-of-tree LLVM build checked here, that path
does not appear to be available in the tested tools.

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
