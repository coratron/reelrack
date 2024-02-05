# Reelrack mk-I Firmware

A custom firmware was written for this application. Some RGB LED strip controllers may have an API, but it was easier to start from scratch as it was also desirable to store the information about the reels on the RGB controller itself. 

## Hardware Requirements

- An ESP32
- 5V supply. 500mA is enough, as the project only intends to use one LED at a time, if you want to customise the fimrware to use more LEDs simultaneously you will have to use a bigger power supply.
- A voltage level shifter to translate the signal (RGB data) from 3.3V to 5V.

### Convenient Off-the-shelf choice: Gledopto GL-C-010WL-D

This little RGB controller contains everything we need (except the power supply) in a compact form factor. It has a USB port on the module so by opening the box we can simply reflash the unit with our custom firmware, and we are good to go.

The pinout is clearly labeled on the case: we have IO2 and IO16 directly exposed on the connector (and shifted to 5V).

A bracket can be printed to attach the GL-C-010WL-D module to the rack, it can be found here:

## Pinout

The data pin for the RGB LED strip can be configured in ```rgb.h```. IO2 is the default pin.