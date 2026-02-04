set(SOURCES
    Lexer.cpp
    Parser.cpp
    ExprPrinter.cpp
    ExprEval.cpp
    FilterParser.cpp
    FilterEval.cpp
    FormulaParser.cpp
    FormulaEval.cpp
    SqlParser.cpp
    SqlEval.cpp
)

add_library(tinyp STATIC ${SOURCES})
