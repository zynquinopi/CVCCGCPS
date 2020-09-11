#include <Arduino.h>
#include <Wire.h>
#include "driver/rmt.h"
#include "TFT_eSPI.h"
#include "Util.h"
#include "GUI.h"
#include "GCCon.h"

//////////////////////////////////////////////////////////////////////////////////
#define RMT_TX_GPIO_NUM  GPIO_NUM_5     // TX GPIO ----
#define RMT_RX_GPIO_NUM  GPIO_NUM_23     // RX GPIO ----
#define RMT_CLK_DIV      80    /*!< RMT counter clock divider */
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */
#define rmt_item32_tIMEOUT_US  9500   /*!< RMT receiver timeout value(us) */

// Calibration
static int lxcalib = 0;
static int lycalib = 0;
static int cxcalib = 0;
static int cycalib = 0;
static int lcalib = 0;
static int rcalib = 0;
//Buttons and sticks
static uint8_t but1_send = 0;
static uint8_t but2_send = 0;
static uint8_t lx_send = 0;
static uint8_t ly_send = 0;
static uint8_t cx_send = 0;
static uint8_t cy_send = 0;
static uint8_t lt_send = 0;
static uint8_t rt_send = 0;

//RMT Transmitter Init
rmt_item32_t items[25];
rmt_config_t rmt_tx;
static void rmt_tx_init(){
    rmt_tx.rmt_mode = RMT_MODE_TX;
    rmt_tx.channel = RMT_CHANNEL_2;
    rmt_tx.gpio_num = RMT_TX_GPIO_NUM;
    rmt_tx.mem_block_num = 1;
    rmt_tx.clk_div = RMT_CLK_DIV;
    rmt_tx.tx_config.loop_en = false;
    rmt_tx.tx_config.carrier_freq_hz = 24000000;
    rmt_tx.tx_config.carrier_level = RMT_CARRIER_LEVEL_HIGH;
    rmt_tx.tx_config.carrier_en = 0;
    rmt_tx.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);
    
    //Fill items[] with console->controller command: 0100 0000 0000 0011 0000 0010
    items[0].duration0 = 3;
    items[0].level0 = 0;
    items[0].duration1 = 1;
    items[0].level1 = 1;
    items[1].duration0 = 1;
    items[1].level0 = 0;
    items[1].duration1 = 3;
    items[1].level1 = 1;
    // int j;
    for(int j = 0; j < 12; j++) {
        items[j+2].duration0 = 3;
        items[j+2].level0 = 0;
        items[j+2].duration1 = 1;
        items[j+2].level1 = 1;
    }
    items[14].duration0 = 1;
    items[14].level0 = 0;
    items[14].duration1 = 3;
    items[14].level1 = 1;
    items[15].duration0 = 1;
    items[15].level0 = 0;
    items[15].duration1 = 3;
    items[15].level1 = 1;
    for(int j = 0; j < 8; j++) {
        items[j+16].duration0 = 3;
        items[j+16].level0 = 0;
        items[j+16].duration1 = 1;
        items[j+16].level1 = 1;
    }
    items[24].duration0 = 1;
    items[24].level0 = 0;
    items[24].duration1 = 3;
    items[24].level1 = 1;
}

//RMT Receiver Init
rmt_config_t rmt_rx;
static void rmt_rx_init(){
    rmt_rx.rmt_mode = RMT_MODE_RX;
    rmt_rx.channel = RMT_CHANNEL_3;
    rmt_rx.gpio_num = RMT_RX_GPIO_NUM;
    rmt_rx.clk_div = RMT_CLK_DIV;
    rmt_rx.mem_block_num = 4;
    rmt_rx.rx_config.idle_threshold = rmt_item32_tIMEOUT_US / 10 * (RMT_TICK_10_US);
    rmt_config(&rmt_rx);
}

