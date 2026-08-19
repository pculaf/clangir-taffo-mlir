#include "ClangIRTAFFO/Conversion/Passes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Transforms/DialectConversion.h"
#include "clang/CIR/Dialect/IR/CIRDialect.h"
#include "clang/CIR/Dialect/IR/CIRTypes.h"

namespace clangir_taffo {
#define GEN_PASS_DEF_CONVERTCIRTOSTANDARDPASS
#include "ClangIRTAFFO/Conversion/Passes.h.inc"
} // namespace clangir_taffo

namespace clangir_taffo {
namespace {

class CIRToStandardTypeConverter : public mlir::TypeConverter {
public:
  explicit CIRToStandardTypeConverter(mlir::MLIRContext *context) {
    addConversion([](mlir::Type type) { return type; });
    addConversion([context](cir::SingleType) -> mlir::Type {
      return mlir::Float32Type::get(context);
    });
  }
};

struct ConvertFuncOp : public mlir::OpConversionPattern<cir::FuncOp> {
  using OpConversionPattern::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(cir::FuncOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    cir::FuncType cirFunctionType = op.getFunctionType();
    if (cirFunctionType.isVarArg())
      return rewriter.notifyMatchFailure(op,
                                         "variadic functions are unsupported");

    llvm::SmallVector<mlir::Type> inputTypes;
    llvm::SmallVector<mlir::Type> resultTypes;
    if (mlir::failed(getTypeConverter()->convertTypes(
            cirFunctionType.getInputs(), inputTypes)) ||
        mlir::failed(getTypeConverter()->convertTypes(
            cirFunctionType.getReturnTypes(), resultTypes)))
      return mlir::failure();

    mlir::TypeConverter::SignatureConversion signatureConversion(
        cirFunctionType.getNumInputs());
    for (auto [index, type] : llvm::enumerate(inputTypes))
      signatureConversion.addInputs(index, type);

    auto functionType =
        mlir::FunctionType::get(rewriter.getContext(), inputTypes, resultTypes);
    auto newFunc = rewriter.create<mlir::func::FuncOp>(
        op.getLoc(), op.getSymName(), functionType);

    if (mlir::Attribute visibility =
            op->getAttr(mlir::SymbolTable::getVisibilityAttrName()))
      newFunc->setAttr(mlir::SymbolTable::getVisibilityAttrName(), visibility);

    rewriter.inlineRegionBefore(op.getBody(), newFunc.getBody(),
                                newFunc.getBody().end());
    if (mlir::failed(rewriter.convertRegionTypes(
            &newFunc.getBody(), *getTypeConverter(), &signatureConversion)))
      return mlir::failure();

    rewriter.eraseOp(op);
    return mlir::success();
  }
};

struct ConvertBinOp : public mlir::OpConversionPattern<cir::BinOp> {
  using OpConversionPattern::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(cir::BinOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    if (!mlir::isa<cir::SingleType>(op.getLhs().getType()))
      return rewriter.notifyMatchFailure(op, "only !cir.float is supported");

    switch (op.getKind()) {
    case cir::BinOpKind::Add:
      rewriter.replaceOpWithNewOp<mlir::arith::AddFOp>(op, adaptor.getLhs(),
                                                       adaptor.getRhs());
      break;
    case cir::BinOpKind::Mul:
      rewriter.replaceOpWithNewOp<mlir::arith::MulFOp>(op, adaptor.getLhs(),
                                                       adaptor.getRhs());
      break;
    default:
      return rewriter.notifyMatchFailure(op,
                                         "unsupported binary operation kind");
    }

    return mlir::success();
  }
};

struct ConvertReturnOp : public mlir::OpConversionPattern<cir::ReturnOp> {
  using OpConversionPattern::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(cir::ReturnOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<mlir::func::ReturnOp>(op, adaptor.getInput());
    return mlir::success();
  }
};

class ConvertCIRToStandardPass
    : public impl::ConvertCIRToStandardPassBase<ConvertCIRToStandardPass> {
public:
  using ConvertCIRToStandardPassBase::ConvertCIRToStandardPassBase;

  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();
    CIRToStandardTypeConverter typeConverter(context);

    mlir::ConversionTarget target(*context);
    target.addLegalOp<mlir::ModuleOp>();
    target
        .addLegalDialect<mlir::arith::ArithDialect, mlir::func::FuncDialect>();
    target.addIllegalDialect<cir::CIRDialect>();

    mlir::RewritePatternSet patterns(context);
    patterns.add<ConvertFuncOp, ConvertBinOp, ConvertReturnOp>(typeConverter,
                                                               context);

    if (mlir::failed(mlir::applyFullConversion(getOperation(), target,
                                               std::move(patterns))))
      signalPassFailure();
  }
};

} // namespace
} // namespace clangir_taffo
