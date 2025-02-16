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

#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "print.h"
#include "iointerface.h"
#include "pwm_reader.h"
#include "controller_command_handler.h"
#include "controller_client_manager.h"

// Network settings

const char *ssid = WIFI_SSID;
const char *pass = WIFI_PASSWORD;

const uint16_t tcpPort = SERVER_TCP_PORT;

#if defined(SERVER_FIXED_IP_ADDRESS)
IPAddress fixedIpAddress;
#endif

// Application code

IOInterface *io;
WiFiServer server(tcpPort);
ControllerCommandHandler *commandHandler;
ControllerClientManager *clientManager;

PwmReader* pwmReader;

String IpAddressToString(const IPAddress &ipAddress)
{
    return String(ipAddress[0]) + String(".") +
           String(ipAddress[1]) + String(".") +
           String(ipAddress[2]) + String(".") +
           String(ipAddress[3]);
}

void setupNetworkServer()
{
#if defined(SERVER_FIXED_IP_ADDRESS)
    bool validIpAddress = fixedIpAddress.fromString(SERVER_FIXED_IP_ADDRESS);
    if (validIpAddress) {
        p("Using fixed IP address %s\n", SERVER_FIXED_IP_ADDRESS);
        WiFi.config(fixedIpAddress);
    } else {
        p("Invalid IP address: %s\n", SERVER_FIXED_IP_ADDRESS);
    }
#endif

    p("Connecting to WiFi network to %s ...\n", ssid);

    pinMode(PIN_LED, OUTPUT);

    int status = WiFi.begin(ssid, pass);

    PinStatus led = HIGH;
    while (status != WL_CONNECTED) {
        digitalWrite(PIN_LED, led);
        status = WiFi.status();
        p("Waiting for WiFi connection to %s, status %d...\n", ssid, status);
        delay(1000);
        led = (led == HIGH) ? LOW : HIGH;
    }

    digitalWrite(PIN_LED, HIGH);

    p("Connected to WiFi network %s\n", ssid);

    server.begin();

    p("TCP server is listening at %s:%d\n", IpAddressToString(WiFi.localIP()).c_str(), tcpPort);
}

void setup()
{
    SERIAL_PORT.begin(SERIAL_PORT_SPEED);
    delay(2000);
    p(APP_VERSION_STRING " running on " BOARD_NAME "\n");

    setupNetworkServer();

    pwmReader = new PwmReader(PIN_PWM, PWM_CAPTURE_WINDOW_DURATION);
    io = new IOInterface(pwmReader);
    commandHandler = new ControllerCommandHandler(io, ROTATOR_AZIMUTH_OFFSET_DEGREES);
    clientManager = new ControllerClientManager(commandHandler);
}

void loop()
{
    commandHandler->performStateChangesIfNecessary();
    clientManager->cleanUp();

    WiFiClient newClient = server.accept();

    if (newClient) {
        p("New client connected\n");
        String clientAddress = IpAddressToString(newClient.remoteIP());
        clientManager->addClient(new ControllerClient(new WiFiClient(newClient), clientAddress, newClient.remotePort()));
    }

    bool pushToClients = clientManager->isTimeToPushToClients();

    if (pushToClients) {
        pwmReader->read();

        clientManager->pushToMonitoringClients("STATE");
    }

    clientManager->processInput();
}
