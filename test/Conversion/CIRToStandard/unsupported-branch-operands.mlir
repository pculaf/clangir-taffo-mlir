// RUN: not clangir-taffo-opt --convert-cir-to-standard %s 2>&1 | FileCheck %s

module {
  cir.func @branch_with_operand(%arg: !cir.float) -> !cir.float {
    cir.br ^target(%arg : !cir.float)
  ^target(%value: !cir.float):
    cir.return %value : !cir.float
  }
}

// CHECK: failed to legalize operation 'cir.br'
