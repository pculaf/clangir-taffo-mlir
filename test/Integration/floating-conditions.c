// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir | clangir-taffo-opt --convert-cir-to-standard | FileCheck %s

extern float set_range(float value, double min, double max, double precision);

float compare_and_select(float x, float y) {
  x = set_range(x, -1.0, 1.0, 0.01);
  y = set_range(y, -1.0, 1.0, 0.01);

  if (x < y)
    return x + y;

  return x * y;
}

float select_if_nonzero(float condition, float x, float y) {
  condition = set_range(condition, -1.0, 1.0, 0.01);
  x = set_range(x, -1.0, 1.0, 0.01);
  y = set_range(y, -1.0, 1.0, 0.01);

  if (condition)
    return x + y;

  return x * y;
}

// CHECK-LABEL: func.func @compare_and_select(
// CHECK:         %[[CMP:.*]] = arith.cmpf olt, %{{.*}}, %{{.*}} : f32
// CHECK:         cf.cond_br %[[CMP]]
// CHECK:         arith.addf
// CHECK:         arith.mulf

// CHECK-LABEL: func.func @select_if_nonzero(
// CHECK:         %[[ZERO:.*]] = arith.constant 0.000000e+00 : f32
// CHECK:         %[[TRUTH:.*]] = arith.cmpf une, %{{.*}}, %[[ZERO]] : f32
// CHECK:         cf.cond_br %[[TRUTH]]
// CHECK:         arith.addf
// CHECK:         arith.mulf
// CHECK-NOT:     cir.cmp
// CHECK-NOT:     cir.cast
