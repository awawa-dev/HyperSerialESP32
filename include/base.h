/* base.h
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

#ifndef BASE_H
#define BASE_H

#include "freertos/semphr.h"

#if defined(SECOND_SEGMENT_START_INDEX)
	#if !defined(SECOND_SEGMENT_DATA_PIN)
		#error "Please define SECOND_SEGMENT_DATA_PIN for second segment"
	#elif !defined(SECOND_SEGMENT_CLOCK_PIN) && !defined(NEOPIXEL_RGBW) && !defined(NEOPIXEL_RGB)
		#error "Please define SECOND_SEGMENT_CLOCK_PIN and SECOND_SEGMENT_DATA_PIN for second segment"
	#endif
#endif

class Base
{
	// LED strip number
	int ledsNumber = 0;
	// NeoPixelBusLibrary primary object
	LED_DRIVER* ledStrip1 = nullptr;
	// NeoPixelBusLibrary second object
	LED_DRIVER2* ledStrip2 = nullptr;
	// frame is set and ready to render
	bool readyToRender = false;

	public:
		// static data buffer for the loop
		uint8_t buffer[MAX_BUFFER + 1] = {0};
		// handle to tasks
		TaskHandle_t processDataHandle = nullptr;
		TaskHandle_t processSerialHandle = nullptr;
		// semaphore to synchronize them
		xSemaphoreHandle i2sXSemaphore;
		// current queue position
		volatile int queueCurrent = 0;
		// queue end position
		volatile int queueEnd = 0;

		inline int getLedsNumber()
		{
			return ledsNumber;
		}

		inline LED_DRIVER* getLedStrip1()
		{
			return ledStrip1;
		}

		inline LED_DRIVER2* getLedStrip2()
		{
			return ledStrip2;
		}

		void initLedStrip(int count)
		{
			if (ledStrip1 != nullptr)
			{
				delete ledStrip1;
				ledStrip1 = nullptr;
			}

			if (ledStrip2 != nullptr)
			{
				delete ledStrip2;
				ledStrip2 = nullptr;
			}

			ledsNumber = count;

			#if defined(SECOND_SEGMENT_START_INDEX)
				if (ledsNumber > SECOND_SEGMENT_START_INDEX)
				{
					#if defined(NEOPIXEL_RGBW) || defined(NEOPIXEL_RGB)
						ledStrip1 = new LED_DRIVER(SECOND_SEGMENT_START_INDEX, DATA_PIN);
						ledStrip1->Begin();
						ledStrip2 = new LED_DRIVER2(ledsNumber - SECOND_SEGMENT_START_INDEX, SECOND_SEGMENT_DATA_PIN);
						ledStrip2->Begin();
					#else
						ledStrip1 = new LED_DRIVER(SECOND_SEGMENT_START_INDEX);
						ledStrip1->Begin(CLOCK_PIN, 12, DATA_PIN, 15);
						ledStrip2 = new LED_DRIVER2(ledsNumber - SECOND_SEGMENT_START_INDEX);
						ledStrip2->Begin(SECOND_SEGMENT_CLOCK_PIN, 12, SECOND_SEGMENT_DATA_PIN, 15);
					#endif
				}
			#endif

			if (ledStrip1 == nullptr)
			{
				#if defined(NEOPIXEL_RGBW) || defined(NEOPIXEL_RGB)
					ledStrip1 = new LED_DRIVER(ledsNumber, DATA_PIN);
					ledStrip1->Begin();
				#else
					ledStrip1 = new LED_DRIVER(ledsNumber);
					ledStrip1->Begin(CLOCK_PIN, 12, DATA_PIN, 15);
				#endif
			}
		}

		/**
		 * @brief Check if there is already prepared frame to display
		 *
		 * @return true
		 * @return false
		 */
		inline bool hasLateFrameToRender()
		{
			return readyToRender;
		}

		inline void dropLateFrame()
		{
			readyToRender = false;
		}

		inline void renderLeds(bool newFrame)
		{
			if (newFrame)
				readyToRender = true;

			if (readyToRender &&
				(ledStrip1 != nullptr && ledStrip1->CanShow()) &&
				!(ledStrip2 != nullptr && !ledStrip2->CanShow()))
			{
				statistics.increaseShow();
				readyToRender = false;

				// display segments
				ledStrip1->Show(false);
				if (ledStrip2 != nullptr)
					ledStrip2->Show(false);
			}
		}

		inline bool setStripPixel(uint16_t pix, ColorDefinition &inputColor)
		{
			if (pix < ledsNumber)
			{
				#if defined(SECOND_SEGMENT_START_INDEX)
					if (pix < SECOND_SEGMENT_START_INDEX)
						ledStrip1->SetPixelColor(pix, inputColor);
					else
					{
						#if defined(SECOND_SEGMENT_REVERSED)
							ledStrip2->SetPixelColor(ledsNumber - pix - 1, inputColor);
						#else
							ledStrip2->SetPixelColor(pix - SECOND_SEGMENT_START_INDEX, inputColor);
						#endif
					}
				#else
					ledStrip1->SetPixelColor(pix, inputColor);
				#endif
			}

			return (pix + 1 < ledsNumber);
		}
} base;

#endif