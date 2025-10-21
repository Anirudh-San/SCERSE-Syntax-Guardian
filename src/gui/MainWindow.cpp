#include "MainWindow.hpp"
#include "CodeEditor.hpp"
#include "ErrorConsole.hpp"
#include "../lexer/Lexer.hpp"
#include "../parser/LR1Parser.hpp"
#include "../common/Error.hpp"

#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QSplitter>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QListWidget>
#include <QRegularExpression>

namespace SCERSE {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , codeEditor(nullptr)
    , errorConsole(nullptr)
    , symbolTableView(nullptr)
    , suggestionsList(nullptr)
    , compileTimer(nullptr)
{
    // ===== Central Widget Setup =====
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    
    // Create main vertical splitter
    mainSplitter = new QSplitter(Qt::Vertical, central);
    
    // Create and add widgets to splitter
    codeEditor = new CodeEditor(mainSplitter);
    errorConsole = new ErrorConsole(mainSplitter);
    symbolTableView = new SymbolTableView(mainSplitter);
    suggestionsList = new QListWidget(mainSplitter);
    
    // Configure suggestions list
    suggestionsList->setWindowTitle("Suggestions");
    suggestionsList->setMaximumHeight(150);
    
    // Add widgets to splitter
    mainSplitter->addWidget(codeEditor);
    mainSplitter->addWidget(errorConsole);
    mainSplitter->addWidget(symbolTableView);
    mainSplitter->addWidget(suggestionsList);
    
    // Set stretch factors (code editor gets most space)
    mainSplitter->setStretchFactor(0, 5);  // Code editor
    mainSplitter->setStretchFactor(1, 2);  // Error console
    mainSplitter->setStretchFactor(2, 2);  // Symbol table
    mainSplitter->setStretchFactor(3, 1);  // Suggestions
    
    // Layout setup
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->addWidget(mainSplitter);
    layout->setContentsMargins(0, 0, 0, 0);
    central->setLayout(layout);
    
    // ===== Create Menus and Status Bar =====
    createMenus();
    createStatusBar();
    
    // ===== Timer Setup for Debounced Compilation =====
    compileTimer = new QTimer(this);
    compileTimer->setSingleShot(true);
    compileTimer->setInterval(500); // 500ms delay after last keystroke
    
    // ===== Setup Connections =====
    setupConnections();
    
    // ===== Window Setup =====
    setWindowTitle("SCERSE - Syntax Guardian");
    resize(1200, 800);
    
    statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow() = default;

void MainWindow::createMenus()
{
    // File Menu
    fileMenu = menuBar()->addMenu("&File");
    
    openFileAction = new QAction("&Open", this);
    openFileAction->setShortcut(QKeySequence::Open);
    connect(openFileAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openFileAction);
    
    saveFileAction = new QAction("&Save", this);
    saveFileAction->setShortcut(QKeySequence::Save);
    connect(saveFileAction, &QAction::triggered, this, &MainWindow::saveFile);
    fileMenu->addAction(saveFileAction);
    
    fileMenu->addSeparator();
    
    exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    fileMenu->addAction(exitAction);
    
    // Edit Menu
    editMenu = menuBar()->addMenu("&Edit");
    // Add edit actions here if needed
    
    // Help Menu
    helpMenu = menuBar()->addMenu("&Help");
    
    aboutAction = new QAction("&About", this);
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About SCERSE",
            "Syntax Guardian (SCERSE)\n"
            "Smart Compiler Error Recovery and Suggestion Engine\n\n"
            "By Anirudh Sanker\n"
            "2023UCP1844\n\n"
            "©2025");
    });
    helpMenu->addAction(aboutAction);
}

void MainWindow::createStatusBar()
{
    statusLabel = new QLabel("Ready", this);
    statusBar()->addWidget(statusLabel);
    
    // Add line and column indicator
    lineColLabel = new QLabel("Line: 1, Col: 1", this);
    statusBar()->addPermanentWidget(lineColLabel);
}

void MainWindow::setupConnections()
{
    // Connect editor text changes to debounced compilation
    connect(codeEditor, &QPlainTextEdit::textChanged, 
            this, &MainWindow::onEditorTextChanged);
    
    // Connect timer timeout to compilation pipeline
    connect(compileTimer, &QTimer::timeout, 
            this, &MainWindow::runCompilerPipeline);
    
    // Connect error console clicks to error highlighting
    connect(errorConsole, &ErrorConsole::errorSelected, 
            this, &MainWindow::highlightErrorLine);
    
    // Connect suggestion list clicks
    connect(suggestionsList, &QListWidget::itemClicked,
            this, &MainWindow::onSuggestionClicked);
    
    // Connect cursor position changes to status bar update
    connect(codeEditor, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateStatusBar);
}

void MainWindow::onEditorTextChanged()
{
    // Restart the compilation timer (debounce)
    compileTimer->start();
    statusBar()->showMessage("Analyzing...");
}

