// RUN: not clangir-taffo-opt --convert-cir-to-standard %s 2>&1 | FileCheck %s

module {
  cir.func @call_indirect(
      %callee: !cir.ptr<!cir.func<(!cir.float) -> !cir.float>>,
      %arg: !cir.float) -> !cir.float {
    %0 = cir.call %callee(%arg) : (!cir.ptr<!cir.func<(!cir.float) -> !cir.float>>, !cir.float) -> !cir.float
    cir.return %0 : !cir.float
  }
}

// CHECK: failed to legalize operation 'cir.call'
