// RUN: clangir-taffo-opt --convert-lifted-cf-loops-to-scf-for %s | FileCheck %s --implicit-check-not=scf.for

// Normalization discards the first two results of this lifted shape. If one
// is used, the pattern must preserve the original loop instead.
module {
  func.func @live_intermediate(%input: f32) -> f32 {
    %step = arith.constant 1 : i32
    %lower = arith.constant 0 : i32
    %upper = arith.constant 4 : i32
    %initial = arith.constant 1.0 : f32
    %exit_value = arith.constant 7.0 : f32
    %result:3 = scf.while (%acc = %initial, %i = %lower)
        : (f32, i32) -> (f32, i32, f32) {
      %condition = arith.cmpi slt, %i, %upper : i32
      %next:2 = scf.if %condition -> (f32, i32) {
        %next_acc = arith.addf %acc, %input : f32
        %next_i = arith.addi %i, %step overflow<nsw> : i32
        scf.yield %next_acc, %next_i : f32, i32
      } else {
        scf.yield %exit_value, %i : f32, i32
      }
      scf.condition(%condition) %next#0, %next#1, %acc : f32, i32, f32
    } do {
    ^bb0(%acc: f32, %i: i32, %exit_acc: f32):
      scf.yield %acc, %i : f32, i32
    }
    return %result#0 : f32
  }
}

// CHECK-LABEL: func.func @live_intermediate(
// CHECK: %[[EXIT:.*]] = arith.constant 7.000000e+00 : f32
// CHECK: %[[RESULT:.*]]:3 = scf.while
// CHECK: %[[NEXT:.*]]:2 = scf.if
// CHECK: } else {
// CHECK-NEXT: scf.yield %[[EXIT]], %{{.*}} : f32, i32
// CHECK: scf.condition(%{{.*}}) %[[NEXT]]#0, %[[NEXT]]#1, %{{.*}} : f32, i32, f32
// CHECK: return %[[RESULT]]#0 : f32
