#include "TinyP/ExprEval.h"

namespace TinyP {
    ExprEval::ExprEval(Vars vars)
    : vars_(std::move(vars)) {}

    void ExprEval::visitExprConst(const ExprConstPtr& f)
    {
        res_ = f;
    }

    void ExprEval::visitExprVar(const ExprVarPtr& f)
    {
        auto it = vars_.find(f->name());
        if (it == vars_.end()) {
            res_ = Error("variable \"" + f->name() + "\" not defined", f->lineNumber(), f->columnNumber());
        } else {
            res_ = std::make_shared<ExprConst>(it->second.first, it->second.second, f->lineNumber(), f->columnNumber());
        }
    }
}
