#ifndef JUMPERS_H
#define JUMPERS_H
#include "stm32f7xx_hal.h"

#ifndef TRUE
#define TRUE                    1
#endif

#ifndef FALSE
#define FALSE                   0
#endif

#define SECTION_TYPE_1234	0
#define SECTION_TYPE_56		1

void Jumpers_ReadSettings(void);

uint8_t Jumpers_DevIsMaster(void);
uint8_t Jumpers_GetDevAddr(void);
uint8_t Jumpers_GetDevSectionType(void);
#endif
