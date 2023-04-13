# HyperSerialESP32
  
Exposes USB high speed serial port at 2Mb baud for driving led strip using NeoPixelBus library. It's intended to replace slow Arduino solutions (level shifter 3.3V to 5v may be required). **Data integrity check (Fletcher's checksum) included in new 'Awa' protocol for HyperHDR. So no more random flashing caused by serial transmission errors.** That option must be checked in HyperHDR to make system works as on the screen below.  
  
**Make sure that your serial chip on the ESP32 can handle 2Mb speed: for example CP2102 can't as its max speed is 1Mb and you can compile a version for that speed but I think that's not the point, cheap CH340G can do it without any problems. CH9102x also should work for you, even at 4Mb speed.**  
  
There is also my fork named HyperSerialWLED available with the support for the AWA serial protocol at @2Mb speed for both ESP8266 and ESP32: https://github.com/awawa-dev/HyperSerialWLED Can't guarantee it will work as stable as HyperSerialESP32 because WLED has a lot of other things to do in the backgrounds (ex. handling Wifi) and timing control for the serial port could be at danger for larger number of LEDs. But you don't need to abandon all the benefits offered by the WLED which can be a big advantage for some users.  
  
RGB to RGBW conversion is calibrated for the neutral white channel BTF SK6812 but it can be easily changed (for cool and warm temperature) in the code. Search for "color calibration". In HyperHDR use "1.5" gamma for red, blue and green for best effect in the "Image Processing" tab.


| LED strip / Device             | HyperSerialESP32 |
|--------------------------------|:----------------:|
| SK6812 cold white              |       yes        |
| SK6812 neutral white           |       yes        |
| WS281x                         |       yes        |
| SPI (APA102, SK9812, HD107...) |       yes        |

# Example of supported boards

**ESP32 MH-ET Live (CP2104 or CH9102x: 4Mb speed) and ESP32-S2 Lolin mini (CDC: 5Mb speed)**  
<p align="center">
<img src="https://user-images.githubusercontent.com/69086569/207587620-1c4c53c8-426c-486e-a6d9-d429fd1b050d.png" /><img src="https://user-images.githubusercontent.com/69086569/207587635-b7816329-0e29-47ee-a75a-bc6c41cdc51f.png" />
</p>

# Data integrity check
  
Why the data integrity check was introduced which causes incompatibility with other software? Because at 2Mb speed many chip-makers allow few percent error in the transmission. And we do not want to have any distracting flashes. Broken frames are abandon without showing them. At 100Hz for 250 leds approximately 1-5% of the frames are broken.
  
# Flashing

There are two versions of the firmware. The 'factory' and the 'base' one. Factory firmware should be flashed to offset 0x0, base firmware to offset 0x10000.

**ESP32-S2 Lolin mini:**

Requires using `esptool.py` to flash the firmware e.g.  

 - `esptool.py write_flash 0x10000 firmware_esp32_s2_mini_SK6812_RGBW_COLD.bin` or
 - `esptool.py write_flash 0x0 firmware_esp32_s2_mini_SK6812_RGBW_COLD.factory.bin`

Troubleshooting: ESP32-S2 Lolin mini recovery procedure if the board is not detected or is malfunctioning.  
1. Put the board into dfu mode using board buttons: press `Rst` + `0` buttons, then release `Rst`, next release `0`  
Do not reset or disconnect the board until the end of the recovery procedure.
2. Execute `esptool.py erase_flash`  
3. Flash 'factory' version of the firmware e.g.  
`esptool.py write_flash 0x0 firmware_esp32_s2_mini_SK6812_RGBW_COLD.factory.bin`  
4. Reset the board manually with the `Rst` button. The board should be detected as a COM port in the system.

**Generic ESP32:**

