#include "ClangIRTAFFO/Conversion/Passes.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/UB/IR/UBOps.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"
#include "clang/CIR/Dialect/IR/CIRDialect.h"

int main(int argc, char **argv) {
  clangir_taffo::registerClangIRTAFFOConversionPasses();

  mlir::DialectRegistry registry;
  registry.insert<cir::CIRDialect, mlir::arith::ArithDialect,
                  mlir::cf::ControlFlowDialect, mlir::func::FuncDialect,
                  mlir::scf::SCFDialect, mlir::ub::UBDialect>();

  return mlir::asMainReturnCode(mlir::MlirOptMain(
      argc, argv, "ClangIR to TAFFO-MLIR integration optimizer\n", registry));
}
