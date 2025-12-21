#include "TinyP/FormulaParser.h"

namespace TinyP {
    FormulaParser::FormulaParser(std::istream& is)
    : Parser(is) {}

    Result<ExprPtr> FormulaParser::parse()
    {
        if (auto err = init()) {
            return err;
        }

        auto res = plusMinusExpr();
        if (res.err()) {
            return res.err();
        }

        if (auto err = match(Token::Eos)) {
            return err;
        }

        return res;
    }

    Result<ExprPtr> FormulaParser::plusMinusExpr()
    {
        auto lhs = multDivExpr();
        if (lhs.err()) {
            return lhs.err();
        }

        while ((lookahead().type == Token::Op) && ((lookahead().value == "+") || (lookahead().value == "-"))) {
            auto op = lookahead().value;
            if (auto err = match(lookahead().type)) {
                return err;
            }
            auto rhs = multDivExpr();
            if (rhs.err()) {
                return rhs.err();
            }
            ExprPtr res = std::make_shared<ExprOp>(lhs.value(), op, rhs.value());
            lhs = res;
        }

        return lhs;
    }

    Result<ExprPtr> FormulaParser::multDivExpr()
    {
        auto lhs = signedExpr();
        if (lhs.err()) {
            return lhs.err();
        }

        while ((lookahead().type == Token::Op) && ((lookahead().value == "*") || (lookahead().value == "/"))) {
            auto op = lookahead().value;
            if (auto err = match(lookahead().type)) {
                return err;
            }
            auto rhs = signedExpr();
            if (rhs.err()) {
                return rhs.err();
            }
            ExprPtr res = std::make_shared<ExprOp>(lhs.value(), op, rhs.value());
            lhs = res;
        }

        return lhs;
    }

    Result<ExprPtr> FormulaParser::signedExpr()
    {
        if ((lookahead().type == Token::Op) && ((lookahead().value == "+") || (lookahead().value == "-"))) {
            auto tmp = lookahead();
            if (auto err = match(lookahead().type)) {
                return err;
            }
            auto rhs = expr();
            if (rhs.err()) {
                return rhs.err();
            }
            ExprPtr res = std::make_shared<ExprConst>("0", true, tmp.lineNumber, tmp.columnNumber);
            res = std::make_shared<ExprOp>(res, tmp.value, rhs.value());
            return res;
        } else {
            return expr();
        }
    }

    Result<ExprPtr> FormulaParser::expr()
    {
        if (lookahead().type == Token::LeftBr) {
            if (auto err = match(lookahead().type)) {
                return err;
            }

            auto ret = plusMinusExpr();
            if (ret.err()) {
                return ret.err();
            }

            if (auto err = match(Token::RightBr)) {
                return err;
            }

            return ret;
        } else {
            return valueExpr();
        }
    }

    Result<ExprPtr> FormulaParser::valueExpr()
    {
        ExprPtr ret;
        if (lookahead().type == Token::Id) {
            auto tmp = lookahead();
            if (auto err = match(lookahead().type)) {
                return err;
            }
            ret = std::make_shared<ExprVar>(tmp.value, tmp.lineNumber, tmp.columnNumber);
        } else {
            auto tmp = constExpr();
            if (tmp.err()) {
                return tmp.err();
            }
            ret = tmp.value();
        }
        return ret;
    }

    Result<ExprConstPtr> FormulaParser::constExpr()
    {
        if (lookahead().type == Token::Literal) {
            auto tmp = lookahead();
            if (auto err = match(lookahead().type)) {
                return err;
            }
            return std::make_shared<ExprConst>(tmp.value, false, tmp.lineNumber, tmp.columnNumber);
        } else {
            auto tmp = lookahead();
            if (auto err = match(Token::Number)) {
                return err;
            }
            return std::make_shared<ExprConst>(tmp.value, true, tmp.lineNumber, tmp.columnNumber);
        }
    }
}
