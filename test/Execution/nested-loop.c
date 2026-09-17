// RUN: clang -fclangir -emit-cir %S/../Integration/nested-loop.c -o %t.cir
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

extern float nested_sum(float x);

// This tests frontend execution. TAFFO optimization is not run here.
float set_range(float value, double min, double max, double precision) {
  return value;
}

int main(void) {
  const float inputs[] = {0.5f, 1.0f, 1.5f};
  for (unsigned i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
    float expected = 6.0f * inputs[i];
    float actual = nested_sum(inputs[i]);
    if (actual != expected) {
      fprintf(stderr, "x=%g: expected %g, got %g\n",
              inputs[i], expected, actual);
      return 1;
    }
  }
  return 0;
}
