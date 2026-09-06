// RUN: clangir-taffo-opt --convert-lifted-cf-loops-to-scf-for %s | FileCheck %s

module {
  func.func @dynamic_while(%initial: i32, %limit: i32) -> i32 {
    %result = scf.while (%value = %initial) : (i32) -> i32 {
      %condition = arith.cmpi slt, %value, %limit : i32
      scf.condition(%condition) %value : i32
    } do {
    ^bb0(%value: i32):
      %step = arith.constant 2 : i32
      %next = arith.muli %value, %step : i32
      scf.yield %next : i32
    }
    return %result : i32
  }
}

// CHECK-LABEL: func.func @dynamic_while(
// CHECK:         scf.while
// CHECK:         arith.muli
// CHECK-NOT:     scf.for
