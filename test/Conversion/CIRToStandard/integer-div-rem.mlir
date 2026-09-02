// RUN: clangir-taffo-opt --convert-cir-to-standard %s | FileCheck %s

!s32i = !cir.int<s, 32>
!u32i = !cir.int<u, 32>

cir.func @signed_div_rem(%lhs: !s32i, %rhs: !s32i) -> !s32i {
  %quotient = cir.binop(div, %lhs, %rhs) : !s32i
  %remainder = cir.binop(rem, %quotient, %rhs) : !s32i
  cir.return %remainder : !s32i
}

cir.func @unsigned_div_rem(%lhs: !u32i, %rhs: !u32i) -> !u32i {
  %quotient = cir.binop(div, %lhs, %rhs) : !u32i
  %remainder = cir.binop(rem, %quotient, %rhs) : !u32i
  cir.return %remainder : !u32i
}

// CHECK-LABEL: func.func @signed_div_rem(
// CHECK:         %[[QUOTIENT:.*]] = arith.divsi %{{.*}}, %{{.*}} : i32
// CHECK:         %[[REMAINDER:.*]] = arith.remsi %[[QUOTIENT]], %{{.*}} : i32
// CHECK:         return %[[REMAINDER]] : i32

// CHECK-LABEL: func.func @unsigned_div_rem(
// CHECK:         %[[QUOTIENT:.*]] = arith.divui %{{.*}}, %{{.*}} : i32
// CHECK:         %[[REMAINDER:.*]] = arith.remui %[[QUOTIENT]], %{{.*}} : i32
// CHECK:         return %[[REMAINDER]] : i32
// CHECK-NOT:     cir.
