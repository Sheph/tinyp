#include "TinyP/SqlEval.h"
#include <cmath>
#include <sstream>

namespace TinyP {
    SqlEval::SqlEval(TablePtr table)
    : table_(std::move(table)), currentRow_(nullptr) {}

    void SqlEval::visitSqlSelectStmt(const SqlSelectStmtPtr& stmt)
    {
        // Verify table matches
        if (table_->name() != stmt->tableName()) {
            queryResult_ = Error("table \"" + stmt->tableName() + "\" not found (have \"" + table_->name() + "\")", 0, 0);
            return;
        }

        // Determine which columns to select
        std::vector<size_t> columnIndices;
        auto result = std::make_shared<QueryResult>();

        if (stmt->selectAll()) {
            // SELECT * - use all columns
            for (size_t i = 0; i < table_->columnCount(); ++i) {
                columnIndices.push_back(i);
                result->addColumn(table_->columns()[i].name, table_->columns()[i].isNumeric);
            }
        } else {
            // Specific columns
            for (const auto& colName : stmt->columns()) {
                auto indexResult = table_->getColumnIndex(colName);
                if (indexResult.err()) {
                    queryResult_ = indexResult.err();
                    return;
                }
                columnIndices.push_back(indexResult.value());
                result->addColumn(colName, table_->columns()[indexResult.value()].isNumeric);
            }
        }

        // Process each row
        for (const auto& row : table_->rows()) {
            currentRow_ = &row;

            // Check WHERE clause if present
            if (stmt->whereClause()) {
                auto whereResult = evaluateWhere(stmt->whereClause());
                if (whereResult.err()) {
                    queryResult_ = whereResult.err();
                    return;
                }
                if (!whereResult.value()) {
                    continue; // Row doesn't match WHERE clause
                }
            }

            // Build result row with selected columns
            Row resultRow;
            for (size_t idx : columnIndices) {
                resultRow.push_back(row[idx]);
            }
            result->addRow(resultRow);
        }

        currentRow_ = nullptr;
        queryResult_ = result;
    }

    Result<bool> SqlEval::evaluateWhere(const ExprPtr& where)
    {
        exprResult_ = Result<ExprConstPtr>();  // Reset
        where->accept(*this);

        if (exprResult_.err()) {
            return exprResult_.err();
        }

        // Result is truthy if not "0"
        const auto& val = exprResult_.value();
        if (val->isNumeric()) {
            return std::atof(val->data().c_str()) != 0;
        }
        return !val->data().empty();
    }

    void SqlEval::visitExprConst(const ExprConstPtr& f)
    {
        exprResult_ = f;
    }

    void SqlEval::visitExprVar(const ExprVarPtr& f)
    {
        if (!currentRow_) {
            exprResult_ = Error("variable evaluation outside row context", f->lineNumber(), f->columnNumber());
            return;
        }

        auto indexResult = table_->getColumnIndex(f->name());
        if (indexResult.err()) {
            exprResult_ = Error("unknown column: " + f->name(), f->lineNumber(), f->columnNumber());
            return;
        }

        const auto& val = (*currentRow_)[indexResult.value()];
        exprResult_ = std::make_shared<ExprConst>(val.data, val.isNumeric, f->lineNumber(), f->columnNumber());
    }

    void SqlEval::visitExprOp(const ExprOpPtr& f)
    {
        // Evaluate left operand
        f->lhs()->accept(*this);
        if (exprResult_.err()) {
            return;
        }
        auto lhs = exprResult_.value();

        // Evaluate right operand
        f->rhs()->accept(*this);
        if (exprResult_.err()) {
            return;
        }
        auto rhs = exprResult_.value();

        const auto& op = f->op();

        // Determine operation type
        if (op == "+" || op == "-" || op == "*" || op == "/") {
            exprResult_ = evalArithmetic(lhs, op, rhs, f->lineNumber(), f->columnNumber());
        } else if (op == "=" || op == "<" || op == ">" || op == "<=" || op == ">=" || op == "<>") {
            exprResult_ = evalComparison(lhs, op, rhs, f->lineNumber(), f->columnNumber());
        } else if (op == "&" || op == "|") {
            exprResult_ = evalLogical(lhs, op, rhs, f->lineNumber(), f->columnNumber());
        } else {
            exprResult_ = Error("unknown operator: " + op, f->lineNumber(), f->columnNumber());
        }
    }