Recommend to use [esphome-flasher](https://github.com/esphome/esphome-flasher/releases)  

For **RGBW LED strip** like RGBW SK6812 NEUTRAL white choose: *firmware_esp32_SK6812_RGBW_NEUTRAL.bin*  
  
For **RGBW LED strip** like RGBW SK6812 COLD white choose: *firmware_esp32_SK6812_RGBW_COLD.bin*  
  
For **RGB LED strip** like WS8212b or RGB SK6812 variant choose: *firmware_esp32_WS281x_RGB.bin*  
  
For **SPI driven RGB LED strip** APA102: *firmware_esp32_SPI_APA102_SK9822_HD107.bin*, WS8201: *firmware_esp32_SPI_WS2801.bin*  
  
If you want to disable your first LED because it's used as a sacrificial level shifter, please use [HyperHDR v19](https://github.com/awawa-dev/HyperHDR/pull/379)  

For the RGBW firmware the white channel is automatically calculated and R,G,B channels are corrected.  
  
# Usage in HyperHDR

**In HyperHDR `Image Processing→Smoothing→Update frequency` you should do not exceed the maximum capacity of the device. Read more here: [how to get statistics](https://github.com/awawa-dev/HyperHDR/wiki/HyperSerial)**

To test the maximum performance in HyperHDR, enable `Image Processing→Smoothing→Continuous output`, set a high value for `Update Frequency` in the same tab and set any color in the `Remote Control` tab as the active effect. Get the statistics and optionally adjust `Update Frequency`. After testing, you need to disable `Continuous output`and set `Update frequency`" according to your results.
  
Configuring HyperHDR v19beta2 or above.
- set `Refresh time` to zero
- set `Baudrate` to 2000000
- enabled `HyperHDR's AWA protocol`.  

Enabling `White channel calibration` is optional, if you want to fine tune the white channel balance of your sk6812 RGBW LED strip.  
`ESP8266/ESP32 handshake` could help you to properly initialize the ESP device and enables statistics output to the logs (you must stop the LED device first to get them).  

![obraz](https://user-images.githubusercontent.com/69086569/207109594-0493fe58-3530-46bb-a0a3-31a110475ed6.png)

   
# Compiling
  
Currently we use PlatformIO to compile the project. Install [Visual Studio Code](https://code.visualstudio.com/) and add [PlatformIO plugin](https://platformio.org/).
This environment will take care of everything and compile the firmware for you. Low-level LED strip support is provided by my highly optimizated (pre-fill I2S DMA modes, turbo I2S parallel mode for up to 2 segments etc) version of Neopixelbus library: [link](https://github.com/awawa-dev/NeoPixelBus).

But there is also an alternative and an easier way. Just fork the project and enable its Github Action. Use the online editor to make changes to the ```platformio.ini``` file, for example change default pin-outs/speed or enable multi-segments support, and save it. Github Action will compile new firmware automatically in the Artifacts archive. It has never been so easy!

Tutorial: https://github.com/awawa-dev/HyperSerialESP32/wiki
  
# Pinout
  
**ESP32:**  
**LED output (non-SPI):** GPIO 2  
**LED output (SPI):** GPIO 4 for Clock, GPIO 2 for Data  


# Multi-Segment Wiring

Using parallel multi-segment allows you to double your Neopixel (e.g. sk6812 RGBW) LED strip refresh rate by dividing it into two smaller equal parts. Both smaller segments are perfectly in sync so you don't need to worry about it. Proposed example of building a multisegment:
- Divide a long or dense strip of LEDs into 2 smaller equal parts. So `SECOND_SEGMENT_START_INDEX` in the HyperSerialESP32 firmware is the total number of LEDs divided by 2.
- Build your first segment traditional way e.g. clockwise, so it starts somewhere in middle of the bottom of frame/TV and ends in the middle of the top of frame/TV
- Start the second segment in the opposite direction to the first one e.g. counterclockwise (`SECOND_SEGMENT_REVERSED` option in the HyperSerialESP32 firmware configuration must be enabled). So it starts somewhere in the middle of the bottom of the frame/TV and ends in the middle of the top of the TV/frame.  Both segments could be optionally connected if possible at the top but only 5v and ground ( NOT the data line).
- The data line starts for both segments somewhere in the middle of the bottom of the TV/frame (where each of the LED strips starts)
- Configuration in HyperHDR does not change! It's should be configured as one, single continues segment. All is done in HyperSerialESP32 firmware transparently and does not affect LED strip configuration in HyperHDR.

You also must configure data pin (and clock pin for SPI LEDs) in the `platformio.ini`. Review the comments at the top of the file:
* `SECOND_SEGMENT_DATA_PIN` - These is data pin for your second strip
* `SECOND_SEGMENT_CLOCK_PIN` - These is clock pin for your second strip (SPI LEDs only, not for sk6812/ws2812b etc)

You add these to your board's config. Be sure to put `-D` in front of each setting. 

Examples of final build_flags for 288 LEDs divided into 2 equal segments in the `platformio.ini`:
```
[env:SK6812_RGBW_COLD]
build_flags = -DNEOPIXEL_RGBW -DCOLD_WHITE -DDATA_PIN=2 ${env.build_flags} -DSECOND_SEGMENT_START_INDEX=144 -DSECOND_SEGMENT_DATA_PIN=4 -DSECOND_SEGMENT_REVERSED
...
[env:WS281x_RGB]
build_flags = -DNEOPIXEL_RGB -DDATA_PIN=2 ${env.build_flags} -DSECOND_SEGMENT_START_INDEX=144 -DSECOND_SEGMENT_DATA_PIN=4 -DSECOND_SEGMENT_REVERSED
...
```
Implementation example:
- The diagram of the board for WS2812b/SK6812 including ESP32 and the SN74AHCT125N 74AHCT125 [level shifter](https://github.com/awawa-dev/HyperHDR/wiki/Level-Shifter).

![HyperSPI](https://user-images.githubusercontent.com/85223482/222923979-f344349a-1f8b-4195-94ca-51721923359e.png)

# Some benchmark results

ESP32 MH-ET LIVE mini is capable of 4Mb serial port speed and ESP32-S2 lolin mini is capable of 5Mb. But to give equal chances for a single-segment mode all models were tested using the default speed of 2Mb which should saturate Neopixel data line. Parallel multi-segment mode uses the highest option available because communication performance is critical here.

**Parallel multi-segments can double your large sk6812/ws2812b setup refresh rate for free. All you need is to properly project & construct the LED strip and use HyperSerialESP32 v9. Parallel communication provides perfect synchronization between Neopixel segments.**

| Parallel multi-segment mode / Device                           | ESP32<br> MH-ET LIVE mini @ 4Mb speed |  ESP32-S2<br> Lolin mini @ 5Mb speed   |
|---------------------------------------------------------------------------------------|--------------------------|------------------------------|
| 300LEDs RGBW<br>Refresh rate/continues output=100Hz<br>SECOND_SEGMENT_START_INDEX=150 |            100           |               100            |
| 600LEDs RGBW<br>Refresh rate/continues output=83Hz <br>SECOND_SEGMENT_START_INDEX=300 |             83           |                83            |
| 900LEDs RGBW<br>Refresh rate/continues output=55Hz <br>SECOND_SEGMENT_START_INDEX=450 |             55           |                55            |

## ESP32 MH-ET LIVE mini 

| Single RGBW LED strip / Device                      | ESP32<br>MH-ET LIVE mini<br>HyperSerialESP32 v9 |
|-----------------------------------------------------|-------------------------------------------------|
| 300LEDs RGBW<br>Refresh rate/continues output=83Hz  |                        83                       |
| 600LEDs RGBW<br>Refresh rate/continues output=42Hz  |                        42                       |
| 900LEDs RGBW<br>Refresh rate/continues output=28Hz  |                        28                       |

## Generic ESP32 with CH340C

| Single RGBW LED strip / Device                      | ESP32 (CH340C)<br>HyperSerialESP32 v9 |
|-----------------------------------------------------|---------------------------------------|
| 300LEDs RGBW<br>Refresh rate/continues output=83Hz  |                   83                  |
| 600LEDs RGBW<br>Refresh rate/continues output=42Hz  |                   42                  |
| 900LEDs RGBW<br>Refresh rate/continues output=28Hz  |                   28                  |

## ESP32-S2 lolin mini performance

| Single RGBW LED strip / Device                      | ESP32-S2 lolin mini<br>HyperSerialESP32 v9 |
|-----------------------------------------------------|---------------------|
| 300LEDs RGBW<br>Refresh rate/continues output=83Hz  |          83         |
| 600LEDs RGBW<br>Refresh rate/continues output=42Hz  |          42         |
| 900LEDs RGBW<br>Refresh rate/continues output=28Hz  |          28         |
  
