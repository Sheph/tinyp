#include "TinyP/FilterEval.h"
#include "TinyP/FilterParser.h"
#include "TinyP/FormulaEval.h"
#include "TinyP/FormulaParser.h"
#include "TinyP/ExprPrinter.h"
#include <iostream>
#include <cstring>

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " filter|formula var=value @var=value ...\n";
        return 1;
    }

    bool isFilter = false;

    if (std::strcmp(argv[1], "filter") == 0) {
        isFilter = true;
    } else if (std::strcmp(argv[1], "formula") != 0) {
        std::cerr << "Specify \"filter\" or \"formula\"\n";
        return 1;
    }

    TinyP::ExprEval::Vars vars;

    for (int i = 2; i < argc; ++i) {
        bool isNumeric = true;
        std::string tmp = argv[i];
        if (!tmp.empty() && tmp[0] == '@') {
            tmp = tmp.substr(1);
            isNumeric = false;
        }
        auto pos = tmp.find_first_of('=');
        if (pos != std::string::npos) {
            auto val = tmp.substr(pos + 1);
            vars[tmp.substr(0, pos)] = {(isNumeric ? std::to_string(std::atof(val.c_str())) : val), isNumeric};
        }
    }

    if (!vars.empty()) {
        std::cout << "vars:\n";
        for (const auto& kv : vars) {
            std::cout << kv.first << "=" << (kv.second.second ? "num(" : "str(") << kv.second.first << ")\n";
        }
    }

    if (isFilter) {
        auto filter = TinyP::FilterParser(std::cin).parse();
        if (filter.err()) {
            std::cerr << "parse error: " << filter.err() << "\n";
            return 1;
        }

        std::cout << "normalized input: ";
        TinyP::ExprPrinter printer(std::cout);
        filter.value()->accept(printer);
        std::cout << "\n";

        TinyP::FilterEval eval(std::move(vars));
        filter.value()->accept(eval);
        if (eval.res().err()) {
            std::cerr << "eval error: " << eval.res().err() << "\n";
            return 1;
        }
        std::cout << "eval result: " << ((eval.res().value()->data() != "0") ? "true" : "false") << "\n";
    } else {
        auto formula = TinyP::FormulaParser(std::cin).parse();
        if (formula.err()) {
            std::cerr << "parse error: " << formula.err() << "\n";
            return 1;
        }

        std::cout << "normalized input: ";
        TinyP::ExprPrinter printer(std::cout);
        formula.value()->accept(printer);
        std::cout << "\n";

        TinyP::FormulaEval eval(std::move(vars));
        formula.value()->accept(eval);
        if (eval.res().err()) {
            std::cerr << "eval error: " << eval.res().err() << "\n";
            return 1;
        }
        std::cout << "eval result: " << *eval.res().value() << "\n";
    }

    return 0;
}
