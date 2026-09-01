// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir | clangir-taffo-opt --convert-cir-to-standard | mlir-opt --canonicalize | FileCheck %s

extern float set_range(float value, double min, double max, double precision);

float merge_rerange_and_continue(_Bool condition, float x, float y) {
  x = set_range(x, -1.0, 1.0, 0.01);
  y = set_range(y, -1.0, 1.0, 0.01);

  float result;
  if (condition)
    result = x + y;
  else
    result = x * y;

  result = set_range(result, -2.0, 2.0, 0.01);
  return result + x;
}

// CHECK-LABEL: func.func @merge_rerange_and_continue(
// CHECK:         cf.cond_br %{{.*}}, ^[[THEN:.*]], ^[[ELSE:.*]]
// CHECK:       ^[[THEN]]:
// CHECK:         %[[SUM:.*]] = arith.addf
// CHECK-NEXT:    cf.br ^[[MERGE:.*]](%[[SUM]] : f32)
// CHECK:       ^[[ELSE]]:
// CHECK:         %[[PRODUCT:.*]] = arith.mulf
// CHECK-NEXT:    cf.br ^[[MERGE]](%[[PRODUCT]] : f32)
// CHECK:       ^[[MERGE]](%[[RESULT:.*]]: f32):
// CHECK:         %[[RERANGED:.*]] = call @set_range(%[[RESULT]],
// CHECK:         arith.addf %[[RERANGED]], %{{.*}} : f32
// CHECK:         return
// CHECK-NOT:     cir.
