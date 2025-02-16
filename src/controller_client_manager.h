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

#ifndef WRC_ROTATOR_CONTROLLER_CONTROLLER_CLIENT_MANAGER_H
#define WRC_ROTATOR_CONTROLLER_CONTROLLER_CLIENT_MANAGER_H

#include "controller_client.h"
#include "controller_command_handler.h"
#include "print.h"

class ControllerClientManager {
private:
    ControllerClient *clients[NETWORK_CLIENT_COUNT]{};
    ControllerCommandHandler *handler;
    unsigned long lastClientPushTime = 0;

public:
    explicit ControllerClientManager(ControllerCommandHandler *handler)
    {
        this->handler = handler;

        for (auto &client : this->clients) {
            client = nullptr;
        }
    }

    bool addClient(ControllerClient* newClient)
    {
        bool freeClientSlotFound = false;
        p("New TCP connection from %s:%d\n", newClient->getRemoteIpAddress().c_str(),
                newClient->getRemotePort());

        for (auto &client : clients) {
            if (client == nullptr) {
                client = newClient;
                freeClientSlotFound = true;
                break;
            }
        }

        if (!freeClientSlotFound) {
            newClient->getNetworkClient()->println("ERROR: TOO MANY CONNECTIONS");
            p("Cannot handle TCP connection from %s:%d: too many connections\n",
                    newClient->getRemoteIpAddress().c_str(), newClient->getRemotePort());
            newClient->stop();
            return false;
        }

        return true;
    }

    void processInput()
    {
        for (auto client : clients) {
            if (client == nullptr) {
                continue;
            }

            int result = client->processInput();

            switch (result) {
                case CLIENT_INPUT_NEW_COMMAND:
                    p("Command from %s:%d: %s\n", client->getRemoteIpAddress().c_str(),
                            client->getRemotePort(), client->getCommand());
                    handler->handleCommand(String(client->getCommand()), client, client->getNetworkClient());
                    break;
                case CLIENT_INPUT_TOO_LONG:
                    client->getNetworkClient()->println("ERROR COMMAND TOO LONG");
                    break;
                default:
                    break;
            }
        }
    }

    bool pushToClients(const char *command, bool (ControllerClientManager::*condition)(ControllerClient *client))
    {
        bool handled = false;

        for (auto client : clients) {
            if (client == nullptr) {
                continue;
            }
            if (!(this->*condition)(client)) {
                continue;
            }

            handled |= handler->handleCommand(String(command), client, client->getNetworkClient());
        }

        return handled;
    }

    bool isClientMonitorEnabled(ControllerClient *client)
    {
        return client->isMonitorEnabled();
    }

    bool pushToMonitoringClients(const char *command)
    {
        return pushToClients(command, &ControllerClientManager::isClientMonitorEnabled);
    }

    bool isTimeToPushToClients()
    {
        unsigned long current_time = millis();
        bool push_to_clients = (current_time - lastClientPushTime) >= CLIENT_PUSH_INTERVAL;

        if (push_to_clients) {
            lastClientPushTime = current_time;
        }

        return push_to_clients;
    }

    void cleanUp()
    {
        for (byte i = 0; i < NETWORK_CLIENT_COUNT; i++) {
            if (clients[i] == nullptr) {
                continue;
            }

            if (clients[i]->cleanUpIfDisconnected()) {
                clients[i] = nullptr;
            }
        }
    }
};

#endif
