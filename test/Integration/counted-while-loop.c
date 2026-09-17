// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: FileCheck %s --check-prefix=CIR --input-file=%t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir \
// RUN:   | clangir-taffo-opt --convert-cir-to-standard \
// RUN:   | mlir-opt --canonicalize --lift-cf-to-scf --canonicalize \
// RUN:   | clangir-taffo-opt --convert-lifted-cf-loops-to-scf-for \
// RUN:   | FileCheck %s --implicit-check-not=scf.while --implicit-check-not=scf.if \
// RUN:       --implicit-check-not=cf.br --implicit-check-not=cf.cond_br

// Conversion coverage; test/Execution/counted-while.c checks execution.

extern float set_range(float value, double min, double max, double precision);

float accumulate_while(int n, float x) {
  x = set_range(x, 0.5, 1.5, 0.01);
  float result = set_range(0.0f, 0.0, 0.0, 0.01);
  int i = 0;
  while (i < n) {
    result += x;
    ++i;
  }
  return result;
}

float accumulate_fixed_while(float x) {
  x = set_range(x, 0.5, 1.5, 0.01);
  float result = set_range(0.0f, 0.0, 0.0, 0.01);
  int i = 0;
  while (i < 4) {
    result += x;
    ++i;
  }
  return result;
}

// CIR-LABEL: cir.func @accumulate_while(
// CIR: cir.while
// CIR-LABEL: cir.func @accumulate_fixed_while(
// CIR: cir.while

// CHECK-LABEL: func.func @accumulate_while(
// CHECK-SAME: %[[N:[a-zA-Z0-9_]+]]: i32, %[[X:[a-zA-Z0-9_]+]]: f32
// CHECK-DAG: %[[ZERO:.*]] = arith.constant 0 : i32
// CHECK-DAG: %[[ONE:.*]] = arith.constant 1 : i32
// CHECK: %[[INPUT:.*]] = call @set_range(%[[X]],
// CHECK: %[[INITIAL:.*]] = call @set_range(
// CHECK: %[[RESULT:.*]] = scf.for %{{.*}} = %[[ZERO]] to %[[N]] step %[[ONE]]
// CHECK-SAME: iter_args(%[[ACC:.*]] = %[[INITIAL]]) -> (f32) : i32 {
// CHECK-NEXT: %[[NEXT:.*]] = arith.addf %[[ACC]], %[[INPUT]] : f32
// CHECK-NEXT: scf.yield %[[NEXT]] : f32
// CHECK-NEXT: }
// CHECK-NEXT: return %[[RESULT]] : f32

// CHECK-LABEL: func.func @accumulate_fixed_while(
// CHECK-DAG: %[[ZERO:.*]] = arith.constant 0 : i32
// CHECK-DAG: %[[END:.*]] = arith.constant 4 : i32
// CHECK-DAG: %[[ONE:.*]] = arith.constant 1 : i32
// CHECK: %[[INPUT:.*]] = call @set_range(
// CHECK: %[[INITIAL:.*]] = call @set_range(
// CHECK: %[[RESULT:.*]] = scf.for %{{.*}} = %[[ZERO]] to %[[END]] step %[[ONE]]
// CHECK-SAME: iter_args(%[[ACC:.*]] = %[[INITIAL]]) -> (f32) : i32 {
// CHECK-NEXT: %[[NEXT:.*]] = arith.addf %[[ACC]], %[[INPUT]] : f32
// CHECK-NEXT: scf.yield %[[NEXT]] : f32
// CHECK-NEXT: }
// CHECK-NEXT: return %[[RESULT]] : f32
