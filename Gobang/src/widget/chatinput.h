#ifndef CHATINPUT_H
#define CHATINPUT_H

#include <QObject>
#include <QTextEdit>

class ChatInput : public QTextEdit
{
    Q_OBJECT
public:
    ChatInput();
};

#endif // CHATINPUT_H
