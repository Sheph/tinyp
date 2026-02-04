#ifndef _TINYP_EXPRVISITOR_H_
#define _TINYP_EXPRVISITOR_H_

#include <memory>

namespace TinyP {
    class Expr;
    class ExprConst;
    class ExprVar;
    class ExprOp;
    class SqlSelectStmt;

    using ExprPtr = std::shared_ptr<Expr>;
    using ExprConstPtr = std::shared_ptr<ExprConst>;
    using ExprVarPtr = std::shared_ptr<ExprVar>;
    using ExprOpPtr = std::shared_ptr<ExprOp>;
    using SqlSelectStmtPtr = std::shared_ptr<SqlSelectStmt>;

    class ExprVisitor
    {
    public:
        ExprVisitor() = default;
        virtual ~ExprVisitor() = default;

        virtual void visitExprOp(const ExprOpPtr&) = 0;
        virtual void visitExprConst(const ExprConstPtr&) = 0;
        virtual void visitExprVar(const ExprVarPtr&) = 0;
        virtual void visitSqlSelectStmt(const SqlSelectStmtPtr&) { }  // Optional, default no-op
    };
}

#endif
