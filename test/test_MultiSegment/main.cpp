/* test_MultiSegment/main.cpp
*
*  MIT License
*
*  Copyright (c) 2023 awawa-dev
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

#define NO_GLOBAL_SERIAL
#define HYPERSERIAL_TESTING

#include <Arduino.h>
#include <NeoPixelBus.h>
#include <unity.h>
#include "calibration.h"

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
/////////////////////// AWA PROTOCOL CORRECTNESS TEST /////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

#define TEST_LEDS_NUMBER 1025
uint8_t _ledBuffer[TEST_LEDS_NUMBER * 3 + 6 + 8];

#define LED_DRIVER ProtocolTester
#define LED_DRIVER2 ProtocolTester
#define SECOND_SEGMENT_START_INDEX 513
#define SECOND_SEGMENT_CLOCK_PIN   100
#define SECOND_SEGMENT_DATA_PIN    101

/**
 * @brief Mockup Serial class to simulate the real communition
 *
 */

class SerialTester
{
		int frameSize = 0;
		int sent = 0;

	public:

		void createTestFrame(bool _white_channel_calibration, uint8_t _white_channel_limit = 0,
						uint8_t _white_channel_red = 0, uint8_t _white_channel_green = 0,
						uint8_t _white_channel_blue = 0)
		{
			_ledBuffer[0] = 'A';
			_ledBuffer[1] = 'w';
			_ledBuffer[2] = (_white_channel_calibration) ? 'A' : 'a';
			_ledBuffer[4] = (TEST_LEDS_NUMBER-1) & 0xff;
			_ledBuffer[3] = ((TEST_LEDS_NUMBER-1) >> 8) & 0xff;
			_ledBuffer[5] = _ledBuffer[3] ^ _ledBuffer[4] ^ 0x55;

			uint8_t* writer = &(_ledBuffer[6]);
			uint8_t* hasher = writer;

			for(int i=0; i < TEST_LEDS_NUMBER; i++)
			{
				*(writer++)=random(255);
				*(writer++)=random(255);
				*(writer++)=random(255);
			}


			if (_white_channel_calibration)
			{
				*(writer++) = _white_channel_limit;
				*(writer++) = _white_channel_red;
				*(writer++) = _white_channel_green;
				*(writer++) = _white_channel_blue;
			}

			uint16_t fletcher1 = 0, fletcher2 = 0, fletcherExt = 0;
			uint8_t position = 0;
			while (hasher < writer)
			{
				fletcherExt = (fletcherExt + (*(hasher) ^ (position++))) % 255;
				fletcher1 = (fletcher1 + *(hasher++)) % 255;
				fletcher2 = (fletcher2 + fletcher1) % 255;
			}
			*(writer++) = (uint8_t)fletcher1;
			*(writer++) = (uint8_t)fletcher2;
			*(writer++) = (uint8_t)((fletcherExt != 0x41) ? fletcherExt : 0xaa);

			frameSize = (int)(writer - _ledBuffer);
			sent = 0;
		}


		inline size_t write(const char * s)
		{
			return 0;
		}

		inline size_t print(unsigned char, int = DEC)
		{
			return 0;
		}

		inline size_t print(char*)
		{
			return 0;
		}		

		int available(void)
		{
			if (sent < frameSize)
			{
				return std::min(std::max((int)(random(64)), 1), frameSize - sent);
			}

			return 0;
		}

		int toSend(void)
		{
			return frameSize - sent;
		}

		int getFrameSize()
		{
			return frameSize;
		}

		size_t read(uint8_t *buffer, size_t size)
		{
			int max = std::min(frameSize - sent, (int)size);
			if (max > 0)
			{
				memcpy(buffer, &(_ledBuffer[sent]), max);
				sent += max;
				return max;
			}
			return 0;
		}

		void println(const String &s)
		{

		}
} SerialPort;


/**
 * @brief Mockup LED driver to verify correctness of the received LEDs color values
 *
 */

class ProtocolTester {
	int ledCount;
	int currentIndex;
	int lastCount;
	bool first;

	public:
		ProtocolTester(int _count, int _pin) : ProtocolTester(_count)
		{
			if (_pin == SECOND_SEGMENT_DATA_PIN)
				first = false;
		}

		ProtocolTester(int _count)
		{
			first = true;
			ledCount = _count;
			currentIndex = 0;
			lastCount = 0;
		}

		bool CanShow()
		{
			return true;
		}

		void Show(bool safe = true)
		{
			lastCount = currentIndex;
			currentIndex = 0;
		}

		void Begin()
		{

		}

