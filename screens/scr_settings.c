#include <string.h>
#include <stdio.h>
#include "nrf_soc.h"
#include "scr_settings.h"
#include "../scr_mngr.h"
#include "../mlcd_draw.h"
#include "../mlcd.h"
#include "../graph.h"
#include "../i18n/i18n.h"
#include "../fs.h"
#include "../rtc.h"
#include "../alarm.h"
#include "../ext_ram.h"
#include "../config.h"
#include "../watchset.h"
#include "dialog_select.h"

#define MARGIN_LEFT 			3
#define SCROLL_HEIGHT			6
#define HEADER_HEIGHT			18
#define SUMMARY_X					100
#define MENU_ITEM_HEIGHT	20
#define MENU_ITEMS_PER_PAGE 7
#define MENU_SWITCH_PADDING_X 10

static int8_t selectedOption = 0;
static int8_t lastSelectedOption = 0;

typedef struct {
	  const uint16_t message_key;
	  void (*select_handler)();
	  void (*long_select_handler)();
	  void (*summary_drawer)(uint8_t x, uint8_t y);
} MENU_OPTION;	

static void opt_handler_change_date() {
    scr_mngr_show_screen(SCR_CHANGE_DATE);
};
	
static void opt_handler_change_time() {
    scr_mngr_show_screen(SCR_CHANGE_TIME);
};

//static void opt_handler_about() {
//    scr_mngr_show_screen(SCR_ABOUT);
//};

static void opt_handler_timer() {
		scr_mngr_show_screen(SCR_TIMER);
};

static void opt_handler_set_alarm() {
		scr_mngr_show_screen(SCR_SET_ALARM);
};

void fs_reformat(void);

static void reformat() {
		fs_reformat();
		scr_mngr_show_screen(SCR_WATCHFACE);
}

static void draw_alarm_switch(uint8_t x, uint8_t y) {
		bool on = is_alarm_active();
		draw_switch(x+MENU_SWITCH_PADDING_X, y, on);
}

static void draw_shake_light_switch(uint8_t x, uint8_t y) {
		default_action* default_actions = config_get_default_global_actions();
		bool on = default_actions[8].action_id;
		draw_switch(x+MENU_SWITCH_PADDING_X, y, on);
}

static void draw_notif_light_switch(uint8_t x, uint8_t y) {
		bool on = get_settings(CONFIG_NOTIFICATION_LIGHT);
		draw_switch(x+MENU_SWITCH_PADDING_X, y, on);
}

static void draw_colors_switch(uint8_t x, uint8_t y) {
		bool on = is_mlcd_inverted();
		draw_switch(x+MENU_SWITCH_PADDING_X, y, on);
}

static void draw_interval_summary(uint8_t x, uint8_t y) {
		uint16_t text = MESSAGE_1_SECOND;
		if (rtc_get_refresh_interval() == RTC_INTERVAL_MINUTE)
				text = MESSAGE_1_MINUTE;
		mlcd_draw_text(I18N_TRANSLATE(text), x, y, MLCD_XRES-SUMMARY_X, NULL, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_CENTER);
}

static void rtc_refresh_toggle() {
		settings_toggle(CONFIG_SLOW_REFRESH);
		rtc_toggle_refresh_interval();
}

static void notif_light_toggle() {
		settings_toggle(CONFIG_NOTIFICATION_LIGHT);
}

static void shake_light_toggle() {
		default_action* default_actions = config_get_default_global_actions();
		if (default_actions[8].action_id == 0)
				default_actions[8].action_id = WATCH_SET_FUNC_TEMPORARY_BACKLIGHT;
		else
				default_actions[8].action_id = 0;
		config_set_default_global_actions(default_actions);
}

// TEST DIALOG
static void select_item_handler(uint8_t item) {
}

