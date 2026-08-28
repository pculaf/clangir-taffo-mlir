// RUN: clangir-taffo-opt --convert-cir-to-standard %s | FileCheck %s

module {
  cir.func @select(%condition: !cir.bool, %lhs: !cir.float,
                   %rhs: !cir.float) -> !cir.float {
    cir.br ^decision
  ^decision:
    cir.brcond %condition ^then, ^else
  ^then:
    cir.return %lhs : !cir.float
  ^else:
    cir.return %rhs : !cir.float
  }
}

// CHECK-LABEL: func.func @select(
// CHECK-SAME:      %[[CONDITION:.*]]: i1, %[[LHS:.*]]: f32,
// CHECK-SAME:      %[[RHS:.*]]: f32) -> f32
// CHECK:         cf.br ^[[DECISION:.*]]
// CHECK:       ^[[DECISION]]:
// CHECK-NEXT:    cf.cond_br %[[CONDITION]], ^[[THEN:.*]], ^[[ELSE:.*]]
// CHECK:       ^[[THEN]]:
// CHECK-NEXT:    return %[[LHS]] : f32
// CHECK:       ^[[ELSE]]:
// CHECK-NEXT:    return %[[RHS]] : f32
// CHECK-NOT:     cir.br
// CHECK-NOT:     cir.brcond
