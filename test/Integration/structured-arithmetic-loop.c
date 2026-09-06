// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir | clangir-taffo-opt --convert-cir-to-standard | mlir-opt --canonicalize --lift-cf-to-scf --canonicalize | clangir-taffo-opt --convert-lifted-cf-loops-to-scf-for | FileCheck %s

extern float set_range(float value, double min, double max, double precision);

float accumulate(float x) {
  x = set_range(x, 0.5, 1.5, 0.01);
  float result = set_range(1.0f, 1.0, 1.0, 0.01);

  for (int i = 0; i < 4; ++i)
    result += x;

  return result;
}

// CHECK-LABEL: func.func @accumulate(
// CHECK:         %[[INPUT:.*]] = call @set_range
// CHECK:         %[[INITIAL:.*]] = call @set_range
// CHECK:         %[[RESULT:.*]] = scf.for %{{.*}} = %{{.*}} to %{{.*}} step %{{.*}} iter_args(%[[ACC:.*]] = %[[INITIAL]])
// CHECK:           %[[NEXT:.*]] = arith.addf %[[ACC]], %[[INPUT]] : f32
// CHECK:           scf.yield %[[NEXT]] : f32
// CHECK:         }
// CHECK:         return %[[RESULT]] : f32
// CHECK-NOT:     scf.while
// CHECK-NOT:     scf.if
// CHECK-NOT:     cf.
// CHECK-NOT:     cir.
