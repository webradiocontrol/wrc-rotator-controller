# Web Radio Control generic antenna rotator controller firmware

The Web Radio Control generic antenna rotator controller firmware provides a general-purpose,
text-based network protocol for controlling an azimuth antenna rotator. The firmware is built on
Raspberry Pico W hardware, so that it can use the Wi-Fi network for interfacing with
computer remote control.

The rotator position is determined using an absolute magnetic shaft encoder,
which is a very accurate and durable sensor.

## Hardware requirements

* Board: Raspberry Pico W

### Suppoerted sensor for rotator position

* Rotator position input: PWM duty cycle read using Raspberry Pico Programmable I/O (PIO) and channel 0
  * Sensor: US Digital MA3 - Miniature Absolute Magnetic Shaft Encoder
    * NOTE: MA3 output needs 5V <-> 3.3V voltage level conversion for Raspberry Pico

## Rotator control

Connect all pins via opto-isolators and relays to protect the Raspberry Pico.

Any Pico GPIO pin can be used for any purpose, the defaults are:

* Rotator position input: GPIO input, pin 2
* CW/CCW direction control: GPIO outputs, pins 5 and 6
* Azimuth < 0 or > 360 signals: GPIO inputs, pins 9 and 10
* Minimum/maximum azimuth signals: GPIO inputs, pins 11 and 12
* Speed control (optional): pin 7 (HIGH for fast speed, LOW for slow speed)
  * While the protocol supports speeds from 0 to 100, currently speeds 0-50 map to a slower speed and 50-100 to a faster speed.

## Configuration

Define the following configuration values in file `src/config.h`:

Set the Wi-Fi network name and password using macros `WIFI_SSID` and `WIFI_PASSWORD`.

Set the desired TCP port for the network server using macros `SERVER_TCP_PORT` (defaults to 1234).

If you need to use a fixed IP address, set desired IP address using macro `SERVER_FIXED_IP_ADDRESS`.
If the setting is commented out, the IP address will be assigned automatically using DHCP.

Set the default speed using `DEFAULT_SPEED` (0-100).

Set azimuth offset using `ROTATOR_AZIMUTH_OFFSET_DEGREES` if needed.

## Build

```bash
platformio run
```

## Flash

```bash
platformio run --target upload
```

## Flash and monitor serial port

```bash
platformio run --target upload --target monitor
```

## Network protocol commands

All successful responses being with `OK` and errors with `ERROR`.

### Get controller state

`STATE` - Get controller state
Example response: `OK STATE AZ=50.9 SPEED=50 FLAGS=`

Where:

* AZ = azimuth in degrees (0-360)
* SPEED = current speed (0-100)
* FLAGS = comma-separated list of flags:
  * CW = clockwise direction rotation active
  * CCW = counter-clockwise direction rotation active
  * T1 = threshold 1 reached (azimuth < 0)
  * T2 = threshold 2 reached (azimuth > 360)
  * L1 = limit 1 reached (minimum azimuth)
  * L2 = limit 2 reached (maximum azimuth)

### Set azimuth

`AZ [degrees]` - Set target azimuth to [degrees]

Response: `OK`

Example command: `AZ 180`

### Get azimuth

`AZ?` - Get current azimuth in degrees

Response: `OK AZ 50.9`

### Move CW/CCW

`MOVE CW` - Start rotation in clockwise direction

Response: `OK`

`MOVE CCW` - Start rotation in counter-clockwise direction

Response: `OK`

### Stop

`STOP` - Stop rotation

Response: `OK`

### Park

`PARK` - Move to park position (azimuth 0)

Response: `OK`

### Reset

`RESET` - Reset controller state: stop + set speed to default

Response: `OK`

### Set speed

`SPEED [0-100]` - Set speed to [0-100]

Response: `OK`

Example command: `SPEED 50`

### Get speed

`SPEED?` - Get current speed

Response: `OK SPEED 50`

### Monitor state

`MONITOR [0|1]` - Monitor state changes: 0 = disable, 1 = enable

Response: `OK MONITOR [0|1]`

When enabled, controller outputs rotator state to the serial port continuously:

Example output: `STATE AZ=50.9 SPEED=50 FLAGS=T1`

### Set azimuth offset

`AZOFFSET [degrees]` - Set azimuth offset to [degrees], can be negative or positive

Response: `OK`

Example command: `AZOFFSET -10`

### Get azimuth offset

`AZOFFSET?` - Get current azimuth offset in degrees

Response: `OK AZOFFSET -10`

### Get azimuth limits

`AZLIMITS?` - Get current azimuth limits in degrees

Response: `OK AZLIMITS MIN=-90 MAX=450`
