#include "TinyP/Parser.h"
#include <sstream>
#include <algorithm>

namespace TinyP {
    Parser::Parser(std::istream& is)
    : lexer_(is) {}

    Error Parser::init()
    {
        auto lookahead = lexer_.getNextToken();
        if (lookahead.err()) {
            return lookahead.err();
        }
        lookahead_ = lookahead.value();
        return {};
    }

    Error Parser::match(Token::Type type, const std::vector<std::string>& values)
    {
        if ((lookahead_.type == type) && (values.empty() || (std::find(values.begin(), values.end(), lookahead_.value) != values.end()))) {
            auto tmp = lexer_.getNextToken();
            if (tmp.err()) {
                return tmp.err();
            }
            lookahead_ = tmp.value();
            return {};
        } else {
            std::ostringstream os;
            os << "expected";
            if (values.empty()) {
                os << " " << Token(type, "");
            } else {
                for (const auto& v : values) {
                    os << " " << Token(type, v);
                }
            }
            os << " instead of " << lookahead_;
            return Error(os.str(), lookahead_.lineNumber, lookahead_.columnNumber);
        }
    }
}
