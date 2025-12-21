#include "TinyP/FilterEval.h"

namespace TinyP {
    FilterEval::FilterEval(Vars vars)
    : ExprEval(std::move(vars)) {}

    void FilterEval::visitExprOp(const ExprOpPtr& f)
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

        bool resVal = false;
        auto rhs = res_.value();
        if (f->op() == "=") {
            if (lhs->isNumeric() ^ rhs->isNumeric()) {
                resVal = false;
            } else if (lhs->isNumeric()) {
                resVal = std::atof(lhs->data().c_str()) == std::atof(rhs->data().c_str());
            } else {
                resVal = lhs->data() == rhs->data();
            }
        } else if (f->op() == "<") {
            if (lhs->isNumeric() ^ rhs->isNumeric()) {
                resVal = false;
            } else if (lhs->isNumeric()) {
                resVal = std::atof(lhs->data().c_str()) < std::atof(rhs->data().c_str());
            } else {
                resVal = lhs->data() < rhs->data();
            }
        } else if (f->op() == ">") {
            if (lhs->isNumeric() ^ rhs->isNumeric()) {
                resVal = false;
            } else if (lhs->isNumeric()) {
                resVal = std::atof(lhs->data().c_str()) > std::atof(rhs->data().c_str());
            } else {
                resVal = lhs->data() > rhs->data();
            }
        } else if (f->op() == "<=") {
            if (lhs->isNumeric() ^ rhs->isNumeric()) {
                resVal = false;
            } else if (lhs->isNumeric()) {
                resVal = std::atof(lhs->data().c_str()) <= std::atof(rhs->data().c_str());
            } else {
                resVal = lhs->data() <= rhs->data();
            }
        } else if (f->op() == ">=") {
            if (lhs->isNumeric() ^ rhs->isNumeric()) {
                resVal = false;
            } else if (lhs->isNumeric()) {
                resVal = std::atof(lhs->data().c_str()) >= std::atof(rhs->data().c_str());
            } else {
                resVal = lhs->data() >= rhs->data();
            }
        } else if (f->op() == "<>") {
            if (lhs->isNumeric() ^ rhs->isNumeric()) {
                resVal = true;
            } else if (lhs->isNumeric()) {
                resVal = std::atof(lhs->data().c_str()) != std::atof(rhs->data().c_str());
            } else {
                resVal = lhs->data() != rhs->data();
            }
        } else if (f->op() == "&") {
            resVal = (lhs->data() != "0") && (rhs->data() != "0");
        } else if (f->op() == "|") {
            resVal = (lhs->data() != "0") || (rhs->data() != "0");
        } else {
            res_ = Error("operator \"" + f->op() + "\" not supported", f->lineNumber(), f->columnNumber());
            return;
        }
        res_ = std::make_shared<ExprConst>((resVal ? "1" : "0"), true, f->lineNumber(), f->columnNumber());
    }
}
