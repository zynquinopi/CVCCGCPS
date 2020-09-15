#include "GCCon.h"

int GCCon::lxcalib;
int GCCon::lycalib;
int GCCon::cxcalib;
int GCCon::cycalib;
int GCCon::lcalib;
int GCCon::rcalib;
uint8_t GCCon::but1_send;
uint8_t GCCon::but2_send;
uint8_t GCCon::lx_send;
uint8_t GCCon::ly_send;
uint8_t GCCon::cx_send;
uint8_t GCCon::cy_send;
uint8_t GCCon::lt_send;
uint8_t GCCon::rt_send;
rmt_item32_t GCCon::items[25];
rmt_config_t GCCon::rmt_tx;
rmt_config_t GCCon::rmt_rx;

GCCon::GCCon(){
  // Calibration
  lxcalib = 0;
  lycalib = 0;
  cxcalib = 0;
  cycalib = 0;
  lcalib = 0;
  rcalib = 0;
  //Buttons and sticks
  but1_send = 0;
  but2_send = 0;
  lx_send = 0;
  ly_send = 0;
  cx_send = 0;
  cy_send = 0;
  lt_send = 0;
  rt_send = 0;
}

void GCCon::init(){
  rmt_tx_init();
  rmt_rx_init();
}

char GCCon::get_button(){
  if      ((but1_send & 0b10000000) >> 7){
    return 'x';
  }else if((but1_send & 0b01000000) >> 6){
    return 'a';
  }else if((but1_send & 0b00100000) >> 5){
    return 'b';
  }else if((but1_send & 0b00010000) >> 4){
    return 'y';
  }else if(((but1_send & 0b00000100) >> 2) & ((but1_send & 0b00000010) >> 1)){
    return 'l';
  }else if((but1_send & 0b00000100) >> 2){
    return 'd';
  }else if((but1_send & 0b00000010) >> 1){
    return 'r';
  }else if(!but1_send){
    return 'u';
  }

  else if ((but2_send & 0b00100000) >> 5){
    return 's';
  }else if((but2_send & 0b00001000) >> 3){
    return 'R';
  }else if((but2_send & 0b00000100) >> 2){
    return 'L';
  }else if((but2_send & 0b00000010) >> 1){
    return 'z';
  }
  
  else{
    return 'n';
  }
}


void GCCon::get_buttons(void *pvParameters){
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
  while(calib_loop < 5){
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
      for(int x = 8; x > -1; x--){
        if((item[x+41].duration0 == 1)){
          lx += pow(2, 8-x-1);
        }
      }
      //LEFT STICK Y
      for(int x = 8; x > -1; x--){
        if((item[x+49].duration0 == 1)){
          ly += pow(2, 8-x-1);
        }
      }
      //C STICK X
      for(int x = 8; x > -1; x--){
        if((item[x+57].duration0 == 1)){
          cx += pow(2, 8-x-1);
        }
      }
      //C STICK Y
      for(int x = 8; x > -1; x--){
        if((item[x+65].duration0 == 1)){
          cy += pow(2, 8-x-1);
        }
      }
      //R AN
      for(int x = 8; x > -1; x--){
        if((item[x+73].duration0 == 1)){
          rt += pow(2, 8-x-1);
        }
      }
      //L AN
      for(int x = 8; x > -1; x--){
        if((item[x+81].duration0 == 1)){
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
    
    while(1){
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
      if(item[33].duration0 == 1 && item[27].duration0 == 1 && item[26].duration0 == 3 && item[25].duration0 == 3){
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
        for(int x = 8; x > -1; x--){
          if(item[x+41].duration0 == 1){
            lx += pow(2, 8-x-1);
          }
        }
        for(int x = 8; x > -1; x--){
        //LEFT STICK Y
          if(item[x+49].duration0 == 1){
            ly += pow(2, 8-x-1);
          }
        }
        //C STICK X
        for(int x = 8; x > -1; x--){
          if(item[x+57].duration0 == 1){
            cx += pow(2, 8-x-1);
          }
        }
        //C STICK Y
        for(int x = 8; x > -1; x--){
          if(item[x+65].duration0 == 1){
            cy += pow(2, 8-x-1);
          }
        }
        //R AN
        for(int x = 8; x > -1; x--){
          if(item[x+73].duration0 == 1){
            rt += pow(2, 8-x-1);
          }
        }
        //L AN
        for(int x = 8; x > -1; x--){
          if(item[x+81].duration0 == 1){
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


void GCCon::rmt_tx_init(){
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
    
  items[0].duration0 = 3;
  //Fill items[] with console->controller command: 0100 0000 0000 0011 0000 0010
  items[0].level0 = 0;
  items[0].duration1 = 1;
  items[0].level1 = 1;
  items[1].duration0 = 1;
  items[1].level0 = 0;
  items[1].duration1 = 3;
  items[1].level1 = 1;
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

void GCCon::rmt_rx_init(){
  rmt_rx.rmt_mode = RMT_MODE_RX;
  rmt_rx.channel = RMT_CHANNEL_3;
  rmt_rx.gpio_num = RMT_RX_GPIO_NUM;
  rmt_rx.clk_div = RMT_CLK_DIV;
  rmt_rx.mem_block_num = 4;
  rmt_rx.rx_config.idle_threshold = rmt_item32_tIMEOUT_US / 10 * (RMT_TICK_10_US);
  rmt_config(&rmt_rx);
}
