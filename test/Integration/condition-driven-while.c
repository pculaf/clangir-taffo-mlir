// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: FileCheck %s --check-prefix=CIR --input-file=%t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir \
// RUN:   | clangir-taffo-opt --convert-cir-to-standard \
// RUN:   | mlir-opt --canonicalize --lift-cf-to-scf --canonicalize \
// RUN:   | clangir-taffo-opt --convert-lifted-cf-loops-to-scf-for \
// RUN:   | FileCheck %s --implicit-check-not=scf.for --implicit-check-not=scf.if \
// RUN:       --implicit-check-not=cf.br --implicit-check-not=cf.cond_br

// Division updates the condition variable, so this remains an scf.while.
// Execution is checked in test/Execution/condition-driven-while.c.
extern float set_range(float value, double min, double max, double precision);

float halve_until_one(int n, float x) {
  x = set_range(x, 0.5, 1.5, 0.01);
  float result = set_range(0.0f, 0.0, 0.0, 0.01);
  while (n > 1) {
    result += x;
    n /= 2;
  }
  return result;
}

// CIR-LABEL: cir.func @halve_until_one
// CIR: cir.while

// CHECK-LABEL: func.func @halve_until_one(
// CHECK-SAME: %[[N:.*]]: i32, %[[X:.*]]: f32)
// CHECK-DAG: %[[ONE:.*]] = arith.constant 1 : i32
// CHECK-DAG: %[[TWO:.*]] = arith.constant 2 : i32
// CHECK: %[[INPUT:.*]] = call @set_range(%[[X]],
// CHECK: %[[INITIAL:.*]] = call @set_range(
// CHECK: %[[RESULT:.*]]:2 = scf.while (%[[COND_N:.*]] = %[[N]], %[[COND_ACC:.*]] = %[[INITIAL]])
// CHECK-SAME: : (i32, f32) -> (i32, f32) {
// CHECK-NEXT: %[[COND:.*]] = arith.cmpi sgt, %[[COND_N]], %[[ONE]] : i32
// CHECK-NEXT: scf.condition(%[[COND]]) %[[COND_N]], %[[COND_ACC]] : i32, f32
// CHECK-NEXT: } do {
// CHECK-NEXT: ^bb0(%[[BODY_N:.*]]: i32, %[[ACC:.*]]: f32):
// CHECK-NEXT: %[[NEXT_ACC:.*]] = arith.addf %[[ACC]], %[[INPUT]] : f32
// CHECK-NEXT: %[[NEXT_N:.*]] = arith.divsi %[[BODY_N]], %[[TWO]] : i32
// CHECK-NEXT: scf.yield %[[NEXT_N]], %[[NEXT_ACC]] : i32, f32
// CHECK-NEXT: }
// CHECK-NEXT: return %[[RESULT]]#1 : f32
