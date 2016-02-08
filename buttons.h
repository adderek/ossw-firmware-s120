#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>
#include <stdint.h>

#define BUTTON_DETECTION_DELAY           2
#define BUTTON_LONG_PRESS_DELAY          450

void buttons_init(void);

void button_is_pushed(uint8_t pin_no, bool *p_is_pushed);

#endif /* BUTTONS_H */
