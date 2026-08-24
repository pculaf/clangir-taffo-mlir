// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: cir-opt --mem2reg %t.cir | clangir-taffo-opt --convert-cir-to-standard | FileCheck %s

float subtract(float lhs, float rhs) { return lhs - rhs; }

// CHECK-LABEL: func.func @subtract(
// CHECK-SAME:      %[[LHS:.*]]: f32, %[[RHS:.*]]: f32) -> f32
// CHECK:         %[[RESULT:.*]] = arith.subf %[[LHS]], %[[RHS]] : f32
// CHECK-NEXT:    return %[[RESULT]] : f32
// CHECK-NOT:     cir.func
// CHECK-NOT:     cir.binop
// CHECK-NOT:     cir.return
