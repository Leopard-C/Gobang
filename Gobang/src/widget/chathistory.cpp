#include "chathistory.h"
#include <ctime>
#include <QDebug>

ChatHistory::ChatHistory(QWidget* parent) :
    QTextEdit(parent)
{
    this->setReadOnly(true);
}

void ChatHistory::addNewChat(const QString &name, const QString &chatContent,
                             const QColor& headerColor, const QColor& contentColor) {
    time_t t = time(nullptr);
    char now[16];
    strftime(now, sizeof(now), "%H:%M:%S", localtime(&t));

    QString header = QString("%1 [%2]\n  ").arg(name).arg(now);

    this->moveCursor(QTextCursor::End);
    this->setTextColor(headerColor);
    this->insertPlainText(header);
    this->setTextColor(contentColor);
    this->insertPlainText(chatContent);
    this->insertPlainText("\n");
    this->moveCursor(QTextCursor::End);
}
