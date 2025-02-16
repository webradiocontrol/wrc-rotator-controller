/**
 * Web Radio Control generic rotator controller firmware
 * Copyright (C) 2024 Mikael Nousiainen OH3BHX <mikael@webradiocontrol.tech>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef WRC_ROTATOR_CONTROLLER_CONTROLLER_CLIENT_H
#define WRC_ROTATOR_CONTROLLER_CONTROLLER_CLIENT_H

#include <Client.h>
#include "print.h"
#include "config.h"

#define CLIENT_INPUT_NEW_COMMAND 1
#define CLIENT_INPUT_WAITING 0
#define CLIENT_INPUT_TOO_LONG (-1)

class ControllerClient {
private:
    char clientCommandBuffer[NETWORK_CLIENT_COMMAND_LENGTH];
    int clientCommandLength;
    char clientCommand[NETWORK_CLIENT_COMMAND_LENGTH];
    bool monitor;
    arduino::Client* client;
    String remoteIpAddress;
    uint16_t remotePort;

public:
    explicit ControllerClient(arduino::Client* networkClient, String remoteIpAddress, uint16_t remotePort)
    {
        this->client = networkClient;
        this->remoteIpAddress = remoteIpAddress;
        this->remotePort = remotePort;
        this->monitor = false;
        clientCommand[0] = '\0';
        clientCommandBuffer[0] = '\0';
        clientCommandLength = 0;
    }

    Print* getNetworkClient()
    {
        return client;
    }

    String getRemoteIpAddress()
    {
        return remoteIpAddress;
    }

    uint16_t getRemotePort() const
    {
        return remotePort;
    }

    bool isMonitorEnabled() const
    {
        return monitor;
    }

    void setMonitorEnabled(bool enable_monitor)
    {
        this->monitor = enable_monitor;
    }

    int processInput()
    {
        while (client->available() > 0) {
            char c = client->read();

            if (clientCommandLength >= (NETWORK_CLIENT_COMMAND_LENGTH - 1)) {
                clientCommandBuffer[0] = '\0';
                clientCommandLength = 0;
                return CLIENT_INPUT_TOO_LONG;
            }

            if (c == '\r' || c == '\t') {
                continue;
            }
            if (c == '\n') {
                strcpy(clientCommand, clientCommandBuffer);
                clientCommandBuffer[0] = '\0';
                clientCommandLength = 0;
                return CLIENT_INPUT_NEW_COMMAND;
            }

            clientCommandBuffer[clientCommandLength] = c;
            clientCommandBuffer[clientCommandLength + 1] = '\0';
            clientCommandLength++;
        }

        return CLIENT_INPUT_WAITING;
    }

    char *getCommand()
    {
        return clientCommand;
    }

    bool connected()
    {
        return client && client->connected();
    }

    void stop()
    {
        return client->stop();
    }

    bool cleanUpIfDisconnected()
    {
        if (!client->connected()) {
            p("Closed TCP connection to %s:%d\n", remoteIpAddress.c_str(), remotePort);
            delay(2);
            client->stop();
            return true;
        }

        return false;
    }
};

#endif
