#pragma once
#include <QSyntaxHighlighter>
#include <QTextDocument>
namespace SCERSE {
class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    SyntaxHighlighter(QTextDocument* parent = nullptr) : QSyntaxHighlighter(parent) {}
};
}
