/* stats.h
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

#ifndef STATISTICS_H
#define STATISTICS_H

// statistics (stats sent only when there is no communication)
class
{
	unsigned long startTime = 0;
	uint16_t goodFrames = 0;
	uint16_t showFrames = 0;
	uint16_t totalFrames = 0;
	uint16_t finalGoodFrames = 0;
	uint16_t finalShowFrames = 0;
	uint16_t finalTotalFrames = 0;

	public:
		/**
		 * @brief Get the start time of the current period
		 *
		 * @return unsigned long
		 */
		inline unsigned long getStartTime()
		{
			return startTime;
		}

		/**
		 * @brief Detected new frame
		 *
		 */
		inline void increaseTotal()
		{
			totalFrames++;
		}

		/**
		 * @brief The frame is received and shown
		 *
		 */
		inline void increaseShow()
		{
			showFrames++;
		}

		/**
		 * @brief The frame is received correctly (not yet displayed)
		 *
		 */
		inline void increaseGood()
		{
			goodFrames++;
		}

		/**
		 * @brief Get number of correctly received frames
		 *
		 * @return uint16_t
		 */
		inline uint16_t getGoodFrames()
		{
			return goodFrames;
		}

		/**
		 * @brief Period restart, save current statistics ans send them later if there is no incoming communication
		 *
		 * @param currentTime
		 */
		void update(unsigned long currentTime)
		{
			if (totalFrames > 0)
			{
				finalShowFrames = showFrames;
				finalGoodFrames = std::min(goodFrames, totalFrames);
				finalTotalFrames = totalFrames;
			}

			startTime = currentTime;
			goodFrames = 0;
			totalFrames = 0;
			showFrames = 0;
		}

		/**
		 * @brief Print last saved statistics to the serial port
		 *
		 * @param curTime
		 * @param taskHandle
		 */
		void print(unsigned long curTime, TaskHandle_t taskHandle1, TaskHandle_t taskHandle2)
		{
			char output[128];

			startTime = curTime;
			goodFrames = 0;
			totalFrames = 0;
			showFrames = 0;

			snprintf(output, sizeof(output), "HyperHDR frames: %u (FPS), receiv.: %u, good: %u, incompl.: %u, mem1: %i, mem2: %i, heap: %i\r\n",
						finalShowFrames, finalTotalFrames,finalGoodFrames,(finalTotalFrames - finalGoodFrames),
						(taskHandle1 != nullptr) ? uxTaskGetStackHighWaterMark(taskHandle1) : 0,
						(taskHandle2 != nullptr) ? uxTaskGetStackHighWaterMark(taskHandle2) : 0,
						ESP.getFreeHeap());
			SerialPort.print(output);

			#if defined(NEOPIXEL_RGBW)
				calibrationConfig.printCalibration();
			#endif
		}

		/**
		 * @brief Reset statistics
		 *
		 */
		void reset(unsigned long currentTime)
		{
			startTime = currentTime;

			finalShowFrames = 0;
			finalGoodFrames = 0;
			finalTotalFrames = 0;

			goodFrames = 0;
			totalFrames = 0;
			showFrames = 0;
		}

		void lightReset(unsigned long curTime, bool hasData)
		{
			if (hasData)
				startTime = curTime;

			goodFrames = 0;
			totalFrames = 0;
			showFrames = 0;
		}

} statistics;

#endif