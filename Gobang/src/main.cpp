#include "mainwindow.h"
#include "network/client.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    Client::InitNetwork();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
