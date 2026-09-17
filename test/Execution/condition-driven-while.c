// RUN: clang -fclangir -emit-cir %S/../Integration/condition-driven-while.c -o %t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir \
// RUN:   | clangir-taffo-opt --convert-cir-to-standard \
// RUN:   | mlir-opt --canonicalize --lift-cf-to-scf --canonicalize \
// RUN:   | clangir-taffo-opt --convert-lifted-cf-loops-to-scf-for \
// RUN:   | mlir-opt --convert-scf-to-cf --convert-arith-to-llvm \
// RUN:       --convert-func-to-llvm --convert-cf-to-llvm --reconcile-unrealized-casts \
// RUN:   | mlir-translate --mlir-to-llvmir -o %t.ll
// RUN: clang %t.ll %s -o %t.exe
// RUN: %t.exe

#include <stdio.h>

extern float halve_until_one(int n, float x);

// This tests frontend execution. TAFFO optimization is not run here.
float set_range(float value, double min, double max, double precision) {
  return value;
}

int main(void) {
  const struct {
    int input;
    float expected;
  } cases[] = {
      {-1, 0.0f}, // Initially false condition.
      {1, 0.0f},  // Boundary of the condition.
      {2, 1.5f},  // One iteration: 2 -> 1.
      {7, 3.0f},  // Integer division: 7 -> 3 -> 1.
      {8, 4.5f},  // Three iterations: 8 -> 4 -> 2 -> 1.
  };

  for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
    float actual = halve_until_one(cases[i].input, 1.5f);
    if (actual != cases[i].expected) {
      fprintf(stderr, "n=%d: expected %g, got %g\n", cases[i].input,
              cases[i].expected, actual);
      return 1;
    }
  }
  return 0;
}
