/* calibration.h
*
*  MIT License
*
*  Copyright (c) 2025 awawa-dev
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

#ifdef NEOPIXEL_RGBW
	typedef RgbwColor ColorDefinition;
#else
	typedef RgbColor ColorDefinition;
#endif


#if !defined(CALIBRATION_H) && (defined(NEOPIXEL_RGBW) || defined(HYPERSERIAL_TESTING))
#define CALIBRATION_H

#include <stdint.h>
#include <algorithm>

#define ROUND_DIVIDE(numer, denom) (((numer) + (denom) / 2) / (denom))

struct
{
	uint8_t white[256];
	uint8_t red[256];
	uint8_t green[256];
	uint8_t blue[256];
} channelCorrection;

class CalibrationConfig
{
	// calibration parameters
	uint8_t gain = 0xFF;
	uint8_t red = 0xA0;
	uint8_t green = 0xA0;
	uint8_t blue = 0xA0;

	/**
	 * @brief Build the LUT table using provided parameters
	 *
	 */
	void prepareCalibration()
	{
		// prepare LUT calibration table, cold white is much better than "neutral" white
		for (uint32_t i = 0; i < 256; i++)
		{
			// color calibration
			uint32_t _gain = gain * i;   // adjust gain
			uint32_t _red = red * i;     // adjust red
			uint32_t _green = green * i; // adjust green
			uint32_t _blue = blue * i;   // adjust blue

			channelCorrection.white[i] = (uint8_t)std::min(ROUND_DIVIDE(_gain, 0xFF), (uint32_t)0xFF);
			channelCorrection.red[i]   = (uint8_t)std::min(ROUND_DIVIDE(_red,  0xFF), (uint32_t)0xFF);
			channelCorrection.green[i] = (uint8_t)std::min(ROUND_DIVIDE(_green,0xFF), (uint32_t)0xFF);
			channelCorrection.blue[i]  = (uint8_t)std::min(ROUND_DIVIDE(_blue, 0xFF), (uint32_t)0xFF);
		}
	}

	public:
		CalibrationConfig()
		{
			prepareCalibration();
		}

		/**
		 * @brief Compare base calibration settings
		 *
		 */
		bool compareCalibrationSettings(uint8_t _gain, uint8_t _red, uint8_t _green, uint8_t _blue)
		{
			return _gain == gain && _red == red && _green == green && _blue == blue;
		}

		/**
		 * @brief Set the parameters that define RGB to RGBW transformation
		 *
		 * @param _gain
		 * @param _red
		 * @param _green
		 * @param _blue
		 */
		void setParamsAndPrepareCalibration(uint8_t _gain, uint8_t _red, uint8_t _green, uint8_t _blue)
		{
			if (gain != _gain || red != _red || green != _green || blue != _blue)
			{
				gain = _gain;
				red = _red;
				green = _green;
				blue = _blue;
				prepareCalibration();
			}
		}

		/**
		 * @brief print RGBW calibration parameters when no data is received
		 *
		 */
		void printCalibration()
		{
			#ifdef SerialPort
				char output[128];
				snprintf(output, sizeof(output),"RGBW => Gain: %i/255, red: %i, green: %i, blue: %i\r\n", gain, red, green, blue);
				SerialPort.print(output);
			#endif
		}
} calibrationConfig;
#endif

