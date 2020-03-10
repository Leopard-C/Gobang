QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#win32 {
#    LIBS += -lpthread libwsock32 libws2_32
#}

# jsoncpp 1.9.2
INCLUDEPATH += inc/json

SOURCES += \
    inc/json/json_reader.cpp \
    inc/json/json_value.cpp \
    inc/json/json_writer.cpp \
    src/chess/base.cpp \
    src/chess/chessboard.cpp \
    src/chess/chessboardvs.cpp \
    src/dialog/chessonline.cpp \
    src/dialog/createroomdialog.cpp \
    src/dialog/joinroomdialog.cpp \
    src/dialog/settingpanel.cpp \
    src/dialog/watchchessonline.cpp \
    src/environment.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/network/api.cpp \
    src/network/client.cpp \
    src/widget/chathistory.cpp \
    src/widget/chatinput.cpp \
    src/widget/imagecropperlabel.cpp

HEADERS += \
    inc/json/json/allocator.h \
    inc/json/json/assertions.h \
    inc/json/json/autolink.h \
    inc/json/json/config.h \
    inc/json/json/forwards.h \
    inc/json/json/json.h \
    inc/json/json/json_features.h \
    inc/json/json/reader.h \
    inc/json/json/value.h \
    inc/json/json/version.h \
    inc/json/json/writer.h \
    inc/json/json_tool.h \
    inc/json/json_valueiterator.inl \
    src/chess/base.h \
    src/chess/chessboard.h \
    src/chess/chessboardvs.h \
    src/chess/player.h \
    src/dialog/chessonline.h \
    src/dialog/createroomdialog.h \
    src/dialog/imagecropperdialog.h \
    src/dialog/joinroomdialog.h \
    src/dialog/settingpanel.h \
    src/dialog/watchchessonline.h \
    src/environment.h \
    src/mainwindow.h \
    src/network/api.h \
    src/network/client.h \
    src/widget/chathistory.h \
    src/widget/chatinput.h \
    src/widget/imagecropperlabel.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
