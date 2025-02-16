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

#ifndef WRC_ROTATOR_CONTROLLER_CONTROLLER_COMMAND_HANDLER_H
#define WRC_ROTATOR_CONTROLLER_CONTROLLER_COMMAND_HANDLER_H

#include "iointerface.h"
#include "controller_client.h"

class ControllerCommandHandler {
private:
    IOInterface *io;
    double azimuthOffset;
    double targetAzimuth = 0;
    bool targetAzSet = false;

public:
    explicit ControllerCommandHandler(IOInterface *io, double azimuth_offset)
    {
        this->io = io;
        this->azimuthOffset = azimuth_offset;
    }

    double getAzimuth()
    {
        double angle = io->getAzimuthAngle();

        if (io->getThreshold1State()) {
            if (angle >= 270 && angle < 360) {
                angle = -(360 - angle);
            }
        } else if (io->getThreshold2State()) {
            if (angle >= 0 && angle < 110) {
                angle += 360;
            }
        }

        return angle + azimuthOffset;
    }

    void setAzimuth(double az)
    {
        double currentAzimuth = getAzimuth();

        targetAzimuth = az;
        targetAzSet = true;

        if (currentAzimuth < (az - ANGLE_THRESHOLD)) {
            io->setCounterClockwise(false);
            io->setClockwise(true);
        } else if (currentAzimuth > (az + ANGLE_THRESHOLD)) {
            io->setClockwise(false);
            io->setCounterClockwise(true);
        } else {
            io->setClockwise(false);
            io->setCounterClockwise(false);
        }
    }

    void performStateChangesIfNecessary()
    {
        double currentAngle = getAzimuth();

        if (targetAzSet) {
            double difference = abs(targetAzimuth - currentAngle);

            if (difference >= ANGLE_DIFFERENCE_FOR_FAST_SPEED) {
                if (io->getSpeed() != SPEED_FAST) {
                    io->setSpeed(SPEED_FAST);
                }
            } else if (difference <= ANGLE_DIFFERENCE_FOR_SLOW_SPEED) {
                if (io->getSpeed() != SPEED_SLOW) {
                    io->setSpeed(SPEED_SLOW);
                }
            }

            if (io->getClockwise()) {
                if (currentAngle >= (targetAzimuth - ANGLE_THRESHOLD)) {
                    io->setClockwise(false);
                    targetAzSet = false;
                }
            }

            if (io->getCounterClockwise()) {
                if (currentAngle <= (targetAzimuth + ANGLE_THRESHOLD)) {
                    io->setCounterClockwise(false);
                    targetAzSet = false;
                }
            }
        }

        if (io->getLimit2State() && io->getClockwise()) {
            io->setClockwise(false);
            targetAzSet = false;
        }
        if (io->getLimit1State() && io->getCounterClockwise()) {
            io->setCounterClockwise(false);
            targetAzSet = false;
        }
    }

    String getFlags()
    {
        String flags = String();

        if (io->getClockwise()) {
            flags.concat("CW,");
        }
        if (io->getCounterClockwise()) {
            flags.concat("CCW,");
        }
        if (io->getThreshold1State()) {
            flags.concat("T1,");
        }
        if (io->getThreshold2State()) {
            flags.concat("T2,");
        }
        if (io->getLimit1State()) {
            flags.concat("L1,");
        }
        if (io->getLimit2State()) {
            flags.concat("L2");
        }
        if (flags.endsWith(",")) {
            flags.remove(flags.length() - 1, 1);
        }

        return flags;
    }

    int getSpeed()
    {
        return io->getSpeed();
    }

    void setSpeed(int speed)
    {
        io->setSpeed(speed);
    }

    void stop()
    {
        io->setClockwise(false);
        io->setCounterClockwise(false);
        targetAzSet = false;
    }

    void park()
    {
        setAzimuth(0);
    }

    void reset()
    {
        stop();
        io->setSpeed(DEFAULT_SPEED);
    }

    void moveClockwise()
    {
        io->setSpeed(SPEED_SLOW);
        io->setCounterClockwise(false);
        io->setClockwise(true);
    }

