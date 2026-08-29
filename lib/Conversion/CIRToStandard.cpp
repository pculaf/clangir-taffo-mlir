#include "ClangIRTAFFO/Conversion/Passes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Transforms/DialectConversion.h"
#include "clang/CIR/Dialect/IR/CIRDialect.h"
#include "clang/CIR/Dialect/IR/CIRTypes.h"

#include <optional>

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
    addConversion([context](cir::DoubleType) -> mlir::Type {
      return mlir::Float64Type::get(context);
    });
    addConversion([context](cir::BoolType) -> mlir::Type {
      return mlir::IntegerType::get(context, 1);
    });
  }
};

static bool isSupportedCIRFloatType(mlir::Type type) {
  return mlir::isa<cir::SingleType, cir::DoubleType>(type);
}

static std::optional<mlir::arith::CmpFPredicate>
convertCIRFloatCmpPredicate(cir::CmpOpKind kind) {
  switch (kind) {
  case cir::CmpOpKind::lt:
    return mlir::arith::CmpFPredicate::OLT;
  case cir::CmpOpKind::le:
    return mlir::arith::CmpFPredicate::OLE;
  case cir::CmpOpKind::gt:
    return mlir::arith::CmpFPredicate::OGT;
  case cir::CmpOpKind::ge:
    return mlir::arith::CmpFPredicate::OGE;
  case cir::CmpOpKind::eq:
    return mlir::arith::CmpFPredicate::OEQ;
  case cir::CmpOpKind::ne:
    return mlir::arith::CmpFPredicate::UNE;
  }
  return std::nullopt;
}

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

    if (op.getBody().empty())
      newFunc.setPrivate();
    else if (mlir::Attribute visibility =
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
    case cir::BinOpKind::Sub:
      rewriter.replaceOpWithNewOp<mlir::arith::SubFOp>(op, adaptor.getLhs(),
                                                       adaptor.getRhs());
      break;
    case cir::BinOpKind::Mul:
      rewriter.replaceOpWithNewOp<mlir::arith::MulFOp>(op, adaptor.getLhs(),
                                                       adaptor.getRhs());
      break;
    case cir::BinOpKind::Div:
      rewriter.replaceOpWithNewOp<mlir::arith::DivFOp>(op, adaptor.getLhs(),
                                                       adaptor.getRhs());
      break;
    default:
      return rewriter.notifyMatchFailure(op,
                                         "unsupported binary operation kind");
    }

    return mlir::success();
  }
};

struct ConvertConstantOp : public mlir::OpConversionPattern<cir::ConstantOp> {
  using OpConversionPattern::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(cir::ConstantOp op, OpAdaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    auto value = mlir::dyn_cast<cir::FPAttr>(op.getValue());
    if (!value)
      return rewriter.notifyMatchFailure(
          op, "only floating constants are supported");

    mlir::Type convertedType = getTypeConverter()->convertType(op.getType());
    auto floatType = mlir::dyn_cast_or_null<mlir::FloatType>(convertedType);
    if (!floatType)
      return rewriter.notifyMatchFailure(op, "constant type is unsupported");

    auto convertedValue = mlir::FloatAttr::get(floatType, value.getValue());
    rewriter.replaceOpWithNewOp<mlir::arith::ConstantOp>(op, convertedValue);
    return mlir::success();
  }
};

struct ConvertUnaryOp : public mlir::OpConversionPattern<cir::UnaryOp> {
  using OpConversionPattern::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(cir::UnaryOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    if (!isSupportedCIRFloatType(op.getType()))
      return rewriter.notifyMatchFailure(
          op, "only floating unary operations are supported");
    if (op.getKind() != cir::UnaryOpKind::Minus)
      return rewriter.notifyMatchFailure(op,
                                         "unsupported unary operation kind");

    rewriter.replaceOpWithNewOp<mlir::arith::NegFOp>(op, adaptor.getInput());
    return mlir::success();
  }
};

