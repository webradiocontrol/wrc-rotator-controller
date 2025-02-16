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

#ifndef WRC_ROTATOR_CONTROLLER_CONFIG_H
#define WRC_ROTATOR_CONTROLLER_CONFIG_H

// Application info

#define APP_NAME "Web Radio Control generic rotator controller server"
#define APP_VERSION "0.1.0"

#define APP_VERSION_STRING APP_NAME " v" APP_VERSION

// Network configuration

#define WIFI_SSID "SET_SSID"
#define WIFI_PASSWORD "SET_PASSWORD"

// Uncomment to define a fixed IP address, otherwise DHCP is used
#define SERVER_FIXED_IP_ADDRESS "SET_IP_ADDRESS"
#define SERVER_TCP_PORT 1234

// Pin definitions

#define PIN_PWM 2 // GP2 = PIN 4, IN: PWM signal from rotator

#define PIN_CCW 3 // GP3 = PIN 5, OUT: Turn counter-clockwise
#define PIN_CW 4 // GP4 = PIN 6, OUT: Turn clockwise
#define PIN_SPEED 5 // GP5 = PIN 7, OUT: Speed control, HIGH for fast speed, LOW for slow speed

#define PIN_THRESHOLD_1 6 // GP6 = PIN 9, IN: Indicator for azimuth < 0
#define PIN_THRESHOLD_2 7 // GP7 = PIN 10, IN: Indicator for azimuth > 360
#define PIN_LIMIT_1 8 // GP8 = PIN 11, IN: Indicator for lowest possible azimuth
#define PIN_LIMIT_2 9 // GP9 = PIN 12, IN: Indicator for highest possible azimuth

// Default settings

#define AZIMUTH_MINIMUM (-90)
#define AZIMUTH_MAXIMUM 450
#define ANGLE_THRESHOLD 0.3

#define CLIENT_PUSH_INTERVAL 100 // milliseconds
#define PWM_CAPTURE_WINDOW_DURATION (20 * 1000) // microseconds

// Speed settings
// While the protocol supports speeds from 0 to 100, currently speeds 0-50 map to a slower speed and 50-100 to a faster speed.

#define SPEED_SLOW 25
#define SPEED_FAST 75
#define DEFAULT_SPEED SPEED_SLOW

#define ANGLE_DIFFERENCE_FOR_FAST_SPEED 30
#define ANGLE_DIFFERENCE_FOR_SLOW_SPEED 10

// Rotator settings

#define ROTATOR_AZIMUTH_OFFSET_DEGREES 0

// Network connection handling

#define NETWORK_CLIENT_COUNT 8
#define NETWORK_CLIENT_COMMAND_LENGTH 32

#endif
