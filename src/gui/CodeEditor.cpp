#include "CodeEditor.hpp"
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QResizeEvent>
#include <QtGlobal>  // For qMax()

namespace SCERSE {

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(232, 242, 254);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(240, 240, 240));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::gray);
            painter.drawText(0, top, lineNumberArea->width() - 4,
                             fontMetrics().height(),
                             Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                      lineNumberAreaWidth(), cr.height()));
}

// Highlight error lines in light red
void CodeEditor::highlightErrorLine(int lineNumber)
{
    QTextCursor cursor(document()->findBlockByNumber(lineNumber - 1));
    QTextEdit::ExtraSelection selection;
    selection.cursor = cursor;
    selection.format.setBackground(QBrush(QColor(255, 210, 210))); // light red
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);

    QList<QTextEdit::ExtraSelection> extras = extraSelections();
    extras.append(selection);
    setExtraSelections(extras);
    centerCursor();
}

// Highlight suggestion line with specified color
void CodeEditor::highlightSuggestionLine(int lineNumber, const QColor& color)
{
    QTextCursor cursor(document()->findBlockByNumber(lineNumber - 1));
    QTextEdit::ExtraSelection selection;
    selection.cursor = cursor;
    selection.format.setBackground(color);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);

    QList<QTextEdit::ExtraSelection> extras = extraSelections();
    extras.append(selection);
    setExtraSelections(extras);
}

// Clear error highlighting (revert to current line highlight)
void CodeEditor::clearErrorHighlighting()
{
    highlightCurrentLine();
}

int CodeEditor::getCurrentLine() const
{
    QTextCursor cursor = textCursor();
    return cursor.blockNumber() + 1;
}

int CodeEditor::getCurrentColumn() const
{
    QTextCursor cursor = textCursor();
    return cursor.positionInBlock() + 1;
}

} // namespace SCERSE
