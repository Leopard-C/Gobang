#ifndef CREATEROOMDIALOG_H
#define CREATEROOMDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

#include "../chess/player.h"

class CreateRoomDialog : public QDialog
{
    Q_OBJECT
public:
    CreateRoomDialog(QWidget* parent = nullptr);

public:
    void setupLayout();

public slots:
    void onCreateRoom();

private:
    void readConfig(const char* filename);
    void writeConfig(const char* filename);

private:
    QLineEdit* editRoomName;
    QLineEdit* editYourName;
    QLineEdit* editServerIp;
    QLineEdit* editServerPort;

    QPushButton* btnCreate;
    QPushButton* btnCancel;
};

#endif // CREATEROOMDIALOG_H
