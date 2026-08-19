configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
  MAIN_CONFIG
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
)

set(CLANGIR_TAFFO_TEST_DEPENDS
  clangir-taffo-opt
)

add_lit_testsuite(check-clangir-taffo
  "Running ClangIR/TAFFO-MLIR regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${CLANGIR_TAFFO_TEST_DEPENDS}
)

set_target_properties(check-clangir-taffo PROPERTIES FOLDER "Tests")

add_lit_testsuites(CLANGIR_TAFFO
  ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${CLANGIR_TAFFO_TEST_DEPENDS}
)
