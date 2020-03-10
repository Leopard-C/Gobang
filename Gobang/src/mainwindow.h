#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void init();
    void setupLayout();

private:
    QPushButton* btnCreateRoom;
    QPushButton* btnJoinRoom;
    QPushButton* btnWatchLive;
};

#endif // MAINWINDOW_H