    void moveCounterClockwise()
    {
        io->setSpeed(SPEED_SLOW);
        io->setClockwise(false);
        io->setCounterClockwise(true);
    }

    bool handleCommand(String command, ControllerClient *client, Print *response)
    {
        command.trim();

        if (command.length() == 0) {
            return true;
        }

        int firstSpaceIndex = command.indexOf(' ');
        String name;
        if (firstSpaceIndex >= 0) {
            name = command.substring(0, firstSpaceIndex);
        } else {
            name = command;
        }

        if (name == "AZ" && firstSpaceIndex > 0) {
            String azString = command.substring(firstSpaceIndex + 1);
            azString.trim();
            double azAngle = azString.toDouble();
            // TODO: detect angle parse errors!

            if (azAngle < AZIMUTH_MINIMUM || azAngle > AZIMUTH_MAXIMUM) {
                response->println("ERROR INVALID AZIMUTH");
                return false;
            }

            setAzimuth(azAngle);
            response->print("OK AZ ");
            response->println(azAngle);
        } else if (name == "AZ?") {
            String azString = String(getAzimuth(), 1);
            response->print("OK AZ ");
            response->println(azString.c_str());
        } else if (name == "MOVE" && firstSpaceIndex > 0) {
            String direction = command.substring(firstSpaceIndex + 1);
            direction.trim();

            if (direction == "CW") {
                moveClockwise();
            } else if (direction == "CCW") {
                moveCounterClockwise();
            } else {
                response->println("ERROR INVALID DIRECTION");
                return false;
            }

            response->print("OK MOVE ");
            response->println(direction);
        } else if (name == "STATE") {
            String azString = String(getAzimuth(), 1);
            String speedString = String(getSpeed());

            response->print("OK STATE AZ=");
            response->print(azString.c_str());
            response->print(" SPEED=");
            response->print(speedString.c_str());
            response->print(" FLAGS=");
            response->println(getFlags());
        } else if (name == "SPEED" && firstSpaceIndex > 0) {
            String speedString = command.substring(firstSpaceIndex + 1);
            speedString.trim();
            double speed = speedString.toInt();
            // TODO: detect speed parse errors!

            if (speed < 0 || speed > 100) {
                response->println("ERROR INVALID SPEED");
                return false;
            }

            setSpeed(speed);
            response->print("OK SPEED ");
            response->println(speed);
        } else if (name == "SPEED?") {
            String speedString = String(getSpeed());
            response->print("OK SPEED ");
            response->println(speedString.c_str());
        } else if (name == "STOP") {
            stop();
            response->println("OK STOP");
        } else if (name == "PARK") {
            park();
            response->println("OK PARK");
        } else if (name == "RESET") {
            reset();
            response->println("OK RESET");
        } else if (name == "MONITOR" && firstSpaceIndex > 0) {
            String monitorString = command.substring(firstSpaceIndex + 1);
            monitorString.trim();
            bool monitor = monitorString.toInt() != 0;
            client->setMonitorEnabled(monitor);
            response->print("OK MONITOR ");
            response->println(monitor ? "1" : "0");
        } else if (name == "INFO") {
            response->println("OK INFO " APP_VERSION_STRING);
        } else if (name == "AZLIMITS?") {
            response->print("OK AZLIMITS MIN=");
            response->print(AZIMUTH_MINIMUM);
            response->print(" MAX=");
            response->println(AZIMUTH_MAXIMUM);
        } else if (name == "AZOFFSET" && firstSpaceIndex > 0) {
            String azString = command.substring(firstSpaceIndex + 1);
            azString.trim();
            double azOffset = azString.toDouble();
            // TODO: detect offset parse errors!

            if (azOffset < -360 || azOffset > 360) {
                response->println("ERROR INVALID AZIMUTH OFFSET");
                return false;
            }

            azimuthOffset = azOffset;
            response->print("OK AZOFFSET ");
            response->println(azOffset);
        } else if (name == "AZOFFSET?") {
            response->print("OK AZOFFSET ");
            response->println(azimuthOffset);
        } else {
            response->println("ERROR INVALID COMMAND");
            return false;
        }

        return true;
    }
};

#endif
