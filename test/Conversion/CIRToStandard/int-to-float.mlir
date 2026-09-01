// RUN: clangir-taffo-opt --convert-cir-to-standard %s | FileCheck %s

!s32i = !cir.int<s, 32>
!u32i = !cir.int<u, 32>

cir.func @signed_to_float(%value: !s32i) -> !cir.float {
  %converted = cir.cast(int_to_float, %value : !s32i), !cir.float
  cir.return %converted : !cir.float
}

cir.func @unsigned_to_float(%value: !u32i) -> !cir.float {
  %converted = cir.cast(int_to_float, %value : !u32i), !cir.float
  cir.return %converted : !cir.float
}

// CHECK-LABEL: func.func @signed_to_float(
// CHECK-SAME:      %[[VALUE:.*]]: i32) -> f32
// CHECK:         %[[CONVERTED:.*]] = arith.sitofp %[[VALUE]] : i32 to f32
// CHECK:         return %[[CONVERTED]] : f32

// CHECK-LABEL: func.func @unsigned_to_float(
// CHECK-SAME:      %[[VALUE:.*]]: i32) -> f32
// CHECK:         %[[CONVERTED:.*]] = arith.uitofp %[[VALUE]] : i32 to f32
// CHECK:         return %[[CONVERTED]] : f32
// CHECK-NOT:     cir.
