#ifndef _TAP_SET_
#define _TAP_SET_

#include "M480.h"

enum{TAP_CLOSE, TAP_OPEN};


#define TAP1		PA14
#define TAP2		PA13
#define TAP3		PA12
#define TAP4		PD0
#define TAP5		PD1
#define TAP6		PD2

void _tap_config(void);
void tap_set(uint8_t channel, uint8_t mode);

#endif
