#ifndef CLANGIR_TAFFO_CONVERSION_PASSES_TD
#define CLANGIR_TAFFO_CONVERSION_PASSES_TD

include "mlir/Pass/PassBase.td"

def ConvertCIRToStandardPass
    : Pass<"convert-cir-to-standard", "mlir::ModuleOp"> {
  let summary = "Convert the supported CIR subset to standard MLIR dialects";

  let dependentDialects = [
    "mlir::arith::ArithDialect",
    "mlir::func::FuncDialect"
  ];
}

#endif // CLANGIR_TAFFO_CONVERSION_PASSES_TD