    Result<ExprConstPtr> SqlEval::evalArithmetic(const ExprConstPtr& lhs, const std::string& op, const ExprConstPtr& rhs,
                                                  std::uint32_t lineNumber, std::uint32_t columnNumber)
    {
        // For arithmetic, both operands should be numeric
        if (!lhs->isNumeric() || !rhs->isNumeric()) {
            // Handle string concatenation for +
            if (op == "+") {
                return std::make_shared<ExprConst>(lhs->data() + rhs->data(), false, lineNumber, columnNumber);
            }
            // Handle string repetition for *
            if (op == "*" && (lhs->isNumeric() || rhs->isNumeric())) {
                std::string str = lhs->isNumeric() ? rhs->data() : lhs->data();
                int count = static_cast<int>(std::atof(lhs->isNumeric() ? lhs->data().c_str() : rhs->data().c_str()));
                std::string result;
                for (int i = 0; i < count; ++i) {
                    result += str;
                }
                return std::make_shared<ExprConst>(result, false, lineNumber, columnNumber);
            }
            std::ostringstream os;
            os << *lhs << " " << op << " " << *rhs << " - operation not supported";
            return Error(os.str(), lineNumber, columnNumber);
        }

        double l = std::atof(lhs->data().c_str());
        double r = std::atof(rhs->data().c_str());
        double result;

        if (op == "+") {
            result = l + r;
        } else if (op == "-") {
            result = l - r;
        } else if (op == "*") {
            result = l * r;
        } else if (op == "/") {
            if (r == 0) {
                return Error("division by zero", lineNumber, columnNumber);
            }
            result = l / r;
        } else {
            return Error("unknown arithmetic operator: " + op, lineNumber, columnNumber);
        }

        return std::make_shared<ExprConst>(std::to_string(result), true, lineNumber, columnNumber);
    }

    Result<ExprConstPtr> SqlEval::evalComparison(const ExprConstPtr& lhs, const std::string& op, const ExprConstPtr& rhs,
                                                  std::uint32_t lineNumber, std::uint32_t columnNumber)
    {
        bool result = false;

        // Type mismatch handling
        if (lhs->isNumeric() != rhs->isNumeric()) {
            // Different types: only <> returns true, = returns false
            if (op == "<>") {
                result = true;
            } else if (op == "=") {
                result = false;
            } else {
                // For <, >, <=, >= with mixed types, return false
                result = false;
            }
        } else if (lhs->isNumeric()) {
            // Both numeric
            double l = std::atof(lhs->data().c_str());
            double r = std::atof(rhs->data().c_str());

            if (op == "=") {
                result = (l == r);
            } else if (op == "<") {
                result = (l < r);
            } else if (op == ">") {
                result = (l > r);
            } else if (op == "<=") {
                result = (l <= r);
            } else if (op == ">=") {
                result = (l >= r);
            } else if (op == "<>") {
                result = (l != r);
            }
        } else {
            // Both strings
            const auto& l = lhs->data();
            const auto& r = rhs->data();

            if (op == "=") {
                result = (l == r);
            } else if (op == "<") {
                result = (l < r);
            } else if (op == ">") {
                result = (l > r);
            } else if (op == "<=") {
                result = (l <= r);
            } else if (op == ">=") {
                result = (l >= r);
            } else if (op == "<>") {
                result = (l != r);
            }
        }

        return std::make_shared<ExprConst>(result ? "1" : "0", true, lineNumber, columnNumber);
    }

    Result<ExprConstPtr> SqlEval::evalLogical(const ExprConstPtr& lhs, const std::string& op, const ExprConstPtr& rhs,
                                               std::uint32_t lineNumber, std::uint32_t columnNumber)
    {
        // For logical operations, non-zero/non-empty is true
        bool lVal = lhs->isNumeric() ? (std::atof(lhs->data().c_str()) != 0) : !lhs->data().empty();
        bool rVal = rhs->isNumeric() ? (std::atof(rhs->data().c_str()) != 0) : !rhs->data().empty();

        bool result;
        if (op == "&") {
            result = lVal && rVal;
        } else if (op == "|") {
            result = lVal || rVal;
        } else {
            return Error("unknown logical operator: " + op, lineNumber, columnNumber);
        }

        return std::make_shared<ExprConst>(result ? "1" : "0", true, lineNumber, columnNumber);
    }
}
