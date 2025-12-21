#include "TinyP/FilterParser.h"

namespace TinyP {
    FilterParser::FilterParser(std::istream& is)
    : Parser(is) {}

    Result<ExprOpPtr> FilterParser::parse()
    {
        if (auto err = init()) {
            return err;
        }

        auto res = orExpr();
        if (res.err()) {
            return res.err();
        }

        if (auto err = match(Token::Eos)) {
            return err;
        }

        return res;
    }

    Result<ExprOpPtr> FilterParser::orExpr()
    {
        auto lhs = andExpr();
        if (lhs.err()) {
            return lhs.err();
        }

        while ((lookahead().type == Token::Op) && (lookahead().value == "|")) {
            if (auto err = match(lookahead().type)) {
                return err;
            }
            auto rhs = andExpr();
            if (rhs.err()) {
                return rhs.err();
            }
            lhs = std::make_shared<ExprOp>(lhs.value(), "|", rhs.value());
        }

        return lhs;
    }

    Result<ExprOpPtr> FilterParser::andExpr()
    {
        auto lhs = expr();
        if (lhs.err()) {
            return lhs.err();
        }

        while ((lookahead().type == Token::Op) && (lookahead().value == "&")) {
            if (auto err = match(lookahead().type)) {
                return err;
            }
            auto rhs = expr();
            if (rhs.err()) {
                return rhs.err();
            }
            lhs = std::make_shared<ExprOp>(lhs.value(), "&", rhs.value());
        }

        return lhs;
    }

    Result<ExprOpPtr> FilterParser::expr()
    {
        if (lookahead().type == Token::LeftBr) {
            if (auto err = match(lookahead().type)) {
                return err;
            }

            auto ret = orExpr();
            if (ret.err()) {
                return ret.err();
            }

            if (auto err = match(Token::RightBr)) {
                return err;
            }

            return ret;
        } else {
            return basicExpr();
        }
    }

    Result<ExprOpPtr> FilterParser::basicExpr()
    {
        auto lhs = valueExpr();
        if (lhs.err()) {
            return lhs.err();
        }

        auto op = lookahead().value;

        if (auto err = match(Token::Op, {"=", "<", ">", "<=", ">=", "<>"})) {
            return err;
        }

        auto rhs = valueExpr();
        if (rhs.err()) {
            return rhs.err();
        }

        return std::make_shared<ExprOp>(lhs.value(), op, rhs.value());
    }

    Result<ExprPtr> FilterParser::valueExpr()
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

    Result<ExprConstPtr> FilterParser::constExpr()
    {
        if (lookahead().type == Token::Literal) {
            auto tmp = lookahead();
            if (auto err = match(lookahead().type)) {
                return err;
            }
            return std::make_shared<ExprConst>(tmp.value, false, tmp.lineNumber, tmp.columnNumber);
        } else {
            return numericExpr();
        }
    }

    Result<ExprConstPtr> FilterParser::numericExpr()
    {
        if (lookahead().type == Token::Number) {
            auto tmp = lookahead();
            if (auto err = match(lookahead().type)) {
                return err;
            }
            return std::make_shared<ExprConst>(tmp.value, true, tmp.lineNumber, tmp.columnNumber);
        } else {
            auto lineNumber = lookahead().lineNumber;
            auto columnNumber = lookahead().columnNumber;
            bool isMinus = (lookahead().value == "-");
            if (auto err = match(Token::Op, {"-", "+"})) {
                return err;
            }
            auto value = lookahead().value;
            if (auto err = match(Token::Number)) {
                return err;
            }
            return std::make_shared<ExprConst>((isMinus ? "-" : "") + value, true, lineNumber, columnNumber);
        }
    }
}
