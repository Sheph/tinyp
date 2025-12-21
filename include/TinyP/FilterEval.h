#ifndef _TINYP_FILTEREVAL_H_
#define _TINYP_FILTEREVAL_H_

#include "TinyP/ExprEval.h"

namespace TinyP {
    class FilterEval : public ExprEval
    {
    public:
        explicit FilterEval(Vars vars);

        void visitExprOp(const ExprOpPtr& f) override;
    };
}

#endif