void MainWindow::runCompilerPipeline()
{
    // Get current code from editor
    QString code = codeEditor->toPlainText();
    
    if (code.isEmpty()) {
        errorConsole->displayErrors({});
        suggestionsList->clear();
        statusBar()->showMessage("Ready - No code to analyze");
        return;
    }
    
    // Create error reporter
    ErrorReporter errorReporter;
    
    // ===== STEP 1: LEXICAL ANALYSIS =====
    qDebug() << "=== Starting Lexical Analysis ===";
    Lexer lexer(code.toStdString());
    std::vector<Token> tokens = lexer.tokenize();
    
    qDebug() << "Tokens generated:" << tokens.size();
    
    // Optional: Log tokens for debugging
    for (const auto &token : tokens) {
        if (token.type != TokenType::EOF_TOKEN) {
            qDebug() << "  Token:" << QString::fromStdString(token.lexeme)
                     << "Type:" << static_cast<int>(token.type)
                     << "Line:" << token.position.line
                     << "Col:" << token.position.column;
        }
    }
    
    // ===== STEP 2: SYNTAX ANALYSIS (PARSING) =====
    qDebug() << "=== Starting Syntax Analysis ===";
    LR1Parser parser;
    ParseResult parseResult = parser.parse(tokens);
    
    qDebug() << "Parse success:" << parseResult.success;
    qDebug() << "Parse errors:" << parseResult.errors.size();
    
    // ===== STEP 3: SEMANTIC ANALYSIS (SYMBOL TABLE) =====
    qDebug() << "=== Starting Semantic Analysis ===";
    currentSymbolTable.clear();
    
    if (parseResult.ast) {
        currentSymbolTable.buildFromAST(parseResult.ast);
        qDebug() << "Symbol table built successfully";
    } else {
        qDebug() << "No AST generated - skipping symbol table build";
    }
    
    // ===== STEP 4: COLLECT ALL ERRORS =====
    std::vector<CompilerError> allErrors;
    
    // Add lexer errors (if any)
    auto lexerErrors = errorReporter.getErrors();
    allErrors.insert(allErrors.end(), lexerErrors.begin(), lexerErrors.end());
    
    // Add parser errors
    allErrors.insert(allErrors.end(), 
                     parseResult.errors.begin(), 
                     parseResult.errors.end());
    
    qDebug() << "Total errors collected:" << allErrors.size();
    
    // ===== STEP 5: UPDATE UI WITH ERRORS =====
    if (errorConsole) {
        errorConsole->displayErrors(allErrors);
    }
    
    if (symbolTableView) {
        symbolTableView->updateSymbolTable(currentSymbolTable);
    }
    
    // Clear previous error highlighting
    codeEditor->clearErrorHighlighting();
    
    // Highlight first error if any exist
    if (!allErrors.empty()) {
        codeEditor->highlightErrorLine(allErrors.front().position.line);
    }
    
    // ===== STEP 6: GENERATE SUGGESTIONS =====
    qDebug() << "=== Generating Suggestions ===";
    auto suggestions = suggestionEngine.generateSuggestions(allErrors, currentSymbolTable);
    
    qDebug() << "Suggestions generated:" << suggestions.size();
    for (const auto& s : suggestions) {
        qDebug() << "  Suggestion:" << QString::fromStdString(s);
    }
    
    displaySuggestions(suggestions);
    
    // ===== STEP 7: UPDATE STATUS BAR =====
    if (allErrors.empty()) {
        statusBar()->showMessage("✓ No errors detected");
    } else {
        statusBar()->showMessage(QString("✗ Found %1 error(s)").arg(allErrors.size()));
    }
    
    qDebug() << "=== Compilation Pipeline Complete ===\n";
}

void MainWindow::displaySuggestions(const std::vector<std::string>& suggestions)
{
    suggestionsList->clear();
    
    if (suggestions.empty()) {
        suggestionsList->addItem("No suggestions available");
        return;
    }
    
    for (const auto& suggestion : suggestions) {
        suggestionsList->addItem(QString::fromStdString(suggestion));
    }
}

void MainWindow::highlightErrorLine(int lineNumber)
{
    if (codeEditor) {
        codeEditor->highlightErrorLine(lineNumber);
        
        // Move cursor to the error line
        QTextCursor cursor = codeEditor->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
        codeEditor->setTextCursor(cursor);
        codeEditor->centerCursor();
    }
}

void MainWindow::onSuggestionClicked(QListWidgetItem *item)
{
    if (!item) return;
    
    int lineNumber = extractLineFromSuggestion(item->text());
    if (lineNumber > 0) {
        codeEditor->highlightSuggestionLine(lineNumber, QColor(200, 255, 200)); // pale green
        
        // Move cursor to suggestion line
        QTextCursor cursor = codeEditor->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
        codeEditor->setTextCursor(cursor);
        codeEditor->centerCursor();
    }
}

int MainWindow::extractLineFromSuggestion(const QString& suggestion)
{
    // Expected format: "Line 5: suggestion text" or "[Line 5] suggestion text"
    QRegularExpression regex(R"((?:Line|line)\s*(\d+))");
    QRegularExpressionMatch match = regex.match(suggestion);
    
    if (match.hasMatch()) {
        return match.captured(1).toInt();
    }
    
    return -1; // No line number found
}

void MainWindow::updateStatusBar()
{
    int line = codeEditor->getCurrentLine();
    int col = codeEditor->getCurrentColumn();
    lineColLabel->setText(QString("Line: %1, Col: %2").arg(line).arg(col));
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open C Source File", "", "C Files (*.c);;All Files (*)");
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file");
        return;
    }
    
    QTextStream in(&file);
    codeEditor->setPlainText(in.readAll());
    file.close();
    
    currentFilePath = fileName;
    setWindowTitle(QString("SCERSE - %1").arg(fileName));
    statusBar()->showMessage(QString("Opened: %1").arg(fileName));
}

void MainWindow::saveFile()
{
    if (currentFilePath.isEmpty()) {
        currentFilePath = QFileDialog::getSaveFileName(this,
            "Save C Source File", "", "C Files (*.c);;All Files (*)");
    }
    
    if (currentFilePath.isEmpty()) return;
    
    QFile file(currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not save file");
        return;
    }
    
    QTextStream out(&file);
    out << codeEditor->toPlainText();
    file.close();
    
    statusBar()->showMessage(QString("Saved: %1").arg(currentFilePath));
}

} // namespace SCERSE
