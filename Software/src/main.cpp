#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <UTFTGLUE.h>
#include "Util.h"
// #include "GUI.h"
// #include "GCCon.h"


#define LCD_RST 32
#define LCD_CS 33
#define LCD_RS 15
#define LCD_WR 4
#define LCD_RD 2
#define LCD_D0 12
#define LCD_D1 13
#define LCD_D2 26
#define LCD_D3 25
#define LCD_D4 17
#define LCD_D5 16
#define LCD_D6 27
#define LCD_D7 14


#define VOLT_STEP 1
#define CRNT_STEP 1
#define S_STBY 0
#define S_STNG 1
#define S_OUTP 2
#define isV false
#define isC true

void update_set_panel();
void update_out_panel();
void display_cursol();
void init_panel();


UTFTGLUE LCD(0,LCD_RS,LCD_WR,LCD_CS,LCD_RST,LCD_RD);
char state = S_STBY;
bool vc_sel = isV;
char cnt = 0;
char gc_key = 0;
char evr_v = 10.0;
char evr_c = 3.0;
char evr_tmp = 0;
int out_v = 0;
int out_c = 0;

void setup(){
	Wire.begin(); //IO21:SDA    IO22:SCL
	Wire1.begin(18, 19, 400000);
	Serial.begin(115200);
	analogSetAttenuation(ADC_11db); //11db:0~3.6V   0db:0~1V
	LCD.InitLCD();
	out_v = analogRead(34);
	out_c = analogRead(35);
	set_evr(0, 0);
	set_evr(1, 0);
	init_panel();
}


void loop(){
	if (Serial.available() > 0){
		gc_key = Serial.read();
		Serial.println(gc_key);
	}
	else {
		gc_key = 0;
	}
	out_v = analogRead(34);
	out_c = analogRead(35);

	if(state == S_STBY){
		switch (gc_key){
			case 'u': //i
				vc_sel = isV;
				display_cursol();
				break;
			case 'd': //k
				vc_sel = isC;
				display_cursol();
				break;
			case 'a':
				state = S_STNG;
				if (vc_sel == isV)
					evr_tmp = evr_v;
				else if (vc_sel == isC)
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
				display_cursol();
				break;
			case 'a':
				state = S_STBY;
				if (vc_sel == isV)
					evr_v = evr_tmp;
				else if (vc_sel == isC)
					evr_c = evr_tmp;
				update_set_panel();
				break;
			case 'u': //i
				if(evr_tmp < 127)
					evr_tmp += 1;
				update_set_panel();
				break;
			case 'd': //k
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

	if(cnt == 1){
		cnt = 0;
		update_out_panel();
	} else{
		cnt += 1;
	}

	delay(100);
}


void update_set_panel(){
	char s[16];
	float val = 0;

	if(state == S_STNG){
	  LCD.setColor(255,255,0);
		if(vc_sel == isV)
			val = evr_to_voltage(evr_tmp);
		else if(vc_sel == isC)
			val = evr_to_current(evr_tmp);
	}
	else{
	  LCD.setColor(255,255,255);
		if(vc_sel == isV)
			val = evr_to_voltage(evr_v);
		else if(vc_sel == isC)
			val = evr_to_current(evr_c);
	}

	LCD.setBackColor(0, 0, 0);

	if(vc_sel == isV){
		LCD.print("set voltage\n", 15, 0);
		dtostrf(val, 2, 3, s);
		LCD.print(s, 15, 15);
		LCD.printf("\t V \n");
	}
	else{
		LCD.print("set current\n", 15, 120);
		dtostrf(val, 1, 3, s);
		LCD.print(s, 15, 135);
		LCD.printf("\t A \n");
	}
}

void update_out_panel(){
	char s[16];
	if(state == S_OUTP)
		LCD.setColor(0, 255, 255);
	else
		LCD.setColor(255, 255, 255);
	LCD.setBackColor(0, 0, 0);

	LCD.print("out voltage\n", 175, 0);
	dtostrf(adc_to_voltage(out_v), 2, 3, s);
	LCD.print(s, 175, 15);
	LCD.printf("\t V \n");

	LCD.print("out current\n", 175, 120);
	dtostrf(adc_to_current(out_c), 1, 3, s);
	LCD.print(s,  175, 135);
	LCD.printf("\t A \n");
}

void display_cursol(){
	if(vc_sel == isV){
		LCD.setColor(255, 0, 0);	
		LCD.fillRect(0, 0, 10, 10);
		LCD.setColor(0, 0, 0);	
		LCD.fillRect(0, 120, 10, 10);
	}
	else if(vc_sel == isC){
		LCD.setColor(0, 0, 0);
		LCD.fillRect(0, 0, 10, 10);
		LCD.setColor(255, 0, 0);	
		LCD.fillRect(0, 120, 10, 10);
	}
}

void init_panel(){
	char s[16];	
	LCD.setColor(0, 0, 0);
	LCD.fillRect(0, 0, 320, 240);

	LCD.setColor(255, 0, 0);	
	LCD.fillRect(0, 0, 10, 10);

	LCD.setColor(255, 255, 255);
	LCD.setBackColor(0, 0, 0);
	
	LCD.print("set voltage\n", 15, 0);
	dtostrf(evr_to_voltage(evr_v), 2, 3, s);
	LCD.print(s, 15, 15);
	LCD.printf("  V \n");

	LCD.print("set current\n", 15, 120);
	dtostrf(evr_to_current(evr_c), 1, 3, s);
	LCD.print(s, 15, 135);
	LCD.printf("\t A \n");


	LCD.print("out voltage\n", 175, 0);
	dtostrf(adc_to_voltage(out_v), 2, 3, s);
	LCD.print(s, 175, 15);
	LCD.printf("\t V \n");

	LCD.print("out current\n", 175, 120);
	dtostrf(adc_to_current(out_c), 1, 3, s);
	LCD.print(s, 175, 135);
	LCD.printf("\t A \n");
}