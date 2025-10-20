#include "MainWindow.hpp"
#include "CodeEditor.hpp"
#include "ErrorConsole.hpp"

#include "../lexer/Lexer.hpp"
#include "../parser/LR1Parser.hpp"
#include "../common/Error.hpp"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>
#include <QSplitter>
#include <QTimer>
#include <QDebug>
#include <QListWidget>

#include <QRegularExpression>


namespace SCERSE {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // ===== Central layout =====
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    // Create splitter to hold editor, error console, and symbol table view
    QSplitter *splitter = new QSplitter(Qt::Vertical, central);
    codeEditor = new CodeEditor(splitter);
    errorConsole = new ErrorConsole(splitter);
    symbolTableView = new SymbolTableView(splitter);

    splitter->addWidget(codeEditor);
    splitter->addWidget(errorConsole);
    splitter->addWidget(symbolTableView);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 1);

    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->addWidget(splitter);
    layout->setContentsMargins(0, 0, 0, 0);
    central->setLayout(layout);

    // ===== Menus =====
    QMenu *fileMenu = menuBar()->addMenu("&File");
    QAction *exitAction = new QAction("E&xit", this);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    fileMenu->addAction(exitAction);

    QMenu *helpMenu = menuBar()->addMenu("&Help");
    QAction *aboutAction = new QAction("&About", this);
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About SCERSE",
            "Syntax Guardian (SCERSE)\n"
            "Smart Compiler Error Recovery and Suggestion Engine\n"
            "By Anirudh Sanker"
            "2023UCP1844"
            "©2025 ");
    });
    helpMenu->addAction(aboutAction);

     suggestionsList = new QListWidget(splitter);
    splitter->addWidget(suggestionsList);
    splitter->setStretchFactor(3, 1);

    // Connect suggestion click to highlight in code editor
    connect(suggestionsList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
    int lineNumber = extractLineFromSuggestion(item->text());
    if (lineNumber > 0) {
        codeEditor->highlightSuggestionLine(lineNumber, QColor(200, 255, 200)); // pale green
    }
    });


    // ===== Status bar =====
    statusBar()->showMessage("Ready");

    // ===== Timer setup for delayed compilation =====
    compileTimer = new QTimer(this);
    compileTimer->setSingleShot(true);
    connect(compileTimer, &QTimer::timeout, this, &MainWindow::runCompilerPipeline);

    // Restart timer whenever text changes (debounce)
    connect(codeEditor, &QPlainTextEdit::textChanged, this, &MainWindow::onEditorTextChanged);

    // Connect error console selections
    connect(errorConsole, &ErrorConsole::errorSelected, this, &MainWindow::highlightErrorLine);

    setWindowTitle("SCERSE - Syntax Guardian");
    resize(900, 600);
}

MainWindow::~MainWindow() = default;

void MainWindow::onEditorTextChanged()
{
    compileTimer->start(500); // waits 500ms after last keystroke
}

void MainWindow::runCompilerPipeline()
{
    QString code = codeEditor->toPlainText();

    SCERSE::ErrorReporter errorReporter;

    // ===== Lexer =====
    SCERSE::Lexer lexer(code.toStdString(), errorReporter);
    std::vector<SCERSE::Token> tokens = lexer.tokenize();

    qDebug() << "Token count:" << tokens.size();
    for (const auto &token : tokens) {
        qDebug() << QString::fromStdString(token.lexeme)
                 << "Type:" << static_cast<int>(token.type)
                 << "Line:" << token.position.line
                 << "Col:" << token.position.column;
    }

    // ===== Parser =====
    SCERSE::LR1Parser parser;
    auto parseResult = parser.parse(tokens);

    // ===== Build or update symbol table from AST =====
    currentSymbolTable.clear();
    if (parseResult.ast) {
        currentSymbolTable.buildFromAST(parseResult.ast);
    }

    // ===== Combine lexer + parser errors =====
    std::vector<SCERSE::CompilerError> allErrors = errorReporter.getErrors();
    allErrors.insert(allErrors.end(),
                     parseResult.errors.begin(),
                     parseResult.errors.end());

    // ===== Update UI =====
    if (errorConsole)
        errorConsole->displayErrors(allErrors);

    if (symbolTableView)
        symbolTableView->updateSymbolTable(currentSymbolTable);

    codeEditor->clearErrorHighlighting();
    if (!allErrors.empty()) {
        codeEditor->highlightErrorLine(allErrors.front().position.line);
    }

    // ===== Suggestion engine =====
    auto suggestions = suggestionEngine.generateSuggestions(allErrors, currentSymbolTable);
    for (const auto& s : suggestions)
        qDebug() << "Suggestion:" << QString::fromStdString(s);

    statusBar()->showMessage(allErrors.empty()
                             ? "No errors detected"
                             : QString("Found %1 errors").arg(allErrors.size()));
}

void MainWindow::onCodeChanged()
{
    // Restart the compile timer when code changes
    compileTimer->start(500); // Compile after 500ms of inactivity
}

void MainWindow::highlightErrorLine(int lineNumber)
{
    codeEditor->highlightErrorLine(lineNumber);
}

int MainWindow::extractLineFromSuggestion(const QString& suggestion)
{
    // Expected format: "[Line 5] suggestion text"
    QRegularExpression regex(R"(\[Line\s+(\d+)\])");
    QRegularExpressionMatch match = regex.match(suggestion);
    
    if (match.hasMatch()) {
        QString lineNumberStr = match.captured(1);
        return lineNumberStr.toInt();
    }
    
    return -1; // If no line number found
}

} // namespace SCERSE
