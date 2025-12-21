#ifndef _TINYP_PARSER_H_
#define _TINYP_PARSER_H_

#include "TinyP/Lexer.h"
#include <vector>

namespace TinyP {
    class Parser
    {
    public:
        explicit Parser(std::istream& is);

    protected:
        Error init();

        Error match(Token::Type type, const std::vector<std::string>& values = {});

        inline const Token& lookahead() const { return lookahead_; }

    private:
        Lexer lexer_;
        Token lookahead_;
    };
}

#endif
