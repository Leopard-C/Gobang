#include "mainwindow.h"
#include "environment.h"

#include "dialog/createroomdialog.h"
#include "dialog/joinroomdialog.h"

#include <QDir>
#include <QHBoxLayout>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    this->setWindowTitle("Gobang");
    //this->setFixedSize(260, 130);
    Env::load();

    init();
    setupLayout();
}

MainWindow::~MainWindow() {
    Env::save();
}

void MainWindow::init() {
    QDir dir;
    if (!dir.exists("data"))
        dir.mkdir("data");
    if (!dir.exists("data/cache"))
        dir.mkdir("data/cache");
}

void MainWindow::setupLayout() {
    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);
    btnCreateRoom = new QPushButton("Create New Room", centralWidget);
    btnJoinRoom = new QPushButton("Join a Room", centralWidget);
    btnWatchLive = new QPushButton("Watch a live", centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(btnCreateRoom);
    mainLayout->addWidget(btnJoinRoom);
    mainLayout->addWidget(btnWatchLive);

    connect(btnCreateRoom, &QPushButton::clicked, this, []{
        CreateRoomDialog* dialogCreateRoom = new CreateRoomDialog(nullptr);
        dialogCreateRoom->show();
    });
    connect(btnJoinRoom, &QPushButton::clicked, this, []{
        JoinRoomDialog* dialogJoinRoom = new JoinRoomDialog(GameRole::PLAYER, nullptr);
        dialogJoinRoom->show();
    });
    connect(btnWatchLive, &QPushButton::clicked, this, []{
        JoinRoomDialog* dialogJoinRoom = new JoinRoomDialog(GameRole::WATCHER, nullptr);
        dialogJoinRoom->show();
    });
}
