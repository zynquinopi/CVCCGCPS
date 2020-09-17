#include <SPIFFS.h>	
#include <TFT_eSPI.h>
#include <SPI.h>
#include "kuma1.h"
#include "kuma2.h"
#include "kuma3.h"
#include "kuma4.h"
#include "kumaH1.h"
#include "kumaH2.h"

const uint16_t bmpWidth = 320;
const uint16_t bmpHeight = 240;

TFT_eSPI LCD = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&LCD);

void drawbmp(String wrfile);
uint16_t read16(fs::File &f);
uint32_t read32(fs::File &f);

File bmpFS;


void setup(void) {
  Serial.begin(115200);
  Serial.println("start");
  LCD.init();
  LCD.setRotation(1);
  LCD.fillScreen(TFT_BLACK);  

  img.setColorDepth(8);

  img.createSprite(bmpWidth, bmpHeight);
  img.setSwapBytes(true);
  img.fillSprite(TFT_BLACK);
  Serial.println("setup ok");
}

void loop() {
  Serial.println("Command1");
  // img.pushImage(0, 0, bmpWidth, bmpHeight, kuma4);
  drawbmp("/kuma1.bmp") ;
  // img.pushImage(0, 0, kumaH1Width, kumaH1Height, kumaH1);
  // img.pushImage(0, 120, kumaH2Width, kumaH2Height, kumaH2);
  img.pushSprite(0, 0);

  // delay(100);

  Serial.println("Command2");
  // img.pushImage(0, 0, pngWidth, pngHeight, kuma1);
  drawbmp("/kuma2.bmp") ;
  img.pushSprite(0, 0);

  // delay(100);

  Serial.println("Command3");
  // img.pushImage(0, 0, pngWidth, pngHeight, kuma2); 
  drawbmp("/kuma3.bmp") ;
  img.pushSprite(0, 0);

  // delay(100);

  Serial.println("Command4");
  img.pushImage(0, 0, pngWidth, pngHeight, kuma3); 
  // drawbmp("/kuma16_4.bmp") ;
  img.pushSprite(0, 0);

  // delay(100);
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

  Serial.println("xxxxx0");

  //wrfile = "/kuma24_1.bmp";

  bmpFS = SPIFFS.open(wrfile.c_str(), "r");// ⑩ファイルを読み込みモードで開く

  Serial.println("xxxxx1");


  if (!bmpFS) {
    Serial.print("File not found");
    return;
  }

  Serial.println("xxxxx2");

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  // uint8_t  r, g, b;

  // uint32_t startTime = millis();

  uint16_t x = 0;
  uint16_t y = 0;

  Serial.println("xxxx");

  if (read16(bmpFS) == 0x4D42) {
    Serial.println("0x4D42");
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

    if ((check == 1) && (check1 == 16) && (check2 == 3)) {
      y += h - 1;

      LCD.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t read_block = 10;

      // uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      // uint8_t lineBuffer[w * 3 + padding];
      // Serial.print("padding:");
      // Serial.println(padding);

      // uint8_t line[bmpWidth / 2 * bmpHeight * 2];
      uint8_t line[w * read_block * 2];
      Serial.print("line:");
      Serial.println(sizeof(line));
      // Serial.println(sizeof(lineBuffer));
      // uint16_t* lptr = (uint16_t*)line;

      // for (row = 0; row < h; row++) {
      //   bmpFS.read(lineBuffer, sizeof(lineBuffer));
      //   uint8_t*  bptr = lineBuffer;
      //   uint16_t* tptr = (uint16_t*)lineBuffer;
      //   // Convert 24 to 16 bit colours
      //   for (col = 0; col < w; col++) {
      //     b = *bptr++;
      //     g = *bptr++;
      //     r = *bptr++;
      //     *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
      //     // *lptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
      //   }

      //   // Push the pixel row to screen, pushImage will crop the line if needed
      //   // y is decremented as the BMP image is drawn bottom up
      //   // LCD.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
      //   // Serial.println("xxxxxxxxx");
      //   // Serial.println(sizeof((uint16_t*)lineBuffer));
      //   // img.pushImage(0, 0, bmpWidth, bmpHeight, (uint16_t*)lineBuffer); 

      // }

      for (row = 0; row < h / read_block; row++) {
        bmpFS.read(line, sizeof(line));
        img.pushImage(0, row * read_block, bmpWidth, read_block, (uint16_t*)line);
      }

      // bmpFS.read(line, sizeof(line));
      // img.pushImage(0, 0, bmpWidth, bmpHeight, (uint16_t*)line);


      // Serial.print("Loaded in "); Serial.print(millis() - startTime);
      // Serial.println(" ms");
    }
    else Serial.println("BMP format not recognized.");
  }
  bmpFS.close();

}




