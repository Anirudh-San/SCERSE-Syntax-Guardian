#include "SyntaxHighlighter.hpp"

namespace SCERSE {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Keyword format (bold blue)
    keywordFormat.setForeground(QColor(0, 0, 255));
    keywordFormat.setFontWeight(QFont::Bold);
    
    QStringList keywordPatterns;
    keywordPatterns << "\\bvar\\b" << "\\bint\\b" << "\\bfloat\\b" 
                    << "\\bbool\\b" << "\\bstring\\b" << "\\bif\\b" 
                    << "\\belse\\b" << "\\bwhile\\b" << "\\bfor\\b" 
                    << "\\bfunction\\b" << "\\breturn\\b" << "\\bconst\\b" 
                    << "\\btrue\\b" << "\\bfalse\\b" << "\\bvoid\\b";
    
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Number format (dark magenta)
    numberFormat.setForeground(QColor(139, 0, 139));
    rule.pattern = QRegularExpression("\\b[0-9]+\\.?[0-9]*\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // String format (dark green)
    stringFormat.setForeground(QColor(0, 128, 0));
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // Operator format (dark red)
    operatorFormat.setForeground(QColor(139, 0, 0));
    QStringList operatorPatterns;
    operatorPatterns << "\\+|\\-|\\*|\\/|%|="
                     << "==|!=|<|<=|>|>="
                     << "&&|\\|\\||!"
                     << ";|,|\\.|\\(|\\)|\\{|\\}|\\[|\\]";
    
    for (const QString &pattern : operatorPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = operatorFormat;
        highlightingRules.append(rule);
    }

    // Comment format (gray, italic)
    commentFormat.setForeground(QColor(128, 128, 128));
    commentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("//[^\\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    // Identifier format (default black)
    identifierFormat.setForeground(QColor(0, 0, 0));
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    // Apply all highlighting rules
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

} // namespace SCERSE
