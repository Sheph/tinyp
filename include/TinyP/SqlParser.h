#ifndef _TINYP_SQLPARSER_H_
#define _TINYP_SQLPARSER_H_

#include "TinyP/Parser.h"
#include "TinyP/SqlStmt.h"

namespace TinyP {
    // Class that parses simple SQL SELECT statements.
    // Grammar:
    // sql_statement -> select_statement
    // select_statement -> SELECT select_list FROM id [WHERE where_clause]
    // select_list -> '*' | column_list
    // column_list -> id (',' id)*
    // where_clause -> or_expr
    // or_expr -> and_expr (OR and_expr)*
    // and_expr -> comparison_expr (AND comparison_expr)*
    // comparison_expr -> add_expr [comp_op add_expr]
    // add_expr -> mult_expr (('+' | '-') mult_expr)*
    // mult_expr -> unary_expr (('*' | '/') unary_expr)*
    // unary_expr -> ['+' | '-'] primary_expr
    // primary_expr -> number | literal | id | '(' or_expr ')'
    // comp_op -> '=' | '<' | '>' | '<=' | '>=' | '<>'
    //
    // Keywords (case insensitive): SELECT, FROM, WHERE, AND, OR
    class SqlParser : public Parser
    {
    public:
        explicit SqlParser(std::istream& is);

        Result<SqlSelectStmtPtr> parse();

    private:
        Result<SqlSelectStmtPtr> selectStatement();
        Result<std::vector<std::string>> selectList();
        Result<std::vector<std::string>> columnList();
        Result<ExprPtr> whereClause();

        // WHERE expression parsing
        Result<ExprPtr> orExpr();
        Result<ExprPtr> andExpr();
        Result<ExprPtr> comparisonExpr();
        Result<ExprPtr> addExpr();
        Result<ExprPtr> multExpr();
        Result<ExprPtr> unaryExpr();
        Result<ExprPtr> primaryExpr();

        // Helper to check for keyword (case insensitive)
        bool isKeyword(const std::string& keyword) const;
        Error matchKeyword(const std::string& keyword);
        bool isComparisonOp() const;
    };
}

#endif
