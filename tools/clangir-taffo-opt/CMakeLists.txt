set(LIBS
  ClangIRTAFFOConversion
  MLIRArithDialect
  MLIRCIR
  MLIRFuncDialect
  MLIRIR
  MLIROptLib
)

add_llvm_executable(clangir-taffo-opt
  clangir-taffo-opt.cpp
)

add_dependencies(clangir-taffo-opt
  ClangIRTAFFOConversionPassIncGen
)

llvm_update_compile_flags(clangir-taffo-opt)
target_link_libraries(clangir-taffo-opt PRIVATE ${LIBS})

mlir_check_all_link_libraries(clangir-taffo-opt)