struct ConvertCallOp : public mlir::OpConversionPattern<cir::CallOp> {
  using OpConversionPattern::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(cir::CallOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    if (op.isIndirect())
      return rewriter.notifyMatchFailure(op, "indirect calls are unsupported");

    for (mlir::Type type : op.getOperandTypes())
      if (!isSupportedCIRFloatType(type))
        return rewriter.notifyMatchFailure(op,
                                           "call operand type is unsupported");
    for (mlir::Type type : op.getResultTypes())
      if (!isSupportedCIRFloatType(type))
        return rewriter.notifyMatchFailure(op,
                                           "call result type is unsupported");

    llvm::SmallVector<mlir::Type> resultTypes;
    if (mlir::failed(
            getTypeConverter()->convertTypes(op.getResultTypes(), resultTypes)))
      return mlir::failure();

    rewriter.replaceOpWithNewOp<mlir::func::CallOp>(
        op, op.getCalleeAttr(), resultTypes, adaptor.getArgs());
    return mlir::success();
  }
};

struct ConvertCmpOp : public mlir::OpConversionPattern<cir::CmpOp> {
  using OpConversionPattern::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(cir::CmpOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    if (!isSupportedCIRFloatType(op.getLhs().getType()))
      return rewriter.notifyMatchFailure(
          op, "only floating comparisons are supported");

    std::optional<mlir::arith::CmpFPredicate> predicate =
        convertCIRFloatCmpPredicate(op.getKind());
    if (!predicate)
      return rewriter.notifyMatchFailure(op, "unsupported comparison kind");

    rewriter.replaceOpWithNewOp<mlir::arith::CmpFOp>(
        op, *predicate, adaptor.getLhs(), adaptor.getRhs());
    return mlir::success();
  }
};

struct ConvertCastOp : public mlir::OpConversionPattern<cir::CastOp> {
  using OpConversionPattern::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(cir::CastOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    if (op.getKind() != cir::CastKind::float_to_bool)
      return rewriter.notifyMatchFailure(op, "unsupported cast kind");
    if (!isSupportedCIRFloatType(op.getSrc().getType()))
      return rewriter.notifyMatchFailure(
          op, "float-to-bool source type is unsupported");

    mlir::Value source = adaptor.getSrc();
    auto zero = rewriter.create<mlir::arith::ConstantOp>(
        op.getLoc(), rewriter.getFloatAttr(source.getType(), 0.0));
    rewriter.replaceOpWithNewOp<mlir::arith::CmpFOp>(
        op, mlir::arith::CmpFPredicate::UNE, source, zero);
    return mlir::success();
  }
};

struct ConvertBranchOp : public mlir::OpConversionPattern<cir::BrOp> {
  using OpConversionPattern::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(cir::BrOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<mlir::cf::BranchOp>(op, op.getDest(),
                                                    adaptor.getDestOperands());
    return mlir::success();
  }
};

struct ConvertCondBranchOp : public mlir::OpConversionPattern<cir::BrCondOp> {
  using OpConversionPattern::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(cir::BrCondOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<mlir::cf::CondBranchOp>(
        op, adaptor.getCond(), op.getDestTrue(), adaptor.getDestOperandsTrue(),
        op.getDestFalse(), adaptor.getDestOperandsFalse());
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
    target.addLegalDialect<mlir::arith::ArithDialect,
                           mlir::cf::ControlFlowDialect,
                           mlir::func::FuncDialect>();
    target.addIllegalDialect<cir::CIRDialect>();

    mlir::RewritePatternSet patterns(context);
    patterns.add<ConvertFuncOp, ConvertBinOp, ConvertConstantOp, ConvertUnaryOp,
                 ConvertCallOp, ConvertCmpOp, ConvertCastOp, ConvertBranchOp,
                 ConvertCondBranchOp, ConvertReturnOp>(typeConverter, context);

    if (mlir::failed(mlir::applyFullConversion(getOperation(), target,
                                               std::move(patterns))))
      signalPassFailure();
  }
};

} // namespace
} // namespace clangir_taffo
