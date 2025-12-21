#include "TinyP/ExprPrinter.h"
#include "TinyP/Expr.h"

namespace TinyP {
    ExprPrinter::ExprPrinter(std::ostream& os)
    : os_(os) {}

    void ExprPrinter::visitExprOp(const ExprOpPtr& f)
    {
        os_ << "(";
        f->lhs()->accept(*this);
        os_ << " " << f->op() << " ";
        f->rhs()->accept(*this);
        os_ << ")";
    }

    void ExprPrinter::visitExprConst(const ExprConstPtr& f)
    {
        if (f->isNumeric()) {
            os_ << f->data();
        } else {
            os_ << "\"" << f->data() << "\"";
        }
    }

    void ExprPrinter::visitExprVar(const ExprVarPtr& f)
    {
        os_ << f->name();
    }
}
