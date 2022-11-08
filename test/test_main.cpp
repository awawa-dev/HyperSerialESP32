/* test_main.cpp
*
*  MIT License
*
*  Copyright (c) 2022 awawa-dev
*
*  https://github.com/awawa-dev/HyperSerialESP32
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in all
*  copies or substantial portions of the Software.

*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*  SOFTWARE.
 */

#include <Arduino.h>
#include <unity.h>
#include "calibration.h"

// old calibration params
float whiteLimit;
uint8_t rCorrection;
uint8_t gCorrection;
uint8_t bCorrection;

uint8_t wChannel[256];
uint8_t rChannel[256];
uint8_t gChannel[256];
uint8_t bChannel[256];

// old calibration procedure
void oldCalibration()
{
	for (uint32_t i = 0; i < 256; i++)
	{
		// color calibration
		float red = rCorrection * i;   // adjust red
		float green = gCorrection * i; // adjust green
		float blue = bCorrection * i;  // adjust blue

		wChannel[i] = (uint8_t)round(std::min(whiteLimit * i, 255.0f));
		rChannel[i] = (uint8_t)round(std::min(red / 0xFF, 255.0f));
		gChannel[i] = (uint8_t)round(std::min(green / 0xFF, 255.0f));
		bChannel[i] = (uint8_t)round(std::min(blue / 0xFF, 255.0f));
	}
}

void compareLut()
{
	for (uint32_t i = 0; i < 256; i++)
	{
		int w = std::abs(int(wChannel[i]) - int(channelCorrection.white[i]));
		int r = std::abs(int(rChannel[i]) - int(channelCorrection.red[i]));
		int g = std::abs(int(gChannel[i]) - int(channelCorrection.green[i]));
		int b = std::abs(int(bChannel[i]) - int(channelCorrection.blue[i]));
		TEST_ASSERT_LESS_THAN(1, (int)(std::max(w, std::max(r, std::max(g, b)))));
	}
}

void testCalibration()
{
	// cold white old calibration (full range)
	whiteLimit = 1.0f;
	rCorrection = 0xA0;
	gCorrection = 0xA0;
	bCorrection = 0xA0;
	oldCalibration();
	// new procedure
	calibrationConfig.setParams(255, 0xA0, 0xA0, 0xA0);
	calibrationConfig.prepareCalibration();
	compareLut();

	// neutral white old calibration (full range)
	whiteLimit = 1.0f;
	rCorrection = 0xB0;
	gCorrection = 0xB0;
	bCorrection = 0x70;
	oldCalibration();
	// new procedure
	calibrationConfig.setParams(255, 0xB0, 0xB0, 0x70);
	calibrationConfig.prepareCalibration();
	compareLut();

	// cold white old calibration (medium range)
	whiteLimit = 0.5019607843137255f;
	rCorrection = 0xA0;
	gCorrection = 0xA0;
	bCorrection = 0xA0;
	oldCalibration();
	// new procedure
	calibrationConfig.setParams(128, 0xA0, 0xA0, 0xA0);
	calibrationConfig.prepareCalibration();
	compareLut();

	// neutral white old calibration (medium range)
	whiteLimit = 0.5019607843137255f;
	rCorrection = 0xB0;
	gCorrection = 0xB0;
	bCorrection = 0x70;
	oldCalibration();
	// new procedure
	calibrationConfig.setParams(128, 0xB0, 0xB0, 0x70);
	calibrationConfig.prepareCalibration();
	compareLut();
}

void setup()
{
	delay(1000);
	UNITY_BEGIN();
	RUN_TEST(testCalibration);
	UNITY_END();
}

void loop()
{
}
