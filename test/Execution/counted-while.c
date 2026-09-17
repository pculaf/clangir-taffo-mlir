// RUN: clang -fclangir -emit-cir %S/../Integration/counted-while-loop.c -o %t.cir
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

extern float accumulate_while(int n, float x);
extern float accumulate_fixed_while(float x);

// This tests frontend execution. TAFFO optimization is not run here.
float set_range(float value, double min, double max, double precision) {
  return value;
}

int main(void) {
  const struct {
    int bound;
    float expected;
  } cases[] = {
      {-1, 0.0f}, // Initially false condition.
      {0, 0.0f},  // Empty iteration interval.
      {1, 1.5f},  // One addition.
      {4, 6.0f},  // Four additions.
  };

  for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
    float actual = accumulate_while(cases[i].bound, 1.5f);
    if (actual != cases[i].expected) {
      fprintf(stderr, "n=%d: expected %g, got %g\n", cases[i].bound,
              cases[i].expected, actual);
      return 1;
    }
  }

  float fixed = accumulate_fixed_while(1.5f);
  if (fixed != 6.0f) {
    fprintf(stderr, "fixed while: expected 6, got %g\n", fixed);
    return 1;
  }
  return 0;
}
