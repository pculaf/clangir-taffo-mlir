// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir | clangir-taffo-opt --convert-cir-to-standard | mlir-opt --canonicalize --lift-cf-to-scf --canonicalize | clangir-taffo-opt --convert-lifted-cf-loops-to-scf-for -o %t.mlir
// RUN: FileCheck %s --implicit-check-not=scf.while --implicit-check-not=scf.if --implicit-check-not=cf.br --implicit-check-not=cf.cond_br --input-file=%t.mlir
// RUN: mlir-opt %t.mlir --convert-scf-to-cf --convert-arith-to-llvm --convert-func-to-llvm --convert-cf-to-llvm --reconcile-unrealized-casts | mlir-translate --mlir-to-llvmir -o %t.ll
// RUN: clang -Daccumulate_runtime=reference -c %s -o %t.reference.o
// RUN: clang %t.ll %t.reference.o %S/Inputs/runtime-bound-loop-driver.c -o %t.exe
// RUN: %t.exe

extern float set_range(float value, double min, double max, double precision);

float accumulate_runtime(int n, float x) {
  x = set_range(x, 0.5, 1.5, 0.01);
  float result = set_range(0.0f, 0.0, 0.0, 0.01);
  for (int i = 0; i < n; ++i)
    result += x;
  return result;
}

// CHECK-LABEL: func.func @accumulate_runtime(
// CHECK-SAME: %[[N:[a-zA-Z0-9_]+]]: i32, %[[X:[a-zA-Z0-9_]+]]: f32
// CHECK-DAG: %[[ZERO:.*]] = arith.constant 0 : i32
// CHECK-DAG: %[[ONE:.*]] = arith.constant 1 : i32
// CHECK: %[[INPUT:.*]] = call @set_range(%[[X]],
// CHECK: %[[INITIAL:.*]] = call @set_range(
// CHECK: %[[RESULT:.*]] = scf.for %{{.*}} = %[[ZERO]] to %[[N]] step %[[ONE]] iter_args(%[[ACC:.*]] = %[[INITIAL]]) -> (f32) : i32 {
// CHECK-NEXT: %[[NEXT:.*]] = arith.addf %[[ACC]], %[[INPUT]] : f32
// CHECK-NEXT: scf.yield %[[NEXT]] : f32
// CHECK-NEXT: }
// CHECK-NEXT: return %[[RESULT]] : f32
