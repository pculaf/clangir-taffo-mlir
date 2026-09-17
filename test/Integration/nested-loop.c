// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir \
// RUN:   | clangir-taffo-opt --convert-cir-to-standard \
// RUN:   | mlir-opt --canonicalize --lift-cf-to-scf --canonicalize \
// RUN:   | clangir-taffo-opt --convert-lifted-cf-loops-to-scf-for \
// RUN:   | FileCheck %s --implicit-check-not=scf.while \
// RUN:       --implicit-check-not=cf.br --implicit-check-not=cf.cond_br

extern float set_range(float value, double min, double max, double precision);

// Frontend coverage only; TAFFO currently underestimates this accumulator range.
float nested_sum(float x) {
  x = set_range(x, 0.5, 1.5, 0.01);
  float result = set_range(0.0f, 0.0, 0.0, 0.01);
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
      result += x;
    }
  }
  return result;
}

// CHECK-LABEL: func.func @nested_sum(
// CHECK-DAG: %[[ZERO:.*]] = arith.constant 0 : i32
// CHECK-DAG: %[[ONE:.*]] = arith.constant 1 : i32
// CHECK-DAG: %[[TWO:.*]] = arith.constant 2 : i32
// CHECK-DAG: %[[THREE:.*]] = arith.constant 3 : i32
// CHECK: %[[INPUT:.*]] = call @set_range(
// CHECK: %[[INITIAL:.*]] = call @set_range(
// CHECK: %[[OUTER:.*]] = scf.for %{{.*}} = %[[ZERO]] to %[[TWO]] step %[[ONE]]
// CHECK-SAME: iter_args(%[[OUTER_ACC:.*]] = %[[INITIAL]]) -> (f32)
// CHECK-NEXT: %[[INNER:.*]] = scf.for %{{.*}} = %[[ZERO]] to %[[THREE]] step %[[ONE]]
// CHECK-SAME: iter_args(%[[INNER_ACC:.*]] = %[[OUTER_ACC]]) -> (f32)
// CHECK-NEXT: %[[SUM:.*]] = arith.addf %[[INNER_ACC]], %[[INPUT]] : f32
// CHECK-NEXT: scf.yield %[[SUM]] : f32
// CHECK-NEXT: }
// CHECK-NEXT: scf.yield %[[INNER]] : f32
// CHECK-NEXT: }
// CHECK-NEXT: return %[[OUTER]] : f32
