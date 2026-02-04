#ifndef _TINYP_SQLSTMT_H_
#define _TINYP_SQLSTMT_H_

#include "TinyP/Expr.h"
#include <vector>
#include <string>

namespace TinyP {
    class SqlSelectStmt : public std::enable_shared_from_this<SqlSelectStmt>, public Expr
    {
    public:
        SqlSelectStmt(std::vector<std::string> columns, std::string tableName, ExprPtr whereClause = nullptr)
        : Expr(0, 0), columns_(std::move(columns)), tableName_(std::move(tableName)), whereClause_(std::move(whereClause)), selectAll_(false) {}

        SqlSelectStmt(std::string tableName, ExprPtr whereClause = nullptr)
        : Expr(0, 0), tableName_(std::move(tableName)), whereClause_(std::move(whereClause)), selectAll_(true) {}

        inline bool selectAll() const { return selectAll_; }
        inline const std::vector<std::string>& columns() const { return columns_; }
        inline const std::string& tableName() const { return tableName_; }
        inline const ExprPtr& whereClause() const { return whereClause_; }

        void accept(ExprVisitor& visitor) override { visitor.visitSqlSelectStmt(shared_from_this()); }

    private:
        std::vector<std::string> columns_;
        std::string tableName_;
        ExprPtr whereClause_;
        bool selectAll_;
    };
}

#endif