		void Begin(int _pin1, int _pin2, int _pin3, int _pin4)
		{
			if (_pin1 == SECOND_SEGMENT_CLOCK_PIN)
				first = false;
		}

		int getLastCount()
		{
			return lastCount;
		}

		/**
		 * @brief Very important: verify LED color, compare it to the origin
		 *
		 * @param indexPixel
		 * @param color
		 */
		#ifdef NEOPIXEL_RGBW
			void SetPixelColor(uint16_t indexPixel, RgbwColor color)
			{
				TEST_ASSERT_EQUAL_INT_MESSAGE(currentIndex, indexPixel, "Unexpected LED index");
				TEST_ASSERT_LESS_THAN_MESSAGE(TEST_LEDS_NUMBER, indexPixel, "LED index out of scope");

				uint8_t *c = (first) ? &(_ledBuffer[6 + indexPixel * 3]) : &(_ledBuffer[6 + (indexPixel + SECOND_SEGMENT_START_INDEX) * 3]);
				uint8_t r = *(c++);
				uint8_t g = *(c++);
				uint8_t b = *(c++);

				uint8_t  w = min(channelCorrection.red[r],
								min(channelCorrection.green[g],
									channelCorrection.blue[b]));
				r -= channelCorrection.red[w];
				g -= channelCorrection.green[w];
				b -= channelCorrection.blue[w];
				w = channelCorrection.white[w];

				TEST_ASSERT_EQUAL_UINT8(r, color.R);
				TEST_ASSERT_EQUAL_UINT8(g, color.G);
				TEST_ASSERT_EQUAL_UINT8(b, color.B);
				TEST_ASSERT_EQUAL_UINT8(w, color.W);

				currentIndex = indexPixel + 1;
				lastCount = 0;
			}
		#else
			void SetPixelColor(uint16_t indexPixel, RgbColor color)
			{
				TEST_ASSERT_EQUAL_INT_MESSAGE(currentIndex, indexPixel, "Unexpected LED index");
				TEST_ASSERT_LESS_THAN_MESSAGE(TEST_LEDS_NUMBER, indexPixel, "LED index out of scope");

				uint8_t *c = (first) ? &(_ledBuffer[6 + indexPixel * 3]) : &(_ledBuffer[6 + (indexPixel + SECOND_SEGMENT_START_INDEX) * 3]);
				uint8_t r = *(c++);
				uint8_t g = *(c++);
				uint8_t b = *(c++);

				TEST_ASSERT_EQUAL_UINT8(r, color.R);
				TEST_ASSERT_EQUAL_UINT8(g, color.G);
				TEST_ASSERT_EQUAL_UINT8(b, color.B);

				currentIndex = indexPixel + 1;
				lastCount = 0;
			}
		#endif
};

#include "main.h"



/**
 * @brief Send RGBW calibration data and verify it all (including proper colors rendering)
 *
 */
void MultiSegmentTest_SendRgbwCalibration()
{
	// set all calibration values to test
	SerialPort.createTestFrame(true, 10, 20, 30, 40);
	base.queueCurrent = 0;
	base.queueEnd = 0;
	statistics.update(0);

	while(SerialPort.toSend() > 0)
	{
		serialTaskHandler();
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, statistics.getGoodFrames(), "Unexpected initial stats value");
	processData();
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, statistics.getGoodFrames(), "Frame is not received");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SECOND_SEGMENT_START_INDEX, base.getLedStrip1()->getLastCount(), "Not all LEDs were set up (segment1)");
	TEST_ASSERT_EQUAL_INT_MESSAGE(TEST_LEDS_NUMBER - SECOND_SEGMENT_START_INDEX, base.getLedStrip2()->getLastCount(), "Not all LEDs were set up(segment2)");
	TEST_ASSERT_EQUAL_MESSAGE(true, calibrationConfig.compareCalibrationSettings(10,20,30,40), "Incorrect calibration result");

	// should not change if the frame doesnt contain calibration data
	SerialPort.createTestFrame(false);
	statistics.update(0);

	while(SerialPort.toSend() > 0)
	{
		serialTaskHandler();
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, statistics.getGoodFrames(), "Unexpected initial stats value");
	processData();
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, statistics.getGoodFrames(), "Frame is not received");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SECOND_SEGMENT_START_INDEX, base.getLedStrip1()->getLastCount(), "Not all LEDs were set up (segment1)");
	TEST_ASSERT_EQUAL_INT_MESSAGE(TEST_LEDS_NUMBER - SECOND_SEGMENT_START_INDEX, base.getLedStrip2()->getLastCount(), "Not all LEDs were set up(segment2)");
	TEST_ASSERT_EQUAL_MESSAGE(true, calibrationConfig.compareCalibrationSettings(10,20,30,40), "Incorrect calibration result");

	// last test
	SerialPort.createTestFrame(true, 255, 128, 128, 128);
	statistics.update(0);

	while(SerialPort.toSend() > 0)
	{
		serialTaskHandler();
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, statistics.getGoodFrames(), "Unexpected initial stats value");
	processData();
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, statistics.getGoodFrames(), "Frame is not received");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SECOND_SEGMENT_START_INDEX, base.getLedStrip1()->getLastCount(), "Not all LEDs were set up (segment1)");
	TEST_ASSERT_EQUAL_INT_MESSAGE(TEST_LEDS_NUMBER - SECOND_SEGMENT_START_INDEX, base.getLedStrip2()->getLastCount(), "Not all LEDs were set up(segment2)");
	TEST_ASSERT_EQUAL_MESSAGE(true, calibrationConfig.compareCalibrationSettings(255,128,128,128), "Incorrect calibration result");
}

