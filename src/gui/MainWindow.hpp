#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QListWidget>
#include "../lexer/Lexer.hpp"
#include "../common/Error.hpp"
#include "SymbolTableView.hpp"
#include "../recovery/SuggestionEngine.hpp"
#include "../semantic/SymbolTable.hpp"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QLabel;
class QSplitter;
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
    void onEditorTextChanged();           // Called when editor text changes
    void runCompilerPipeline();           // Main analysis function
    void highlightErrorLine(int lineNumber); // Highlight error in editor
    void onSuggestionClicked(QListWidgetItem *item); // Handle suggestion clicks

private:
    // UI Components
    CodeEditor *codeEditor;
    ErrorConsole *errorConsole;
    SymbolTableView *symbolTableView;
    QListWidget *suggestionsList;
    QSplitter *mainSplitter;
    
    // Timer for debounced compilation
    QTimer *compileTimer;
    
    // Menus
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *helpMenu;
    
    // Actions
    QAction *exitAction;
    QAction *aboutAction;
    QAction *openFileAction;
    QAction *saveFileAction;
    
    // Status bar elements
    QLabel *statusLabel;
    QLabel *lineColLabel;
    
    // Backend components
    SuggestionEngine suggestionEngine;
    SymbolTable currentSymbolTable;
    
    // Current file path
    QString currentFilePath;
    
    // Helper functions
    void createMenus();
    void createStatusBar();
    void setupConnections();
    void updateStatusBar();
    void displaySuggestions(const std::vector<std::string>& suggestions);
    
    // File operations
    void openFile();
    void saveFile();
    
    // Utility
    int extractLineFromSuggestion(const QString& suggestion);
};

} // namespace SCERSE
