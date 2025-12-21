#ifndef _TINYP_EXPREVAL_H_
#define _TINYP_EXPREVAL_H_

#include "TinyP/ExprVisitor.h"
#include "TinyP/Expr.h"
#include "TinyP/Error.h"
#include <unordered_map>

namespace TinyP {
    class ExprEval : public ExprVisitor
    {
    public:
        using Vars = std::unordered_map<std::string, std::pair<std::string, bool>>; // (name, [value, isNumeric])

        explicit ExprEval(Vars vars);

        inline const Result<ExprConstPtr>& res() const { return res_; }

        void visitExprConst(const ExprConstPtr& f) override;
        void visitExprVar(const ExprVarPtr& f) override;

    protected:
        Result<ExprConstPtr> res_;

    private:
        const Vars vars_;
    };
}

#endif
