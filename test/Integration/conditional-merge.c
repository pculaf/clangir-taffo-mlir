// RUN: clang -fclangir -emit-cir %s -o %t.cir
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir | clangir-taffo-opt --convert-cir-to-standard | FileCheck %s --check-prefix=CF
// RUN: cir-opt --cir-flatten-cfg --mem2reg %t.cir | clangir-taffo-opt --convert-cir-to-standard | mlir-opt --lift-cf-to-scf | FileCheck %s --check-prefix=SCF

extern float set_range(float value, double min, double max, double precision);

float merge_after_if(_Bool condition, float x, float y) {
  x = set_range(x, -1.0, 1.0, 0.01);
  y = set_range(y, -1.0, 1.0, 0.01);

  float result;
  if (condition)
    result = x + y;
  else
    result = x * y;

  return result;
}

// CF-LABEL: func.func @merge_after_if(
// CF-SAME:      %[[CONDITION:.*]]: i1, %[[X:.*]]: f32,
// CF-SAME:      %[[Y:.*]]: f32) -> f32
// CF:         cf.cond_br %[[CONDITION]], ^[[THEN:.*]], ^[[ELSE:.*]]
// CF:       ^[[THEN]]:
// CF:         %[[SUM:.*]] = arith.addf
// CF-NEXT:    cf.br ^[[MERGE:.*]](%[[SUM]] : f32)
// CF:       ^[[ELSE]]:
// CF:         %[[PRODUCT:.*]] = arith.mulf
// CF-NEXT:    cf.br ^[[MERGE]](%[[PRODUCT]] : f32)
// CF:       ^[[MERGE]](%[[RESULT:.*]]: f32):
// CF:         return %[[RESULT]] : f32
// CF-NOT:     cir.br
// CF-NOT:     cir.brcond

// SCF-LABEL: func.func @merge_after_if(
// SCF:         %[[RESULT:.*]] = scf.if %{{.*}} -> (f32) {
// SCF:           %[[SUM:.*]] = arith.addf
// SCF-NEXT:      scf.yield %[[SUM]] : f32
// SCF:         } else {
// SCF:           %[[PRODUCT:.*]] = arith.mulf
// SCF-NEXT:      scf.yield %[[PRODUCT]] : f32
// SCF:         }
// SCF:         return %[[RESULT]] : f32
// SCF-NOT:     cf.br
// SCF-NOT:     cf.cond_br
