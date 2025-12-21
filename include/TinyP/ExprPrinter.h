#ifndef _TINYP_EXPRPRINTER_H_
#define _TINYP_EXPRPRINTER_H_

#include "TinyP/ExprVisitor.h"
#include <ostream>

namespace TinyP {
    class ExprPrinter : public ExprVisitor
    {
    public:
        explicit ExprPrinter(std::ostream& os);

        void visitExprOp(const ExprOpPtr& f) override;
        void visitExprConst(const ExprConstPtr& f) override;
        void visitExprVar(const ExprVarPtr& f) override;

    private:
        std::ostream& os_;
    };
}

#endif
