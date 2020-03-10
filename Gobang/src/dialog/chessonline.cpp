#include "chessonline.h"
#include "settingpanel.h"
#include "../network/api.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMetaType>
#include <QSpacerItem>

#include <thread>
#include <chrono>


ChessOnline::ChessOnline(Player* you, Player* rival, QWidget* parent) :
    QDialog(parent), playerYou(you), playerRival(rival)
{
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setWindowTitle("Gobang");
    Qt::WindowFlags flags = this->windowFlags();
    flags |= Qt::WindowCloseButtonHint;
    flags |= Qt::WindowMinimizeButtonHint;
    this->setWindowFlags(flags);
    //this->setFixedSize(880, 645);
    this->setFixedWidth(880);

    setupLayout();

    qRegisterMetaType<Json::Value>("Json::Value");
    connect(this, &ChessOnline::sigProcessMsgTypeChat, this, &ChessOnline::processMsgTypeChat);
    connect(this, &ChessOnline::sigProcessMsgTypeNotify, this, &ChessOnline::processMsgTypeNotify);
    connect(this, &ChessOnline::sigProcessMsgTypeCommand, this, &ChessOnline::processMsgTypeCommand);
    connect(this, &ChessOnline::sigProcessMsgTypeResponse, this, &ChessOnline::processMsgTypeResponse);
    void sigCritical(QWidget* parent, const QString& title, const QString& text, int button);
    void sigInformation(QWidget* parent, const QString& title, const QString& text, int button);
    connect(this, &ChessOnline::sigCritical,
            this, [](QWidget* parent, const QString& title, const QString& text, int button){
                QMessageBox::critical(parent, title, text, QMessageBox::StandardButton(button));
            });
    connect(this, &ChessOnline::sigInformation,
            this, [](QWidget* parent, const QString& title, const QString& text, int button){
                QMessageBox::information(parent, title, text, QMessageBox::StandardButton(button));
            });
}

ChessOnline::~ChessOnline() {
    if (playerYou)
        delete playerYou;
    if (playerRival)
        delete playerRival;

    if (client) {
        client->disconnect();
        delete client;
        client = nullptr;
    }
}

void ChessOnline::setupLayout() {
    // Chessboard in left space
    chessBoard = new ChessBoardVS(this);
    connect(chessBoard, &ChessBoardVS::sigSetPieceByCursor, this, &ChessOnline::onSetPieceByCursor);
    connect(chessBoard, &ChessBoard::sigWin, this, &ChessOnline::onGameWin);
    connect(chessBoard, &ChessBoard::sigDraw, this, &ChessOnline::onGameDraw);
    chessBoard->ignoreMouseEvent(true);

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
            this, &ChessOnline::onCopyToClipboard);
    labelRoomId = new QLabel(frame1);
    labelRoomName = new QLabel(frame1);
    labelServerIp = new QLabel(frame1);
    labelServerPort = new QLabel(frame1);
    labelYourName1 = new QLabel(frame1);

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
    gridLayout1->addWidget(labelYourName1, 4, 1, 1, 2);

    // Frame 2:
    //      Player Info
    QFrame* frame2 = new QFrame(this);
    frame2->setFrameShape(QFrame::Panel);
    frame2->setFrameShadow(QFrame::Raised);
    frame2->setStyleSheet("background:#999999");
    frame2->setLineWidth(2);
    frame2->setMidLineWidth(3);
    labelYourName2 = new QLabel(frame2);
    labelRivalName = new QLabel(frame2);
    labelYourTurn = new QLabel(frame2);
    labelRivalTurn = new QLabel(frame2);
    whiteChess.load("res/white.png");
    blackChess.load("res/black.png");
    inPreparation.load("res/preparing.png");
    sameColorWithBg.load("res/bgcolor.png");
    rivalDisconnect.load("res/disconnect.png");
    if (playerYou->getType() == CHESS_BLACK) {
        yourChess = &blackChess;
        rivalChess = &whiteChess;
        isYourTurn = true;
    }
    else {
        yourChess = &whiteChess;
        rivalChess = &blackChess;
        isYourTurn = false;
    }
    labelYourTurn->setPixmap(*yourChess);
    labelRivalTurn->setPixmap(*rivalChess);

    QGridLayout* gridLayout2 = new QGridLayout(frame2);
    gridLayout2->addWidget(new QLabel("PLAYER1: ", frame2), 2, 0, Qt::AlignRight);
    gridLayout2->addWidget(labelYourName2, 2, 1);
    gridLayout2->addWidget(labelYourTurn, 2, 2);
    gridLayout2->addWidget(new QLabel("PLAYER2: ", frame2), 3, 0, Qt::AlignRight);
    gridLayout2->addWidget(labelRivalName, 3, 1);
    gridLayout2->addWidget(labelRivalTurn, 3, 2);
    gridLayout2->setColumnStretch(0, 2);
    gridLayout2->setColumnStretch(1, 2);
    gridLayout2->setColumnStretch(2, 1);
    gridLayout2->setRowMinimumHeight(2, 30);
    gridLayout2->setRowMinimumHeight(3, 30);

    // Buttons
    btnStart = new QPushButton("Start", this);
    btnScreenshot = new QPushButton(this);
    btnExchange = new QPushButton(this);
    btnSetting = new QPushButton(this);
    btnScreenshot->setIcon(QIcon("res/screenshot.ico"));
    btnScreenshot->setToolTip("Save image of the current chessboard.");
    btnScreenshot->setFixedWidth(25);
    btnExchange->setIcon(QIcon("res/exchange.ico"));
    btnExchange->setToolTip("Exchange chess type");
    btnExchange->setFixedWidth(25);
    btnSetting->setIcon(QIcon("res/setting.ico"));
    btnSetting->setFixedWidth(25);
    connect(btnStart, &QPushButton::clicked, this, &ChessOnline::onStart);
    connect(btnScreenshot, &QPushButton::clicked, this, &ChessOnline::onScreenshot);
    connect(btnExchange, &QPushButton::clicked, this, &ChessOnline::onExchangeChessType);
    connect(btnSetting, &QPushButton::clicked, this, &ChessOnline::onShowSettingPanel);

    // Buttons Layout
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(btnStart);
    btnLayout->addStretch();
    btnLayout->addWidget(btnScreenshot);
    btnLayout->addWidget(btnExchange);
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

