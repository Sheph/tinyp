#include "TinyP/SqlParser.h"
#include <algorithm>
#include <sstream>

namespace TinyP {
    SqlParser::SqlParser(std::istream& is)
    : Parser(is) {}

    bool SqlParser::isKeyword(const std::string& keyword) const
    {
        if (lookahead().type != Token::Id) {
            return false;
        }
        std::string val = lookahead().value;
        std::transform(val.begin(), val.end(), val.begin(), ::toupper);
        return val == keyword;
    }

    Error SqlParser::matchKeyword(const std::string& keyword)
    {
        if (isKeyword(keyword)) {
            return match(Token::Id);
        }
        std::ostringstream os;
        os << "expected keyword " << keyword << " instead of " << lookahead();
        return Error(os.str(), lookahead().lineNumber, lookahead().columnNumber);
    }

    bool SqlParser::isComparisonOp() const
    {
        if (lookahead().type != Token::Op) {
            return false;
        }
        const auto& v = lookahead().value;
        return v == "=" || v == "<" || v == ">" || v == "<=" || v == ">=" || v == "<>";
    }

    Result<SqlSelectStmtPtr> SqlParser::parse()
    {
        if (auto err = init()) {
            return err;
        }

        auto res = selectStatement();
        if (res.err()) {
            return res.err();
        }

        if (auto err = match(Token::Eos)) {
            return err;
        }

        return res;
    }

    Result<SqlSelectStmtPtr> SqlParser::selectStatement()
    {
        // Match SELECT keyword
        if (auto err = matchKeyword("SELECT")) {
            return err;
        }

        // Parse select list
        auto selectListResult = selectList();
        if (selectListResult.err()) {
            return selectListResult.err();
        }

        // Match FROM keyword
        if (auto err = matchKeyword("FROM")) {
            return err;
        }

        // Match table name
        if (lookahead().type != Token::Id) {
            std::ostringstream os;
            os << "expected table name instead of " << lookahead();
            return Error(os.str(), lookahead().lineNumber, lookahead().columnNumber);
        }
        std::string tableName = lookahead().value;
        if (auto err = match(Token::Id)) {
            return err;
        }

        // Optional WHERE clause
        ExprPtr where = nullptr;
        if (isKeyword("WHERE")) {
            auto whereResult = whereClause();
            if (whereResult.err()) {
                return whereResult.err();
            }
            where = whereResult.value();
        }

        // Create the statement
        if (selectListResult.value().empty()) {
            // SELECT *
            return std::make_shared<SqlSelectStmt>(tableName, where);
        } else {
            return std::make_shared<SqlSelectStmt>(selectListResult.value(), tableName, where);
        }
    }

    Result<std::vector<std::string>> SqlParser::selectList()
    {
        if (lookahead().type == Token::Asterisk) {
            if (auto err = match(Token::Asterisk)) {
                return err;
            }
            return std::vector<std::string>{}; // Empty means SELECT *
        } else {
            return columnList();
        }
    }

    Result<std::vector<std::string>> SqlParser::columnList()
    {
        std::vector<std::string> columns;

        // First column
        if (lookahead().type != Token::Id) {
            std::ostringstream os;
            os << "expected column name instead of " << lookahead();
            return Error(os.str(), lookahead().lineNumber, lookahead().columnNumber);
        }
        columns.push_back(lookahead().value);
        if (auto err = match(Token::Id)) {
            return err;
        }

        // Additional columns
        while (lookahead().type == Token::Comma) {
            if (auto err = match(Token::Comma)) {
                return err;
            }
            if (lookahead().type != Token::Id) {
                std::ostringstream os;
                os << "expected column name instead of " << lookahead();
                return Error(os.str(), lookahead().lineNumber, lookahead().columnNumber);
            }
            columns.push_back(lookahead().value);
            if (auto err = match(Token::Id)) {
                return err;
            }
        }

        return columns;
    }

    Result<ExprPtr> SqlParser::whereClause()
    {
        if (auto err = matchKeyword("WHERE")) {
            return err;
        }
        return orExpr();
    }

    Result<ExprPtr> SqlParser::orExpr()
    {
        auto lhs = andExpr();
        if (lhs.err()) {
            return lhs.err();
        }

        while ((lookahead().type == Token::Op && lookahead().value == "|") || isKeyword("OR")) {
            if (lookahead().type == Token::Op) {
                if (auto err = match(Token::Op)) {
                    return err;
                }
            } else {
                if (auto err = matchKeyword("OR")) {
                    return err;
                }
            }
            auto rhs = andExpr();
            if (rhs.err()) {
                return rhs.err();
            }
            ExprPtr newLhs = std::make_shared<ExprOp>(lhs.value(), "|", rhs.value());
            lhs = newLhs;
        }

        return lhs;
    }

