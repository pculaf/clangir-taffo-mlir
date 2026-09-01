// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir | clangir-taffo-opt --convert-cir-to-standard | mlir-opt --canonicalize | FileCheck %s

extern float set_range(float value, double min, double max, double precision);

float loop_control(int n, float x, float y) {
  x = set_range(x, -10.0, 10.0, 0.01);
  y = set_range(y, -10.0, 10.0, 0.01);

  for (int i = 0; i < n; ++i) {
    if (i == 4)
      return x + y;
  }

  return x * y;
}

// CHECK-LABEL: func.func @loop_control(
// CHECK:         cf.br ^[[HEADER:.*]](%{{.*}} : i32)
// CHECK:       ^[[HEADER]](%[[INDEX:.*]]: i32):
// CHECK:         %[[LOOP_CMP:.*]] = arith.cmpi slt, %[[INDEX]], %{{.*}} : i32
// CHECK:         cf.cond_br %[[LOOP_CMP]]
// CHECK:         %[[EXIT_CMP:.*]] = arith.cmpi eq, %[[INDEX]], %{{.*}} : i32
// CHECK:         cf.cond_br %[[EXIT_CMP]]
// CHECK:         arith.addf
// CHECK:         return
// CHECK:         %[[NEXT:.*]] = arith.addi %[[INDEX]], %{{.*}} overflow<nsw> : i32
// CHECK:         cf.br ^[[HEADER]](%[[NEXT]] : i32)
// CHECK:         arith.mulf
// CHECK:         return
// CHECK-NOT:     cir.
