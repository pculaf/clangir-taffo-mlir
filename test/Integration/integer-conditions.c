// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir | clangir-taffo-opt --convert-cir-to-standard | mlir-opt --canonicalize | FileCheck %s

extern float set_range(float value, double min, double max, double precision);

float select_signed(int condition, float x, float y) {
  x = set_range(x, -1.0, 1.0, 0.01);
  y = set_range(y, -1.0, 1.0, 0.01);

  if (condition < 10)
    return x + y;

  return x * y;
}

float select_unsigned(unsigned condition, float x, float y) {
  x = set_range(x, -1.0, 1.0, 0.01);
  y = set_range(y, -1.0, 1.0, 0.01);

  if (condition >= 10u)
    return x + y;

  return x * y;
}

// CHECK-LABEL: func.func @select_signed(%{{.*}}: i32,
// CHECK:         %[[SIGNED_LIMIT:.*]] = arith.constant 10 : i32
// CHECK:         %[[SIGNED_CMP:.*]] = arith.cmpi slt, %{{.*}}, %[[SIGNED_LIMIT]] : i32
// CHECK:         cf.cond_br %[[SIGNED_CMP]]
// CHECK:         arith.addf
// CHECK:         return
// CHECK:         arith.mulf
// CHECK:         return

// CHECK-LABEL: func.func @select_unsigned(%{{.*}}: i32,
// CHECK:         %[[UNSIGNED_LIMIT:.*]] = arith.constant 10 : i32
// CHECK:         %[[UNSIGNED_CMP:.*]] = arith.cmpi uge, %{{.*}}, %[[UNSIGNED_LIMIT]] : i32
// CHECK:         cf.cond_br %[[UNSIGNED_CMP]]
// CHECK:         arith.addf
// CHECK:         return
// CHECK:         arith.mulf
// CHECK:         return
// CHECK-NOT:     cir.
