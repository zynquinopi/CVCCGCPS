#include <TFT_eSPI.h>
#include <SPI.h>
#include "kuma1.h"
#include "kuma2.h"
#include "kuma3.h"

TFT_eSPI LCD = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&LCD);

void setup(void) {
  LCD.init();
  LCD.setRotation(1);
  LCD.fillScreen(TFT_BLACK);  

  img.createSprite(pngWidth, pngHeight);
  img.setSwapBytes(true);
  img.fillSprite(TFT_BLACK);
}

void loop() {
  img.pushImage(0, 0, pngWidth, pngHeight, kuma1); 
  img.pushSprite(32, 24);  

  img.pushImage(0, 0, pngWidth, pngHeight, kuma2); 
  img.pushSprite(32, 24);

  img.pushImage(0, 0, pngWidth, pngHeight, kuma3); 
  img.pushSprite(32, 24);
}






