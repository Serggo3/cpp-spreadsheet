#include "formula.h"

#include "FormulaAST.h"

#include <sstream>
#include <set>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
    class Formula : public FormulaInterface {
    public:
        Formula(std::string expression) : ast_(ParseFormulaAST(expression)) {};

        Value Evaluate(const SheetInterface& arg) const override {
            Value value{};
            try {
                value = ast_.Execute(arg);
            }
            catch (const FormulaError& error) {
                value = error;
            }
            return value;
        }

        std::string GetExpression() const override {
            std::ostringstream out;
            ast_.PrintFormula(out);
            return out.str();
        }

        std::vector<Position> GetReferencedCells() const override {
            std::vector<Position> result;
            const auto& pos = ast_.GetCells();
            std::set<Position> temp(pos.begin(), pos.end());
            result = { temp.begin(), temp.end() };
            return result;
        }

    private:
        FormulaAST ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(expression);
    }
    catch (...) {
        throw FormulaException("Parsing string expression formula - failure"s);
    }
}