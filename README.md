# AsusWRT Xiaomi
This is AsusWRT port to Xiaomi Mi 4C router. I no longer have this device and finished work with it. 

## Features
- Updated packages and libraries
- MFP, WPA3 and OWE support
- Repeater mode support (without WPA3)
- Big JFFS partition with unlocked autorun scripts

## Supported devices
- None

## How to install
1. Download image from Releases page or build it from source
2. Flash it to a router from stock firmware (need exploit), unlocked bootloader or SPI programmer
Procedure is very similar to installation of OpenWRT/Padavan; flash address is the same (OS1 partition)

## How to go back to OEM firmware
1. Download OpenWRT initramfs image, flash it to linux partition with command `mtd-write -i <image> -d linux` and reboot
2. Then follow OpenWRT instruction described here: https://openwrt.org/toh/xiaomi/xiaomi_mi_router_4c#openwrt_back_to_stock

## How to build image from source
1. cd release/src-ra-4300
2. add tools/brcm/K26/hndtools-mipsel-uclibc-4.2.4/bin to PATH
3. make rt-4c

## Important notes
- I do not take responsibility for any damages - you do everything on your own risk
- Depsite last version is stable and I am not aware of any issues, I have already finished my work on this project and would no longer support it.
