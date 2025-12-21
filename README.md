TinyParser (https://github.com/Sheph/tinyp)
=========================

![mit license][img_license]

[img_license]: https://img.shields.io/badge/license-MIT-lightgrey.svg

### 1. About
-------------------

This is a tiny [LALR(1)](https://en.wikipedia.org/wiki/LALR_parser) parser for math formulas (such as e.g. `-(3 + 7) * 90 / 200.123`).

"write a math formula parser" is a common coding interview task and in general, something similar may come handy in day-to-day work,
e.g. writing a simple [DSL](https://en.wikipedia.org/wiki/Domain-specific_language) like e.g. [wireshark filter expressions](https://wiki.wireshark.org/DisplayFilters).

Sure, you can use `flex` / `bison` / `llvm` whatever, but for some simple tasks this is an overkill.

The thing is, almost every time I see people do this it's some kind of overly complicated thing with tons of code, some crazy stack-based state machines or whatever.
Here I want to show how simple this task actually is, the code in this repo:

* Pure standard C++11, no third-party dependencies
* Can parse any kind of math formula, e.g. `-(3 + 7) * 90 / 200.123`
* Provides error reporting with line and column numbers, e.g. `parse error: 1:5: expected ")" instead of end of string`
* Supports variables, e.g. `myvar1 * (123 + myvar2)` where `myvar1` and `myvar2` are variables defined outside of formula
* Supports both numbers and string literals, e.g. `("abc" + "hello \" world") * 12 + "test"` also works
* Has "pretty-printer" that can print a parsed expression in "normalized form"
* Bonus! Besides parsing math formulas there's a parser for simple "filter expressions", e.g. `(a >= 12 & c <> -56) | a > c`

And it's all done with very little code that's very simple. e.g. parsing code is basically [BNF](https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_form) translated into C++:

* [TinyP::FormulaParser](include/TinyP/FormulaParser.h)
* [TinyP::FilterParser](include/TinyP/FilterParser.h)

Inspite of their simplicity, LALR(1) parsers are pretty powerful, they can be used to parse langugaes such as `C`, `Lua` and `Java`. This particular code base can
be easily extended to support more complicated DSLs.

[TinyP::Expr](include/TinyP/Expr.h) and [TinyP::ExprVisitor](include/TinyP/ExprVisitor.h) can be extended to support
more complicated expressions.

One can come up with more `TinyP::ExprVisitor` implementations that can do more besides [pretty-printing](include/TinyP/ExprPrinter.h) and
[evaluating](include/TinyP/FormulaEval.h) expressions and so on.

### 2. Build
-------------------

```bash
cmake . && make
```

### 3. Run
-------------------

cd into `out/bin` and run e.g.:

```bash
echo "-(3 + 7) * 90 / 200.123" | ./tinyp_tool formula
```

or:

```bash
echo "(a >= 12 & c <> -56) | a > c" | ./tinyp_tool filter a=1 c=2
```

you can use files as input e.g.:

```bash
cat ./formula1.txt | ./tinyp_tool formula
```

you can use numeric and string variables e.g.:

```bash
echo "(2 + var1) * var2" | ./tinyp_tool formula @var1=test var2=3
```

### 4. Examples
-------------------

some formula:
```bash
$ echo "(1.34 * -2 / 12.3) * 97.45 - 111.123" | ./tinyp_tool formula
normalized input: ((((1.34 * (0 - 2)) / 12.3) * 97.45) - 111.123)
eval result: num(-132.356)
```

multiply string:
```bash
$ echo "\"test\" * -(-((6)))" | ./tinyp_tool formula
normalized input: ("test" * (0 - (0 - 6)))
eval result: str(testtesttesttesttesttest)
```

javascript [banana](https://gomakethings.com/banana/) (well, not exactly 😄):

```bash
$ echo "\"ba\"++\"aa\"" | ./tinyp_tool formula
normalized input: ("ba" + (0 + "aa"))
eval result: str(ba0aa)
```

more like javascript [banana](https://gomakethings.com/banana/) 😄:
```bash
$ echo "\"ba\"+0/0+\"a\"" | ./tinyp_tool formula
normalized input: (("ba" + (0 / 0)) + "a")
eval result: str(ba-nana)
```

parse error:
```bash
$ echo "(1.34 * -  -2 / 12.3) * 97.45 - 111.123" | ./tinyp_tool formula
parse error: 1:12: expected number instead of operator(-)
```

eval error:
```bash
$ echo "5 + \"A\"*v1*\"B\"" | ./tinyp_tool formula v1=5
vars:
v1=num(5.000000)
normalized input: (5 + (("A" * v1) * "B"))
eval error: 1:5: str(AAAAA) * str(B) - operation not supported
```

some vars:
```bash
$ echo "5+var1*var2+6" | ./tinyp_tool formula var1=6 var2=5
vars:
var2=num(5.000000)
var1=num(6.000000)
normalized input: ((5 + (var1 * var2)) + 6)
eval result: num(41)
```

now let's just change type of `var1` 😄:
```bash
$ echo "5+var1*var2+6" | ./tinyp_tool formula @var1=6 var2=5
vars:
var2=num(5.000000)
var1=str(6)
normalized input: ((5 + (var1 * var2)) + 6)
eval result: str(5666666)
```