/**
 * @brief Send 100 RGB/RGBW frames and verify it all (including proper colors rendering)
 *
 */
void MultiSegmentTest_Send100Frames()
{
	base.queueCurrent = 0;
	base.queueEnd = 0;

	for(int i = 0; i < 100; i++)
	{
		SerialPort.createTestFrame(false);
		statistics.update(0);

		while(SerialPort.toSend() > 0)
		{
			serialTaskHandler();
		}
		TEST_ASSERT_EQUAL_INT_MESSAGE(0, statistics.getGoodFrames(), "Unexpected initial stats value");
		processData();
		TEST_ASSERT_EQUAL_INT_MESSAGE(1, statistics.getGoodFrames(), "Frame is not received");
		TEST_ASSERT_EQUAL_INT_MESSAGE(SECOND_SEGMENT_START_INDEX, base.getLedStrip1()->getLastCount(), "Not all LEDs were set up (segment1)");
		TEST_ASSERT_EQUAL_INT_MESSAGE(TEST_LEDS_NUMBER - SECOND_SEGMENT_START_INDEX, base.getLedStrip2()->getLastCount(), "Not all LEDs were set up(segment2)");
	}
}

/**
 * @brief Send 200 RGB/RGBW valid/invalid frames and verify it all (including proper colors rendering)
 *
 */
void MultiSegmentTest_Send200UncertainFrames()
{
	base.queueCurrent = 0;
	base.queueEnd = 0;

	for(int i = 0; i < 200; i++)
	{
		SerialPort.createTestFrame(false);
		statistics.update(0);

		bool damaged = (random(255) % 2) == 0;
		int index;
		uint8_t backup;
		if (damaged)
		{
			index = random(SerialPort.getFrameSize());
			backup = _ledBuffer[index];
			_ledBuffer[index] =  backup ^ 0xff;
		}

		while(SerialPort.toSend() > 0)
		{
			serialTaskHandler();
		}
		TEST_ASSERT_EQUAL_INT_MESSAGE(0, statistics.getGoodFrames(), "Unexpected initial stats value");
		processData();
		if (damaged)
		{
			char buffer[128];
  			snprintf(buffer, sizeof(buffer), "Damaged frame was received: [%d]=>%d", index, backup);
			TEST_ASSERT_EQUAL_INT_MESSAGE(0, statistics.getGoodFrames(), buffer);
			base.getLedStrip1()->Show();
			base.getLedStrip2()->Show();
			frameState.setState(AwaProtocol::HEADER_A);
		}
		else
		{
			TEST_ASSERT_EQUAL_INT_MESSAGE(1, statistics.getGoodFrames(), "Frame is not received");
			TEST_ASSERT_EQUAL_INT_MESSAGE(SECOND_SEGMENT_START_INDEX, base.getLedStrip1()->getLastCount(), "Not all LEDs were set up (segment1)");
			TEST_ASSERT_EQUAL_INT_MESSAGE(TEST_LEDS_NUMBER - SECOND_SEGMENT_START_INDEX, base.getLedStrip2()->getLastCount(), "Not all LEDs were set up(segment2)");
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////// UNIT TEST ROUTINES //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void setup()
{
	delay(1500);
	randomSeed(analogRead(0));
	UNITY_BEGIN();
	#ifdef NEOPIXEL_RGBW
		RUN_TEST(MultiSegmentTest_SendRgbwCalibration);
	#endif
	RUN_TEST(MultiSegmentTest_Send100Frames);
	RUN_TEST(MultiSegmentTest_Send200UncertainFrames);
	UNITY_END();
}

void loop()
{
}