static void test_handler() {
		pack_dialog_select(0, &select_item_handler, FONT_OPTION_NORMAL, I18N_TRANSLATE(MESSAGE_ABOUT),
				12, "gypq\0TWO\0THREE\0FOUR\0FIVE\0SIX\0SEVEN\0EIGHT\0NINE\0TEN\0ELEVEN\0TWELVE\0");
		scr_mngr_show_screen_with_param(SCR_DIALOG_SELECT, EXT_RAM_DATA_DIALOG_TEXT);
}

static const MENU_OPTION settings_menu[] = {
		{MESSAGE_TIMER, opt_handler_timer, opt_handler_timer, NULL},
		{MESSAGE_ALARM_CLOCK, opt_handler_set_alarm, alarm_toggle, draw_alarm_switch},
		{MESSAGE_DISPLAY, mlcd_colors_toggle, mlcd_colors_toggle, draw_colors_switch},
		{MESSAGE_SHAKE_LIGHT, shake_light_toggle, shake_light_toggle, draw_shake_light_switch},
		{MESSAGE_NOTIF_LIGHT, notif_light_toggle, notif_light_toggle, draw_notif_light_switch},
		{MESSAGE_RTC_REFRESH, rtc_refresh_toggle, rtc_refresh_toggle, draw_interval_summary},
	  {MESSAGE_DATE, opt_handler_change_date, opt_handler_change_date, NULL},
		{MESSAGE_TIME, opt_handler_change_time, opt_handler_change_time, NULL},
		{MESSAGE_FORMAT, reformat, reformat, NULL},
		{MESSAGE_RESTART, NVIC_SystemReset, NVIC_SystemReset, NULL},
		{MESSAGE_ABOUT, test_handler, test_handler, NULL}
};

static const uint8_t SIZE_OF_MENU = sizeof(settings_menu)/sizeof(MENU_OPTION);

static void draw_option(uint_fast8_t item) {
		uint_fast8_t yPos = HEADER_HEIGHT + 4 + MENU_ITEM_HEIGHT * (item % MENU_ITEMS_PER_PAGE);
  	mlcd_draw_text(I18N_TRANSLATE(settings_menu[item].message_key), MARGIN_LEFT, yPos, MLCD_XRES-MARGIN_LEFT, NULL, FONT_OPTION_NORMAL, HORIZONTAL_ALIGN_LEFT);
		void (*s_drawer)(uint8_t x, uint8_t y) = settings_menu[item].summary_drawer;
		if (s_drawer != NULL)
				s_drawer(SUMMARY_X, yPos);
		if (item == selectedOption)
				fillRectangle(0, yPos-2, SUMMARY_X-MARGIN_LEFT, MENU_ITEM_HEIGHT);
}

static void scr_settings_draw_options() {
		uint8_t page_no = selectedOption / MENU_ITEMS_PER_PAGE;
		uint8_t start_item = page_no * MENU_ITEMS_PER_PAGE;
		uint8_t items_no;
		if (SIZE_OF_MENU - start_item < MENU_ITEMS_PER_PAGE)
				items_no = SIZE_OF_MENU - start_item;
		else
				items_no = MENU_ITEMS_PER_PAGE;
		for (int i=0; i<items_no; i++) {
				draw_option(start_item+i);
		}
		if (page_no > 0)
				fillUp(MLCD_XRES/2, HEADER_HEIGHT-SCROLL_HEIGHT-1, SCROLL_HEIGHT);
		if (page_no + 1 < CEIL(SIZE_OF_MENU, MENU_ITEMS_PER_PAGE))
				fillDown(MLCD_XRES/2, MLCD_YRES-2, SCROLL_HEIGHT);
		if (lastSelectedOption / MENU_ITEMS_PER_PAGE == 1 && page_no == 0) {
				mlcd_clear_rect(0, 0, MLCD_XRES, HEADER_HEIGHT);
				scr_mngr_draw_notification_bar();
		}
}

