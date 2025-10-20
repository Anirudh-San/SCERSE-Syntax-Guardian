#pragma once

#include <QTimer>
#include <QPlainTextEdit>
#include <QMainWindow>
#include <QListWidget>
#include "../lexer/Lexer.hpp"
#include "../common/Error.hpp"

#include "gui/SymbolTableView.hpp"
#include "recovery/SuggestionEngine.hpp"
#include "../semantic/SymbolTable.hpp"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QLabel;
QT_END_NAMESPACE

namespace SCERSE {

// Forward declarations
class CodeEditor;
class ErrorConsole;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCodeChanged();                  // Called whenever code is edited
    void highlightErrorLine(int lineNumber);  // Highlight a line when error clicked
    void onEditorTextChanged();
    void runCompilerPipeline();

private:

    // Editor widget
    CodeEditor *codeEditor;

    // Error console widget
    ErrorConsole *errorConsole;

    // Symbol table view widget
    SymbolTableView *symbolTableView;

    // Timer
    QTimer *compileTimer;

    // Menus
    QMenu *fileMenu;
    QMenu *helpMenu;

    // Actions
    QAction *exitAction;
    QAction *aboutAction;

    //For Suggestions IRT
    QListWidget *suggestionsList;

    // Status bar elements
    QLabel *statusLabel;

    // Suggestion engine
    SuggestionEngine suggestionEngine;

    // Current symbol table
    SymbolTable currentSymbolTable;

    // Helper functions (optional)
    void createMenus();
    void createStatusBar();
    void setupConnections();

    //For IRT Suggestions
    int extractLineFromSuggestion(const QString& suggestion);
};

} // namespace SCERSE
