#include <SPIFFS.h>	
#include <TFT_eSPI.h>
#include <SPI.h>
// #include "kuma1.h"
// #include "kuma2.h"
// #include "kuma3.h"

const uint16_t pngWidth = 320;
const uint16_t pngHeight = 240;

TFT_eSPI LCD = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&LCD);

void drawbmp(String wrfile);
uint16_t read16(fs::File &f);
uint32_t read32(fs::File &f);

File bmpFS;


void setup(void) {
  LCD.init();
  LCD.setRotation(1);
  LCD.fillScreen(TFT_BLACK);  

  img.setColorDepth(8);

  img.createSprite(pngWidth, pngHeight);
  img.setSwapBytes(true);
  img.fillSprite(TFT_BLACK);
}

void loop() {
  //img.pushImage(0, 0, pngWidth, pngHeight, kuma1);
  drawbmp("/kuma24_1.bmp") ;
  img.pushSprite(0, 0);

  delay(100);

  //img.pushImage(0, 0, pngWidth, pngHeight, kuma2);
  drawbmp("/kuma24_2.bmp") ;
  img.pushSprite(0, 0);

  delay(100);

  //img.pushImage(0, 0, pngWidth, pngHeight, kuma3); 
  drawbmp("/kuma24_3.bmp") ;
  img.pushSprite(0, 0);

  delay(100);

  //img.pushImage(0, 0, pngWidth, pngHeight, kuma3); 
  drawbmp("/kuma24_4.bmp") ;
  img.pushSprite(0, 0);

  delay(100);
}

uint16_t read16(fs::File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

void drawbmp(String wrfile){


  SPIFFS.begin();	// ③SPIFFS開始  
  //String wrfile;

  //wrfile = "/kuma24_1.bmp";

  bmpFS = SPIFFS.open(wrfile.c_str(), "r");// ⑩ファイルを読み込みモードで開く


  if (!bmpFS) {
    Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;

  // uint32_t startTime = millis();

  uint16_t x = 0;
  uint16_t y = 0;

  
  if (read16(bmpFS) == 0x4D42) {
    // Serial.println("0x4D42");
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    uint16_t check;
    check = read16(bmpFS);
    uint16_t check1;
    check1 = read16(bmpFS);
    uint32_t check2;
    check2 = read32(bmpFS);
    // Serial.println(check);
    // Serial.println(check1);
    // Serial.println(check2);

    if ((check == 1) && (check1 == 24) && (check2 == 0)) {
      y += h - 1;

      LCD.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++) {
        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
        // Convert 24 to 16 bit colours
        for (col = 0; col < w; col++) {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        // LCD.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
        img.pushImage(0, 0, pngWidth, pngHeight, (uint16_t*)lineBuffer); 

      }
      // Serial.print("Loaded in "); Serial.print(millis() - startTime);
      // Serial.println(" ms");
    }
    else Serial.println("BMP format not recognized.");
  }
  bmpFS.close();

}




