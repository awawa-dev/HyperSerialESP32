/* framestate.h
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

#ifndef FRAMESTATE_H
#define FRAMESTATE_H

/**
 * @brief my AWA frame protocol definition
 *
 */
enum class AwaProtocol
{
	HEADER_A,
	HEADER_w,
	HEADER_a,
	HEADER_HI,
	HEADER_LO,
	HEADER_CRC,
	VERSION2_GAIN,
	VERSION2_RED,
	VERSION2_GREEN,
	VERSION2_BLUE,
	RED,
	GREEN,
	BLUE,
	FLETCHER1,
	FLETCHER2,
	FLETCHER_EXT
};

/**
 * @brief Contains current state of the incoming frame
 *
 */
class
{
	volatile AwaProtocol state = AwaProtocol::HEADER_A;
	bool protocolVersion2 = false;
	uint8_t CRC = 0;
	uint16_t count = 0;
	uint16_t currentLed = 0;
	uint16_t fletcher1 = 0;
	uint16_t fletcher2 = 0;
	uint16_t fletcherExt = 0;
	uint8_t position = 0;

	public:
		ColorDefinition color;

		/**
		 * @brief Reset statistics for new frame
		 *
		 * @param input
		 */
		inline void init(byte input)
		{
			currentLed = 0;
			count = input * 0x100;
			CRC = input;
			fletcher1 = 0;
			fletcher2 = 0;
			fletcherExt = 0;
			position = 0;
			base.dropLateFrame();
		}

		/**
		 * @brief get computed CRC
		 *
		 * @return uint8_t
		 */
		inline uint8_t getCRC()
		{
			return CRC;
		}

		/**
		 * @brief Get the color count reported by the frame
		 *
		 * @return uint16_t
		 */
		inline uint16_t getCount()
		{
			return count;
		}

		/**
		 * @brief Get the Fletcher1 total sum
		 *
		 * @return uint16_t
		 */
		inline uint16_t getFletcher1()
		{
			return fletcher1;
		}

		/**
		 * @brief Get the Fletcher2 total sum
		 *
		 * @return uint16_t
		 */
		inline uint16_t getFletcher2()
		{
			return fletcher2;
		}

		/**
		 * @brief Get the FletcherExt total sum
		 *
		 * @return uint16_t
		 */
		inline uint16_t getFletcherExt()
		{
			return (fletcherExt != 0x41) ? fletcherExt : 0xaa;
		}

		/**
		 * @brief Get and increase the current Led index
		 *
		 * @return uint16_t
		 */
		inline uint16_t getCurrentLedIndex()
		{
			return currentLed++;
		}

		/**
		 * @brief Set if frame protocol version 2 (contains calibration data)
		 *
		 * @param newVer
		 */
		inline void setProtocolVersion2(bool newVer)
		{
			protocolVersion2 = newVer;
		}

		/**
		 * @brief Verify if frame protocol version 2 (contains calibration data)
		 *
		 * @return true
		 * @return false
		 */
		inline bool isProtocolVersion2()
		{
			return protocolVersion2;
		}

		/**
		 * @brief  Set new AWA frame state
		 *
		 * @param newState
		 */
		inline void setState(AwaProtocol newState)
		{
			state = newState;
		}

		/**
		 * @brief Get current AWA frame state
		 *
		 * @return AwaProtocol
		 */
		inline AwaProtocol getState()
		{
			return state;
		}

		/**
		 * @brief Update CRC based on current and previuos input
		 *
		 * @param input
		 */
		inline void computeCRC(byte input)
		{
			count += input;
			CRC = CRC ^ input ^ 0x55;
		}

		/**
		 * @brief Update Fletcher checksumn for incoming input
		 *
		 * @param input
		 */
		inline void addFletcher(byte input)
		{
			fletcher1 = (fletcher1 + (uint16_t)input) % 255;
			fletcher2 = (fletcher2 + fletcher1) % 255;
			fletcherExt = (fletcherExt + (input ^ (position++))) % 255;
		}

		/**
		 * @brief Check if the calibration data was updated and calculate new one
		 *
		 */
		inline void updateIncomingCalibration()
		{
			#ifdef NEOPIXEL_RGBW
				if (protocolVersion2)
				{
					calibrationConfig.setParamsAndPrepareCalibration(calibration.gain, calibration.red, calibration.green, calibration.blue);
				}
			#endif
		}


		#ifdef NEOPIXEL_RGBW
			/**
			* @brief Compute && correct the white channel
			*
			*/
			inline void rgb2rgbw()
			{
				color.W = min(channelCorrection.red[color.R],
								min(channelCorrection.green[color.G],
									channelCorrection.blue[color.B]));
				color.R -= channelCorrection.red[color.W];
				color.G -= channelCorrection.green[color.W];
				color.B -= channelCorrection.blue[color.W];
				color.W = channelCorrection.white[color.W];
			}
		#endif

		/**
		 * @brief Incoming calibration data
		 *
		 */
		struct
		{
			uint8_t gain = 0;
			uint8_t red = 0;
			uint8_t green = 0;
			uint8_t blue = 0;
		} calibration;

} frameState;

#endif