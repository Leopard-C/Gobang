#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "jsoncpp/json/json.h"


bool sendJsonMsg(const std::string& msg, int fd);
bool sendJsonMsg(const Json::Value& jsonMsg, int fd);

int recvJsonMsg(Json::Value& root, int fd);

