/* powercontrol.h
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

#ifndef POWERCONTROL_H
#define POWERCONTROL_H

#if LED_POWER_INVERT
	#define SET_RELAY_HIGH() powerOff()
	#define SET_RELAY_LOW() powerOn()
#else
	#define SET_RELAY_HIGH() powerOn()
	#define SET_RELAY_LOW() powerOff()
#endif

/**
 * @brief Contains logic for turning on and off the power to leds using external relay
 *
 */
class
{
	// timeout after which the leds will be turned off if no reset is applied
	const unsigned long POWER_OFF_PERIOD = 5 * 1000;

	// last timestamp power off timer got reset
	volatile unsigned long lastPowerOffResetTimestamp = 0;

	// caching the PIN state to avoid unnecessary calls to the GPIO register
	volatile int currentPowerPinMode = LOW;

	public:
		void init()
		{
			pinMode(LED_POWER_PIN, OUTPUT);
			lastPowerOffResetTimestamp = millis();
			powerOff();
		}

		inline void SET_RELAY_HIGH()
		{
			if (currentPowerPinMode != HIGH)
			{
				currentPowerPinMode = HIGH;
				digitalWrite(LED_POWER_PIN, currentPowerPinMode);
			}
		}

		inline void SET_RELAY_LOW()
		{
			if (currentPowerPinMode != LOW)
			{
				currentPowerPinMode = LOW;
				digitalWrite(LED_POWER_PIN, currentPowerPinMode);
			}
		}

		void update(bool hasData)
		{
			if (hasData)
			{
				powerOn();
				lastPowerOffResetTimestamp = millis();
			}
			else if (millis() - lastPowerOffResetTimestamp > POWER_OFF_PERIOD)
			{
				powerOff();
			}
		}

} powerControl;


#endif
