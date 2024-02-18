#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <iostream>
#include <cassert>

using namespace std::literals;

void Sheet::SetCell(Position pos, std::string text) {
    ValidPosition(pos);
    if (size_.rows < pos.row + 1) {
        size_.rows = pos.row + 1;
    }
    if (size_.cols < pos.col + 1) {
        size_.cols = pos.col + 1;
    }
    if (sheet_container.count(pos) == 0) {
        sheet_container[pos] = std::make_unique<Cell>(*this);
    }
    sheet_container[pos]->Set(text);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    ValidPosition(pos);
    if (sheet_container.count(pos) == 0) {
        return nullptr;
    }
    return sheet_container.at(pos).get();
}

CellInterface* Sheet::GetCell(Position pos) {
    ValidPosition(pos);
    if (sheet_container.count(pos) == 0) {
        return nullptr;
    }
    return sheet_container.at(pos).get();
}

void Sheet::ClearCell(Position pos) {
    ValidPosition(pos);
    if (!GetCell(pos)) {
        return; 
    }
    sheet_container.erase(pos);
    if (size_.rows == pos.row + 1 || size_.cols == pos.col + 1) {
        CalcPrintSize();
    }
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        for (int j = 0; j < size_.cols; ++j) {
            const auto& cell = GetCell({ i, j });
            Cell::Value value;
            if (cell) {
                value = cell->GetValue();
            }
            switch (value.index()) {
            case 0: {
                std::string temp;
                if (size_.cols == j + 1) {
                    temp = "";
                }
                else {
                    temp = "\t";
                }
                output << std::get<0>(value) << temp;
                break;
            }
            case 1: {
                std::string temp;
                if (size_.cols == j + 1) {
                    temp = "";
                }
                else {
                    temp = "\t";
                }
                output << std::get<1>(value) << temp;
                break;
            }
            case 2: {
                std::string temp;
                if (size_.cols == j + 1) {
                    temp = "";
                }
                else {
                    temp = "\t";
                }
                output << std::get<2>(value) << temp;
                break;
            }
            default:
                assert(false);
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        for (int j = 0; j < size_.cols; ++j) {
            const auto& cell = GetCell({ i, j });
            std::string temp;
            if (cell) {
                temp = cell->GetText();
            }
            std::string tmp;
            if (size_.cols == j + 1) {
                tmp = "";
            }
            else {
                tmp = "\t";
            }
            output << temp << tmp;
        }
        output << '\n';
    }
}

void Sheet::ValidPosition(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position"s);
    }
}

void Sheet::CalcPrintSize() {
    Size new_size{ -1, -1 };
    for (const auto& [key, value] : sheet_container) {
        if (new_size.rows < key.row) {
            new_size.rows = key.row;
        }
        if (new_size.cols < key.col) {
            new_size.cols = key.col;
        }
    }
    size_ = { new_size.rows + 1, new_size.cols + 1 };
}


std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}