#ifndef QRCODE_BUSINESS_LAYER_
#define QRCODE_BUSINESS_LAYER_

#include "stdint.h"

void creat_ercode(uint16_t x, uint16_t y, uint8_t version);
void delete_ercode(uint16_t x, uint16_t y, uint8_t version);



#endif
