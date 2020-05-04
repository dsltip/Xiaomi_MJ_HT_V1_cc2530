#ifndef FLASH_H
#define FLASH_H

#include "hal_types.h"
#include "hal_defs.h"

void flash_page_read(uint8 bank, uint8 page,  uint16 offset, void *buf,   uint16 len);
void flash_page_erase(uint8 bank, uint8 page);
void flash_page_write(uint8 bank, uint8 page, uint16 offset, void *buf, uint16 len);


#endif