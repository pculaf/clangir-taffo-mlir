// RUN: clangir-taffo-opt --convert-cir-to-standard %s | FileCheck %s

!s32i = !cir.int<s, 32>
!u32i = !cir.int<u, 32>

cir.func @signed_to_bool(%value: !s32i) -> !cir.bool {
  %condition = cir.cast(int_to_bool, %value : !s32i), !cir.bool
  cir.return %condition : !cir.bool
}

cir.func @unsigned_to_bool(%value: !u32i) -> !cir.bool {
  %condition = cir.cast(int_to_bool, %value : !u32i), !cir.bool
  cir.return %condition : !cir.bool
}

// CHECK-LABEL: func.func @signed_to_bool(
// CHECK-SAME:      %[[VALUE:.*]]: i32) -> i1
// CHECK:         %[[ZERO:.*]] = arith.constant 0 : i32
// CHECK:         %[[CONDITION:.*]] = arith.cmpi ne, %[[VALUE]], %[[ZERO]] : i32
// CHECK:         return %[[CONDITION]] : i1

// CHECK-LABEL: func.func @unsigned_to_bool(
// CHECK-SAME:      %[[VALUE:.*]]: i32) -> i1
// CHECK:         %[[ZERO:.*]] = arith.constant 0 : i32
// CHECK:         %[[CONDITION:.*]] = arith.cmpi ne, %[[VALUE]], %[[ZERO]] : i32
// CHECK:         return %[[CONDITION]] : i1
// CHECK-NOT:     cir.
