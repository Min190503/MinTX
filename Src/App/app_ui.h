#ifndef APP_UI_H
#define APP_UI_H

#include <stdint.h>

typedef enum {
	TAB_MAIN = 0,
	TAB_CH = 1,
	TAB_CAL = 2,
	TAB_ST = 3
}UITab_t;

void UI_Init(void);
void UI_Update(void);
void UI_SwitchTab(UITab_t tab);

#endif
