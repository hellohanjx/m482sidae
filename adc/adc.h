#ifndef _ADC_H_
#define _ADC_H_

#include "M480.h"

void _adc_config(void);

uint16_t _get_internal_senser_adc(void);
uint16_t _get_external_senser_adc(void);


#endif
