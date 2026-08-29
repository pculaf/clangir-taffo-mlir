// RUN: clangir-taffo-opt --convert-cir-to-standard %s | FileCheck %s

module {
  cir.func @comparisons(%lhs: !cir.float, %rhs: !cir.float) -> !cir.bool {
    %lt = cir.cmp(lt, %lhs, %rhs) : !cir.float, !cir.bool
    %le = cir.cmp(le, %lhs, %rhs) : !cir.float, !cir.bool
    %gt = cir.cmp(gt, %lhs, %rhs) : !cir.float, !cir.bool
    %ge = cir.cmp(ge, %lhs, %rhs) : !cir.float, !cir.bool
    %eq = cir.cmp(eq, %lhs, %rhs) : !cir.float, !cir.bool
    %ne = cir.cmp(ne, %lhs, %rhs) : !cir.float, !cir.bool
    cir.return %ne : !cir.bool
  }

  cir.func @float_truth(%value: !cir.float) -> !cir.bool {
    %condition = cir.cast(float_to_bool, %value : !cir.float), !cir.bool
    cir.return %condition : !cir.bool
  }

  cir.func @double_truth(%value: !cir.double) -> !cir.bool {
    %condition = cir.cast(float_to_bool, %value : !cir.double), !cir.bool
    cir.return %condition : !cir.bool
  }
}

// CHECK-LABEL: func.func @comparisons(
// CHECK-SAME:      %[[LHS:.*]]: f32, %[[RHS:.*]]: f32) -> i1
// CHECK:         arith.cmpf olt, %[[LHS]], %[[RHS]] : f32
// CHECK:         arith.cmpf ole, %[[LHS]], %[[RHS]] : f32
// CHECK:         arith.cmpf ogt, %[[LHS]], %[[RHS]] : f32
// CHECK:         arith.cmpf oge, %[[LHS]], %[[RHS]] : f32
// CHECK:         arith.cmpf oeq, %[[LHS]], %[[RHS]] : f32
// CHECK:         %[[NE:.*]] = arith.cmpf une, %[[LHS]], %[[RHS]] : f32
// CHECK:         return %[[NE]] : i1

// CHECK-LABEL: func.func @float_truth(
// CHECK-SAME:      %[[FLOAT:.*]]: f32) -> i1
// CHECK:         %[[FZERO:.*]] = arith.constant 0.000000e+00 : f32
// CHECK:         %[[FCOND:.*]] = arith.cmpf une, %[[FLOAT]], %[[FZERO]] : f32
// CHECK:         return %[[FCOND]] : i1

// CHECK-LABEL: func.func @double_truth(
// CHECK-SAME:      %[[DOUBLE:.*]]: f64) -> i1
// CHECK:         %[[DZERO:.*]] = arith.constant 0.000000e+00 : f64
// CHECK:         %[[DCOND:.*]] = arith.cmpf une, %[[DOUBLE]], %[[DZERO]] : f64
// CHECK:         return %[[DCOND]] : i1
// CHECK-NOT:     cir.cmp
// CHECK-NOT:     cir.cast
