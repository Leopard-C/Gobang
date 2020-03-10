#include "watchchessonline.h"
#include "../network/api.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMetaType>

#include <thread>


WatchChessOnline::WatchChessOnline(const QString& yourName, QWidget* parent) :
    QDialog(parent), yourName(yourName)
{
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setFixedWidth(880);

    setupLayout();

    qRegisterMetaType<Json::Value>("Json::Value");
    connect(this, &WatchChessOnline::sigProcessMsgTypeChat, this, &WatchChessOnline::processMsgTypeChat);
    connect(this, &WatchChessOnline::sigProcessMsgTypeNotify, this, &WatchChessOnline::processMsgTypeNotify);
    connect(this, &WatchChessOnline::sigProcessMsgTypeCommand, this, &WatchChessOnline::processMsgTypeCommand);
    connect(this, &WatchChessOnline::sigProcessMsgTypeResponse, this, &WatchChessOnline::processMsgTypeResponse);
}

WatchChessOnline::~WatchChessOnline() {
    if (client) {
        client->disconnect();
        delete client;
        client = nullptr;
    }
}

void WatchChessOnline::setupLayout() {
    // Chessboard in left space
    chessBoard = new ChessBoard(this);
    connect(chessBoard, &ChessBoard::sigWin, this, &WatchChessOnline::onGameWin);
    connect(chessBoard, &ChessBoard::sigDraw, this, &WatchChessOnline::onGameDraw);

    // Frame 1:
    //      Room Info
    QFrame* frame1 = new QFrame(this);
    frame1->setFrameShape(QFrame::Panel);
    frame1->setFrameShadow(QFrame::Raised);
    frame1->setStyleSheet("background:#777777");
    frame1->setLineWidth(2);
    frame1->setMidLineWidth(3);
    QPushButton* btnCopy = new QPushButton(frame1);
    btnCopy->setIcon(QIcon("res/copy.ico"));
    btnCopy->setFlat(true);
    btnCopy->setToolTip("Copy to clipboard");
    connect(btnCopy, &QPushButton::clicked,
            this, &WatchChessOnline::onCopyToClipboard);
    labelRoomId = new QLabel(frame1);
    labelRoomName = new QLabel(frame1);
    labelServerIp = new QLabel(frame1);
    labelServerPort = new QLabel(frame1);
    labelYourName = new QLabel(frame1);

    QGridLayout* gridLayout1 = new QGridLayout(frame1);
    gridLayout1->setVerticalSpacing(10);
    gridLayout1->addWidget(new QLabel("Room   ID :", frame1), 0, 0, Qt::AlignRight);
    gridLayout1->addWidget(labelRoomId, 0, 1);
    gridLayout1->addWidget(btnCopy, 0, 2, Qt::AlignRight);
    gridLayout1->addWidget(new QLabel("Room Name :", frame1), 1, 0, Qt::AlignRight);
    gridLayout1->addWidget(labelRoomName, 1, 1, 1, 2);
    gridLayout1->addWidget(new QLabel("Server IP :", frame1), 2, 0, Qt::AlignRight);
    gridLayout1->addWidget(labelServerIp, 2, 1, 1, 2);
    gridLayout1->addWidget(new QLabel("Server Port :", frame1), 3, 0, Qt::AlignRight);
    gridLayout1->addWidget(labelServerPort, 3, 1, 1, 2);
    gridLayout1->addWidget(new QLabel("Your Name :", frame1), 4, 0, Qt::AlignRight);
    gridLayout1->addWidget(labelYourName, 4, 1, 1, 2);

    // Frame 2:
    //      Player Info
    QFrame* frame2 = new QFrame(this);
    frame2->setFrameShape(QFrame::Panel);
    frame2->setFrameShadow(QFrame::Raised);
    frame2->setStyleSheet("background:#999999");
    frame2->setLineWidth(2);
    frame2->setMidLineWidth(3);
    labelPlayer1Name = new QLabel(frame2);
    labelPlayer2Name = new QLabel(frame2);
    labelPlayer1Turn = new QLabel(frame2);
    labelPlayer2Turn = new QLabel(frame2);
    whiteChess.load("res/white.png");
    blackChess.load("res/black.png");
    inPreparation.load("res/turn.png");
    sameColorWithBg.load("res/bgcolor.png");
    playerDisconnect.load("res/disconnect.png");
    labelPlayer1Turn->setPixmap(sameColorWithBg);
    labelPlayer2Turn->setPixmap(sameColorWithBg);

    QGridLayout* gridLayout2 = new QGridLayout(frame2);
    gridLayout2->addWidget(new QLabel("PLAYER1: ", frame2), 2, 0, Qt::AlignRight);
    gridLayout2->addWidget(labelPlayer1Name, 2, 1);
    gridLayout2->addWidget(labelPlayer1Turn, 2, 2);
    gridLayout2->addWidget(new QLabel("PLAYER2: ", frame2), 3, 0, Qt::AlignRight);
    gridLayout2->addWidget(labelPlayer2Name, 3, 1);
    gridLayout2->addWidget(labelPlayer2Turn, 3, 2);
    gridLayout2->setColumnStretch(0, 2);
    gridLayout2->setColumnStretch(1, 2);
    gridLayout2->setColumnStretch(2, 1);
    gridLayout2->setRowMinimumHeight(2, 30);
    gridLayout2->setRowMinimumHeight(3, 30);

    // Buttons
    btnScreenshot = new QPushButton(this);
    btnSetting = new QPushButton(this);
    btnScreenshot->setIcon(QIcon("res/screenshot.ico"));
    btnScreenshot->setToolTip("Save image of the current chessboard.");
    btnScreenshot->setFixedWidth(25);
    btnSetting->setIcon(QIcon("res/setting.ico"));
    btnSetting->setFixedWidth(25);
    connect(btnScreenshot, &QPushButton::clicked, this, &WatchChessOnline::onScreenshot);
    connect(btnSetting, &QPushButton::clicked, this, &WatchChessOnline::onShowSettingPanel);

    // Buttons Layout
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addStretch();
    btnLayout->addWidget(btnScreenshot);
    btnLayout->addWidget(btnSetting);
    btnLayout->addSpacerItem(new QSpacerItem(20, 20));

    chatHistory = new ChatHistory(this);
    chatInput = new QTextEdit(this);
    chatHistory->addNewChat("System", "Welcome!", Qt::red);
    chatHistory->setFixedHeight(240);
    chatInput->setFixedHeight(60);
    chatInput->installEventFilter(this);

    // Vertical layout in right space
    QVBoxLayout* vertLayout = new QVBoxLayout();
    vertLayout->addWidget(frame1);
    vertLayout->addStretch();
    vertLayout->addWidget(frame2);
    vertLayout->addLayout(btnLayout);
    vertLayout->addStretch();
    vertLayout->addWidget(chatHistory);
    vertLayout->addStretch();
    vertLayout->addWidget(chatInput);

    // mainLayout
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(chessBoard);
    mainLayout->addLayout(vertLayout);
}

