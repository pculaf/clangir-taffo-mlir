// RUN: not clangir-taffo-opt --convert-cir-to-standard %s 2>&1 | FileCheck %s

!s32i = !cir.int<s, 32>

cir.func @saturated_add(%lhs: !s32i, %rhs: !s32i) -> !s32i {
  %result = cir.binop(add, %lhs, %rhs) sat : !s32i
  cir.return %result : !s32i
}

// CHECK: failed to legalize operation 'cir.binop'
