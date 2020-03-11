#include "gobangserver.h"

#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

GobangServer server;

void handleCtrlC(int num);
void exitFunc();

int main(int argc, char** argv) {
    atexit(exitFunc);
    signal(SIGINT, handleCtrlC);

    srand(time(NULL));

    if (!server.start(10032)) {
        std::cerr << "Start failed!" << std::endl;
        return 1;
    }

    return 0;
}

void handleCtrlC(int num) {
    exit(0);
}

void exitFunc() {
    server.stop();
}

