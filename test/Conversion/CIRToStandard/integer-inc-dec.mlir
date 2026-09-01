// RUN: clangir-taffo-opt --convert-cir-to-standard %s | FileCheck %s

!s32i = !cir.int<s, 32>
!u32i = !cir.int<u, 32>

cir.func @signed_inc_dec(%value: !s32i) -> !s32i {
  %incremented = cir.unary(inc, %value) nsw : !s32i, !s32i
  %decremented = cir.unary(dec, %incremented) nsw : !s32i, !s32i
  cir.return %decremented : !s32i
}

cir.func @unsigned_inc_dec(%value: !u32i) -> !u32i {
  %incremented = cir.unary(inc, %value) : !u32i, !u32i
  %decremented = cir.unary(dec, %incremented) : !u32i, !u32i
  cir.return %decremented : !u32i
}

// CHECK-LABEL: func.func @signed_inc_dec(
// CHECK-SAME:      %[[VALUE:.*]]: i32) -> i32
// CHECK:         %[[ONE:.*]] = arith.constant 1 : i32
// CHECK:         %[[INC:.*]] = arith.addi %[[VALUE]], %[[ONE]] overflow<nsw> : i32
// CHECK:         %[[ONE_2:.*]] = arith.constant 1 : i32
// CHECK:         %[[DEC:.*]] = arith.subi %[[INC]], %[[ONE_2]] overflow<nsw> : i32
// CHECK:         return %[[DEC]] : i32

// CHECK-LABEL: func.func @unsigned_inc_dec(
// CHECK-SAME:      %[[VALUE:.*]]: i32) -> i32
// CHECK:         %[[ONE:.*]] = arith.constant 1 : i32
// CHECK:         %[[INC:.*]] = arith.addi %[[VALUE]], %[[ONE]] : i32
// CHECK:         %[[ONE_2:.*]] = arith.constant 1 : i32
// CHECK:         %[[DEC:.*]] = arith.subi %[[INC]], %[[ONE_2]] : i32
// CHECK:         return %[[DEC]] : i32
// CHECK-NOT:     cir.
