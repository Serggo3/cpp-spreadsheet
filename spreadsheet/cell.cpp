#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <algorithm>

using namespace std;

// Реализуйте следующие методы
Cell::Cell(SheetInterface& sheet) : sheet_(sheet), impl_(std::make_unique<EmptyImpl>()) {}

void Cell::Set(std::string text) {
    reference_container.clear();
    if (!text.empty()) {
        if (text[0] == FORMULA_SIGN && text.size() > 1) {
            unique_ptr<Impl> temp_impl(make_unique<FormulaImpl>(text.substr(1), sheet_));
            const auto& ref_cells(temp_impl->GetReferencedCells());
            CircularDependency(ref_cells);
            if (!ref_cells.empty()) {
                UpdateAllDependencies(ref_cells);
            }
            impl_ = std::move(temp_impl);
        }
        else {
            impl_ = make_unique<TextImpl>(std::move(text));
        }
    }
    else {
        impl_ = make_unique<EmptyImpl>();
    }
    InvalidateAllCache();
}

void Cell::Clear() {
    impl_ = make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

void Cell::CircularDependency(const std::vector<Position>& reference_cells) const {
    unordered_set<CellInterface*> visited_cells;
    CircularDependencyRecursion(reference_cells, visited_cells);
}

void Cell::CircularDependencyRecursion(const std::vector<Position>& reference_cells,
    std::unordered_set<CellInterface*>& visited_cells) const {
    for (const auto& pos : reference_cells) {
        CellInterface* reference_cell(sheet_.GetCell(pos));
        if (reference_cell == this) {
            throw CircularDependencyException("found cyclic dependencies"s);
        }
        if (reference_cell && visited_cells.count(reference_cell) == 0) {
            const auto& temp_ref_cells(reference_cell->GetReferencedCells());
            if (!temp_ref_cells.empty()) {
                CircularDependencyRecursion(temp_ref_cells, visited_cells);
            }
            visited_cells.insert(reference_cell);
        }
    }
}

void Cell::UpdateAllDependencies(const std::vector<Position>& new_reference_cells) {
    reference_container.clear();
    for_each(dependent_container.begin(), dependent_container.end(),
        [this](Cell* dependent_cell) {
            dependent_cell->dependent_container.erase(this);
        });

    for (const auto& pos : new_reference_cells) {
        if (!sheet_.GetCell(pos)) {
            sheet_.SetCell(pos, ""s);
        }
        Cell* new_referenced_cell = dynamic_cast<Cell*>(sheet_.GetCell(pos));
        reference_container.insert(new_referenced_cell);
        new_referenced_cell->dependent_container.insert(this);
    }
}

void Cell::InvalidateAllCache() {
    impl_->ResetCache();
    std::unordered_set<Cell*> visited_cells;
    InvalidateCache(visited_cells);
}

void Cell::InvalidateCache(std::unordered_set<Cell*>& visited_cells) {
    for (const auto& dependent_cell : dependent_container) {
        dependent_cell->impl_->ResetCache();
        if (visited_cells.count(dependent_cell) == 0) {
            if (!dependent_cell->dependent_container.empty()) {
                dependent_cell->InvalidateCache(visited_cells);
            }
            visited_cells.insert(dependent_cell);
        }
    }
}

std::vector<Position> Cell::Impl::GetReferencedCells() const {
    return {};
}

std::optional<FormulaInterface::Value> Cell::Impl::GetCache() const {
    return nullopt;
}

void Cell::Impl::ResetCache() {}

CellInterface::Value Cell::EmptyImpl::GetValue() const {
    return ""s;
}

std::string Cell::EmptyImpl::GetText() const {
    return ""s;
}


Cell::TextImpl::TextImpl(std::string expression) : value_(std::move(expression)) {}

CellInterface::Value Cell::TextImpl::GetValue() const {
    if (value_[0] == ESCAPE_SIGN) {
        return value_.substr(1);
    }

    return value_;
}

std::string Cell::TextImpl::GetText() const {
    return value_;
}

Cell::FormulaImpl::FormulaImpl(std::string expression, const SheetInterface& sheet)
    : formula_(ParseFormula(std::move(expression))), sheet_(sheet) {}

CellInterface::Value Cell::FormulaImpl::GetValue() const {
    if (!cache_.has_value()) {
        cache_ = formula_->Evaluate(sheet_);
    }

    switch (cache_.value().index()) {
    case 0:
        return get<0>(cache_.value());
    case 1:
        return get<1>(cache_.value());
    default:
        assert(false);
        throw std::runtime_error("CellInterface::Value Cell::FormulaImpl::GetValue() error"s);
    }
}

std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

void Cell::FormulaImpl::ResetCache() {
    cache_.reset();
}

std::optional<FormulaInterface::Value> Cell::FormulaImpl::GetCache() const {
    return cache_;
}