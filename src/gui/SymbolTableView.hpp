#pragma once
#include <QTreeWidget>
#include "../semantic/SymbolTable.hpp"

namespace SCERSE {
class SymbolTableView : public QTreeWidget {
    Q_OBJECT
public:
    SymbolTableView(QWidget* parent = nullptr);
    void updateSymbolTable(const SymbolTable& table);
};
}
