#ifndef CLANGIR_TAFFO_CONVERSION_PASSES_H
#define CLANGIR_TAFFO_CONVERSION_PASSES_H

#include "mlir/Pass/Pass.h"

namespace clangir_taffo {
#define GEN_PASS_DECL_CONVERTCIRTOSTANDARDPASS
#include "ClangIRTAFFO/Conversion/Passes.h.inc"

#define GEN_PASS_REGISTRATION
#include "ClangIRTAFFO/Conversion/Passes.h.inc"
} // namespace clangir_taffo

#endif // CLANGIR_TAFFO_CONVERSION_PASSES_H
