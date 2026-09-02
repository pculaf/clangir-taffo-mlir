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
    addConversion([context](cir::IntType type) -> mlir::Type {
      return mlir::IntegerType::get(context, type.getWidth());
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

static std::optional<mlir::arith::CmpIPredicate>
convertCIRIntCmpPredicate(cir::CmpOpKind kind, bool isSigned) {
  switch (kind) {
  case cir::CmpOpKind::lt:
    return isSigned ? mlir::arith::CmpIPredicate::slt
                    : mlir::arith::CmpIPredicate::ult;
  case cir::CmpOpKind::le:
    return isSigned ? mlir::arith::CmpIPredicate::sle
                    : mlir::arith::CmpIPredicate::ule;
  case cir::CmpOpKind::gt:
    return isSigned ? mlir::arith::CmpIPredicate::sgt
                    : mlir::arith::CmpIPredicate::ugt;
  case cir::CmpOpKind::ge:
    return isSigned ? mlir::arith::CmpIPredicate::sge
                    : mlir::arith::CmpIPredicate::uge;
  case cir::CmpOpKind::eq:
    return mlir::arith::CmpIPredicate::eq;
  case cir::CmpOpKind::ne:
    return mlir::arith::CmpIPredicate::ne;
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
    mlir::Type operandType = op.getLhs().getType();
    if (mlir::isa<cir::SingleType>(operandType)) {
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
        return rewriter.notifyMatchFailure(
            op, "unsupported floating binary operation kind");
      }
      return mlir::success();
    }

    auto integerType = mlir::dyn_cast<cir::IntType>(operandType);
    if (!integerType)
      return rewriter.notifyMatchFailure(op,
                                         "binary operation type is unsupported");
    if (op.getSaturated())
      return rewriter.notifyMatchFailure(
          op, "saturated integer arithmetic is unsupported");

    mlir::arith::IntegerOverflowFlags overflowFlags =
        mlir::arith::IntegerOverflowFlags::none;
    if (op.getNoSignedWrap())
      overflowFlags =
          overflowFlags | mlir::arith::IntegerOverflowFlags::nsw;
    if (op.getNoUnsignedWrap())
      overflowFlags =
          overflowFlags | mlir::arith::IntegerOverflowFlags::nuw;

    switch (op.getKind()) {
    case cir::BinOpKind::Add:
      rewriter.replaceOpWithNewOp<mlir::arith::AddIOp>(
          op, adaptor.getLhs(), adaptor.getRhs(), overflowFlags);
      return mlir::success();
    case cir::BinOpKind::Sub:
      rewriter.replaceOpWithNewOp<mlir::arith::SubIOp>(
          op, adaptor.getLhs(), adaptor.getRhs(), overflowFlags);
      return mlir::success();
    case cir::BinOpKind::Mul:
      rewriter.replaceOpWithNewOp<mlir::arith::MulIOp>(
          op, adaptor.getLhs(), adaptor.getRhs(), overflowFlags);
      return mlir::success();
    case cir::BinOpKind::Div:
      if (integerType.isSigned())
        rewriter.replaceOpWithNewOp<mlir::arith::DivSIOp>(
            op, adaptor.getLhs(), adaptor.getRhs());
      else
        rewriter.replaceOpWithNewOp<mlir::arith::DivUIOp>(
            op, adaptor.getLhs(), adaptor.getRhs());
      return mlir::success();
    case cir::BinOpKind::Rem:
      if (integerType.isSigned())
        rewriter.replaceOpWithNewOp<mlir::arith::RemSIOp>(
            op, adaptor.getLhs(), adaptor.getRhs());
      else
        rewriter.replaceOpWithNewOp<mlir::arith::RemUIOp>(
            op, adaptor.getLhs(), adaptor.getRhs());
      return mlir::success();
    default:
      return rewriter.notifyMatchFailure(
          op, "unsupported integer binary operation kind");
    }
  }
};

struct ConvertConstantOp : public mlir::OpConversionPattern<cir::ConstantOp> {
  using OpConversionPattern::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(cir::ConstantOp op, OpAdaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    mlir::Type convertedType = getTypeConverter()->convertType(op.getType());
    if (auto value = mlir::dyn_cast<cir::FPAttr>(op.getValue())) {
      auto floatType = mlir::dyn_cast_or_null<mlir::FloatType>(convertedType);
      if (!floatType)
        return rewriter.notifyMatchFailure(op, "constant type is unsupported");

      auto convertedValue = mlir::FloatAttr::get(floatType, value.getValue());
      rewriter.replaceOpWithNewOp<mlir::arith::ConstantOp>(op, convertedValue);
      return mlir::success();
    }

    if (auto value = mlir::dyn_cast<cir::IntAttr>(op.getValue())) {
      auto integerType =
          mlir::dyn_cast_or_null<mlir::IntegerType>(convertedType);
      if (!integerType)
        return rewriter.notifyMatchFailure(op, "constant type is unsupported");

      auto convertedValue =
          mlir::IntegerAttr::get(integerType, value.getValue());
      rewriter.replaceOpWithNewOp<mlir::arith::ConstantOp>(op, convertedValue);
      return mlir::success();
    }

    return rewriter.notifyMatchFailure(op, "constant value is unsupported");
  }
};

struct ConvertUnaryOp : public mlir::OpConversionPattern<cir::UnaryOp> {
  using OpConversionPattern::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(cir::UnaryOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    if (isSupportedCIRFloatType(op.getType())) {
      if (op.getKind() != cir::UnaryOpKind::Minus)
        return rewriter.notifyMatchFailure(
            op, "unsupported floating unary operation kind");

      rewriter.replaceOpWithNewOp<mlir::arith::NegFOp>(op, adaptor.getInput());
      return mlir::success();
    }

    if (mlir::isa<cir::IntType>(op.getType())) {
      mlir::Value input = adaptor.getInput();
      mlir::arith::IntegerOverflowFlags overflowFlags =
          op.getNoSignedWrap() ? mlir::arith::IntegerOverflowFlags::nsw
                               : mlir::arith::IntegerOverflowFlags::none;

      switch (op.getKind()) {
      case cir::UnaryOpKind::Inc: {
        auto one = rewriter.create<mlir::arith::ConstantOp>(
            op.getLoc(), rewriter.getIntegerAttr(input.getType(), 1));
        rewriter.replaceOpWithNewOp<mlir::arith::AddIOp>(op, input, one,
                                                         overflowFlags);
        return mlir::success();
      }
      case cir::UnaryOpKind::Dec: {
        auto one = rewriter.create<mlir::arith::ConstantOp>(
            op.getLoc(), rewriter.getIntegerAttr(input.getType(), 1));
        rewriter.replaceOpWithNewOp<mlir::arith::SubIOp>(op, input, one,
                                                         overflowFlags);
        return mlir::success();
      }
      case cir::UnaryOpKind::Plus:
        rewriter.replaceOp(op, input);
        return mlir::success();
      case cir::UnaryOpKind::Minus: {
        auto zero = rewriter.create<mlir::arith::ConstantOp>(
            op.getLoc(), rewriter.getIntegerAttr(input.getType(), 0));
        rewriter.replaceOpWithNewOp<mlir::arith::SubIOp>(op, zero, input,
                                                         overflowFlags);
        return mlir::success();
      }
      default:
        return rewriter.notifyMatchFailure(
            op, "unsupported integer unary operation kind");
      }
    }

    return rewriter.notifyMatchFailure(op,
                                       "unary operation type is unsupported");
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
    mlir::Type operandType = op.getLhs().getType();
    if (isSupportedCIRFloatType(operandType)) {
      std::optional<mlir::arith::CmpFPredicate> predicate =
          convertCIRFloatCmpPredicate(op.getKind());
      if (!predicate)
        return rewriter.notifyMatchFailure(op, "unsupported comparison kind");

      rewriter.replaceOpWithNewOp<mlir::arith::CmpFOp>(
          op, *predicate, adaptor.getLhs(), adaptor.getRhs());
      return mlir::success();
    }

    if (auto integerType = mlir::dyn_cast<cir::IntType>(operandType)) {
      std::optional<mlir::arith::CmpIPredicate> predicate =
          convertCIRIntCmpPredicate(op.getKind(), integerType.isSigned());
      if (!predicate)
        return rewriter.notifyMatchFailure(op, "unsupported comparison kind");

      rewriter.replaceOpWithNewOp<mlir::arith::CmpIOp>(
          op, *predicate, adaptor.getLhs(), adaptor.getRhs());
      return mlir::success();
    }

    return rewriter.notifyMatchFailure(op, "comparison type is unsupported");
  }
};

struct ConvertCastOp : public mlir::OpConversionPattern<cir::CastOp> {
  using OpConversionPattern::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(cir::CastOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    if (op.getKind() == cir::CastKind::int_to_bool) {
      if (!mlir::isa<cir::IntType>(op.getSrc().getType()))
        return rewriter.notifyMatchFailure(
            op, "integer-to-bool source type is unsupported");
      if (!mlir::isa<cir::BoolType>(op.getType()))
        return rewriter.notifyMatchFailure(
            op, "integer-to-bool result type is unsupported");

      mlir::Value source = adaptor.getSrc();
      auto zero = rewriter.create<mlir::arith::ConstantOp>(
          op.getLoc(), rewriter.getIntegerAttr(source.getType(), 0));
      rewriter.replaceOpWithNewOp<mlir::arith::CmpIOp>(
          op, mlir::arith::CmpIPredicate::ne, source, zero);
      return mlir::success();
    }

    if (op.getKind() == cir::CastKind::int_to_float) {
      auto sourceType = mlir::dyn_cast<cir::IntType>(op.getSrc().getType());
      if (!sourceType)
        return rewriter.notifyMatchFailure(
            op, "integer-to-float source type is unsupported");
      if (!mlir::isa<cir::SingleType>(op.getType()))
        return rewriter.notifyMatchFailure(
            op, "only !cir.float integer conversion results are supported");

      mlir::Type resultType = getTypeConverter()->convertType(op.getType());
      if (!resultType)
        return mlir::failure();

      if (sourceType.isSigned())
        rewriter.replaceOpWithNewOp<mlir::arith::SIToFPOp>(op, resultType,
                                                           adaptor.getSrc());
      else
        rewriter.replaceOpWithNewOp<mlir::arith::UIToFPOp>(op, resultType,
                                                           adaptor.getSrc());
      return mlir::success();
    }

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
