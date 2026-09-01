// RUN: clangir-taffo-opt --convert-cir-to-standard %s | FileCheck %s

!s32i = !cir.int<s, 32>
!u32i = !cir.int<u, 32>

cir.func @signed_comparisons(%lhs: !s32i, %rhs: !s32i) -> !cir.bool {
  %constant = cir.const #cir.int<10> : !s32i
  %lt = cir.cmp(lt, %lhs, %constant) : !s32i, !cir.bool
  %le = cir.cmp(le, %lhs, %rhs) : !s32i, !cir.bool
  %gt = cir.cmp(gt, %lhs, %rhs) : !s32i, !cir.bool
  %ge = cir.cmp(ge, %lhs, %rhs) : !s32i, !cir.bool
  %eq = cir.cmp(eq, %lhs, %rhs) : !s32i, !cir.bool
  %ne = cir.cmp(ne, %lhs, %rhs) : !s32i, !cir.bool
  cir.return %ne : !cir.bool
}

cir.func @unsigned_comparisons(%lhs: !u32i, %rhs: !u32i) -> !cir.bool {
  %lt = cir.cmp(lt, %lhs, %rhs) : !u32i, !cir.bool
  %le = cir.cmp(le, %lhs, %rhs) : !u32i, !cir.bool
  %gt = cir.cmp(gt, %lhs, %rhs) : !u32i, !cir.bool
  %ge = cir.cmp(ge, %lhs, %rhs) : !u32i, !cir.bool
  %eq = cir.cmp(eq, %lhs, %rhs) : !u32i, !cir.bool
  %ne = cir.cmp(ne, %lhs, %rhs) : !u32i, !cir.bool
  cir.return %ne : !cir.bool
}

// CHECK-LABEL: func.func @signed_comparisons(%{{.*}}: i32, %{{.*}}: i32) -> i1
// CHECK:         arith.constant 10 : i32
// CHECK:         arith.cmpi slt
// CHECK:         arith.cmpi sle
// CHECK:         arith.cmpi sgt
// CHECK:         arith.cmpi sge
// CHECK:         arith.cmpi eq
// CHECK:         arith.cmpi ne

// CHECK-LABEL: func.func @unsigned_comparisons(%{{.*}}: i32, %{{.*}}: i32) -> i1
// CHECK:         arith.cmpi ult
// CHECK:         arith.cmpi ule
// CHECK:         arith.cmpi ugt
// CHECK:         arith.cmpi uge
// CHECK:         arith.cmpi eq
// CHECK:         arith.cmpi ne
// CHECK-NOT:     cir.
