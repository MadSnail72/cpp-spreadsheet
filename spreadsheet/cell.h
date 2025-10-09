#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    // Основные методы интерфейса
    void Set(std::string text);  // Устанавливает содержимое ячейки (текст/формула)
    void Clear();                // Очищает ячейку, удаляет из графа зависимостей

    Value GetValue() const override;                    // Возвращает вычисленное значение (с кэшем)
    std::string GetText() const override;               // Возвращает исходный текст ячейки
    std::vector<Position> GetReferencedCells() const override;  // Список ячеек, на которые ссылается формула

    bool IsReferenced() const;   // Проверяет, есть ли ячейки, зависящие от данной

private:
    class Impl;          // Базовый класс для всех типов содержимого
    class EmptyImpl;     // Пустая ячейка
    class TextImpl;      // Текстовая ячейка
    class FormulaImpl;   // Формульная ячейка (с кэшем)
    
    // Проверка циклических зависимостей
    bool WouldIntroduceCircularDependency(const Impl& new_impl) const;
    
    // Инвалидация кэша (два варианта: с visited и без)
    void InvalidateCacheRecursive(bool force = false);
    void InvalidateCacheRecursive(std::unordered_set<Cell*>& visited, bool force = false);

    std::unique_ptr<Impl> impl_;  // Текущее содержимое ячейки

    Sheet& sheet_;  // Ссылка на таблицу для доступа к другим ячейкам
    
    // ГРАФ ЗАВИСИМОСТЕЙ: двунаправленное хранение рёбер
    std::unordered_set<Cell*> l_nodes_;  // Входящие рёбра: кто зависит от данной ячейки (потребители)
    std::unordered_set<Cell*> r_nodes_;  // Исходящие рёбра: на кого ссылается данная ячейка (зависимости)
    
    // l_nodes_ используется для:
    // - Инвалидации кэша всех потребителей при изменении ячейки
    // - Проверки циклических зависимостей (DFS по потребителям)
    // r_nodes_ используется для:
    // - Быстрого удаления старых рёбер при изменении формулы
    // - Корректной очистки графа при Clear()
};