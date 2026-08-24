// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: cir-opt --mem2reg %t.cir | clangir-taffo-opt --convert-cir-to-standard | FileCheck %s --check-prefix=CONVERT
// RUN: cir-opt --mem2reg %t.cir | clangir-taffo-opt --convert-cir-to-standard | mlir-opt --canonicalize | FileCheck %s --check-prefix=CANON

extern float set_range(float value, double min, double max, double precision);

float ranged_add(float a, float b) {
  a = set_range(a, -1.0, 1.0, 0.01);
  b = set_range(b, 0.0, 2.0, 0.01);
  return a + b;
}

float ranged_mul(float a, float b) {
  a = set_range(a, -2.0, 2.0, 0.01);
  b = set_range(b, 0.5, 1.5, 0.01);
  return a * b;
}

float ranged_div(float a, float b) {
  a = set_range(a, -2.0, 2.0, 0.01);
  b = set_range(b, 0.5, 1.5, 0.01);
  return a / b;
}

// CONVERT:      func.func private @set_range(f32, f64, f64, f64) -> f32
// CONVERT-LABEL: func.func @ranged_add(
// CONVERT:      arith.constant
// CONVERT:      arith.negf
// CONVERT:      call @set_range
// CONVERT:      arith.addf
// CONVERT-LABEL: func.func @ranged_mul(
// CONVERT:      call @set_range
// CONVERT:      arith.mulf
// CONVERT-LABEL: func.func @ranged_div(
// CONVERT:      call @set_range
// CONVERT:      arith.divf
// CONVERT-NOT:  cir.

// CANON-LABEL: func.func @ranged_add(
// CANON:       arith.constant -1.000000e+00 : f64
// CANON-NOT:   arith.negf
// CANON-LABEL: func.func @ranged_mul(
// CANON:       arith.constant -2.000000e+00 : f64
// CANON-NOT:   arith.negf
// CANON-LABEL: func.func @ranged_div(
// CANON:       arith.constant -2.000000e+00 : f64
// CANON-NOT:   arith.negf
