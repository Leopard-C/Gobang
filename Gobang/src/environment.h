#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <QString>
#include <QColor>
#include "chess/base.h"

class Env {
public:
    static QString NAME;
    static BackgroundType BG_TYPE;
    static QColor BG_COLOR;
    static int BG_TRANSPARENCY;

public:
    static bool load();
    static bool save();
};


#endif // ENVIRONMENT_H
