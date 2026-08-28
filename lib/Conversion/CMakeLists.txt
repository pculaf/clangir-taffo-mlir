add_mlir_library(ClangIRTAFFOConversion
  CIRToStandard.cpp

  DEPENDS
  ClangIRTAFFOConversionPassIncGen

  LINK_LIBS PUBLIC
  MLIRArithDialect
  MLIRCIR
  MLIRControlFlowDialect
  MLIRFuncDialect
  MLIRIR
  MLIRPass
  MLIRTransforms
)
