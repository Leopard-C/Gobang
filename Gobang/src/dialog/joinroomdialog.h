#ifndef JOINROOMDIALOG_H
#define JOINROOMDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>

enum class GameRole {
    PLAYER = 0,
    WATCHER = 1
};

class JoinRoomDialog : public QDialog
{
    Q_OBJECT
public:
    JoinRoomDialog(GameRole rol, QWidget* parent = nullptr);
    ~JoinRoomDialog() { }

public slots:
    void onJoinRoomAsPlayer();
    void onJoinRoomAsWatcher();
    void onPasteFromClipboard();

private:
    void setupLayout();
    bool checkEmpty();
    void readConfig(const char* filename);
    void writeConfig(const char* filename);

private:
    QLineEdit* editRoomId;
    QLineEdit* editYourName;
    QLineEdit* editServerIp;
    QLineEdit* editServerPort;

    QPushButton* btnPaste;
    QPushButton* btnJoin;
    QPushButton* btnCancel;

    GameRole role;
};

#endif // JOINROOMDIALOG_H
