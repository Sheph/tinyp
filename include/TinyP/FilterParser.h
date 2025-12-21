#ifndef _TINYP_FILTERPARSER_H_
#define _TINYP_FILTERPARSER_H_

#include "TinyP/Parser.h"
#include "TinyP/Expr.h"

namespace TinyP {
    /*
     * Class that parses a filter string into 'ExprOp' object.
     * 'is' has the following BNF syntax (terminals marked as "/.../"):
     * all -> or_expr
     * or_expr -> and_expr /|/ or_expr | and_expr
     * and_expr -> expr /&/ and_expr | expr
     * expr -> basic_expr | /(/ or_expr /)/
     * basic_expr -> value_expr relation value_expr
     * relation -> /=/ | /</ | />/ | /<=/ | />=/ | /<>/
     * value_expr -> const_expr | var
     * const_expr -> /literal/ | numeric_expr
     * numeric_expr -> /-/ /number/ | /+/ /number/ | /number/
     * var -> /id/
     */
    class FilterParser : public Parser
    {
    public:
        explicit FilterParser(std::istream& is);

        Result<ExprOpPtr> parse();

    private:
        Result<ExprOpPtr> orExpr();
        Result<ExprOpPtr> andExpr();
        Result<ExprOpPtr> expr();
        Result<ExprOpPtr> basicExpr();
        Result<ExprPtr> valueExpr();
        Result<ExprConstPtr> constExpr();
        Result<ExprConstPtr> numericExpr();
    };
}

#endif