void ChessOnline::setClient(Client *clientIn) {
    if (client) {
        client->disconnect();
        delete client;
    }
    client = clientIn;
    labelServerIp->setText(client->getServerIp());
    labelServerPort->setText(QString::number(client->getServerPort()));
    startRecvMsg();
}

void ChessOnline::updateInfo() {
    labelRoomName->setText(roomName);
    labelRoomId->setText(QString::number(roomId));
    labelYourName1->setText(playerYou->getName());
    labelYourName2->setText(playerYou->getName());
    labelRivalName->setText(playerRival->getName());
}

void ChessOnline::changeTurn() {
    isYourTurn = !isYourTurn;
    if (isYourTurn) {
        labelRivalTurn->setPixmap(sameColorWithBg);
        labelYourTurn->setPixmap(*yourChess);
    }
    else {
        labelRivalTurn->setPixmap(*rivalChess);
        labelYourTurn->setPixmap(sameColorWithBg);
    }
}

void ChessOnline::startRecvMsg() {
    std::thread t([this](){
        while (true) {
            Json::Value root;
            int n = this->client->recvJsonMsg(root);
            if (n == -1) {
                if (!activeDisconnect) {
                    emit sigCritical(nullptr, "Error", "You lose connection with server.",
                                     QMessageBox::Ok);
                }
                break;
            }
            else if (n == 1)
                this->parseJsonMsg(root);
            //else
            //  //error, just ignore
        }
        qDebug() << "Quit thread";
    });

    t.detach();
}

void ChessOnline::exchangeChessType() {
    ChessType tmp = playerYou->getType();
    playerYou->setType(playerRival->getType());
    playerRival->setType(tmp);
    std::swap(yourChess, rivalChess);
    labelYourTurn->setPixmap(*yourChess);
    labelRivalTurn->setPixmap(*rivalChess);
}


/**************************************************************
 *
 *          Slots
 *
**************************************************************/

void ChessOnline::onStart() {
    if (gameStatus == GAME_END) {
        //gameStatus = GAME_PREPARE;
        //btnStart->setText("Preparing");
        chessBoard->init();
        API::prepareGame(client, playerYou->getName());
    }
    else if (gameStatus == GAME_RUNNING) {
        int button = QMessageBox::question(this, "Confirm", "Confirm to quit?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (button == QMessageBox::Yes) {
            isForceQuit = true;
            this->close();
        }
    }
    else if (gameStatus == GAME_PREPARE) {
        gameStatus = GAME_END;
        btnStart->setText("Start");
        API::cancelPrepareGame(client, playerYou->getName());
        labelYourTurn->setPixmap(sameColorWithBg);
    }
}

