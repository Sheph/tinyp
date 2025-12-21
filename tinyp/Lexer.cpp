#include "TinyP/Lexer.h"

namespace TinyP {
    const std::string Lexer::specialChars_ = "*+-/=()\"<>&| \t\r\n";
    const std::string Lexer::whiteChars_ = " \t\r\n";

    Lexer::Lexer(std::istream& is)
    : is_(is) {}

    Result<Token> Lexer::getNextToken()
    {
        skipWhite();

        char ch = getChar();

        if (is_.eof()) {
            return Token(Token::Eos, "", lineNumber_, columnNumber_);
        }

        Result<Token> token;

        switch (ch) {
        case '=':
        case '+':
        case '-':
        case '*':
        case '&':
        case '|':
        case '/': token = Token(Token::Op, ch, lineNumber_, columnNumber_); break;
        case '(': token = Token(Token::LeftBr, ch, lineNumber_, columnNumber_); break;
        case ')': token = Token(Token::RightBr, ch, lineNumber_, columnNumber_); break;
        case '\"': token = getLiteral(); break;
        case '<': {
            char ch2 = is_.get();
            if (is_.eof()) {
                is_.clear();
                token = Token(Token::Op, ch, lineNumber_, columnNumber_);
            } else {
                switch (ch2) {
                case '>':
                case '=':
                    token = Token(Token::Op, ch, ch2, lineNumber_, columnNumber_);
                    ++columnNumber_;
                    ++prevColumnNumber_;
                    break;
                default:
                    is_.putback(ch2);
                    token = Token(Token::Op, ch, lineNumber_, columnNumber_);
                    break;
                }
            }
            break;
        }
        case '>': {
            char ch2 = is_.get();
            if (is_.eof()) {
                is_.clear();
                token = Token(Token::Op, ch, lineNumber_, columnNumber_);
            } else {
                switch (ch2) {
                case '=':
                    token = Token(Token::Op, ch, ch2, lineNumber_, columnNumber_);
                    ++columnNumber_;
                    ++prevColumnNumber_;
                    break;
                default:
                    is_.putback(ch2);
                    token = Token(Token::Op, ch, lineNumber_, columnNumber_);
                    break;
                }
            }
            break;
        }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.':
            putChar(ch);
            token = getNumber();
            break;
        default:
            putChar(ch);
            token = getId();
            break;
        };

        return token;
    }

    Result<Token> Lexer::getLiteral()
    {
        std::string ret;

        bool wasEscape = false;

        auto lineNumber = lineNumber_;
        auto columnNumber = columnNumber_;

        while (true) {
            char ch = getChar();

            if (is_.eof()) {
                break;
            }

            switch (ch) {
            case '\"':
                if (!wasEscape) {
                    return Token(Token::Literal, std::move(ret), lineNumber, columnNumber);
                }
            case '\\':
                if (wasEscape) {
                    ret.append(1, ch);
                }
                wasEscape = !wasEscape;
                break;
            default:
                if (wasEscape) {
                    ret.append(1, '\\');
                    wasEscape = false;
                }
                ret.append(1, ch);
            }
        }

        return Error("'\"' expected", lineNumber_, columnNumber_);
    }

    Result<Token> Lexer::getNumber()
    {
        std::string ret;
        bool seenPoint = false;

        auto lineNumber = lineNumber_;
        auto columnNumber = columnNumber_;

        while (true) {
            char ch = getChar();

            if (is_.eof()) {
                break;
            }

            if (specialChars_.find(ch) != std::string::npos) {
                putChar(ch);
                break;
            }

            switch (ch) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': break;
            case '.':
                if (seenPoint) {
                    return Error("bad numeric constant", lineNumber_, columnNumber_);
                }
                seenPoint = true;
                break;
            default: return Error("bad numeric constant", lineNumber_, columnNumber_);
            }

            ret.append(1, ch);
        }

        if (seenPoint && (ret.size() < 2)) {
            return Error("bad numeric constant", lineNumber_, columnNumber_);
        } else {
            return Token(Token::Number, std::move(ret), lineNumber, columnNumber);
        }
    }

    Result<Token> Lexer::getId()
    {
        std::string ret;

        auto lineNumber = lineNumber_;
        auto columnNumber = columnNumber_;

        while (true) {
            char ch = getChar();

            if (is_.eof()) {
                break;
            }

            if (specialChars_.find(ch) != std::string::npos) {
                putChar(ch);
                break;
            }

            ret.append(1, ch);
        }

        return Token(Token::Id, std::move(ret), lineNumber, columnNumber);
    }

    void Lexer::skipWhite()
    {
        while (true) {
            char ch = getChar();

            if (is_.eof()) {
                break;
            }

            if (whiteChars_.find(ch) == std::string::npos) {
                putChar(ch);
                return;
            }
        }
    }

    char Lexer::getChar()
    {
        if (is_.eof()) {
            return '\0';
        }

        char ch = is_.get();

        prevColumnNumber_ = columnNumber_;

        if (is_.eof()) {
            ++columnNumber_;
            return '\0';
        }

        switch (ch) {
        case '\r': break;
        case '\n':
            ++lineNumber_;
            columnNumber_ = 0;
            break;
        default: ++columnNumber_;
        }

        return ch;
    }

    void Lexer::putChar(char ch)
    {
        switch (ch) {
        case '\r': break;
        case '\n':
            --lineNumber_;
            columnNumber_ = prevColumnNumber_;
            break;
        default: --columnNumber_;
        }
        is_.putback(ch);
    }
}

std::ostream& operator<<(std::ostream& os, const TinyP::Token& t)
{
    if (t.value.empty()) {
        switch (t.type) {
        case TinyP::Token::Id: os << "identifier"; break;
        case TinyP::Token::Number: os << "number"; break;
        case TinyP::Token::Literal: os << "literal"; break;
        case TinyP::Token::Op: os << "operator"; break;
        case TinyP::Token::LeftBr: os << "\"(\""; break;
        case TinyP::Token::RightBr: os << "\")\""; break;
        case TinyP::Token::Eos: os << "end of string"; break;
        default: os << "?"; break;
        }
    } else {
        switch (t.type) {
        case TinyP::Token::Id: os << "identifier(" << t.value << ")"; break;
        case TinyP::Token::Number: os << "number(" << t.value << ")"; break;
        case TinyP::Token::Literal: os << "literal(" << t.value << ")"; break;
        case TinyP::Token::Op: os << "operator(" << t.value << ")"; break;
        case TinyP::Token::LeftBr: os << "\"(\""; break;
        case TinyP::Token::RightBr: os << "\")\""; break;
        case TinyP::Token::Eos: os << "end of string"; break;
        default: os << "?(" << t.value << ")"; break;
        }
    }
    return os;
}
