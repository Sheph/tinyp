#ifndef _TINYP_ERROR_H_
#define _TINYP_ERROR_H_

#include <string>
#include <ostream>
#include <cinttypes>
#include <cassert>

namespace TinyP {
    class Error
    {
    public:
        Error() = default;
        Error(std::string msg, std::uint32_t lineNumber, std::uint32_t columnNumber)
        : msg_(std::move(msg)), lineNumber_(lineNumber), columnNumber_(columnNumber)
        {
        }

        explicit operator bool() const { return !msg_.empty(); }

        inline const std::string& msg() const { return msg_; }
        inline std::uint32_t lineNumber() const { return lineNumber_; }
        inline std::uint32_t columnNumber() const { return columnNumber_; }

    private:
        std::string msg_;
        std::uint32_t lineNumber_ = 0;
        std::uint32_t columnNumber_ = 0;
    };

    template <typename T>
    class Result
    {
    public:
        Result() = default;
        Result(Error err) : err_(std::move(err)) {}
        Result(T value) : value_(std::move(value)) {}

        inline const Error& err() const { return err_; }

        inline const T& value() const
        {
            assert(!err());
            return value_;
        }

        inline T& value()
        {
            assert(!err());
            return value_;
        }

    private:
        Error err_;
        T value_{};
    };
}

inline std::ostream& operator<<(std::ostream& os, const TinyP::Error& err)
{
    if (err) {
        os << err.lineNumber() << ":" << err.columnNumber() << ": " << err.msg();
    }
    return os;
}

#endif