void ChessOnline::onCopyToClipboard() {
    QString content = QString("Room ID:%1\nRoom Name:%2\nServer IP:%3\nServer Port:%4")
                          .arg(roomId).arg(roomName)
                          .arg(client->getServerIp()).arg(client->getServerPort());
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(content);
}

void ChessOnline::onShowSettingPanel() {
    SettingsInfo settings;
    settings.bgType = chessBoard->getBgType();
    settings.bgColor = chessBoard->getBgColor();
    settings.bgTransparency = chessBoard->getBgTransparency();
    settings.yourName = playerYou->getName();

    SettingPanel* panel = new SettingPanel(this, settings, playerRival);
    connect(panel, &SettingPanel::sigSettingsChanged, this, &ChessOnline::onSettingsChanged);

    panel->show();
}


// Save chessboard image
//
void ChessOnline::onScreenshot() {
    QPixmap& image = chessBoard->getImage();
    QString filename = QFileDialog::getSaveFileName(this, "Save image", "", "pictures (*.png)");
    if (filename.isNull())
        return;

    if (!image.save(filename, "PNG")) {
        QMessageBox::critical(this, "Error", "Save image failed!", QMessageBox::Ok);
    }
}

void ChessOnline::onSetPieceByCursor(int row, int col, int type) {
    API::notifyNewPiece(client, row , col, type);
    changeTurn();
}

void ChessOnline::onSettingsChanged(const SettingsInfo& settings) {
    QPixmap pixmap;
    pixmap.load("data/cache/bg.png");
    chessBoard->setBgType(settings.bgType);
    chessBoard->setBgColor(settings.bgColor);
    chessBoard->setBgPicture(pixmap);
    chessBoard->setBgTransparecny(settings.bgTransparency);
    chessBoard->flush();
    if (playerYou->getName() != settings.yourName) {
        playerYou->setName(settings.yourName);
        API::notifyRivalInfo(client, playerYou->getName());
        updateInfo();
    }
}

void ChessOnline::onExchangeChessType() {
    API::exchangeChessType(client);
}

void ChessOnline::onGameWin(int type) {
    if (type == playerYou->getType()) {
        API::notifyGameOverWin(client, type);
        chatHistory->addNewChat("System", "Game over! You won!", Qt::red);
        QMessageBox::information(this, "Game Over", "You won!", QMessageBox::Ok);
    }
    else {
        chatHistory->addNewChat("System", "Game over! You lose!", Qt::red);
        QMessageBox::information(this, "Game Over", "You lose!", QMessageBox::Ok);
    }

    gameOver();
}

void ChessOnline::onGameDraw() {
    if (playerYou->getType() == CHESS_BLACK)
        API::notifyGameOverDraw(client);
    QMessageBox::information(this, "Game Over", "End in a draw.", QMessageBox::Ok);
    chatHistory->addNewChat("System", "Game over! End in a draw", Qt::red);
    gameOver();
}

void ChessOnline::gameOver() {
    gameStatus = GAME_END;
    chessBoard->ignoreMouseEvent(true);
    btnStart->setText("New Game");
    btnExchange->setEnabled(true);
}

/**************************************************************
 *
 *  Overrid event from QDialog
 *
**************************************************************/

bool ChessOnline::eventFilter(QObject *watched, QEvent *event)
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
                    if (playerRival->getName() == "NULL") {
                        QMessageBox::information(this, "Prompt",
                                                 "Please wait for the other player to join.",
                                                 QMessageBox::Ok);
                        return true;
                    }
                    QString inputMsg = chatInput->toPlainText();
                    API::sendChatMessage(client, inputMsg, playerYou->getName());
                    chatHistory->addNewChat(playerYou->getName(), inputMsg);
                    chatInput->clear();
                }
                return true;
            }
        }
    }
    return QDialog::eventFilter(watched, event);
}

void ChessOnline::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_C) {
        qDebug() << "c";
        chatInput->setFocus();
    }
}

