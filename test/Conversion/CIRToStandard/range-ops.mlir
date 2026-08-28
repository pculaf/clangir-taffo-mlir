// RUN: clangir-taffo-opt --convert-cir-to-standard %s | FileCheck %s

module {
  cir.func @set_range(!cir.float, !cir.double, !cir.double, !cir.double) -> !cir.float

  cir.func @apply_range(%arg0: !cir.float) -> !cir.float {
    %0 = cir.const #cir.fp<1.000000e+00> : !cir.double
    %1 = cir.unary(minus, %0) : !cir.double, !cir.double
    %2 = cir.const #cir.fp<2.000000e+00> : !cir.double
    %3 = cir.const #cir.fp<1.000000e-02> : !cir.double
    %4 = cir.call @set_range(%arg0, %1, %2, %3) : (!cir.float, !cir.double, !cir.double, !cir.double) -> !cir.float
    cir.return %4 : !cir.float
  }
}

// CHECK:       func.func private @set_range(f32, f64, f64, f64) -> f32
// CHECK-LABEL: func.func @apply_range(
// CHECK-SAME:      %[[ARG:.*]]: f32) -> f32
// CHECK:         %[[ONE:.*]] = arith.constant 1.000000e+00 : f64
// CHECK:         %[[MIN:.*]] = arith.negf %[[ONE]] : f64
// CHECK:         %[[MAX:.*]] = arith.constant 2.000000e+00 : f64
// CHECK:         %[[PRECISION:.*]] = arith.constant 1.000000e-02 : f64
// CHECK:         %[[RANGED:.*]] = call @set_range(%[[ARG]], %[[MIN]], %[[MAX]], %[[PRECISION]]) : (f32, f64, f64, f64) -> f32
// CHECK-NEXT:    return %[[RANGED]] : f32
// CHECK-NOT:     cir.
