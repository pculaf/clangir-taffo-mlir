// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir \
// RUN:   | clangir-taffo-opt --convert-cir-to-standard \
// RUN:   | mlir-opt --canonicalize --lift-cf-to-scf --canonicalize \
// RUN:   | clangir-taffo-opt --convert-lifted-cf-loops-to-scf-for \
// RUN:   | FileCheck %s --implicit-check-not=scf.while \
// RUN:       --implicit-check-not=cf.br --implicit-check-not=cf.cond_br

extern float set_range(float value, double min, double max, double precision);

float conditional_sum(float x) {
  x = set_range(x, 0.5, 1.5, 0.01);
  float result = set_range(0.0f, 0.0, 0.0, 0.01);
  for (int i = 0; i < 4; ++i) {
    if (i < 2)
      result += x;
  }
  return result;
}

// CHECK-LABEL: func.func @conditional_sum(
// CHECK-DAG: %[[ZERO:.*]] = arith.constant 0 : i32
// CHECK-DAG: %[[ONE:.*]] = arith.constant 1 : i32
// CHECK-DAG: %[[TWO:.*]] = arith.constant 2 : i32
// CHECK-DAG: %[[FOUR:.*]] = arith.constant 4 : i32
// CHECK: %[[INPUT:.*]] = call @set_range(
// CHECK: %[[INITIAL:.*]] = call @set_range(
// CHECK: %[[LOOP:.*]] = scf.for %[[I:.*]] = %[[ZERO]] to %[[FOUR]] step %[[ONE]]
// CHECK-SAME: iter_args(%[[ACC:.*]] = %[[INITIAL]]) -> (f32)
// CHECK-NEXT: %[[COND:.*]] = arith.cmpi slt, %[[I]], %[[TWO]] : i32
// CHECK-NEXT: %[[SELECTED:.*]] = scf.if %[[COND]] -> (f32) {
// CHECK-NEXT: %[[SUM:.*]] = arith.addf %[[ACC]], %[[INPUT]] : f32
// CHECK-NEXT: scf.yield %[[SUM]] : f32
// CHECK-NEXT: } else {
// CHECK-NEXT: scf.yield %[[ACC]] : f32
// CHECK-NEXT: }
// CHECK-NEXT: scf.yield %[[SELECTED]] : f32
// CHECK-NEXT: }
// CHECK-NEXT: return %[[LOOP]] : f32
