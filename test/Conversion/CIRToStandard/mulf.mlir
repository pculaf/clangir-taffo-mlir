// RUN: clangir-taffo-opt --convert-cir-to-standard %s | FileCheck %s

module {
  cir.func @multiply(%arg0: !cir.float, %arg1: !cir.float) -> !cir.float {
    %result = cir.binop(mul, %arg0, %arg1) : !cir.float
    cir.return %result : !cir.float
  }
}

// CHECK-LABEL: func.func @multiply(
// CHECK-SAME:      %[[LHS:.*]]: f32, %[[RHS:.*]]: f32) -> f32
// CHECK:         %[[RESULT:.*]] = arith.mulf %[[LHS]], %[[RHS]] : f32
// CHECK-NEXT:    return %[[RESULT]] : f32
// CHECK-NOT:     cir.