    Result<ExprPtr> SqlParser::andExpr()
    {
        auto lhs = comparisonExpr();
        if (lhs.err()) {
            return lhs.err();
        }

        while ((lookahead().type == Token::Op && lookahead().value == "&") || isKeyword("AND")) {
            if (lookahead().type == Token::Op) {
                if (auto err = match(Token::Op)) {
                    return err;
                }
            } else {
                if (auto err = matchKeyword("AND")) {
                    return err;
                }
            }
            auto rhs = comparisonExpr();
            if (rhs.err()) {
                return rhs.err();
            }
            ExprPtr newLhs = std::make_shared<ExprOp>(lhs.value(), "&", rhs.value());
            lhs = newLhs;
        }

        return lhs;
    }

    Result<ExprPtr> SqlParser::comparisonExpr()
    {
        auto lhs = addExpr();
        if (lhs.err()) {
            return lhs.err();
        }

        // Optional comparison operator
        if (isComparisonOp()) {
            auto op = lookahead().value;
            if (auto err = match(Token::Op)) {
                return err;
            }
            auto rhs = addExpr();
            if (rhs.err()) {
                return rhs.err();
            }
            ExprPtr result = std::make_shared<ExprOp>(lhs.value(), op, rhs.value());
            return result;
        }

        return lhs;
    }

    Result<ExprPtr> SqlParser::addExpr()
    {
        auto lhs = multExpr();
        if (lhs.err()) {
            return lhs.err();
        }

        while (lookahead().type == Token::Op &&
               (lookahead().value == "+" || lookahead().value == "-")) {
            auto op = lookahead().value;
            if (auto err = match(Token::Op)) {
                return err;
            }
            auto rhs = multExpr();
            if (rhs.err()) {
                return rhs.err();
            }
            ExprPtr newLhs = std::make_shared<ExprOp>(lhs.value(), op, rhs.value());
            lhs = newLhs;
        }

        return lhs;
    }

    Result<ExprPtr> SqlParser::multExpr()
    {
        auto lhs = unaryExpr();
        if (lhs.err()) {
            return lhs.err();
        }

        while (lookahead().type == Token::Asterisk ||
               (lookahead().type == Token::Op && lookahead().value == "/")) {
            std::string op = (lookahead().type == Token::Asterisk) ? "*" : "/";
            if (lookahead().type == Token::Asterisk) {
                if (auto err = match(Token::Asterisk)) {
                    return err;
                }
            } else {
                if (auto err = match(Token::Op)) {
                    return err;
                }
            }
            auto rhs = unaryExpr();
            if (rhs.err()) {
                return rhs.err();
            }
            ExprPtr newLhs = std::make_shared<ExprOp>(lhs.value(), op, rhs.value());
            lhs = newLhs;
        }

        return lhs;
    }

    Result<ExprPtr> SqlParser::unaryExpr()
    {
        if (lookahead().type == Token::Op &&
            (lookahead().value == "+" || lookahead().value == "-")) {
            auto tmp = lookahead();
            if (auto err = match(Token::Op)) {
                return err;
            }
            auto operand = primaryExpr();
            if (operand.err()) {
                return operand.err();
            }
            // Convert unary +/- to binary operation with 0
            ExprPtr zero = std::make_shared<ExprConst>("0", true, tmp.lineNumber, tmp.columnNumber);
            ExprPtr result = std::make_shared<ExprOp>(zero, tmp.value, operand.value());
            return result;
        }
        return primaryExpr();
    }

    Result<ExprPtr> SqlParser::primaryExpr()
    {
        if (lookahead().type == Token::LeftBr) {
            if (auto err = match(Token::LeftBr)) {
                return err;
            }
            auto expr = orExpr();
            if (expr.err()) {
                return expr.err();
            }
            if (auto err = match(Token::RightBr)) {
                return err;
            }
            return expr;
        } else if (lookahead().type == Token::Number) {
            auto tmp = lookahead();
            if (auto err = match(Token::Number)) {
                return err;
            }
            ExprPtr result = std::make_shared<ExprConst>(tmp.value, true, tmp.lineNumber, tmp.columnNumber);
            return result;
        } else if (lookahead().type == Token::Literal) {
            auto tmp = lookahead();
            if (auto err = match(Token::Literal)) {
                return err;
            }
            ExprPtr result = std::make_shared<ExprConst>(tmp.value, false, tmp.lineNumber, tmp.columnNumber);
            return result;
        } else if (lookahead().type == Token::Id && !isKeyword("AND") && !isKeyword("OR")) {
            auto tmp = lookahead();
            if (auto err = match(Token::Id)) {
                return err;
            }
            ExprPtr result = std::make_shared<ExprVar>(tmp.value, tmp.lineNumber, tmp.columnNumber);
            return result;
        } else {
            std::ostringstream os;
            os << "expected expression instead of " << lookahead();
            return Error(os.str(), lookahead().lineNumber, lookahead().columnNumber);
        }
    }
}
