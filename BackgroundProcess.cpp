#include "BackgroundProcess.h"
#include "LocalSocket.h"

#include <stdexcept>
#include <sys/stat.h>
#include <sys/syslog.h>

#include <unistd.h>

namespace {

    void daemonize() {
        auto pid = fork();
        if (pid < 0) {
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            exit(EXIT_SUCCESS);
        }

        if (setsid() < 0) {
            exit(EXIT_FAILURE);
        }

        pid = fork();
        if (pid < 0) {
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            exit(EXIT_SUCCESS);
        }

        umask(0);
        chdir("/");

        auto maximumOpenFiles = sysconf(_SC_OPEN_MAX);
        for (auto currentFile = 0; currentFile <= maximumOpenFiles; currentFile++) {
            close(currentFile);
        }
    }

    void processMain() {
        LocalSocket localSocket{ LocalSocket::Usage::Server };

        syslog(LOG_NOTICE, "Created socket for local host communication");
        syslog(LOG_NOTICE, "Started");

        while (true) {
            if (!localSocket.isReadable()) {
                continue;
            }

            auto message = localSocket.read();
            syslog(LOG_NOTICE, "Received message from local host: %s", message.text.c_str());

            if (message.text == BackgroundProcess::Messages::LIST_DEVICES) {
                syslog(LOG_NOTICE, "Sending device list response");
                localSocket.sendTo("asd", message.sender);
            } else if (message.text == BackgroundProcess::Messages::EXIT) {
                syslog(LOG_NOTICE, "Shuting down");
                return;
            }
        }
    }
}

void BackgroundProcess::run() {
    daemonize();

    openlog("DeviceBackgroundProcess", LOG_PID, LOG_DAEMON);

    try {
        processMain();
    } catch (const std::runtime_error& e) {
        syslog(LOG_ERR, "%s", e.what());
    }

    closelog();
}
