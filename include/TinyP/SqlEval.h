#ifndef _TINYP_SQLEVAL_H_
#define _TINYP_SQLEVAL_H_

#include "TinyP/ExprVisitor.h"
#include "TinyP/Expr.h"
#include "TinyP/Error.h"
#include "TinyP/Table.h"
#include "TinyP/SqlStmt.h"

namespace TinyP {
    // Result of a SQL query
    class QueryResult
    {
    public:
        QueryResult() = default;

        void addColumn(const std::string& name, bool isNumeric = false) {
            columns_.push_back(Column(name, isNumeric));
        }

        void addRow(const Row& row) {
            rows_.push_back(row);
        }

        inline const std::vector<Column>& columns() const { return columns_; }
        inline const std::vector<Row>& rows() const { return rows_; }
        inline size_t columnCount() const { return columns_.size(); }
        inline size_t rowCount() const { return rows_.size(); }

    private:
        std::vector<Column> columns_;
        std::vector<Row> rows_;
    };

    using QueryResultPtr = std::shared_ptr<QueryResult>;

    // Evaluator for SQL statements.
    // Usage:
    //   SqlEval eval(table);
    //   stmt->accept(eval);
    //   auto result = eval.result();
    class SqlEval : public ExprVisitor
    {
    public:
        explicit SqlEval(TablePtr table);

        // Get the query result after execution
        inline const Result<QueryResultPtr>& result() const { return queryResult_; }

        // ExprVisitor interface
        void visitSqlSelectStmt(const SqlSelectStmtPtr& stmt) override;
        void visitExprOp(const ExprOpPtr& f) override;
        void visitExprConst(const ExprConstPtr& f) override;
        void visitExprVar(const ExprVarPtr& f) override;

    private:
        // Evaluate WHERE expression for current row
        Result<bool> evaluateWhere(const ExprPtr& where);

        // Expression evaluation helpers
        Result<ExprConstPtr> evalArithmetic(const ExprConstPtr& lhs, const std::string& op, const ExprConstPtr& rhs,
                                            std::uint32_t lineNumber, std::uint32_t columnNumber);
        Result<ExprConstPtr> evalComparison(const ExprConstPtr& lhs, const std::string& op, const ExprConstPtr& rhs,
                                            std::uint32_t lineNumber, std::uint32_t columnNumber);
        Result<ExprConstPtr> evalLogical(const ExprConstPtr& lhs, const std::string& op, const ExprConstPtr& rhs,
                                         std::uint32_t lineNumber, std::uint32_t columnNumber);

        TablePtr table_;
        const Row* currentRow_;  // Current row being evaluated
        Result<ExprConstPtr> exprResult_;  // Result of expression evaluation
        Result<QueryResultPtr> queryResult_;  // Result of query execution
    };
}

#endif
