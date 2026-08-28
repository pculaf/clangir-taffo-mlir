// RUN: clangir-taffo-opt --convert-cir-to-standard %s | FileCheck %s

module {
  cir.func @forward(%value: !cir.float) -> !cir.float {
    cir.br ^exit(%value : !cir.float)
  ^exit(%forwarded: !cir.float):
    cir.return %forwarded : !cir.float
  }

  cir.func @select(%condition: !cir.bool, %lhs: !cir.float,
                   %rhs: !cir.float) -> !cir.float {
    cir.brcond %condition ^merge(%lhs : !cir.float),
                               ^merge(%rhs : !cir.float)
  ^merge(%value: !cir.float):
    cir.return %value : !cir.float
  }
}

// CHECK-LABEL: func.func @forward(
// CHECK-SAME:      %[[VALUE:.*]]: f32) -> f32
// CHECK:         cf.br ^[[EXIT:.*]](%[[VALUE]] : f32)
// CHECK:       ^[[EXIT]](%[[FORWARDED:.*]]: f32):
// CHECK-NEXT:    return %[[FORWARDED]] : f32

// CHECK-LABEL: func.func @select(
// CHECK-SAME:      %[[CONDITION:.*]]: i1, %[[LHS:.*]]: f32,
// CHECK-SAME:      %[[RHS:.*]]: f32) -> f32
// CHECK:         cf.cond_br %[[CONDITION]],
// CHECK-SAME:      ^[[MERGE:.*]](%[[LHS]] : f32),
// CHECK-SAME:      ^[[MERGE]](%[[RHS]] : f32)
// CHECK:       ^[[MERGE]](%[[SELECTED:.*]]: f32):
// CHECK-NEXT:    return %[[SELECTED]] : f32
// CHECK-NOT:     cir.br
// CHECK-NOT:     cir.brcond
