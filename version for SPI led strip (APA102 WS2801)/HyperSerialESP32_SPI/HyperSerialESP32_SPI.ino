#include <NeoPixelBus.h>
////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////          CONFIG SECTION STARTS               /////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool      skipFirstLed = false;    // if set the first led in the strip will be set to black (for level shifters)
int       serialSpeed = 2000000;   // serial port speed
                                   // data output on the hardware SPI pins: MOSI for data, and MSCLK or CLK for clock
#define   is_WS2801                // for WS2801 use NeoPixelBus<NeoRgbFeature, NeoWs2801Spi2MhzMethod>, otherwise comment it with //, choose NeoxxxFeature that fits you
#define   DATA_PIN   2             // PIN: data output for LED strip
#define   CLOCK_PIN  0             // PIN: clock output for LED strip

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////            CONFIG SECTION ENDS               /////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int       pixelCount  = 0;        // This is dynamic, don't change it

#ifdef is_WS2801
      NeoPixelBus<NeoRbgFeature, NeoWs2801Spi2MhzMethod>* strip = NULL;
#else
      NeoPixelBus<DotStarBgrFeature, DotStarEsp32DmaHspiMethod>* strip = NULL;
#endif

void Init(int count)
{
    if (strip != NULL)
        delete strip;
        
    pixelCount = count;
    #ifdef is_WS2801
          strip = new NeoPixelBus<NeoRbgFeature, NeoWs2801Spi2MhzMethod>(pixelCount);
    #else
          strip = new NeoPixelBus<DotStarBgrFeature, DotStarEsp32DmaHspiMethod>(pixelCount);
    #endif
    strip->Begin(CLOCK_PIN, 12, DATA_PIN, 15);
}

enum class AwaProtocol {
    HEADER_A,
    HEADER_w,
    HEADER_a,
    HEADER_HI,
    HEADER_LO,
    HEADER_CRC,
    RED,
    GREEN,
    BLUE,
    FLETCHER1,
    FLETCHER2
};

// static data buffer for the loop
#define MAX_BUFFER 2048
uint8_t     buffer[MAX_BUFFER];
AwaProtocol state = AwaProtocol::HEADER_A;
uint8_t     CRC = 0;
uint16_t    count = 0;
uint16_t    currentPixel = 0;
uint16_t    fletcher1 = 0;
uint16_t    fletcher2 = 0;

RgbColor    inputColor;

// stats
unsigned long     stat_start  = 0;
uint16_t          stat_good = 0;
uint16_t          stat_frames  = 0;
uint16_t          stat_final_good = 0;
uint16_t          stat_final_frames  = 0;
bool              wantShow = false;

inline void ShowMe()
{
    if (wantShow == true && strip != NULL && strip->CanShow())
    {
        stat_good++;;
        wantShow = false;
        strip->Show();
    }  
}

void readSerialData()
{
    unsigned long curTime = millis();
    uint16_t bufferPointer = 0;
    uint16_t internalIndex = min(Serial.available(), MAX_BUFFER);

    if (internalIndex > 0)
        internalIndex = Serial.readBytes(buffer, internalIndex);

    // stats
    if (internalIndex > 0 && curTime - stat_start > 1000)
    {
       if (stat_frames > 0 && stat_frames >= stat_good )
       {
          stat_final_good = stat_good;
          stat_final_frames = stat_frames;
       }
       
       stat_start  = curTime;
       stat_good   = 0;
       stat_frames = 0;
    }
    else if (curTime - stat_start > 5000)
    {
       stat_start  = curTime;
       stat_good   = 0;
       stat_frames = 0;
       
       Serial.write("HyperSerialEsp8266 version 5 SPI.\r\nStatistics for the last full 1 second cycle.\r\n");
       Serial.write("Frames per second: ");
       Serial.print(stat_final_frames);     
       Serial.write("\r\nGood frames: ");
       Serial.print(stat_final_good); 
       Serial.write("\r\nBad frames:  ");
       Serial.print(stat_final_frames - stat_final_good); 
       Serial.write("\r\n-------------------------\r\n");
    }

    if (state == AwaProtocol::HEADER_A)
        ShowMe();

    while (bufferPointer < internalIndex)
    {
        byte input = buffer[bufferPointer++];
        switch (state)
        {
        case AwaProtocol::HEADER_A:
            if (input == 'A') state = AwaProtocol::HEADER_w;
            break;

        case AwaProtocol::HEADER_w:
            if (input == 'w') state = AwaProtocol::HEADER_a;
            else              state = AwaProtocol::HEADER_A;
            break;

        case AwaProtocol::HEADER_a:
            if (input == 'a') state = AwaProtocol::HEADER_HI;
            else              state = AwaProtocol::HEADER_A;
            break;

        case AwaProtocol::HEADER_HI:
            stat_frames++;
            currentPixel = 0;
            count = input * 0x100;
            CRC = input;
            fletcher1 = 0;
            fletcher2 = 0;
            state = AwaProtocol::HEADER_LO;
            break;

        case AwaProtocol::HEADER_LO:
            count += input;
            CRC = CRC ^ input ^ 0x55;
            state = AwaProtocol::HEADER_CRC;
            break;

        case AwaProtocol::HEADER_CRC:
            if (CRC == input)
            {
                if (count+1 != pixelCount) Init(count+1);
                state = AwaProtocol::RED;
            }
            else
                state = AwaProtocol::HEADER_A;
            break;

        case AwaProtocol::RED:
            inputColor.R = input;
            fletcher1 = (fletcher1 + (uint16_t)input) % 255;
            fletcher2 = (fletcher2 + fletcher1) % 255;

            state = AwaProtocol::GREEN;
            break;

        case AwaProtocol::GREEN:
            inputColor.G = input;
            fletcher1 = (fletcher1 + (uint16_t)input) % 255;
            fletcher2 = (fletcher2 + fletcher1) % 255;

            state = AwaProtocol::BLUE;
            break;

        case AwaProtocol::BLUE:
            inputColor.B = input;
                    
            fletcher1 = (fletcher1 + (uint16_t)input) % 255;
            fletcher2 = (fletcher2 + fletcher1) % 255;

            if (currentPixel == 0 && skipFirstLed)
            {
                strip->SetPixelColor(currentPixel++, RgbColor(0, 0, 0));
            }
            else
                setStripPixel(currentPixel++, inputColor);

            if (count-- > 0) state = AwaProtocol::RED;
            else state = AwaProtocol::FLETCHER1;
            break;

        case AwaProtocol::FLETCHER1:
            if (input != fletcher1) state = AwaProtocol::HEADER_A;
            else state = AwaProtocol::FLETCHER2;
            break;

        case AwaProtocol::FLETCHER2:
            if (input == fletcher2) 
            {
                wantShow = true;
                ShowMe();            
            }
            state = AwaProtocol::HEADER_A;
            break;
        }
    }
}

inline void setStripPixel(uint16_t pix, RgbColor& inputColor)
{
    if (pix < pixelCount && strip != NULL)
    {
        strip->SetPixelColor(pix, inputColor);
    }
}

void setup()
{
    // Init serial port
    Serial.begin(serialSpeed);
    Serial.setTimeout(50);  
    Serial.setRxBufferSize(2048);
  
    // Display config
    Serial.write("\r\nWelcome!\r\nAwa driver 5 SPI.\r\n");    

    Serial.write("Color mode: RGB\r\n");

    if (skipFirstLed)
      Serial.write("First LED: disabled\r\n");
    else
      Serial.write("First LED: enabled\r\n");    
}

void loop()
{
    readSerialData();
}