void WatchChessOnline::setClient(Client *clientIn) {
    if (client) {
        client->disconnect();
        delete client;
    }
    client = clientIn;
    labelServerIp->setText(client->getServerIp());
    labelServerPort->setText(QString::number(client->getServerPort()));

    // background thread
    startRecvMsg();
}

void WatchChessOnline::updateInfo() {
    labelRoomName->setText(roomName);
    labelRoomId->setText(QString::number(roomId));
    labelYourName->setText(yourName);
    labelPlayer1Name->setText(player1.getName());
    labelPlayer2Name->setText(player2.getName());
}

void WatchChessOnline::startRecvMsg() {
    std::thread t([this](){
        while (true) {
            Json::Value root;
            int n = this->client->recvJsonMsg(root);
            if (n == -1) {
                chatHistory->addNewChat("System", "You lose connection with server.", Qt::red);
                break;
            }
            else if (n == 1)
                this->parseJsonMsg(root);
            //else
            //  //error, just ignore
        }
    });

    t.detach();
}


/**************************************************************
 *
 *          Slots
 *
**************************************************************/

void WatchChessOnline::onCopyToClipboard() {
    QString content = QString("Room ID:%1\nRoom Name:%2\nServer IP:%3\nServer Port:%4")
                          .arg(roomId).arg(roomName)
                          .arg(client->getServerIp()).arg(client->getServerPort());
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(content);
}

void WatchChessOnline::onShowSettingPanel() {
    SettingsInfo settings;
    settings.bgType = chessBoard->getBgType();
    settings.bgColor = chessBoard->getBgColor();
    settings.bgTransparency = chessBoard->getBgTransparency();
    settings.yourName = yourName;

    SettingPanel* panel = new SettingPanel(this, settings, nullptr);
    connect(panel, &SettingPanel::sigSettingsChanged, this, &WatchChessOnline::onSettingsChanged);
    panel->show();
}

