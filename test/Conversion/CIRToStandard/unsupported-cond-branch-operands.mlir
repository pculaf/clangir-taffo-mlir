// RUN: not clangir-taffo-opt --convert-cir-to-standard %s 2>&1 | FileCheck %s

module {
  cir.func @cond_branch_with_operands(%condition: !cir.bool,
                                      %value: !cir.float) -> !cir.float {
    cir.brcond %condition ^then(%value : !cir.float), ^else(%value : !cir.float)
  ^then(%then_value: !cir.float):
    cir.return %then_value : !cir.float
  ^else(%else_value: !cir.float):
    cir.return %else_value : !cir.float
  }
}

// CHECK: failed to legalize operation 'cir.brcond'