static void get_buttons(void *pvParameters){
    uint8_t but1 = 0;
    uint8_t but2 = 0;
    uint8_t dpad = 0x08;//Released
    uint8_t lx = 0;
    uint8_t ly = 0;
    uint8_t cx = 0;
    uint8_t cy = 0;
    uint8_t lt = 0;
    uint8_t rt = 0;
    
    //Sample and find calibration value for sticks
    int calib_loop = 0;
    int xsum = 0;
    int ysum = 0;
    int cxsum = 0;
    int cysum = 0;
    int lsum = 0;
    int rsum = 0;
    while(calib_loop < 5)
    {
        lx = 0;
        ly = 0;
        cx = 0;
        cy = 0;
        lt = 0;
        rt = 0;
        rmt_write_items(rmt_tx.channel, items, 25, 0);
        rmt_rx_start(rmt_rx.channel, 1);
        
        vTaskDelay(10);
        
        rmt_item32_t* item = (rmt_item32_t*) (RMT_CHANNEL_MEM(rmt_rx.channel));
        if(item[33].duration0 == 1 && item[27].duration0 == 1 && item[26].duration0 == 3 && item[25].duration0 == 3){
            
            //LEFT STICK X
            for(int x = 8; x > -1; x--)
            {
                if((item[x+41].duration0 == 1))
                {
                    lx += pow(2, 8-x-1);
                }
            }

            //LEFT STICK Y
            for(int x = 8; x > -1; x--)
            {
                if((item[x+49].duration0 == 1))
                {
                    ly += pow(2, 8-x-1);
                }
            }
            //C STICK X
            for(int x = 8; x > -1; x--)
            {
                if((item[x+57].duration0 == 1))
                {
                    cx += pow(2, 8-x-1);
                }
            }

            //C STICK Y
            for(int x = 8; x > -1; x--)
            {
                if((item[x+65].duration0 == 1))
                {
                    cy += pow(2, 8-x-1);
                }
            }

            //R AN
            for(int x = 8; x > -1; x--)
            {
                if((item[x+73].duration0 == 1))
                {
                    rt += pow(2, 8-x-1);
                }
            }

            //L AN
            for(int x = 8; x > -1; x--)
            {
                if((item[x+81].duration0 == 1))
                {
                    lt += pow(2, 8-x-1);
                }
            }
            
            xsum += lx;
            ysum += ly;
            cxsum += cx;
            cysum += cy;
            lsum += lt;
            rsum += rt;
            calib_loop++;
        }
    }
    
    //Set Stick Calibration
    lxcalib = 127-(xsum/5);
    lycalib = 127-(ysum/5);
    cxcalib = 127-(cxsum/5);
    cycalib = 127-(cysum/5);
    lcalib = 127-(lsum/5);
    rcalib = 127-(rsum/5);

    
    while(1)
    {
        but1 = 0;
        but2 = 0;
        dpad = 0x08;
        lx = 0;
        ly = 0;
        cx = 0;
        cy = 0;
        lt = 0;
        rt = 0;
        
        //Write command to controller
        rmt_write_items(rmt_tx.channel, items, 25, 0);
        rmt_rx_start(rmt_rx.channel, 1);
        
        vTaskDelay(6); //6ms between sample
        
        rmt_item32_t* item = (rmt_item32_t*) (RMT_CHANNEL_MEM(rmt_rx.channel));
        
        //Check first 3 bits and high bit at index 33
        if(item[33].duration0 == 1 && item[27].duration0 == 1 && item[26].duration0 == 3 && item[25].duration0 == 3)
        {
            
            //Button report: first item is item[25]
            //0 0 1 S Y X B A
            //1 L R Z U D R L
            //Joystick X (8bit)
            //Joystick Y (8bit)
            //C-Stick X (8bit)
            //C-Stick Y (8bit)
            //L Trigger Analog (8/4bit)
            //R Trigger Analog (8/4bit)
            
            //Buttons1
            if(item[32].duration0 == 1) but1 += 0x40;//A
            if(item[31].duration0 == 1) but1 += 0x20;//B
            if(item[30].duration0 == 1) but1 += 0x80;//X
            if(item[29].duration0 == 1) but1 += 0x10;//Y
            //DPAD
            if(item[40].duration0 == 1) dpad = 0x06;//L
            if(item[39].duration0 == 1) dpad = 0x02;//R
            if(item[38].duration0 == 1) dpad = 0x04;//D
            if(item[37].duration0 == 1) dpad = 0x00;//U
            
            //Buttons2
            if(item[36].duration0 == 1) but2 += 0x02;//Z
            if(item[35].duration0 == 1) but2 += 0x08;//RB
            if(item[34].duration0 == 1) but2 += 0x04;//LB
            if(item[28].duration0 == 1) but2 += 0x20;//START/OPTIONS/+
            if(but2 == 0x22)  but2 += 0x10;//Select =  Z + Start

            //LEFT STICK X
            for(int x = 8; x > -1; x--)
            {
                if(item[x+41].duration0 == 1)
                {
                    lx += pow(2, 8-x-1);
                }
            }

            //LEFT STICK Y
            for(int x = 8; x > -1; x--)
            {
                if(item[x+49].duration0 == 1)
                {
                    ly += pow(2, 8-x-1);
                }
            }
            
            //C STICK X
            for(int x = 8; x > -1; x--)
            {
                if(item[x+57].duration0 == 1)
                {
                    cx += pow(2, 8-x-1);
                }
            }
            
            //C STICK Y
            for(int x = 8; x > -1; x--)
            {
                if(item[x+65].duration0 == 1)
                {
                    cy += pow(2, 8-x-1);
                }
            }
            
            //R AN
            for(int x = 8; x > -1; x--)
            {
                if(item[x+73].duration0 == 1)
                {
                    rt += pow(2, 8-x-1);
                }
            }
            
            //L AN
            for(int x = 8; x > -1; x--)
            {
                if(item[x+81].duration0 == 1)
                {
                    lt += pow(2, 8-x-1);
                }
            }
            
            but1_send = but1 + dpad;
            but2_send = but2;
            lx_send = lx + lxcalib;
            ly_send = ly + lycalib;
            cx_send = cx + cxcalib;
            cy_send = cy + cycalib;
            lt_send = lt;
            rt_send = rt;
            
        }else{
            //log_info("read fail");
        }
        
    }
}
//////////////////////////////////////////////////////////////////////////////////



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
  rmt_tx_init();
  rmt_rx_init();
  xTaskCreatePinnedToCore(get_buttons, "get_buttons", 2048, NULL, 1, NULL, 1);
  LCD.init();
  LCD.setRotation(1);

  out_v = analogRead(34);
  out_c = analogRead(35);
  set_evr(0, 0);
  set_evr(1, 0);
  init_gui();
}


void loop(){
  Serial.println(but1_send, BIN);
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