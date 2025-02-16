/**
 * Based on: https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/PwmIn
 *
 * MIT License
 *
 * Copyright (c) 2021 GitJer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PICO_PWMIN_H
#define PICO_PWMIN_H

#include <Arduino.h>
#include "hardware/pio.h"

#include "PwmIn.pio.h"

// class that sets up and reads PWM pulses: PwmIn. It has three functions:
// readPeriod (in seconds)
// readPulsewidth (in seconds)
// readDutycycle (between 0 and 1)
// read pulsewidth, period, and calculate the dutycycle

class PwmIn
{
public:
    // constructor
    // input = pin that receives the PWM pulses.
    PwmIn(uint input, unsigned long timeoutMicros = 20 * 1000) : timeout(timeoutMicros)
    {
        // pio 0 is used
        pio = pio0;
        // state machine 0
        sm = 0;
        // configure the used pins
        pio_gpio_init(pio, input);
        // load the pio program into the pio memory
        uint offset = pio_add_program(pio, &PwmIn_program);
        // make a sm config
        pio_sm_config c = PwmIn_program_get_default_config(offset);
        // set the 'jmp' pin
        sm_config_set_jmp_pin(&c, input);
        // set the 'wait' pin (uses 'in' pins)
        sm_config_set_in_pins(&c, input);
        // set shift direction
        sm_config_set_in_shift(&c, false, false, 0);
        // init the pio sm with the config
        pio_sm_init(pio, sm, offset, &c);
        // enable the sm
        pio_sm_set_enabled(pio, sm, true);
    }

    // readPeriod (in seconds)
    float readPeriod(void)
    {
        // one clock cycle is 1/125000000 seconds
        return (period * 0.000000008);
    }

    // readPulsewidth (in seconds)
    float readPulsewidth(void)
    {
        // one clock cycle is 1/125000000 seconds
        return (pulsewidth * 0.000000008);
    }

    // readDutycycle (between 0 and 1)
    float readDutycycle(void)
    {
        return ((float)pulsewidth / (float)period);
    }

    // read pulsewidth and period for one pulse
    void readPwm(float *readings)
    {
        *(readings + 0) = (float)pulsewidth * 0.000000008;
        *(readings + 1) = (float)period * 0.000000008;
        *(readings + 2) = ((float)pulsewidth / (float)period);
    }

    // read the period and pulsewidth
    bool read()
    {
        unsigned long start = micros();
        // clear the FIFO: do a new measurement
        pio_sm_clear_fifos(pio, sm);
        // wait for the FIFO to contain two data items: pulsewidth and period
        while (pio_sm_get_rx_fifo_level(pio, sm) < 2) {
            // timeout after 1 ms
            if ((micros() - start) > timeout) {
                return false;
            }
        }
        // read pulse width from the FIFO
        pulsewidth = pio_sm_get(pio, sm);
        // read period from the FIFO
        period = pio_sm_get(pio, sm) + pulsewidth;
        // the measurements are taken with 2 clock cycles per timer tick
        pulsewidth = 2 * pulsewidth;
        // calculate the period in clock cycles:
        period = 2 * period;

        return true;
    }

private:
    // the pio instance
    PIO pio;
    // the state machine
    uint sm;
    // data about the PWM input measured in pio clock cycles
    uint32_t pulsewidth, period;
    // timeout in microseconds
    unsigned long timeout;
};

#endif