static void scr_settings_refresh_screen() {
	  scr_mngr_redraw_notification_bar();
	  if (lastSelectedOption == selectedOption)
				return;
	  if (lastSelectedOption / MENU_ITEMS_PER_PAGE != selectedOption / MENU_ITEMS_PER_PAGE) {
				// on page change
				mlcd_clear_rect(0, HEADER_HEIGHT, MLCD_XRES, MLCD_YRES-HEADER_HEIGHT);
				scr_settings_draw_options();
	  } else {
				// on item change
				uint_fast8_t yPos = HEADER_HEIGHT + 2 + MENU_ITEM_HEIGHT * (selectedOption % MENU_ITEMS_PER_PAGE);
				fillRectangle(0, yPos, SUMMARY_X-MARGIN_LEFT, MENU_ITEM_HEIGHT);
				yPos = HEADER_HEIGHT + 2 + MENU_ITEM_HEIGHT * (lastSelectedOption % MENU_ITEMS_PER_PAGE);
				fillRectangle(0, yPos, SUMMARY_X-MARGIN_LEFT, MENU_ITEM_HEIGHT);
		}
		lastSelectedOption = selectedOption;
}

static void scr_settings_init() {
		//selectedOption = 0;
		//lastSelectedOption = 0xFF;
	
		spiffs_file fd = SPIFFS_open(&fs, "u/settings", SPIFFS_RDONLY, 0);
		if (fd >= 0) {
				SPIFFS_lseek(&fs, fd, 0, SPIFFS_SEEK_SET);
				scr_mngr_show_screen_with_param(SCR_WATCH_SET, 2<<24 | fd);
		}
}

static void scr_settings_draw_screen() {
	  scr_mngr_draw_notification_bar();
		scr_settings_draw_options();
}

static void scr_refresh_summary() {
		void (*s_drawer)(uint8_t, uint8_t) = settings_menu[selectedOption].summary_drawer;
		if (s_drawer != NULL) {
				uint_fast8_t yPos = HEADER_HEIGHT+MENU_ITEM_HEIGHT*(selectedOption%MENU_ITEMS_PER_PAGE);
				mlcd_clear_rect(0, yPos+2, MLCD_XRES, MENU_ITEM_HEIGHT);
				draw_option(selectedOption);
		}
}

static bool scr_settings_handle_button_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_BACK:
					  scr_mngr_show_screen(SCR_WATCHFACE);
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_UP:
				    if (selectedOption > 0)
								selectedOption--;
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
				    if (selectedOption+1 < SIZE_OF_MENU)
								selectedOption++;
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_SELECT:
					  settings_menu[selectedOption].select_handler();
						scr_refresh_summary();
				    return true;
		}
		return false;
}

static bool scr_settings_handle_button_long_pressed(uint32_t button_id) {
	  switch (button_id) {
			  case SCR_EVENT_PARAM_BUTTON_UP:
				    if (selectedOption > MENU_ITEMS_PER_PAGE)
								selectedOption -= MENU_ITEMS_PER_PAGE;
						else
								selectedOption = 0;
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_DOWN:
				    if (selectedOption + MENU_ITEMS_PER_PAGE < SIZE_OF_MENU)
								selectedOption += MENU_ITEMS_PER_PAGE;
						else
								selectedOption = SIZE_OF_MENU-1;
				    return true;
			  case SCR_EVENT_PARAM_BUTTON_SELECT: {
						void (*ls_handler)() = settings_menu[selectedOption].long_select_handler;
						if (ls_handler != NULL) {
								ls_handler();
								scr_refresh_summary();
								return true;
						}
				}
		}
		return false;
}

bool scr_settings_handle_event(uint32_t event_type, uint32_t event_param) {
	  switch(event_type) {
			  case SCR_EVENT_INIT_SCREEN:
				    scr_settings_init();
				    return true;
        case SCR_EVENT_DRAW_SCREEN:
            scr_settings_draw_screen();
            return true;
        case SCR_EVENT_REFRESH_SCREEN:
            scr_settings_refresh_screen();
            return true;
			  case SCR_EVENT_BUTTON_PRESSED:
				    return scr_settings_handle_button_pressed(event_param);
			  case SCR_EVENT_BUTTON_LONG_PRESSED: {
						return scr_settings_handle_button_long_pressed(event_param);
				}
		}
		return false;
}
