#include "TinyP/FormulaEval.h"
#include <sstream>

namespace TinyP {
    FormulaEval::FormulaEval(Vars vars)
    : ExprEval(std::move(vars)) {}

    void FormulaEval::visitExprOp(const ExprOpPtr& f)
    {
        f->lhs()->accept(*this);
        if (res_.err()) {
            return;
        }
        auto lhs = res_.value();
        f->rhs()->accept(*this);
        if (res_.err()) {
            return;
        }
        auto rhs = res_.value();

        if (f->op() == "*") {
            if (lhs->isNumeric() && rhs->isNumeric()) {
                res_ = std::make_shared<ExprConst>(std::to_string(std::atof(lhs->data().c_str()) * std::atof(rhs->data().c_str())), true, f->lineNumber(),
                    f->columnNumber());
            } else if (!lhs->isNumeric() && rhs->isNumeric()) {
                // Repeat string N times.
                int n = std::atoi(rhs->data().c_str());
                std::ostringstream os;
                if (n >= 0) {
                    for (; n > 0; --n) {
                        os << lhs->data();
                    }
                    res_ = std::make_shared<ExprConst>(os.str(), false, f->lineNumber(), f->columnNumber());
                } else {
                    os << *lhs << " " << f->op() << " " << *rhs << " - cannot repeat string negative number of times";
                    res_ = Error(os.str(), f->lineNumber(), f->columnNumber());
                }
            } else {
                std::ostringstream os;
                os << *lhs << " " << f->op() << " " << *rhs << " - operation not supported";
                res_ = Error(os.str(), f->lineNumber(), f->columnNumber());
            }
        } else if (f->op() == "/") {
            if (lhs->isNumeric() && rhs->isNumeric()) {
                res_ = std::make_shared<ExprConst>(std::to_string(std::atof(lhs->data().c_str()) / std::atof(rhs->data().c_str())), true, f->lineNumber(),
                    f->columnNumber());
            } else {
                std::ostringstream os;
                os << *lhs << " " << f->op() << " " << *rhs << " - operation not supported";
                res_ = Error(os.str(), f->lineNumber(), f->columnNumber());
            }
        } else if (f->op() == "+") {
            if (lhs->isNumeric() && rhs->isNumeric()) {
                res_ = std::make_shared<ExprConst>(std::to_string(std::atof(lhs->data().c_str()) + std::atof(rhs->data().c_str())), true, f->lineNumber(),
                    f->columnNumber());
            } else {
                res_ = std::make_shared<ExprConst>(lhs->data() + rhs->data(), false, f->lineNumber(), f->columnNumber());
            }
        } else if (f->op() == "-") {
            if (lhs->isNumeric() && rhs->isNumeric()) {
                res_ = std::make_shared<ExprConst>(std::to_string(std::atof(lhs->data().c_str()) - std::atof(rhs->data().c_str())), true, f->lineNumber(),
                    f->columnNumber());
            } else {
                std::ostringstream os;
                os << *lhs << " " << f->op() << " " << *rhs << " - operation not supported";
                res_ = Error(os.str(), f->lineNumber(), f->columnNumber());
            }
        } else {
            res_ = Error("operator \"" + f->op() + "\" not supported", f->lineNumber(), f->columnNumber());
        }
    }
}
