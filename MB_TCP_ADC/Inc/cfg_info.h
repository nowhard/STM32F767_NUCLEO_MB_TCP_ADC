/*
Модуль хранения энергонезависимой конфигурации во FLASH. 
Обеспечена отказоустойчивость к отключению питания во время обновления (транзакция записи)
Используется 2 сектора FLASH, только один из которых активен.
При обновлении данных сначала производится запись в неактивный сектор, после чего стирается активный и назначается новый активный
Признаком активности служит метка в начале сектора.
Структура хранимой информации описана структурой sConfigInfo
Для инициализации при первом старте или каком либо сбое используются значения по умолчанию, хранимые в "const sConfigInfo configInfoHard",
инициализацию которой рекомендуется выполнять в головном модуле (main.c, например)

ВАЖНО!
При использовании модуля должен использоваться sct файл с выделенными 2-мя секторами для хранения информации. 
Например:
LR_IROM2 0x08000000 0x00004000  {    ; load region size_region
  FLASH_CFG1 0x08000000 0x00004000  {  ; load address = execution address
   *.o (.flash_cfg1)
  }
}
LR_IROM3 0x08004000 0x00004000  {    ; load region size_region
  FLASH_CFG2 0x08004000 0x00004000  {  ; load address = execution address
   *.o (.flash_cfg2)
  }
}

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!Модуль целесообразно использовать в случае нечастого изменения конфигурационной информации (в связи с ограничением числа циклов перезаписи FLASH)!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
#ifndef _INCL_CFG_INFO_H
#define _INCL_CFG_INFO_H

#include "data_converter.h"
#include "main.h"
#include "adc_dcmi.h"

#define LABEL_CFG_SECTOR	"CFG_SECTOR"

#define LOAD_RESISTORS_NUM		14

typedef struct
{
	uint8_t ip_addr_0;
	uint8_t ip_addr_1;
	uint8_t ip_addr_2;
	uint8_t ip_addr_3;

	uint16_t port;

}sIPAddress;

typedef struct
{
	uint8_t channelMask;
	enADCSamplerate sampleRate;
	float freqCorrectionFactor;
	stCalibrChannel calibrChannel[ADC_CHN_NUM];
} sConfigADC;

// конфигурационная информация (EEPROM)
typedef struct 
{
	char Label[16];           
	sIPAddress IPAdress_Server;
	sConfigADC ConfigADC;
	float resistors[LOAD_RESISTORS_NUM];//список нагрузочных резисторов (Ом)
} sConfigInfo;

extern sConfigInfo configInfo;			// структура для хранения конфигурации прибора в ОЗУ
// процедура считывания конфигурационной информации прибора
int ConfigInfoRead (void);
//****************************************************************************
// Обновление информации о приборе (в виде структуры) во FLASH из MyInfoVar
int ConfigInfoWrite(void);
int StartConfigInfoWrite(void);

#endif

