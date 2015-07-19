#include "vibration.h"
#include "nordic_common.h"
#include "app_timer.h"
#include "board.h"
#include "nrf_gpio.h"

static app_timer_id_t      m_vibration_timer_id;
static uint32_t m_pattern;
static uint8_t m_next_step;

static void vibration_motor_on(void) {
    nrf_gpio_pin_set(VIBRATION_MOTOR);
}

static void vibration_motor_off(void) {
    nrf_gpio_pin_clear(VIBRATION_MOTOR);
}

void vibration_next_step(void) {
		if (m_pattern >> (15 - m_next_step) & 0x1) {
				vibration_motor_on();
		} else {
			  vibration_motor_off();
		}
		
	  m_next_step++;
	  if (m_next_step >= (m_pattern >> 28)) {
				m_next_step = 0;
		}
}

static void vibration_timeout_handler(void * p_context) {
    UNUSED_PARAMETER(p_context);
		vibration_next_step();
}

void vibration_init(void) {
	
    nrf_gpio_cfg_output(VIBRATION_MOTOR);
    nrf_gpio_pin_clear(VIBRATION_MOTOR);
	
    uint32_t err_code;	 
		
    err_code = app_timer_create(&m_vibration_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                vibration_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

void vibration_vibrate(uint32_t pattern) {
	  m_pattern = pattern;
	  m_next_step = 0;
	  vibration_next_step();
	  uint32_t step_length = (pattern >> 16) & 0x3FF;
	
    uint32_t err_code;	 
    err_code = app_timer_start(m_vibration_timer_id, APP_TIMER_TICKS(step_length, 0), NULL);
    APP_ERROR_CHECK(err_code);
}

void vibration_stop() {
    uint32_t err_code;	 
	  err_code = app_timer_stop(m_vibration_timer_id);
    APP_ERROR_CHECK(err_code);
	  vibration_motor_off();
}
