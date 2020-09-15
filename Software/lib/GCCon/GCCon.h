#ifndef _GCConH_
#define _GCConH_

#include <Arduino.h>
#include <driver/rmt.h>

#define RMT_TX_GPIO_NUM  GPIO_NUM_5      /*!< TX GPIO */
#define RMT_RX_GPIO_NUM  GPIO_NUM_23     /*!< RX GPIO */
#define RMT_CLK_DIV      80    /*!< RMT counter clock divider */
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */
#define rmt_item32_tIMEOUT_US  9500   /*!< RMT receiver timeout value(us) */

class GCCon {
  public:
    GCCon();
    void init();
    static void get_buttons(void *pvParameters);
    char get_button();


  private:
    void rmt_tx_init();
    void rmt_rx_init();

    static rmt_item32_t items[25];
    static rmt_config_t rmt_tx;
    static rmt_config_t rmt_rx;
    // Calibration
    static int lxcalib;
    static int lycalib;
    static int cxcalib;
    static int cycalib;
    static int lcalib;
    static int rcalib;
    //Buttons and sticks
    static uint8_t but1_send;
    static uint8_t but2_send;
    static uint8_t lx_send;
    static uint8_t ly_send;
    static uint8_t cx_send;
    static uint8_t cy_send;
    static uint8_t lt_send;
    static uint8_t rt_send;
};

#endif // ends #ifndef _GCConH_
