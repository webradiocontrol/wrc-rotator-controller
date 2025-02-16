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
#include <cstdarg>
#include "print.h"

#define PRINTF_BUFFER_LENGTH 256

Print &printer = SERIAL_PORT;

void p(const char *fmt, ...)
{
    char buf[PRINTF_BUFFER_LENGTH];

/*    if (!SERIAL_PORT) {
        SERIAL_PORT.end();
        SERIAL_PORT.begin(SERIAL_PORT_SPEED);
        SERIAL_PORT.println("Opened new!");
    }*/

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printer.print(buf);

    printer.flush();
}

#ifdef F

void p(const __FlashStringHelper *fmt, ...)
{
    char buf[PRINTF_BUFFER_LENGTH];
    va_list args;
    va_start(args, fmt);
#ifdef __AVR__
    vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args); // progmem for AVR
#else
    vsnprintf(buf, sizeof(buf), (const char *) fmt, args); // for the rest of the world
#endif
    va_end(args);
    printer.print(buf);
}

#endif
