#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:
    ~Sheet() override = default;

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

	// Можете дополнить ваш класс нужными полями и методами


private:
    struct Hasher {
        size_t operator()(const Position& pos) const {
            return pos.row + pos.col * 37;
        }
    };
    std::unordered_map<Position, std::unique_ptr<Cell>, Hasher> sheet_container;
    Size size_ = { 0, 0 };
    static void ValidPosition(Position pos);
    void CalcPrintSize();
	// Можете дополнить ваш класс нужными полями и методами
};