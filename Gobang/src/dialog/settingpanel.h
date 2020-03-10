#ifndef SETTINGPANNEL_H
#define SETTINGPANNEL_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>

#include "../chess/base.h"
#include "../chess/player.h"


struct SettingsInfo {
    BackgroundType bgType;
    int bgTransparency;
    QColor bgColor;
    QString yourName;
};


class SettingPanel : public QDialog
{
    Q_OBJECT
public:
    SettingPanel(QWidget* parent, const SettingsInfo& settingsIn, Player* playerRival);

    SettingsInfo getSettings() { return settings; }

signals:
    void sigSettingsChanged(const SettingsInfo&);
public slots:
    void onOk();
    void onApply();
    void onBgTypeChanged(int idx);
    void onChooseColor();
    void onChoosePicture();
    void onResetColor();

private:
    void setupLayout();
    void fillContent();
    void setBgColor(QColor color);

private:
    QLineEdit* editYourName;

    QLabel* labelColor;
    QLabel* labelColorBox;
    QLabel* labelPicPath;
    QLabel* labelTransparency;

    QComboBox* comboBoxBackgroundType;
    QSlider* sliderTransparency;

    QPushButton* btnChoosePicture;
    QPushButton* btnChooseColor;
    QPushButton* btnResetColor;
    QPushButton* btnApply;
    QPushButton* btnOk;
    QPushButton* btnCancel;

    SettingsInfo settings;
    Player* playerRival = nullptr;

    QPixmap bgImg;
    QColor colorTemp;

    bool flagClose = false;
};

#endif // SETTINGPANNEL_H
