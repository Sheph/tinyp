#ifndef _TINYP_FORMULAPARSER_H_
#define _TINYP_FORMULAPARSER_H_

#include "TinyP/Parser.h"
#include "TinyP/Expr.h"

namespace TinyP {
    /*
     * Class that parses a formula string into 'Expr' object.
     * 'is' has the following BNF syntax (terminals marked as "/.../"):
     * all -> plus_minus_expr
     * plus_minus_expr -> plus_minus_expr /+,-/ plus_minus_expr | mult_div_expr
     * mult_div_expr -> signed_expr /x,// mult_div_expr | signed_expr
     * signed_expr -> /-/ /expr/ | /+/ /expr/ | /expr/
     * expr -> value_expr | /(/ plus_minus_expr /)/
     * value_expr -> const_expr | var
     * const_expr -> /literal/ | /number/
     * var -> /id/
     */
    class FormulaParser : public Parser
    {
    public:
        explicit FormulaParser(std::istream& is);

        Result<ExprPtr> parse();

    private:
        Result<ExprPtr> plusMinusExpr();
        Result<ExprPtr> multDivExpr();
        Result<ExprPtr> signedExpr();
        Result<ExprPtr> expr();
        Result<ExprPtr> valueExpr();
        Result<ExprConstPtr> constExpr();
    };
}

#endif
