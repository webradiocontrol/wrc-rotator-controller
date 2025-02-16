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

#ifndef WRC_ROTATOR_CONTROLLER_PWM_READER_H
#define WRC_ROTATOR_CONTROLLER_PWM_READER_H

#include <Arduino.h>

#include "PwmIn.h"

class PwmReader {
private:
    PwmIn* pwmIn;

public:
    explicit PwmReader(uint pin, unsigned long timeoutMicros) {
        pinMode(pin, INPUT_PULLDOWN);
        pwmIn = new PwmIn(pin, timeoutMicros);
    }

    void read() {
        pwmIn->read();
    };

    double angle() {
        return 360 * pwmIn->readDutycycle();
    };
};

#endif
