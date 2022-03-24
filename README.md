# AsusWRT Xiaomi
This is AsusWRT port to Xiaomi Mi 4C router and possibly other MT7628 devices

## Features
- Updated packages and libraries
- MFP, WPA3 and OWE support
- Repeater mode support (without WPA3)
- Big JFFS partition with unlocked autorun scripts

## Supported devices
- Xiaomi Mi 4C router

## How to install
1. Download image from Releases page or build it from source
2. Flash it to a router from stock firmware (need exploit), unlocked bootloader or SPI programmer
Procedure is very similar to installation of OpenWRT/Padavan; flash address is the same (OS1 partition)

## How to build image from source
1. cd release/src-ra-4300
2. add tools/brcm/K26/hndtools-mipsel-uclibc-4.2.4/bin to PATH
3. make model (currently available model is: rt-4c)

## Important note
- I do not take responsibility for any damages - you do everything on your own risk
