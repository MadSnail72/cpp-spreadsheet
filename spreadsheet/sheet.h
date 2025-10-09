#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class CellHasher {
public:
    size_t operator()(const Position p) const {
        const uint64_t a = static_cast<uint64_t>(static_cast<uint32_t>(p.row));
        const uint64_t b = static_cast<uint64_t>(static_cast<uint32_t>(p.col));
        uint64_t x = (a << 32) ^ b;
        x += 0x9e3779b97f4a7c15ull;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
        x = x ^ (x >> 31);
        return static_cast<size_t>(x);
    }
};

class CellComparator {
public:
    bool operator()(const Position& lhs, const Position& rhs) const {
        return lhs == rhs;
    }
};

class Sheet : public SheetInterface {
public:
    // Хеш-таблица для хранения ячеек с O(1) доступом по позиции
    using Table = std::unordered_map<Position, std::unique_ptr<Cell>, CellHasher, CellComparator>;

    ~Sheet();

    // Основные методы интерфейса таблицы
    void SetCell(Position pos, std::string text) override;  // Создаёт/изменяет ячейку, проверяет циклы

    const CellInterface* GetCell(Position pos) const override;  // O(1) доступ к ячейке
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;  // Очищает ячейку, обновляет граф зависимостей

    Size GetPrintableSize() const override;  // Размер области с непустыми ячейками

    void PrintValues(std::ostream& output) const override;  // Печать вычисленных значений
    void PrintTexts(std::ostream& output) const override;   // Печать исходных текстов

    // Внутренние методы для работы с графом зависимостей
    // Возвращают «сырые» указатели на ячейку без валидации позиции и исключений
    const Cell* findCellRaw(Position pos) const noexcept;
    Cell* findCellRaw(Position pos) noexcept;

private:
    Table cells_;  // Хранилище ячеек: Position -> unique_ptr<Cell>
    
    // Последовательность операций при SetCell:
    // 1) Валидация позиции
    // 2) Создание ячейки при отсутствии
    // 3) Вызов Cell::Set() с проверкой циклов
    // 4) При ошибке - состояние таблицы не меняется
};