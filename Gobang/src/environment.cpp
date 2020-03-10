#include "environment.h"

#include <json/json.h>
#include <fstream>
#include <QString>

QString         Env::NAME               = "player";
BackgroundType  Env::BG_TYPE            = BG_PURE_COLOR;
QColor            Env::BG_COLOR         = QColor(200, 150, 100);
int             Env::BG_TRANSPARENCY    = 255;

bool Env::load() {
    Json::Value root;
    Json::Reader reader;
    std::ifstream ifs("data/environment.json");
    if (!ifs.is_open()) {
        return true;
    }
    if (!reader.parse(ifs, root, false)) {
        ifs.close();
        return false;
    }

    if (!root["NAME"].isNull())
        NAME = QString::fromStdString(root["NAME"].asString());
    if (!root["BG_TYPE"].isNull())
        BG_TYPE = BackgroundType(root["BG_TYPE"].asInt());
    if (!root["BG_COLOR"].isNull())
        BG_COLOR = root["BG_COLOR"].asUInt();
    if (!root["BG_TRANSPARENT"].isNull())
        BG_TRANSPARENCY = root["BG_TRANSPARENT"].asInt();

    ifs.close();
    return true;
}

bool Env::save() {
    Json::Value root;
    std::ofstream ofs("data/environment.json");
    if (!ofs.is_open())
        return false;

    root["NAME"] = NAME.toStdString();
    root["BG_TYPE"] = int(BG_TYPE);
    root["BG_COLOR"] = BG_COLOR.rgb();
    root["BG_TRANSPARENT"] =BG_TRANSPARENCY;

    ofs << root.toStyledString();
    ofs.close();
    return true;
}
