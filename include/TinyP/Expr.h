#ifndef _TINYP_EXPR_H_
#define _TINYP_EXPR_H_

#include "TinyP/ExprVisitor.h"
#include <iostream>
#include <cstdint>

namespace TinyP {
    class Expr
    {
    public:
        Expr(std::uint32_t lineNumber, std::uint32_t columnNumber)
        : lineNumber_(lineNumber), columnNumber_(columnNumber) {}
        virtual ~Expr() = default;

        inline std::uint32_t lineNumber() const { return lineNumber_; }
        inline std::uint32_t columnNumber() const { return columnNumber_; }

        virtual void accept(ExprVisitor& visitor) = 0;

    private:
        const std::uint32_t lineNumber_;
        const std::uint32_t columnNumber_;
    };

    class ExprConst : public std::enable_shared_from_this<ExprConst>, public Expr
    {
    public:
        ExprConst(std::string data, bool isNumeric, std::uint32_t lineNumber = 0, std::uint32_t columnNumber = 0)
        : Expr(lineNumber, columnNumber), data_(std::move(data)), isNumeric_(isNumeric) {}

        inline const std::string& data() const { return data_; }
        inline bool isNumeric() const { return isNumeric_; }

        inline void accept(ExprVisitor& visitor) override { visitor.visitExprConst(shared_from_this()); };

    private:
        const std::string data_;
        const bool isNumeric_;
    };

    class ExprVar : public std::enable_shared_from_this<ExprVar>, public Expr
    {
    public:
        explicit ExprVar(std::string name, std::uint32_t lineNumber = 0, std::uint32_t columnNumber = 0)
        : Expr(lineNumber, columnNumber), name_(std::move(name)) {}

        inline const std::string& name() const { return name_; }

        inline void accept(ExprVisitor& visitor) override { visitor.visitExprVar(shared_from_this()); };

    private:
        const std::string name_;
    };

    class ExprOp : public std::enable_shared_from_this<ExprOp>, public Expr
    {
    public:
        ExprOp(ExprPtr lhs, std::string op, ExprPtr rhs)
        : Expr(lhs->lineNumber(), lhs->columnNumber()), lhs_(std::move(lhs)), op_(std::move(op)), rhs_(std::move(rhs)) {}

        inline const ExprPtr& lhs() const { return lhs_; }
        inline const std::string& op() const { return op_; }
        inline const ExprPtr& rhs() const { return rhs_; }

        inline void accept(ExprVisitor& visitor) override { visitor.visitExprOp(shared_from_this()); };

    private:
        const ExprPtr lhs_;
        const std::string op_;
        const ExprPtr rhs_;
    };
}

inline std::ostream& operator<<(std::ostream& os, const TinyP::ExprConst& expr)
{
    if (expr.isNumeric()) {
        os << "num(" << std::atof(expr.data().c_str()) << ")";
    } else {
        os << "str(" << expr.data() << ")";
    }
    return os;
}

#endif
