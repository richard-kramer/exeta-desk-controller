# Exeta Desk Controller

Remote control your [Exeta motorized desk](https://exeta.de/) over the network.

This works with an Arduino board reading the central control box signals and simulating button presses on the controller.

## Compatibility

This code was written for the control panel CT01-CH and attaches directly to this. I'm not sure, if or how this should
be adapted to work with newer exeta desks/controllers or different brands of motorized desks.

It was also developed to be installed on a [WEMOS D1 Mini](https://www.az-delivery.de/products/d1-mini).
Compatibility with other boards is untested.

## Installation

### Physical connection

***DISCLAIMER:* Do this at your own risk and keep in mind that your are probably voiding your warranty. Me or my
instructions are not responsible for possible damages.** I just want to share my knowledge after modding my own desk.

Screw open your control unit (mine is a touch controlled one) and take out the PCB. Be careful to not accidentally cut
a wire

Connect your cables from the D1 Mini to the controller PCB by soldering them according to the schematic:

![D1 Mini to controller pcb connection](doc/schematics/exeta-to-d1mini.png)

### Software installation

**TODO: finalize documentation**

- dependencies
- credentials
- platformio
  - usb permissions

## Schematics

Schematics can be found in [`doc/schematics/`](doc/schematics).

I also took a few [photos of my own controller](doc/photos) and the connected D1 Mini. Unfortunately I has the great idea to secure
my soldered wires with hot glue, so it is really hard to see, whats soldered where.

### Controller Signaling

The controller sends signals (`HIGH`) over the wires 5 to 8 on button press according to the following mapping:

| Button | Active Pins |
| --- | --- |
| M | 8 |
| 1 | 5 |
| 2 | 6 + 7 |
| 3 | 5 + 7 |
| Up | 6 |
| Down | 7 |

These pins are connected to the D1 Mini as `OUTPUT`. By setting them to `HIGH`, the board simulates a button press on
the controller.

Additionally, it receives SoftwareSerial (UART?) packages from the central control box via Pin 4 (`SoftwareSerial` RX)
with a baud rate of `9600`. TX is unused. To use this information on the D1 mini, the RX wire is connected as `INPUT` on
the arduino and configured as `SoftwareSerial` RX pin.

Pin 1 and 2 are used for 5V power.

I did not check (or forgot) what pin 3 does. But it is not needed to control the desk.
