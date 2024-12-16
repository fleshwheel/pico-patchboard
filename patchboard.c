#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"

#include "bsp/board.h"
#include "tusb.h"

enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
const uint LED_PIN = PICO_DEFAULT_LED_PIN;

void led_blinking_task(void);
void midi_task(void);

uint8_t OUTPUT_PINS[10] = { 0, 1, 2, 3, 4, 21, 22, 26, 27, 28 };
uint8_t INPUT_PINS[15] = { 5, 6, 7, 8, 9,
			   10, 11, 12, 13, 14,
			   16, 17, 18, 19, 20 };

int main() {
  board_init();

  for (int i = 0; i < 10; i++) {
    uint8_t p = OUTPUT_PINS[i];
    gpio_init(p);
    gpio_set_dir(p, GPIO_IN);
  }

  for (int j = 0; j < 15; j++) {
    uint8_t p = INPUT_PINS[j];
    gpio_init(p);
    gpio_set_dir(p, GPIO_IN);
    gpio_set_pulls(p, false, true);
  }

  tusb_init();

  while (1)
  {
    tud_task(); // tinyusb device task
    midi_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// MIDI Task
//--------------------------------------------------------------------+

int matrix[10][15];


void send_midi(uint8_t status, uint8_t data1, uint8_t data2) {
  uint8_t msg[3];
  msg[0] = status;                 
  msg[1] = data1;
  msg[2] = data2;                      
  tud_midi_n_stream_write(0, 0, msg, 3);  
}


bool led_state = true;
void midi_task(void) {
  for (uint8_t i = 0; i < 10; i++) {
    uint8_t p_out = OUTPUT_PINS[i];
    gpio_set_dir(p_out, GPIO_OUT);
    gpio_put(p_out, 1);
    sleep_us(15);
    for (uint8_t j = 0; j < 15; j++) {
      uint8_t p_in = INPUT_PINS[j];
      uint8_t value = gpio_get(p_in);

      if (value != matrix[i][j]) {
	board_led_write(led_state);
	led_state = !led_state;

	if (value) {
	  send_midi(0x90, i, j);
	}
	else {
	  send_midi(0x80, i, j);
	}
      }
      matrix[i][j] = value;
    }
    gpio_set_dir(p_out, GPIO_IN);
    sleep_us(15);
  }
}

