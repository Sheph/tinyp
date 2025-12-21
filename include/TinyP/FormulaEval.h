#ifndef _TINYP_FORMULAEVAL_H_
#define _TINYP_FORMULAEVAL_H_

#include "TinyP/ExprEval.h"

namespace TinyP {
    class FormulaEval : public ExprEval
    {
    public:
        explicit FormulaEval(Vars vars);

        void visitExprOp(const ExprOpPtr& f) override;
    };
}

#endif
