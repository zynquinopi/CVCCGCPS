#include <Arduino.h>
#include <Wire.h>
#include "TFT_eSPI.h"
#include "Util.h"
#include "GUI.h"
#include "GCCon.h"

#define OUT_PANEL_PERIOD 1
#define S_STBY 0
#define S_STNG 1
#define S_OUTP 2
#define IS_V false
#define IS_C true

void update_set_panel();
void update_out_panel();
void display_cursor();
void init_gui();

TFT_eSPI LCD = TFT_eSPI();
char state = S_STBY;
bool vc_sel = IS_V;
char out_panel_period_cnt = 0;
char gc_key = 'n';
char evr_v = 10.0;
char evr_c = 3.0;
char evr_tmp = 0;
int out_v = 0;
int out_c = 0;


void setup(){
  Serial.begin(115200);
  Wire.begin(); //IO21:SDA    IO22:SCL
  Wire1.begin(19, 18, 400000);
  analogSetAttenuation(ADC_11db); //11db:0~3.6V   0db:0~1V
  LCD.init();
  LCD.setRotation(1);

  out_v = analogRead(34);
  out_c = analogRead(35);
  set_evr(0, 0);
  set_evr(1, 0);
  init_gui();
}


void loop(){
  if (Serial.available() > 0){
    gc_key = Serial.read();
    Serial.println(gc_key);
  }
  else {
    gc_key = 'n';
  }
  out_v = analogRead(34);
  out_c = analogRead(35);

  if(state == S_STBY){
    switch (gc_key){
      case 'u':
        vc_sel = IS_V;
        display_cursor();
        break;
      case 'd':
        vc_sel = IS_C;
        display_cursor();
        break;
      case 'a':
        state = S_STNG;
        if (vc_sel == IS_V)
          evr_tmp = evr_v;
        else if (vc_sel == IS_C)
          evr_tmp = evr_c;
        update_set_panel();
        break;
      case 's':
        state = S_OUTP;
        set_evr(0, evr_v);
        break;
      default:
        break;
    }
  }
  else if(state == S_STNG){
    switch (gc_key){
      case 'b':
        state = S_STBY;
        update_set_panel();
        display_cursor();
        break;
      case 'a':
        state = S_STBY;
        if (vc_sel == IS_V)
          evr_v = evr_tmp;
        else if (vc_sel == IS_C)
          evr_c = evr_tmp;
        update_set_panel();
        break;
      case 'u':
        if(evr_tmp < 127)
          evr_tmp += 1;
        update_set_panel();
        break;
      case 'd':
        if(evr_tmp > 0)
          evr_tmp -= 1;
        update_set_panel();
        break;
      default:
        break;
    }
  }
  else if(state == S_OUTP){
    switch (gc_key){
      case 'b':
        state = S_STBY;
        set_evr(0, 0);
        break;
      default:
        break;
    }
  }

  if(out_panel_period_cnt == OUT_PANEL_PERIOD){
    out_panel_period_cnt = 0;
    update_out_panel();
  } else{
    out_panel_period_cnt += 1;
  }

  delay(100);
}


void update_set_panel(){
	char s[16];
	float val = 0;

	if(state == S_STNG){
    LCD.setTextColor(TFT_YELLOW, TFT_BLACK);
		if(vc_sel == IS_V)
			val = evr_to_voltage(evr_tmp);
		else if(vc_sel == IS_C)
			val = evr_to_current(evr_tmp);
	}
	else{
    LCD.setTextColor(TFT_WHITE, TFT_BLACK);

		if(vc_sel == IS_V)
			val = evr_to_voltage(evr_v);
		else if(vc_sel == IS_C)
			val = evr_to_current(evr_c);
	}


	if(vc_sel == IS_V){
		LCD.drawString("set voltage", 15, 0);
		dtostrf(val, 2, 3, s);
		LCD.drawString(s, 15, 15);
		LCD.drawString("V", 65, 15);
	}
	else{
		LCD.drawString("set current", 15, 120);
		dtostrf(val, 1, 3, s);
		LCD.drawString(s, 15, 135);
		LCD.drawString("A", 65, 135);
	}
}


void update_out_panel(){
	char s[16];
	if(state == S_OUTP)
    LCD.setTextColor(TFT_YELLOW, TFT_BLACK);
	else
    LCD.setTextColor(TFT_WHITE, TFT_BLACK);

	LCD.drawString("out voltage", 175, 0);
	dtostrf(adc_to_voltage(out_v), 2, 3, s);
	LCD.drawString(s, 175, 15);
	LCD.drawString("V", 225, 15);

	LCD.drawString("out current", 175, 120);
	dtostrf(adc_to_current(out_c), 1, 3, s);
	LCD.drawString(s, 175, 135);
	LCD.drawString("A", 225, 135);
}


void display_cursor(){
	if(vc_sel == IS_V){
    LCD.fillRect(0, 0, 10, 10, TFT_RED);
    LCD.fillRect(0, 120, 10, 10, TFT_BLACK);
	}
	else if(vc_sel == IS_C){
    LCD.fillRect(0, 0, 10, 10, TFT_BLACK);
    LCD.fillRect(0, 120, 10, 10, TFT_RED);
	}
}


void init_gui(){
  char s[16];	
  LCD.fillRect(0, 0, 320, 240, TFT_BLACK);

  LCD.fillRect(0, 0, 10, 10, TFT_RED);

  LCD.setTextColor(TFT_WHITE, TFT_BLACK);

  LCD.drawString("set voltage", 15, 0);
  dtostrf(evr_to_voltage(evr_v), 2, 3, s);
  LCD.drawString(s, 15, 15);
  LCD.drawString("V", 65, 15);

  LCD.drawString("set current", 15, 120);
  dtostrf(evr_to_current(evr_c), 1, 3, s);
  LCD.drawString(s, 15, 135);
  LCD.drawString("A", 65, 135);

  LCD.drawString("out voltage", 175, 0);
  dtostrf(adc_to_voltage(out_v), 2, 3, s);
  LCD.drawString(s, 175, 15);
  LCD.drawString("V", 225, 15);

  LCD.drawString("out current", 175, 120);
  dtostrf(adc_to_current(out_c), 1, 3, s);
  LCD.drawString(s, 175, 135);
  LCD.drawString("A", 225, 135);
}