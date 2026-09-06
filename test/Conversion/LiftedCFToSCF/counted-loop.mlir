// RUN: clangir-taffo-opt --convert-lifted-cf-loops-to-scf-for %s | FileCheck %s

module {
  func.func @accumulate(%input: f32) -> f32 {
    %poison_i32 = ub.poison : i32
    %poison_f32 = ub.poison : f32
    %step = arith.constant 1 : i32
    %lower = arith.constant 0 : i32
    %upper = arith.constant 4 : i32
    %initial = arith.constant 1.0 : f32
    %result:3 = scf.while (%acc = %initial, %i = %lower)
        : (f32, i32) -> (f32, i32, f32) {
      %condition = arith.cmpi slt, %i, %upper : i32
      %next:2 = scf.if %condition -> (f32, i32) {
        %next_acc = arith.addf %acc, %input : f32
        %next_i = arith.addi %i, %step overflow<nsw> : i32
        scf.yield %next_acc, %next_i : f32, i32
      } else {
        scf.yield %poison_f32, %poison_i32 : f32, i32
      }
      scf.condition(%condition) %next#0, %next#1, %acc : f32, i32, f32
    } do {
    ^bb0(%acc: f32, %i: i32, %exit_acc: f32):
      scf.yield %acc, %i : f32, i32
    }
    return %result#2 : f32
  }
}

// CHECK-LABEL: func.func @accumulate(
// CHECK:         %[[RESULT:.*]] = scf.for %[[I:.*]] = %{{.*}} to %{{.*}} step %{{.*}} iter_args(%[[ACC:.*]] = %{{.*}})
// CHECK:           %[[NEXT:.*]] = arith.addf %[[ACC]], %{{.*}} : f32
// CHECK:           scf.yield %[[NEXT]] : f32
// CHECK:         }
// CHECK:         return %[[RESULT]] : f32
// CHECK-NOT:     scf.while
// CHECK-NOT:     scf.if
