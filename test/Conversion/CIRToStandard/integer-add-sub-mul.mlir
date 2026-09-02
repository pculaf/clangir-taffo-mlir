// RUN: clangir-taffo-opt --convert-cir-to-standard %s | FileCheck %s

!s32i = !cir.int<s, 32>
!u32i = !cir.int<u, 32>

cir.func @signed_add_sub_mul(%lhs: !s32i, %rhs: !s32i) -> !s32i {
  %sum = cir.binop(add, %lhs, %rhs) nsw : !s32i
  %difference = cir.binop(sub, %sum, %rhs) nsw : !s32i
  %product = cir.binop(mul, %difference, %rhs) nsw : !s32i
  cir.return %product : !s32i
}

cir.func @unsigned_add_sub_mul(%lhs: !u32i, %rhs: !u32i) -> !u32i {
  %sum = cir.binop(add, %lhs, %rhs) nuw : !u32i
  %difference = cir.binop(sub, %sum, %rhs) nuw : !u32i
  %product = cir.binop(mul, %difference, %rhs) nuw : !u32i
  cir.return %product : !u32i
}

// CHECK-LABEL: func.func @signed_add_sub_mul(
// CHECK:         %[[SUM:.*]] = arith.addi %{{.*}}, %{{.*}} overflow<nsw> : i32
// CHECK:         %[[DIFFERENCE:.*]] = arith.subi %[[SUM]], %{{.*}} overflow<nsw> : i32
// CHECK:         %[[PRODUCT:.*]] = arith.muli %[[DIFFERENCE]], %{{.*}} overflow<nsw> : i32
// CHECK:         return %[[PRODUCT]] : i32

// CHECK-LABEL: func.func @unsigned_add_sub_mul(
// CHECK:         %[[SUM:.*]] = arith.addi %{{.*}}, %{{.*}} overflow<nuw> : i32
// CHECK:         %[[DIFFERENCE:.*]] = arith.subi %[[SUM]], %{{.*}} overflow<nuw> : i32
// CHECK:         %[[PRODUCT:.*]] = arith.muli %[[DIFFERENCE]], %{{.*}} overflow<nuw> : i32
// CHECK:         return %[[PRODUCT]] : i32
// CHECK-NOT:     cir.
