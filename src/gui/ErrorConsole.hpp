#pragma once

#include <QTableWidget>
#include <vector>
#include "../common/Error.hpp"

namespace SCERSE {

class ErrorConsole : public QTableWidget {
    Q_OBJECT
public:
    explicit ErrorConsole(QWidget* parent = nullptr);
    void displayErrors(const std::vector<CompilerError>& errors);

signals:
    void errorSelected(int lineNumber);

private slots:
    void onCellClicked(int row, int column);
};

} // namespace SCERSE
