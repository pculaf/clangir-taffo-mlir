# TAFFO-MLIR / ClangIR Integration

This project investigates how TAFFO-MLIR could be connected to a ClangIR-based
C/C++ frontend.

The broader goal is to understand what integration path is possible between
ClangIR and TAFFO-MLIR. The current work focuses on building the relevant
toolchains and documenting what they can actually emit today.

## Current Status

Two LLVM/ClangIR configurations have been built and checked.

The first one is the LLVM version pinned by TAFFO-MLIR. TAFFO-MLIR builds
successfully against this version, so it remains the working TAFFO-MLIR setup.

The second one is a separate top-of-tree LLVM build. It was built to inspect the
current upstream ClangIR/CIR toolchain state without disturbing the working
TAFFO-MLIR build.

## Working TAFFO-MLIR Build

TAFFO-MLIR was built successfully against the LLVM snapshot requested by the
TAFFO-MLIR repository.

LLVM configuration:

- commit: `1f5b6ae89fbc88d22c323fa56d8bdad9f7b695c3`
- version: `21.0.0git`
- build type: Debug with assertions
- ClangIR/CIR support enabled with `CLANG_ENABLE_CIR=ON`

TAFFO-MLIR status:

- TAFFO-MLIR builds successfully against this LLVM/MLIR version.
- `taffo-opt` is available from the TAFFO-MLIR build.
- This is currently the usable TAFFO-MLIR toolchain.

ClangIR/CIR tools available in this build:

- `cir-opt`
- `cir-translate`
- `cir-lsp-server`

Observed capabilities:

- `clang -S -Xclang -emit-cir` can emit CIR MLIR from C input.
- `cir-opt --cir-to-llvm` lowers CIR to LLVM dialect MLIR.
- `cir-translate --cir-to-llvmir` translates CIR to textual LLVM IR.

## Top-Of-Tree ClangIR Check

A separate top-of-tree LLVM/Clang/MLIR/CIR build was also created.

LLVM configuration:

- branch: `main`
- commit: `0f79ba29f371a1acc0b592fdaf58a58a77f0496a`
- version: `23.0.0git`
- build type: Debug with assertions
- ClangIR/CIR support enabled with `CLANG_ENABLE_CIR=ON`

Installed tools include:

- `clang`
- `mlir-opt`
- `cir-opt`
- `cir-translate`
- `cir-lsp-server`
- `lld`
- `ld.lld`

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

An internal pass-dump check also did not show a hidden high-level MLIR stage.
The IR stayed in `cir.*` until direct conversion to `llvm.*`.

## TAFFO-MLIR Against Top-Of-Tree LLVM

TAFFO-MLIR was also configured against the top-of-tree LLVM install.

Result:

- configure succeeded
- compilation failed because of LLVM/MLIR API changes
- no source fixes were attempted

The main observed incompatibilities were:

- changed MLIR dataflow analysis API signatures
- changed `mlir::constantTripCount(...)` signature

Therefore, the working TAFFO-MLIR build remains the one based on the
TAFFO-pinned LLVM snapshot. The top-of-tree LLVM build is currently useful for
investigating ClangIR/CIR, but not for building TAFFO-MLIR directly.

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

## Reference Files

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
