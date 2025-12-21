#ifndef _TINYP_LEXER_H_
#define _TINYP_LEXER_H_

#include "TinyP/Error.h"
#include <iostream>

namespace TinyP {
    struct Token
    {
        enum Type
        {
            Id = 0,
            Number,
            Literal,
            Op,
            LeftBr,
            RightBr,
            Eos
        };

        Token() = default;
        Token(Type type, std::string value, std::uint32_t lineNumber = 0, std::uint32_t columnNumber = 0)
        : type(type), value(std::move(value)), lineNumber(lineNumber), columnNumber(columnNumber) {}
        Token(Type type, char value, std::uint32_t lineNumber = 0, std::uint32_t columnNumber = 0)
        : type(type), value(1, value), lineNumber(lineNumber), columnNumber(columnNumber) {}
        Token(Type type, char v0, char v1, std::uint32_t lineNumber = 0, std::uint32_t columnNumber = 0)
        : type(type), value(2, v0), lineNumber(lineNumber), columnNumber(columnNumber)
        {
            value[1] = v1;
        }

        Type type = Eos;
        std::string value;
        std::uint32_t lineNumber = 0;
        std::uint32_t columnNumber = 0;
    };

    class Lexer
    {
    public:
        explicit Lexer(std::istream& is);

        Result<Token> getNextToken();

    private:
        static const std::string specialChars_;
        static const std::string whiteChars_;

        Result<Token> getLiteral();
        Result<Token> getNumber();
        Result<Token> getId();

        void skipWhite();

        char getChar();
        void putChar(char ch);

        std::istream& is_;
        std::uint32_t lineNumber_ = 1;
        std::uint32_t columnNumber_ = 0;
        std::uint32_t prevColumnNumber_ = 0;
    };
}

std::ostream& operator<<(std::ostream& os, const TinyP::Token& t);

#endif