void WatchChessOnline::onScreenshot() {
    QPixmap& image = chessBoard->getImage();
    QString filename = QFileDialog::getSaveFileName(this, "Save image", "", "pictures (*.png)");
    if (filename.isNull())
        return;

    if (!image.save(filename, "PNG")) {
        QMessageBox::critical(nullptr, "Error", "Save image failed!", QMessageBox::Ok);
    }
}

void WatchChessOnline::onSettingsChanged(const SettingsInfo& settings) {
    QPixmap pixmap;
    pixmap.load("data/cache/bg.png");
    chessBoard->setBgType(settings.bgType);
    chessBoard->setBgColor(settings.bgColor);
    chessBoard->setBgPicture(pixmap);
    chessBoard->setBgTransparecny(settings.bgTransparency);
    chessBoard->flush();
    yourName = settings.yourName;
    labelYourName->setText(yourName);
}

void WatchChessOnline::onGameWin(int type) {
    if (type == CHESS_BLACK) {
        QMessageBox::information(nullptr, "Game Over", "Black chess won!", QMessageBox::Ok);
        chatHistory->addNewChat("System", "Game over! Black chess won!", Qt::red);
    }
    else {
        QMessageBox::information(nullptr, "Game Over", "White chess won!", QMessageBox::Ok);
        chatHistory->addNewChat("System", "Game over! White chess won!", Qt::red);
    }
    gameOver();
}

void WatchChessOnline::onGameDraw() {
    QMessageBox::information(nullptr, "Game Over", "End in a draw.", QMessageBox::Ok);
    chatHistory->addNewChat("System", "Game over! End in a draw", Qt::red);
    gameOver();
}

void WatchChessOnline::gameOver() {
    labelPlayer1Turn->setPixmap(sameColorWithBg);
    labelPlayer2Turn->setPixmap(sameColorWithBg);
}


/**************************************************************
 *
 *  Overrid event from QDialog
 *
**************************************************************/

bool WatchChessOnline::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == chatInput) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
                if (keyEvent->modifiers() == Qt::ShiftModifier || keyEvent->modifiers() == Qt::ControlModifier) {
                    chatInput->insertPlainText("\n");
                }
                else {
                    if (chatInput->toPlainText().isEmpty())
                        return true;
                    QString inputMsg = chatInput->toPlainText();
                    API::sendChatMessage(client, inputMsg, yourName);
                    chatHistory->addNewChat(yourName, inputMsg);
                    chatInput->clear();
                }
                return true;
            }
        }
    }
    return QDialog::eventFilter(watched, event);
}

void WatchChessOnline::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_C) {
        chatInput->setFocus();
    }
}

void WatchChessOnline::closeEvent(QCloseEvent *ev) {
    if (!isForceQuit) {
        int button = QMessageBox::question(this, "Close window",
                                           "Confirm to close the window?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (button != QMessageBox::Yes) {
            ev->ignore();
            return;
        }
    }

    isQuit = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (client) {
        activeDisconnect = true;
        client->disconnect();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        delete client;
        client = nullptr;
    }
}

/**************************************************************
 *
 * Parse and process recieved msg
 *
**************************************************************/

void WatchChessOnline::parseJsonMsg(const Json::Value &root) {
    if (root["type"].isNull())
        return;

    std::string msgType = root["type"].asString();

    if (msgType == "command") {
        if (!root["cmd"].isNull())
            emit sigProcessMsgTypeCommand(root);
    }
    else if (msgType == "response") {
        if (!root["res_cmd"].isNull())
            emit sigProcessMsgTypeResponse(root);
    }
    else if (msgType == "notify") {
        if (!root["sub_type"].isNull())
            emit sigProcessMsgTypeNotify(root);
    }
    else if (msgType == "chat") {
        emit sigProcessMsgTypeChat(root);
    }
}

void WatchChessOnline::processMsgTypeChat(const Json::Value &root) {
    if (!root["message"].isNull() && !root["sender"].isNull()) {
        QString sender = QString::fromStdString(root["sender"].asString());
        QString chatMsg = QString::fromStdString(root["message"].asString());
        chatHistory->addNewChat(sender, chatMsg);
    }
}

