#include "ClangIRTAFFO/Conversion/Passes.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"
#include "clang/CIR/Dialect/IR/CIRDialect.h"

int main(int argc, char **argv) {
  clangir_taffo::registerConvertCIRToStandardPass();

  mlir::DialectRegistry registry;
  registry.insert<cir::CIRDialect, mlir::arith::ArithDialect,
                  mlir::cf::ControlFlowDialect, mlir::func::FuncDialect>();

  return mlir::asMainReturnCode(mlir::MlirOptMain(
      argc, argv, "ClangIR to TAFFO-MLIR integration optimizer\n", registry));
}
