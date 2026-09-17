// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir | clangir-taffo-opt --convert-cir-to-standard | mlir-opt --canonicalize --lift-cf-to-scf --canonicalize | clangir-taffo-opt --convert-lifted-cf-loops-to-scf-for -o %t.mlir
// RUN: FileCheck %s --implicit-check-not=scf.while --implicit-check-not=scf.if --implicit-check-not=cf.br --implicit-check-not=cf.cond_br --input-file=%t.mlir
// RUN: mlir-opt %t.mlir --convert-scf-to-cf --convert-arith-to-llvm --convert-func-to-llvm --convert-cf-to-llvm --reconcile-unrealized-casts | mlir-translate --mlir-to-llvmir -o %t.ll
// RUN: clang -Daccumulate_strided=reference_strided -Daccumulate_fixed_stride=reference_fixed_stride -c %s -o %t.reference.o
// RUN: clang %t.ll %t.reference.o %S/Inputs/strided-accumulator-loop-driver.c -o %t.exe
// RUN: %t.exe

extern float set_range(float value, double min, double max, double precision);

float accumulate_strided(int n, float x) {
  x = set_range(x, 0.5, 1.5, 0.01);
  float result = set_range(0.0f, 0.0, 0.0, 0.01);
  for (int i = 2; i < n; i += 3)
    result += x;
  return result;
}

float accumulate_fixed_stride(float x) {
  x = set_range(x, 0.5, 1.5, 0.01);
  float result = set_range(0.0f, 0.0, 0.0, 0.01);
  for (int i = 2; i < 10; i += 3)
    result += x;
  return result;
}

// CHECK-LABEL: func.func @accumulate_strided(
// CHECK-SAME: %[[N:[a-zA-Z0-9_]+]]: i32, %[[X:[a-zA-Z0-9_]+]]: f32
// CHECK-DAG: %[[START:.*]] = arith.constant 2 : i32
// CHECK-DAG: %[[STEP:.*]] = arith.constant 3 : i32
// CHECK: %[[INPUT:.*]] = call @set_range(%[[X]],
// CHECK: %[[INITIAL:.*]] = call @set_range(
// CHECK: %[[RESULT:.*]] = scf.for %{{.*}} = %[[START]] to %[[N]] step %[[STEP]] iter_args(%[[ACC:.*]] = %[[INITIAL]]) -> (f32) : i32 {
// CHECK-NEXT: %[[NEXT:.*]] = arith.addf %[[ACC]], %[[INPUT]] : f32
// CHECK-NEXT: scf.yield %[[NEXT]] : f32
// CHECK-NEXT: }
// CHECK-NEXT: return %[[RESULT]] : f32

// CHECK-LABEL: func.func @accumulate_fixed_stride(
// CHECK-DAG: %[[START:.*]] = arith.constant 2 : i32
// CHECK-DAG: %[[END:.*]] = arith.constant 10 : i32
// CHECK-DAG: %[[STEP:.*]] = arith.constant 3 : i32
// CHECK: %[[INPUT:.*]] = call @set_range(
// CHECK: %[[INITIAL:.*]] = call @set_range(
// CHECK: %[[RESULT:.*]] = scf.for %{{.*}} = %[[START]] to %[[END]] step %[[STEP]] iter_args(%[[ACC:.*]] = %[[INITIAL]]) -> (f32) : i32 {
// CHECK-NEXT: %[[NEXT:.*]] = arith.addf %[[ACC]], %[[INPUT]] : f32
// CHECK-NEXT: scf.yield %[[NEXT]] : f32
// CHECK-NEXT: }
// CHECK-NEXT: return %[[RESULT]] : f32
