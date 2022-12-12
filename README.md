# HyperSerialESP32
  
Exposes USB high speed serial port at 2Mb baud for driving led strip using NeoPixelBus library. It's intended to replace slow Arduino solutions (level shifter 3.3V to 5v may be required). **Data integrity check (Fletcher's checksum) included in new 'Awa' protocol for HyperHDR. So no more random flashing caused by serial transmission errors.** That option must be checked in HyperHDR to make system works as on the screen below.  
  
**Make sure that your serial chip on the ESP32 can handle 2Mb speed: for example CP2102 can't as its max speed is 1Mb and you can compile a version for that speed but I think that's not the point, cheap CH340G can do it without any problems.**  
  
There is also my fork named HyperSerialWLED available with the support for the AWA serial protocol at @2Mb speed for both ESP8266 and ESP32: https://github.com/awawa-dev/HyperSerialWLED Can't guarantee it will work as stable as HyperSerialESP32 because WLED has a lot of other things to do in the backgrounds (ex. handling Wifi) and timing control for the serial port could be at danger for larger number of LEDs. But you don't need to abandon all the benefits offered by the WLED which can be a big advantage for some users.  
  
RGB to RGBW conversion is calibrated for the neutral white channel BTF SK6812 but it can be easily changed (for cool and warm temperature) in the code. Search for "color calibration". In HyperHDR use "1.5" gamma for red, blue and green for best effect in the "Image Processing" tab.


| LED strip / Device             | HyperSerialESP32 |
|--------------------------------|:----------------:|
| SK6812 cold white              |       yes        |
| SK6812 neutral white           |       yes        |
| WS281x                         |       yes        |
| SPI (APA102, SK9812, HD107...) |       yes        |
  
# Data integrity check
  
Why the data integrity check was introduced which causes incompatibility with other software? Because at 2Mb speed many chip-makers allow few percent error in the transmission. And we do not want to have any distracting flashes. Broken frames are abandon without showing them. At 100Hz for 250 leds approximately 1-5% of the frames are broken.
  
# Flashing
  
Recommend to use [esphome-flasher](https://github.com/esphome/esphome-flasher/releases)  

ESP32-S2 lolin mini requires special firmware version (also provided)

Generic ESP32:

For **RGBW LED strip** like RGBW SK6812 NEUTRAL white choose: *firmware_esp32_SK6812_RGBW_NEUTRAL.bin*  
  
For **RGBW LED strip** like RGBW SK6812 COLD white choose: *firmware_esp32_SK6812_RGBW_COLD.bin*  
  
For **RGB LED strip** like WS8212b or RGB SK6812 variant choose: *firmware_esp32_WS281x_RGB.bin*  
  
For **SPI driven RGB LED strip** APA102: *firmware_esp32_SPI_APA102_SK9822_HD107.bin*, WS8201: *firmware_esp32_SPI_WS2801.bin*  
  
If you want to disable your first LED because it's used as a sacrificial level shifter, please use [HyperHDR v19](https://github.com/awawa-dev/HyperHDR/pull/379)  

For the RGBW firmware the white channel is automatically calculated and R,G,B channels are corrected.  
  
# Usage in HyperHDR
  
Make sure you are using HyperHDR v19beta2 or above, set `Refresh time` to zero, `Baudrate` to 2000000 and you enabled `HyperHDR's AWA protocol`.  
Enabling `White channel calibration` is optional, if you want to fine tune the white channel balance of your sk6812 RGBW LED strip.  
`ESP8266/ESP32 handshake` could help you to properly initialize the ESP device and enables statistics available in the logs (you must stop the LED device first to get them).  

![obraz](https://user-images.githubusercontent.com/69086569/207076047-42f4e4f4-9724-475c-b781-186878d214fc.png)
   
# Compiling
  
Currently we use PlatformIO to compile the project. Install [Visual Studio Code](https://code.visualstudio.com/) and add [PlatformIO plugin](https://platformio.org/).
This environment will take care of everything and compile the firmware for you.

But there is also an alternative and an easier way. Just fork the project and enable its Github Action. Use the online editor to make changes to the ```platformio.ini``` file, for example change default pin-outs/speed or enable multi-segments support, and save it. Github Action will compile new firmware automatically in the Artifacts archive. It has never been so easy!

Tutorial: https://github.com/awawa-dev/HyperSerialESP32/wiki
  
# Pinout
  
**ESP32:**  
**LED output (non-SPI):** GPIO 2  
**LED output (SPI):** GPIO 4 for Clock, GPIO 2 for Data  

# Some benchmark results

ESP32 MH-ET LIVE mini is capable of 4Mb serial port speed and ESP32-S2 lolin mini is capable of 5Mb. But to give equal chances all models were tested using the default speed of 2Mb.

## Multi-segments can double your large sk6812/ws2812b setup refresh rate for free. All you need is to properly project & construct the LED strip and use HyperSerialESP32 v8.

| LED strip / Device                                                               | ESP32<br>MH-ET LIVE mini |
|----------------------------------------------------------------------------------|--------------------------|
| 300LEDs<br>Refresh rate/continues output=100Hz<br>SECOND_SEGMENT_START_INDEX=150 |            93-97         |
| 600LEDs<br>Refresh rate/continues output=100Hz<br>SECOND_SEGMENT_START_INDEX=300 |            78-79         |
| 900LEDs<br>Refresh rate/continues output=100Hz<br>SECOND_SEGMENT_START_INDEX=450 |            55-56         |

## Comparing v6.1 and v8 version (single segment) refresh rate using MH-ET LIVE mini

| LED strip / Device                             | ESP32<br>MH-ET LIVE mini<br>HyperSerialESP32 v6.1 | ESP32<br>MH-ET LIVE mini<br>HyperSerialESP32 v8 |
|------------------------------------------------|---------------------------------------------------|-------------------------------------------------|
| 300LEDs<br>Refresh rate/continues output=100Hz |                       81-83                       |                      80-83                      |
| 600LEDs<br>Refresh rate/continues output=60Hz  |                       39-40                       |                      41-42                      |
| 900LEDs<br>Refresh rate/continues output=40Hz  |                       21-26                       |                      26-28                      |

## Comparing v6.1 and v8 version (single segment) refresh rate using generic ESP32 with CH340C

| LED strip / Device                             | ESP32 (CH340C)<br>HyperSerialESP32 v6.1 | ESP32 (CH340C)<br>HyperSerialESP32 v8 |
|------------------------------------------------|-----------------------------------------|---------------------------------------|
| 300LEDs<br>Refresh rate/continues output=100Hz |                  72-78                  |                 81-83                 |
| 600LEDs<br>Refresh rate/continues output=60Hz  |                  33-38                  |                 39-42                 |
| 900LEDs<br>Refresh rate/continues output=40Hz  |                  21-25                  |                 26-28                 |

## ESP32-S2 lolin mini performance

| LED strip / Device                             | ESP32-S2 lolin mini<br>HyperSerialESP32 v8 |
|------------------------------------------------|--------------------------------------------|
| 300LEDs<br>Refresh rate/continues output=100Hz |                    80-84                   |
| 600LEDs<br>Refresh rate/continues output=60Hz  |                     42                     |
| 900LEDs<br>Refresh rate/continues output=40Hz  |                    27-28                   |
  
# Disclaimer
  
You use it on your own risk.  
Don't touch these firmwares if you don't know how to put the device in the programming mode if something goes wrong.  
As per the MIT license, I assume no liability for any damage to you or any other person or equipment.  
