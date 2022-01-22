#ifndef _KEY_H_
#define _KEY_H_

#include "M480.h"

#define BUTTON_CHANGE		PA2
#define BUTTON_DOWN			PA4


enum{B_CHANGE, B_DOWN, B_Combination};
enum{SEC_1, SEC_5, MS_50};

void task_key(void);
void _key_config(void);


#endif
