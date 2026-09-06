#include "ClangIRTAFFO/Conversion/Passes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SCF/Transforms/Patterns.h"
#include "mlir/IR/IRMapping.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace clangir_taffo {
#define GEN_PASS_DEF_CONVERTLIFTEDCFLOOPSTOSCFFORPASS
#include "ClangIRTAFFO/Conversion/Passes.h.inc"
} // namespace clangir_taffo

namespace clangir_taffo {
namespace {

class NormalizeLiftedCountedLoop
    : public mlir::OpRewritePattern<mlir::scf::WhileOp> {
public:
  using OpRewritePattern::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(mlir::scf::WhileOp whileOp,
                  mlir::PatternRewriter &rewriter) const override {
    mlir::Block *before = whileOp.getBeforeBody();
    mlir::Block *after = whileOp.getAfterBody();
    unsigned numCarried = before->getNumArguments();

    if (numCarried == 0 || before->getOperations().size() != 3)
      return rewriter.notifyMatchFailure(
          whileOp, "expected a comparison, scf.if, and scf.condition");

    auto comparison =
        mlir::dyn_cast<mlir::arith::CmpIOp>(before->getOperations().front());
    auto ifOp = mlir::dyn_cast<mlir::scf::IfOp>(
        *std::next(before->getOperations().begin()));
    auto condition =
        mlir::dyn_cast<mlir::scf::ConditionOp>(before->getOperations().back());
    if (!comparison || !ifOp || !condition)
      return rewriter.notifyMatchFailure(
          whileOp, "expected a comparison, scf.if, and scf.condition");

    if (comparison.getResult() != ifOp.getCondition() ||
        comparison.getResult() != condition.getCondition())
      return rewriter.notifyMatchFailure(
          whileOp, "comparison must control both the if and while condition");

    if (ifOp.getNumResults() != numCarried ||
        condition.getArgs().size() <= numCarried ||
        condition.getArgs().take_front(numCarried) != ifOp.getResults())
      return rewriter.notifyMatchFailure(
          whileOp, "expected the if results followed by loop exit values");

    if (!ifOp.getThenRegion().hasOneBlock() ||
        !ifOp.getElseRegion().hasOneBlock())
      return rewriter.notifyMatchFailure(whileOp,
                                         "expected single-block if regions");

    auto thenYield = mlir::dyn_cast<mlir::scf::YieldOp>(
        ifOp.getThenRegion().front().getTerminator());
    if (!thenYield || thenYield.getResults().size() != numCarried)
      return rewriter.notifyMatchFailure(
          whileOp, "then region must yield all loop-carried values");

    if (after->getOperations().size() != 1)
      return rewriter.notifyMatchFailure(
          whileOp, "expected a pass-through after region");
    auto afterYield =
        mlir::dyn_cast<mlir::scf::YieldOp>(after->getTerminator());
    if (!afterYield || afterYield.getResults().size() != numCarried)
      return rewriter.notifyMatchFailure(
          whileOp, "after region must yield all loop-carried values");
    for (auto [yielded, argument] :
         llvm::zip_equal(afterYield.getResults(),
                         after->getArguments().take_front(numCarried)))
      if (yielded != argument)
        return rewriter.notifyMatchFailure(
            whileOp, "after region must pass loop-carried values through");

    for (mlir::OpResult result : whileOp.getResults().take_front(numCarried))
      if (!result.use_empty())
        return rewriter.notifyMatchFailure(
            whileOp, "intermediate lifted loop results must be unused");

    llvm::SmallVector<unsigned> exitResultIndices;
    for (mlir::Value value : condition.getArgs().drop_front(numCarried)) {
      auto argument = mlir::dyn_cast<mlir::BlockArgument>(value);
      if (!argument || argument.getOwner() != before)
        return rewriter.notifyMatchFailure(
            whileOp, "exit values must be loop-carried block arguments");
      exitResultIndices.push_back(argument.getArgNumber());
    }

    auto normalizedWhile = rewriter.create<mlir::scf::WhileOp>(
        whileOp.getLoc(), whileOp.getInits().getTypes(), whileOp.getInits(),
        [&](mlir::OpBuilder &builder, mlir::Location location,
            mlir::ValueRange arguments) {
          mlir::IRMapping mapping;
          mapping.map(before->getArguments(), arguments);
          mlir::Operation *newComparison =
              builder.clone(*comparison.getOperation(), mapping);
          builder.create<mlir::scf::ConditionOp>(
              location, newComparison->getResult(0), arguments);
        },
        [&](mlir::OpBuilder &builder, mlir::Location location,
            mlir::ValueRange arguments) {
          mlir::IRMapping mapping;
          mapping.map(before->getArguments(), arguments);
          for (mlir::Operation &operation :
               ifOp.getThenRegion().front().without_terminator())
            builder.clone(operation, mapping);

          llvm::SmallVector<mlir::Value> yieldedValues;
          for (mlir::Value value : thenYield.getResults())
            yieldedValues.push_back(mapping.lookupOrDefault(value));
          builder.create<mlir::scf::YieldOp>(location, yieldedValues);
        });

    for (auto [oldResult, carriedIndex] : llvm::zip_equal(
             whileOp.getResults().drop_front(numCarried), exitResultIndices))
      rewriter.replaceAllUsesWith(oldResult,
                                  normalizedWhile.getResult(carriedIndex));

    rewriter.eraseOp(whileOp);
    return mlir::success();
  }
};

class ConvertLiftedCFLoopsToSCFForPass
    : public impl::ConvertLiftedCFLoopsToSCFForPassBase<
          ConvertLiftedCFLoopsToSCFForPass> {
public:
  using ConvertLiftedCFLoopsToSCFForPassBase::
      ConvertLiftedCFLoopsToSCFForPassBase;

  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();

    mlir::RewritePatternSet normalizationPatterns(context);
    normalizationPatterns.add<NormalizeLiftedCountedLoop>(context);
    if (mlir::failed(mlir::applyPatternsGreedily(
            getOperation(), std::move(normalizationPatterns)))) {
      signalPassFailure();
      return;
    }

    mlir::RewritePatternSet upliftPatterns(context);
    mlir::scf::populateUpliftWhileToForPatterns(upliftPatterns);
    if (mlir::failed(mlir::applyPatternsGreedily(getOperation(),
                                                 std::move(upliftPatterns))))
      signalPassFailure();
  }
};

} // namespace
} // namespace clangir_taffo