void ChessOnline::closeEvent(QCloseEvent *ev) {
    if (!isForceQuit) {
        int button = QMessageBox::question(this, "Close window",
                                           "Confirm to close the window?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (button != QMessageBox::Yes) {
            ev->ignore();
            return;
        }
    }

    Env::NAME = playerYou->getName();
    Env::BG_TYPE = chessBoard->getBgType();
    Env::BG_COLOR = chessBoard->getBgColor();
    Env::BG_TRANSPARENCY = chessBoard->getBgTransparency();

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

void ChessOnline::parseJsonMsg(const Json::Value &root) {
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

void ChessOnline::processMsgTypeChat(const Json::Value &root) {
    if (!root["message"].isNull() && !root["sender"].isNull()) {
        QString sender = QString::fromStdString(root["sender"].asString());
        QString chatMsg = QString::fromStdString(root["message"].asString());
        chatHistory->addNewChat(sender, chatMsg);
    }
}

void ChessOnline::processMsgTypeCommand(const Json::Value &root) {
    std::string cmd = root["cmd"].asString();

    if (cmd == "prepare") {
        if (QString::fromStdString(root["player_name"].asString()) == playerRival->getName()) {
            labelRivalTurn->setPixmap(inPreparation);
        }
    }
    else if (cmd == "exchange") {
        int button = QMessageBox::question(nullptr, "Request",
                                           "The rival requests to exchange the tyep of piece.\nDo you accept?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (button == QMessageBox::Yes) {
            API::responseExchageChessType(client, true);
            exchangeChessType();
        }
        else {
            API::responseExchageChessType(client, false);
        }
    }
}

void ChessOnline::processMsgTypeResponse(const Json::Value &root) {
    std::string res_cmd = root["res_cmd"].asString();

    if (res_cmd == "prepare") {
        if (root["status"].asInt() == STATUS_ERROR) {
            QMessageBox::information(this, "Error", QString::fromStdString(root["desc"].asString()),
                                     QMessageBox::Ok);
        }
        else {
            gameStatus = GAME_PREPARE;
            btnStart->setText("Preparing");
            labelYourTurn->setPixmap(inPreparation);
        }
    }
    else if (res_cmd == "exchange") {
        if (root["accept"].isNull())
            return;
        if (root["accept"].asBool())
            exchangeChessType();
        else
            QMessageBox::information(nullptr, "Prompt",
                                     "The rival refuesed to exchange piece type.",
                                     QMessageBox::Ok);
    }
}

void ChessOnline::processMsgTypeNotify(const Json::Value &root) {
    std::string sub_type = root["sub_type"].asString();

    if (sub_type == "cancel_prepare") {
        gameStatus = GAME_END;
        labelRivalTurn->setPixmap(sameColorWithBg);
    }
    else if (sub_type == "game_start") {
        gameStatus = GAME_RUNNING;
        btnStart->setText("Quit");
        btnExchange->setEnabled(false);

        chessBoard->init();
        chessBoard->ignoreMouseEvent(false);
        chessBoard->setYourChessType(playerYou->getType());

        if (playerYou->getType() == CHESS_BLACK) {
            yourChess = &blackChess;
            rivalChess = &whiteChess;
            labelYourTurn->setPixmap(*yourChess);
            labelRivalTurn->setPixmap(sameColorWithBg);
            isYourTurn = true;
        }
        else {
            yourChess = &whiteChess;
            rivalChess = &blackChess;
            labelYourTurn->setPixmap(sameColorWithBg);
            labelRivalTurn->setPixmap(*rivalChess);
            isYourTurn = false;
        }
    }
    else if (sub_type == "rival_info") {
        if (root["player_name"].isNull())
            return;
        playerRival->setName(QString::fromStdString(root["player_name"].asString()));
        updateInfo();
    }
    else if (sub_type == "new_piece") {
        if (root["row"].isNull() || root["col"].isNull() || root["chess_type"].isNull())
            return;
        int row = root["row"].asInt();
        int col = root["col"].asInt();
        int type = root["chess_type"].asInt();
        if (ChessType(type) != playerRival->getType())
            return;     // won't happen
        chessBoard->setPiece(row, col);
        changeTurn();
    }
    else if (sub_type == "disconnect") {
        labelRivalTurn->setPixmap(rivalDisconnect);
        gameStatus = GAME_END;
        btnStart->setText("Start");
        QMessageBox::information(nullptr, "Prompt", "The rival quit the game.", QMessageBox::Ok);
    }
}