void WatchChessOnline::processMsgTypeCommand(const Json::Value &root) {
    std::string cmd = root["cmd"].asString();

    if (cmd == "prepare") {
        if (QString::fromStdString(root["player_name"].asString()) == player1.getName())
            labelPlayer1Turn->setPixmap(inPreparation);
        else
            labelPlayer2Turn->setPixmap(inPreparation);
    }
}

void WatchChessOnline::processMsgTypeResponse(const Json::Value &root) {
    std::string res_cmd = root["res_cmd"].asString();
    // ...
}

void WatchChessOnline::processMsgTypeNotify(const Json::Value &root) {
    std::string sub_type = root["sub_type"].asString();

    if (sub_type == "cancel_prepare") {
        if (QString::fromStdString(root["player_name"].asString()) == player1.getName())
            labelPlayer1Turn->setPixmap(*player1Chess);
        else
            labelPlayer2Turn->setPixmap(*player2Chess);
    }
    else if (sub_type == "chessboard") {
        Json::Value layout = root["layout"];
        Json::Value lastPiece = root["last_piece"];
        if (layout.isNull() || !layout.isArray())
            return;
        int pieces[15][15];
        for (int row = 0; row < 15; ++row) {
            for (int col = 0; col < 15; ++col) {
                pieces[row][col] = layout[row * 15 + col].asInt();
            }
        }
        int row = lastPiece["row"].asInt();
        int col = lastPiece["col"].asInt();
        ChessType type = ChessType(lastPiece["type"].asInt());
        chessBoard->init(pieces, {row, col, type});
        if (type == CHESS_NULL)
            return;
    }
    else if (sub_type == "game_start") {
        chessBoard->init();

        if (player1.getType() == CHESS_BLACK) {
            player1Chess = &blackChess;
            player2Chess = &whiteChess;
            labelPlayer1Turn->setPixmap(*player1Chess);
            labelPlayer2Turn->setPixmap(sameColorWithBg);
            isPlayer1Turn = true;
        }
        else {
            player1Chess = &whiteChess;
            player2Chess = &blackChess;
            labelPlayer1Turn->setPixmap(sameColorWithBg);
            labelPlayer2Turn->setPixmap(*player2Chess);
            isPlayer1Turn = false;
        }
    }
    else if (sub_type == "player_info") {
        if (root["player1_name"].isNull() || root["player2_name"].isNull() ||
                root["player1_chess_type"].isNull() || root["player2_chess_type"].isNull()) {
            return;
        }
        player1.setName(QString::fromStdString(root["player1_name"].asString()));
        player1.setType(ChessType(root["player1_chess_type"].asInt()));
        player2.setName(QString::fromStdString(root["player2_name"].asString()));
        player2.setType(ChessType(root["player2_chess_type"].asInt()));
        if (player1.getType() == CHESS_BLACK) {
            player1Chess = &blackChess;
            player2Chess = &whiteChess;
            labelPlayer1Turn->setPixmap(*player1Chess);
            labelPlayer2Turn->setPixmap(sameColorWithBg);
            isPlayer1Turn = true;
        }
        else {
            player1Chess = &whiteChess;
            player2Chess = &blackChess;
            labelPlayer1Turn->setPixmap(sameColorWithBg);
            labelPlayer2Turn->setPixmap(*player2Chess);
            isPlayer1Turn = false;
        }
        updateInfo();
    }
    else if (sub_type == "new_piece") {
        if (root["row"].isNull() || root["col"].isNull() || root["chess_type"].isNull())
            return;
        int row = root["row"].asInt();
        int col = root["col"].asInt();
        int type = root["chess_type"].asInt();
        chessBoard->setPiece(row, col, ChessType(type));
        if (type == player1.getType()) {
            labelPlayer1Turn->setPixmap(sameColorWithBg);
            labelPlayer2Turn->setPixmap(*player2Chess);
        }
        else {
            labelPlayer1Turn->setPixmap(*player1Chess);
            labelPlayer2Turn->setPixmap(sameColorWithBg);
        }
    }
    else if (sub_type == "disconnect") {
        if (root["player_name"].isNull())
            return;
        QString name = QString::fromStdString(root["player_name"].asString());
        if (name == player1.getName()) {
            labelPlayer1Turn->setPixmap(playerDisconnect);
        }
        else {
            labelPlayer1Turn->setPixmap(playerDisconnect);
        }
    }
}
