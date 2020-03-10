#ifndef WATCHCHESSONLINE_H
#define WATCHCHESSONLINE_H

#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTextEdit>

#include "../chess/chessboard.h"
#include "../chess/player.h"
#include "../network/client.h"
#include "../widget/chathistory.h"
#include "settingpanel.h"


class WatchChessOnline : public QDialog
{
    Q_OBJECT
public:
    WatchChessOnline(const QString& yourName, QWidget* parent = nullptr);
    ~WatchChessOnline();

public:
    void setClient(Client* clientIn);
    void setRoomName(const QString& nameIn) { roomName = nameIn; }
    void setRoomId(int idIn) { roomId = idIn; }

    void updateInfo();

    // Using for multi-thread
    // QWidget(and subclassed) should only be used or modified in GUI thread
    // The program may crash if QWidget was used in other thread
    // This problem can be solved by using "signals and slots"
signals:
    void sigProcessMsgTypeCommand(const Json::Value& root);
    void sigProcessMsgTypeNotify(const Json::Value& root);
    void sigProcessMsgTypeResponse(const Json::Value& root);
    void sigProcessMsgTypeChat(const Json::Value& root);

public slots:
    void onCopyToClipboard();
    void onScreenshot();
    void onShowSettingPanel();
    void onGameWin(int type);
    void onGameDraw();
    void onSettingsChanged(const SettingsInfo& settings);

protected:
    virtual void closeEvent(QCloseEvent *) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    void setupLayout();
    void startRecvMsg();
    void exchangeChessType();
    void gameOver();

    /*******************************
     * Parse and process recv msg
    *******************************/
private:
    void parseJsonMsg(const Json::Value& root);
private slots:
    void processMsgTypeCommand(const Json::Value& root);
    void processMsgTypeNotify(const Json::Value& root);
    void processMsgTypeResponse(const Json::Value& root);
    void processMsgTypeChat(const Json::Value& root);

private:
    ChessBoard* chessBoard = nullptr;
    Client* client = nullptr;
    Player player1;
    Player player2;

    int roomId;
    QString roomName;
    QString yourName;

    QPushButton* btnScreenshot = nullptr;
    QPushButton* btnSetting = nullptr;

    QLabel* labelRoomId;
    QLabel* labelRoomName;
    QLabel* labelServerIp;
    QLabel* labelServerPort;
    QLabel* labelYourName;
    QLabel* labelPlayer1Name;
    QLabel* labelPlayer2Name;
    QLabel* labelPlayer1Turn;
    QLabel* labelPlayer2Turn;

    QPixmap whiteChess;
    QPixmap blackChess;
    QPixmap inPreparation;
    QPixmap sameColorWithBg;
    QPixmap playerDisconnect;
    QPixmap* player1Chess;
    QPixmap* player2Chess;

    bool isQuit = false;
    bool isForceQuit = false;
    bool activeDisconnect = false;

    bool isPlayer1Turn = false;

    ChatHistory* chatHistory;
    QTextEdit* chatInput;
};

#endif // WATCHCHESSONLINE_H
