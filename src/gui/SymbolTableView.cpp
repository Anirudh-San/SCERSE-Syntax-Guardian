#include "SymbolTableView.hpp"
#include <QHeaderView>
#include "../common/Types.hpp"  // for DataType and to_string()

using namespace SCERSE;

SymbolTableView::SymbolTableView(QWidget *parent)
    : QTreeWidget(parent)
{
    // Set up columns and headers
    setColumnCount(3);
    setHeaderLabels({"Name", "Type", "Scope Level"});
    header()->setStretchLastSection(true);
}

void SymbolTableView::updateSymbolTable(const SymbolTable &table)
{
    clear();

    // Fetch all symbols from the table
    auto allSymbols = table.getAllSymbols();

    for (const auto &sym : allSymbols)
    {
        QStringList rowData = {
            QString::fromStdString(sym.name),
            QString::fromStdString(SCERSE::to_string(sym.type)),  // ✅ uses centralized helper
            QString::number(sym.scopeLevel)
        };

        addTopLevelItem(new QTreeWidgetItem(rowData));
    }
}
