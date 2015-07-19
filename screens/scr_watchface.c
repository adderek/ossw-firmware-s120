#include <string.h>
#include "scr_watchface.h"
#include "../scr_mngr.h"
#include "../rtc.h"
#include "../mlcd.h"
#include "../scr_controls.h"
#include "../vibration.h"
#include "nrf_delay.h"
		
		
#include "../notifications.h"
#include "../ext_ram.h"
		
static NUMBER_CONTROL_DATA hour_ctrl_data;
		
static const SCR_CONTROL_NUMBER_CONFIG hour_config = {
		NUMBER_RANGE_0__99,
	  4,
	  4,
	  NUMBER_FORMAT_FLAG_ZERO_PADDED | 4 << 22 | 8 << 16 | 66 << 8 | 76,
	  (uint32_t (*)(uint32_t, uint8_t))rtc_get_current_hour,
	  0,
    &hour_ctrl_data
};

static NUMBER_CONTROL_DATA minutes_ctrl_data;

static const SCR_CONTROL_NUMBER_CONFIG minutes_config = {
		NUMBER_RANGE_0__99,
	  4,
	  85,
	  NUMBER_FORMAT_FLAG_ZERO_PADDED | 4 << 22 | 6 << 16 | 66 << 8 | 76,
	  (uint32_t (*)(uint32_t, uint8_t))rtc_get_current_minutes,
	  0,
    &minutes_ctrl_data
};

static NUMBER_CONTROL_DATA seconds_ctrl_data;

static const SCR_CONTROL_PROGRESS_BAR_CONFIG seconds_config = {
	  0,
	  MLCD_YRES - 3,
	  MLCD_XRES,
	  2,
	  60,
		0,
	  rtc_get_current_seconds,
	  0,
    &seconds_ctrl_data
};

static const SCR_CONTROL_DEFINITION controls[] = {
	  {SCR_CONTROL_NUMBER, (void*)&hour_config},
		{SCR_CONTROL_NUMBER, (void*)&minutes_config},
		{SCR_CONTROL_HORIZONTAL_PROGRESS_BAR, (void*)&seconds_config}
};

static const SCR_CONTROLS_DEFINITION controls_definition = {
	  sizeof(controls)/sizeof(SCR_CONTROL_DEFINITION),
	  (SCR_CONTROL_DEFINITION*)controls
};

static void scr_watchface_refresh_time() {
	  scr_controls_redraw(&controls_definition);
}

static void scr_watchface_handle_button_pressed(uint32_t button_id) {
    switch (button_id) {
        case SCR_EVENT_PARAM_BUTTON_UP:
            scr_mngr_show_screen(SCR_WATCH_SET);
            break;
        case SCR_EVENT_PARAM_BUTTON_DOWN:
						{
            uint8_t data[] = {1, 0, 5, 0, 15, '6', '6', '0', '1', '3', '5', '1', '1', '5', 0, 'M', 'a', 'r', 'z', 'e', 'n', 'i', 'u', 'n', 'i', 'a', 0 };
						
	          ext_ram_write_data(0x1C00, data, sizeof(data));
			      notifications_notify(8, 0x1C00, 10000, (2<<28) | 500<<16 | 2<<14);
						}
						break;
				
				   
    }
}

static void scr_watchface_handle_button_long_pressed(uint32_t button_id) {
    switch (button_id) {
        case SCR_EVENT_PARAM_BUTTON_SELECT:
            scr_mngr_show_screen(SCR_SETTINGS);
            break;
    }
}

static void scr_watchface_draw() {
	  scr_controls_draw(&controls_definition);
}

void scr_watchface_handle_event(uint32_t event_type, uint32_t event_param) {
    switch(event_type) {
        case SCR_EVENT_DRAW_SCREEN:
            scr_watchface_draw();
            break;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_watchface_refresh_time();
            break;
        case SCR_EVENT_BUTTON_PRESSED:
            scr_watchface_handle_button_pressed(event_param);
            break;
        case SCR_EVENT_BUTTON_LONG_PRESSED:
            scr_watchface_handle_button_long_pressed(event_param);
            break;
    }
}
