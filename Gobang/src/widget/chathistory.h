#ifndef CHATHISTORY_H
#define CHATHISTORY_H

#include <QObject>
#include <QTextEdit>

class ChatHistory : public QTextEdit {
public:
    ChatHistory(QWidget* parent = nullptr);

    void addNewChat(const QString& name, const QString& chatContent,
                    const QColor& color = Qt::blue, const QColor& contentColor = Qt::black);
};

#endif // CHATHISTORY_H
