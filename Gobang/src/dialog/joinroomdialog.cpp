#include "joinroomdialog.h"
#include "../network/api.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QStringList>
#include <QLabel>
#include <QMessageBox>

#include <json/json.h>
#include <fstream>

#include "../network/client.h"
#include "../environment.h"
#include "chessonline.h"
#include "watchchessonline.h"


JoinRoomDialog::JoinRoomDialog(GameRole role, QWidget* parent) :
    QDialog(parent), role(role)
{
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    if (role == GameRole::PLAYER)
        this->setWindowTitle("Join a room");
    else
        this->setWindowTitle("Watch a game");
    this->setFixedWidth(300);
    setupLayout();
    readConfig("data/cache/latest_server.json");
}

void JoinRoomDialog::setupLayout() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    editRoomId = new QLineEdit(this);
    editServerIp = new QLineEdit(this);
    editServerPort = new QLineEdit(this);
    editYourName = new QLineEdit(this);
    editRoomId->setValidator(new QIntValidator(1000, 9999, this));
    editRoomId->setFocus();

    QHBoxLayout* roomIdLayout = new QHBoxLayout();
    roomIdLayout->addWidget(editRoomId);
    roomIdLayout->addWidget(new QLabel("(1000 ~ 9999)", this));

    QHBoxLayout* serverInfoLayout = new QHBoxLayout();
    serverInfoLayout->addWidget(editServerIp, 3);
    serverInfoLayout->addWidget(editServerPort, 1);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow(new QLabel("Room ID:", this), roomIdLayout);
    formLayout->addRow(new QLabel("Server Addr:", this), serverInfoLayout);
    formLayout->addRow(new QLabel("Your Name:", this), editYourName);
    mainLayout->addLayout(formLayout);

    btnJoin = new QPushButton("Join", this);
    btnCancel = new QPushButton("Cancel", this);
    btnPaste = new QPushButton(this);
    btnPaste->setIcon(QIcon("res/paste.ico"));
    btnPaste->setFlat(true);
    btnJoin->setDefault(true);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(btnPaste);
    btnLayout->addStretch();
    btnLayout->addWidget(btnJoin);
    btnLayout->addStretch();
    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    if (role == GameRole::PLAYER)
        connect(btnJoin, &QPushButton::clicked, this, &JoinRoomDialog::onJoinRoomAsPlayer);
    else
        connect(btnJoin, &QPushButton::clicked, this, &JoinRoomDialog::onJoinRoomAsWatcher);
    connect(btnCancel, &QPushButton::clicked, this, &JoinRoomDialog::close);
    connect(btnPaste, &QPushButton::clicked, this, &JoinRoomDialog::onPasteFromClipboard);
}

bool JoinRoomDialog::checkEmpty() {
    if (editRoomId->text().isEmpty()) {
        QMessageBox::critical(this, "Error", "Please input room id", QMessageBox::Ok);
        return false;
    }
    if (editYourName->text().isEmpty()) {
        QMessageBox::critical(this, "Error", "Please input your name", QMessageBox::Ok);
        return false;
    }
    if (editServerIp->text().isEmpty()) {
        QMessageBox::critical(this, "Error", "Please input server ip", QMessageBox::Ok);
        return false;
    }
    if (editServerPort->text().isEmpty()) {
        QMessageBox::critical(this, "Error", "Please input server port", QMessageBox::Ok);
        return false;
    }
    return true;
}

