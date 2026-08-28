// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir | clangir-taffo-opt --convert-cir-to-standard | FileCheck %s

extern float set_range(float value, double min, double max, double precision);

float conditional_range(_Bool condition, float x, float y) {
  x = set_range(x, -1.0, 1.0, 0.01);
  y = set_range(y, -1.0, 1.0, 0.01);

  if (condition)
    return x + y;

  return x * y;
}

// CHECK-LABEL: func.func @conditional_range(
// CHECK-SAME:      %[[CONDITION:.*]]: i1, %[[X:.*]]: f32,
// CHECK-SAME:      %[[Y:.*]]: f32) -> f32
// CHECK:         call @set_range
// CHECK:         call @set_range
// CHECK:         cf.br
// CHECK:         cf.cond_br %[[CONDITION]]
// CHECK:         arith.addf
// CHECK:         return
// CHECK:         cf.br
// CHECK:         arith.mulf
// CHECK:         return
// CHECK-NOT:     cir.br
// CHECK-NOT:     cir.brcond
