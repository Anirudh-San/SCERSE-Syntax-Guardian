#include "ErrorConsole.hpp"
#include <QHeaderView>
#include <QTableWidgetItem>

namespace SCERSE {

ErrorConsole::ErrorConsole(QWidget* parent)
    : QTableWidget(parent)
{
    setColumnCount(3);
    setHorizontalHeaderLabels({"Line", "Column", "Message"});
    horizontalHeader()->setStretchLastSection(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(this, &QTableWidget::cellClicked,
            this, &ErrorConsole::onCellClicked);
}

void ErrorConsole::displayErrors(const std::vector<CompilerError>& errors) {
    setRowCount(static_cast<int>(errors.size()));

    for (int i = 0; i < errors.size(); ++i) {
        setItem(i, 0, new QTableWidgetItem(QString::number(errors[i].position.line)));
        setItem(i, 1, new QTableWidgetItem(QString::number(errors[i].position.column)));
        setItem(i, 2, new QTableWidgetItem(QString::fromStdString(errors[i].message)));
    }
}

void ErrorConsole::onCellClicked(int row, int /*column*/) {
    if (row < 0 || row >= rowCount()) return;
    bool ok = false;
    int line = item(row, 0)->text().toInt(&ok);
    if (ok) emit errorSelected(line);
}

} // namespace SCERSE
