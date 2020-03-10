#include "createroomdialog.h"
#include "../network/client.h"
#include "../network/api.h"
#include "../environment.h"
#include "chessonline.h"

#include <QDebug>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

#include <json/json.h>

#include <fstream>


CreateRoomDialog::CreateRoomDialog(QWidget* parent) :
    QDialog(parent)
{
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setWindowTitle("Create a room");
    this->setFixedWidth(300);

    setupLayout();
    readConfig("data/cache/latest_server.json");
}

void CreateRoomDialog::setupLayout() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    editRoomName = new QLineEdit(this);
    editYourName = new QLineEdit(this);
    editServerIp = new QLineEdit(this);
    editServerPort = new QLineEdit(this);

    QHBoxLayout* serverInfoLayout = new QHBoxLayout();
    serverInfoLayout->addWidget(editServerIp, 3);
    serverInfoLayout->addWidget(editServerPort, 1);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow(new QLabel("Room Name: ", this), editRoomName);
    formLayout->addRow(new QLabel("Your Name: ", this), editYourName);
    formLayout->addRow(new QLabel("Server Addr: ", this), serverInfoLayout);
    mainLayout->addLayout(formLayout);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnCreate = new QPushButton("Create", this);
    btnCancel = new QPushButton("Cancel", this);
    editRoomName->setFocus();
    btnCreate->setDefault(true);
    connect(btnCreate, &QPushButton::clicked, this, &CreateRoomDialog::onCreateRoom);
    connect(btnCancel, &QPushButton::clicked, this, &CreateRoomDialog::close);
    btnLayout->addStretch();
    btnLayout->addWidget(btnCreate);
    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);
}


void CreateRoomDialog::onCreateRoom() {
    if (editRoomName->text().isEmpty()) {
        QMessageBox::critical(this, "Error", "Please input room name", QMessageBox::Ok);
        return;
    }
    if (editYourName->text().isEmpty()) {
        QMessageBox::critical(this, "Error", "Please input your name", QMessageBox::Ok);
        return;
    }
    if (editServerIp->text().isEmpty()) {
        QMessageBox::critical(this, "Error", "Please input server ip", QMessageBox::Ok);
        return;
    }
    if (editServerPort->text().isEmpty()) {
        QMessageBox::critical(this, "Error", "Please input server port", QMessageBox::Ok);
        return;
    }

    QString roomName = editRoomName->text();
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

    API::createRoom(client, roomName, yourName);

    Json::Value res;
    int ret = client->recvJsonMsg(res);
    if (ret <= 0 || !API::isTypeResponse(res, "create_room") || res["room_id"].isNull()) {
        QMessageBox::critical(this, "Error", "Create room failed", QMessageBox::Ok);
        delete client;
        return;
    }

    int roomId = res["room_id"].asInt();
    Player* playerYou = new Player(yourName, CHESS_BLACK, true);
    Player* playerRival = new Player("NULL", CHESS_WHITE, false);

    ChessOnline* onlineChess = new ChessOnline(playerYou, playerRival, nullptr);
    onlineChess->setClient(client);
    onlineChess->setRoomId(roomId);
    onlineChess->setRoomName(roomName);
    onlineChess->updateInfo();
    onlineChess->show();

    writeConfig("data/cache/latest_server.json");
    this->close();
}


void CreateRoomDialog::readConfig(const char *filename) {
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

void CreateRoomDialog::writeConfig(const char *filename) {
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
