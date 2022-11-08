/* calibration.h
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

#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdint.h>
#include <algorithm>

#define ROUND_DIVIDE(numer, denom) (((numer) + (denom) / 2) / (denom))

struct {
		uint8_t white[256];
		uint8_t red[256];
		uint8_t green[256];
		uint8_t blue[256];
} channelCorrection;

struct {
	uint8_t gain = 0xFF;
	#ifdef COLD_WHITE
		uint8_t red = 0xA0;   // adjust red   -> white in 0-0xFF range
		uint8_t green = 0xA0; // adjust green -> white in 0-0xFF range
		uint8_t blue = 0xA0;  // adjust blue  -> white in 0-0xFF range
	#else
		uint8_t red = 0xB0;   // adjust red   -> white in 0-0xFF range
		uint8_t green = 0xB0; // adjust green -> white in 0-0xFF range
		uint8_t blue = 0x70;  // adjust blue  -> white in 0-0xFF range
	#endif

	void setParams(uint8_t _gain, uint8_t _red, uint8_t _green, uint8_t _blue)
	{
		gain = _gain;
		red = _red;
		green = _green;
		blue = _blue;
	}

	void prepareCalibration()
	{	
		// prepare LUT calibration table, cold white is much better than "neutral" white
		for (uint32_t i = 0; i < 256; i++)
		{
			// color calibration
			uint32_t _gain = uint32_t(gain) * i;  // adjust gain
			uint32_t _red = red * i;     // adjust red
			uint32_t _green = green * i; // adjust green
			uint32_t _blue = blue * i;   // adjust blue

			channelCorrection.white[i] = (uint8_t)std::min(ROUND_DIVIDE(_gain, 0xFF), (uint32_t)0xFF);
			channelCorrection.red[i]   = (uint8_t)std::min(ROUND_DIVIDE(_red,  0xFF), (uint32_t)0xFF);
			channelCorrection.green[i] = (uint8_t)std::min(ROUND_DIVIDE(_green,0xFF), (uint32_t)0xFF);
			channelCorrection.blue[i]  = (uint8_t)std::min(ROUND_DIVIDE(_blue, 0xFF), (uint32_t)0xFF);
		}
	}
} calibrationConfig;
#endif

