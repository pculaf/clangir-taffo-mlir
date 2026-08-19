// RUN: clangir-taffo-opt %s | FileCheck %s

module {
  cir.func @identity(%arg0: !cir.float) -> !cir.float {
    cir.return %arg0 : !cir.float
  }
}

// CHECK-LABEL: cir.func @identity
// CHECK:         cir.return