void JoinRoomDialog::onJoinRoomAsPlayer() {
    if (!checkEmpty())
        return;

    int roomId = editRoomId->text().toInt();
    QString yourName = editYourName->text();
    QString serverIp = editServerIp->text();
    int serverPort = editServerPort->text().toInt();

    if (yourName.length() > 10) {
        QMessageBox::critical(this, "Error", "Your name is too long.\n[MAX: 10]", QMessageBox::Ok);
        return;
    }
    if (editYourName->text().compare("NULL", Qt::CaseInsensitive) == 0 ||
        editYourName->text().compare("System", Qt::CaseInsensitive) == 0)
    {
        QMessageBox::critical(this, "Error", "Change name failed. Please try another name.",
                              QMessageBox::Ok);
        return;
    }

    Client* client = new Client();
    if (!client->connectServer(serverIp.toStdString().c_str(), serverPort)) {
        QMessageBox::critical(this, "Error", "Connect to server failed", QMessageBox::Ok);
        delete client;
        return;
    }

    // Declared in: api.h
    API::joinRoom(client, roomId, yourName);

    Json::Value res;
    int ret = client->recvJsonMsg(res);
    if (ret <= 0 || !API::isTypeResponse(res, "join_room")) {
        QMessageBox::critical(this, "Error", "Join room failed", QMessageBox::Ok);
        delete client;
        return;
    }

    if (res["status"].asInt() == STATUS_ERROR) {
        QString errMsg = QString::fromStdString(res["desc"].asString());
        QMessageBox::critical(this, "Error", errMsg, QMessageBox::Ok);
        delete client;
        return;
    }

    QString roomName = QString::fromStdString(res["room_name"].asString());
    QString rivalName = QString::fromStdString(res["rival_name"].asString());
    Player* playerYou = new Player(yourName, CHESS_WHITE, true);
    Player* playerRival = new Player(rivalName, CHESS_BLACK, false);
    Env::NAME = yourName;

    ChessOnline* onlineChess = new ChessOnline(playerYou, playerRival, nullptr);
    onlineChess->setClient(client);
    onlineChess->setRoomId(roomId);
    onlineChess->setRoomName(roomName);
    onlineChess->updateInfo();
    onlineChess->show();

    writeConfig("data/cache/latest_server.json");
    this->close();
}

void JoinRoomDialog::onJoinRoomAsWatcher() {
    if (!checkEmpty())
        return;

    int roomId = editRoomId->text().toInt();
    QString yourName = editYourName->text();
    QString serverIp = editServerIp->text();
    int serverPort = editServerPort->text().toInt();

    Client* client = new Client();
    if (!client->connectServer(serverIp.toStdString().c_str(), serverPort)) {
        QMessageBox::critical(this, "Error", "Connect to server failed", QMessageBox::Ok);
        delete client;
        return;
    }

    // Declared in: api.h
    API::watchRoom(client, roomId, yourName);

    Json::Value res;
    int ret = client->recvJsonMsg(res);
    if (ret <= 0 || !API::isTypeResponse(res, "watch_room")) {
        QMessageBox::critical(this, "Error", "Join the room failed", QMessageBox::Ok);
        delete client;
        return;
    }

    if (res["status"].asInt() == STATUS_ERROR) {
        QString errMsg = QString::fromStdString(res["desc"].asString());
        QMessageBox::critical(this, "Error", errMsg, QMessageBox::Ok);
        delete client;
        return;
    }

    QString roomName = QString::fromStdString(res["room_name"].asString());

    WatchChessOnline* onlineChess = new WatchChessOnline(yourName, nullptr);
    onlineChess->setClient(client);
    onlineChess->setRoomId(roomId);
    onlineChess->setRoomName(roomName);
    onlineChess->updateInfo();
    onlineChess->show();

    writeConfig("data/cache/latest_server.json");
    this->close();
}


// Paste the roomInfo from system clipboard
//
void JoinRoomDialog::onPasteFromClipboard() {
    QClipboard* clipboard = QApplication::clipboard();
    QString content = clipboard->text();
    content.replace('\n', ':');
    QStringList list = content.split(':', QString::SkipEmptyParts);
    if (list.length() != 8)
        return;
    for (int i = 0; i < 8; i += 2) {
        if (list.at(i).compare("Room ID", Qt::CaseInsensitive) == 0)
            editRoomId->setText(list.at(i+1));
        else if (list.at(i).compare("Server Ip", Qt::CaseInsensitive) == 0)
            editServerIp->setText(list.at(i+1));
        else if (list.at(i).compare("Server Port", Qt::CaseInsensitive) == 0)
            editServerPort->setText(list.at(i+1));
    }
}


void JoinRoomDialog::readConfig(const char *filename) {
    Json::Reader reader;
    Json::Value  root;
    std::ifstream ifs(filename);

    if (!ifs.is_open())
        return;

    // Environment::load();
    editYourName->setText(Env::NAME);

    if (reader.parse(ifs, root, false)) {
        if (!root["server_ip"].isNull())
            editServerIp->setText(QString::fromStdString(root["server_ip"].asString()));
        if (!root["server_port"].isNull())
            editServerPort->setText(QString::fromStdString(root["server_port"].asString()));
    }

    ifs.close();
}

void JoinRoomDialog::writeConfig(const char *filename) {
    Json::Value root;
    std::ofstream ofs(filename);

    if (!ofs.is_open())
        return;

    if (!editServerIp->text().isEmpty())
        root["server_ip"] = editServerIp->text().toStdString();
    if (!editServerPort->text().isEmpty())
        root["server_port"] = editServerPort->text().toStdString();

    ofs << root.toStyledString();
    ofs.close();
}

