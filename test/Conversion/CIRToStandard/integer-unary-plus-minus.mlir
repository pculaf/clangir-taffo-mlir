// RUN: clangir-taffo-opt --convert-cir-to-standard %s | FileCheck %s

!s32i = !cir.int<s, 32>
!u32i = !cir.int<u, 32>

cir.func @signed_plus_minus(%value: !s32i) -> !s32i {
  %positive = cir.unary(plus, %value) : !s32i, !s32i
  %negative = cir.unary(minus, %positive) nsw : !s32i, !s32i
  cir.return %negative : !s32i
}

cir.func @unsigned_minus(%value: !u32i) -> !u32i {
  %negative = cir.unary(minus, %value) : !u32i, !u32i
  cir.return %negative : !u32i
}

// CHECK-LABEL: func.func @signed_plus_minus(
// CHECK-SAME:      %[[VALUE:.*]]: i32) -> i32
// CHECK-NEXT:    %[[ZERO:.*]] = arith.constant 0 : i32
// CHECK-NEXT:    %[[NEGATIVE:.*]] = arith.subi %[[ZERO]], %[[VALUE]] overflow<nsw> : i32
// CHECK-NEXT:    return %[[NEGATIVE]] : i32

// CHECK-LABEL: func.func @unsigned_minus(
// CHECK-SAME:      %[[VALUE:.*]]: i32) -> i32
// CHECK-NEXT:    %[[ZERO:.*]] = arith.constant 0 : i32
// CHECK-NEXT:    %[[NEGATIVE:.*]] = arith.subi %[[ZERO]], %[[VALUE]] : i32
// CHECK-NEXT:    return %[[NEGATIVE]] : i32
// CHECK-NOT:     cir.
