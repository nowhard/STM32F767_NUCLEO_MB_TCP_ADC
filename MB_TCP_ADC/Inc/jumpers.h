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

extern uint8_t jumpersDevAddr;
extern uint8_t	jumpersDevIsMaster;
extern uint8_t jumpersDevSectionType;
void Jumpers_ReadSettings(void);

#endif
