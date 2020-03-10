#include "settingpanel.h"
//#include "../environment.h"
#include "../chess/player.h"
#include "../chess/base.h"
#include "imagecropperdialog.h"


#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>

SettingPanel::SettingPanel(QWidget* parent, const SettingsInfo& settingsIn,
                           Player* playerRivalIn) :
    QDialog(parent), settings(settingsIn), playerRival(playerRivalIn)
{
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setWindowTitle("Setting Panel");
    this->setFixedWidth(300);

    bgImg.load("data/cache/bg.png");
    colorTemp = settings.bgColor;
    setupLayout();
}

void SettingPanel::setupLayout() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    labelColor = new QLabel("Color:", this);
    labelPicPath = new QLabel("Picture:", this);
    labelColorBox = new QLabel(this);
    labelTransparency = new QLabel("Transparency:", this);
    setBgColor(settings.bgColor);

    editYourName = new QLineEdit(settings.yourName, this);

    btnChooseColor = new QPushButton(this);
    btnChoosePicture = new QPushButton(this);
    btnResetColor = new QPushButton(this);
    btnChooseColor->setFixedWidth(30);
    btnResetColor->setFixedWidth(30);
    btnChoosePicture->setFixedWidth(30);
    btnResetColor->setIcon(QIcon("res/reset.ico"));
    btnChooseColor->setIcon(QIcon("res/color-palette.ico"));
    btnChoosePicture->setIcon(QIcon("res/select-file.ico"));
    connect(btnChooseColor, &QPushButton::clicked, this, &SettingPanel::onChooseColor);
    connect(btnChoosePicture, &QPushButton::clicked, this, &SettingPanel::onChoosePicture);
    connect(btnResetColor, &QPushButton::clicked, this, &SettingPanel::onResetColor);

    comboBoxBackgroundType = new QComboBox(this);
    comboBoxBackgroundType->addItem("Pure Color");
    comboBoxBackgroundType->addItem("Picture");
    connect(comboBoxBackgroundType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onBgTypeChanged(int)));

    QHBoxLayout* bgColorLayout = new QHBoxLayout();
    bgColorLayout->addWidget(labelColorBox);
    bgColorLayout->addWidget(btnChooseColor);
    bgColorLayout->addWidget(btnResetColor);

    sliderTransparency = new QSlider(Qt::Horizontal, this);
    sliderTransparency->setRange(0, 255);
    sliderTransparency->setSingleStep(5);
    sliderTransparency->setValue(settings.bgTransparency);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnApply = new QPushButton("Apply", this);
    btnOk = new QPushButton("Ok", this);
    btnCancel = new QPushButton("Cancel", this);
    btnOk->setFocus();
    btnOk->setDefault(true);
    connect(btnApply, &QPushButton::clicked, this, &SettingPanel::onApply);
    connect(btnOk, &QPushButton::clicked, this, &SettingPanel::onOk);
    connect(btnCancel, &QPushButton::clicked, this, &SettingPanel::close);
    btnLayout->addStretch();
    btnLayout->addWidget(btnApply);
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow(new QLabel("Your Name:", this), editYourName);
    formLayout->addRow(new QLabel("Background: ", this), comboBoxBackgroundType);
    formLayout->addRow(labelColor, bgColorLayout);
    formLayout->addRow(labelPicPath, btnChoosePicture);
    formLayout->addRow(labelTransparency, sliderTransparency);

    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(btnLayout);

    if (settings.bgType == BG_PURE_COLOR) {
        comboBoxBackgroundType->setCurrentIndex(0);
        btnChoosePicture->setEnabled(false);
        labelPicPath->setEnabled(false);
        sliderTransparency->setEnabled(false);
    }
    else {
        comboBoxBackgroundType->setCurrentIndex(1);
        btnChooseColor->setEnabled(false);
        labelColorBox->setEnabled(false);
        labelColor->setEnabled(false);
    }
}

void SettingPanel::setBgColor(QColor color) {
    QPixmap pixmap(150, 20);
    pixmap.fill(color);
    labelColorBox->setPixmap(pixmap);
}

void SettingPanel::onBgTypeChanged(int idx) {
    if (idx == 0) {
        btnChooseColor->setEnabled(true);
        labelColorBox->setEnabled(true);
        labelColor->setEnabled(true);
        btnChoosePicture->setEnabled(false);
        labelPicPath->setEnabled(false);
        sliderTransparency->setEnabled(false);
    }
    else {
        btnChooseColor->setEnabled(false);
        labelColorBox->setEnabled(false);
        labelColor->setEnabled(false);
        btnChoosePicture->setEnabled(true);
        labelPicPath->setEnabled(true);
        sliderTransparency->setEnabled(true);
    }
}

void SettingPanel::onApply() {
    if (editYourName->text().length() > 10) {
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
    if (playerRival && editYourName->text().compare(playerRival->getName(), Qt::CaseInsensitive) == 0) {
        QMessageBox::critical(this, "Error", "Your name can't be the same with your rival.",
                              QMessageBox::Ok);
        return;
    }

    settings.bgType = (comboBoxBackgroundType->currentIndex() == 0 ? BG_PURE_COLOR : BG_PICTURE);
    if (settings.bgType == BG_PICTURE) {
        if (bgImg.isNull()) {
            QMessageBox::critical(this, "Error", "No picture selected", QMessageBox::Ok);
            flagClose = false;
            return;
        }
        else {
            bgImg.save("data/cache/bg.png", "PNG");
            flagClose = true;
        }
    }

    settings.yourName = editYourName->text();
    settings.bgColor = colorTemp;
    settings.bgTransparency = sliderTransparency->value();

    emit sigSettingsChanged(this->settings);
}

void SettingPanel::onOk() {
    onApply();
    if (flagClose)
        this->close();
}

void SettingPanel::onChooseColor() {
    QColor tmp = QColorDialog::getColor(colorTemp);
    if (tmp.isValid()) {
        colorTemp = tmp;
        setBgColor(colorTemp);
    }
}

void SettingPanel::onResetColor() {
    colorTemp = QColor(200, 150, 100);
    setBgColor(colorTemp);
}

void SettingPanel::onChoosePicture() {
    QString filename = QFileDialog::getOpenFileName(this, "Choose a picture",
                                                    "", "picture (*.jpg *.png *.bmp)");

    if (!filename.isNull()) {
        bgImg = ImageCropperDialog::getCroppedImage(filename, 600, 400, CropperShape::SQUARE);
    }
}
