// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir | clangir-taffo-opt --convert-cir-to-standard | mlir-opt --canonicalize | FileCheck %s

extern float set_range(float value, double min, double max, double precision);

float scale_by_signed(int count, float x) {
  x = set_range(x, -10.0, 10.0, 0.01);
  float factor = set_range((float)count, -100.0, 100.0, 0.01);
  return x * factor;
}

float scale_by_unsigned(unsigned count, float x) {
  x = set_range(x, -10.0, 10.0, 0.01);
  float factor = set_range((float)count, 0.0, 100.0, 0.01);
  return x * factor;
}

// CHECK-LABEL: func.func @scale_by_signed(
// CHECK-SAME:      %[[COUNT:.*]]: i32, %{{.*}}: f32) -> f32
// CHECK:         %[[FACTOR:.*]] = arith.sitofp %[[COUNT]] : i32 to f32
// CHECK:         call @set_range(%[[FACTOR]]
// CHECK:         arith.mulf
// CHECK:         return

// CHECK-LABEL: func.func @scale_by_unsigned(
// CHECK-SAME:      %[[COUNT:.*]]: i32, %{{.*}}: f32) -> f32
// CHECK:         %[[FACTOR:.*]] = arith.uitofp %[[COUNT]] : i32 to f32
// CHECK:         call @set_range(%[[FACTOR]]
// CHECK:         arith.mulf
// CHECK:         return
// CHECK-NOT:     cir.
