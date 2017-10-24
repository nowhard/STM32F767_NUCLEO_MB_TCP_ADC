#ifndef MB_TCP_REGS_H
#define MB_TCP_REGS_H



typedef struct
{
	uint16_t hour;
	uint16_t minute;
	uint16_t second;
}stTimeProc;

#define IS_TIME_HOURS(__TIME__) (((__TIME__) >=0) && ((__TIME__) <= 23))
#define IS_TIME_MINUTS(__TIME__) (((__TIME__) >=0) && ((__TIME__) <= 59))
#define IS_TIME_SECONDS(__TIME__) (((__TIME__) >=0) && ((__TIME__) <= 59))

/* ----------------------- Defines ------------------------------------------*/
/******************************************************************************
						INPUT REGS
******************************************************************************/

#define REG_INPUT_START         1001
#define REG_INPUT_NREGS         64
#define REG_HOLDING_START       2001
#define REG_HOLDING_NREGS       128

//-------ADC RESULT REGS--------
#define ADC_CHANNEL_0_RAW	 							 0
#define ADC_CHANNEL_1_RAW	 							 1
#define ADC_CHANNEL_2_RAW	 							 2
#define ADC_CHANNEL_3_RAW	 							 3
#define ADC_CHANNEL_4_RAW	 							 4
#define ADC_CHANNEL_5_RAW	 							 5
#define ADC_CHANNEL_0_RESULT	 					 6
#define ADC_CHANNEL_1_RESULT	 					 8
#define ADC_CHANNEL_2_RESULT	 					10 
#define ADC_CHANNEL_3_RESULT	 					12
#define ADC_CHANNEL_4_RESULT	 					14
#define ADC_CHANNEL_5_RESULT	 					16
#define ADC_CHANNEL_CONV	 							18

#define ADC_TIMESTAMP_CURRENT		 				20
#define ADC_SAMPLING_FREQ_STATE					24

#define DEV_SET_OUTPUTS_SEQUENCE_IN_PROGRESS	 25
//---------PYRO SQUIB REGS------------
#define ADC_PYRO_SQUIB_0		 						26
#define ADC_PYRO_SQUIB_1	 							28
#define ADC_PYRO_SQUIB_2		 						30
#define ADC_PYRO_SQUIB_3		 						32
#define ADC_PYRO_SQUIB_4		 						34
#define ADC_PYRO_SQUIB_5		 						36
#define ADC_PYRO_SQUIB_6		 						38
#define ADC_PYRO_SQUIB_7		 						40


#define PYRO_SQUIB_PIR_STATE						 42
#define PYRO_SQUIB_PIR_IN_LINE					 43
//--------ERROR & FAULT SIGNALS--------
#define PYRO_SQUIB_PIR_ERROR	 					 44
#define PYRO_SQUIB_MB_CONNECT_ERROR	 		 45
#define FAULT_OUT_1_SIG	 								 46
#define FAULT_OUT_7_SIG	 								 47
#define FAULT_250A_SIG	 								 48
#define FAULT_150A_SIG	 								 49
#define FAULT_75A_SIG	 									 50
#define FAULT_7_5A_SIG	 								 51

/******************************************************************************
						HOLDING REGS
******************************************************************************/

//------ADC UDP NET SETTINGS REGS----------
#define SERVER_IP_REG_0													0
#define SERVER_IP_REG_1													1
#define SERVER_IP_REG_2													2
#define SERVER_IP_REG_3													3
#define SERVER_PORT_REG													4
//------ADC SETTINGS------------------------
#define ADC_CHANNEL_0_K													5
#define ADC_CHANNEL_0_B													7
#define ADC_CHANNEL_1_K													9
#define ADC_CHANNEL_1_B					 								11
#define ADC_CHANNEL_2_K													13
#define ADC_CHANNEL_2_B													15
#define ADC_CHANNEL_3_K													17
#define ADC_CHANNEL_3_B													19
#define ADC_CHANNEL_4_K													21
#define ADC_CHANNEL_4_B													23
#define ADC_CHANNEL_5_K													25
#define ADC_CHANNEL_5_B													27
#define ADC_SAMPLERATE_FREQ_CORRECTION_FACTOR		30
#define ADC_STARTED															32
#define ADC_UDP _PACKET_TRANSFER_ENABLE					33
#define ADC_TIMESTAMP_RESET											34
//------ADC SETTINGS------------------------
#define DEV_SET_OUTPUTS_0												35
#define DEV_SET_OUTPUTS_1												36
#define DEV_SET_OUTPUTS_2												37
#define DEV_SET_OUTPUTS_3												38
#define DEV_SET_OUTPUTS_ALL											39
#define DEV_SET_OUTPUTS_SEQUENCE_STATE_1				43
#define DEV_SET_OUTPUTS_SEQUENCE_STATE_2				47
#define DEV_SET_OUTPUTS_SEQUENCE_STATE_END			51
#define DEV_SET_OUTPUTS_SEQUENCE_TIME						55
#define DEV_SET_OUTPUTS_SEQUENCE_NUM_CYCLES			56
#define DEV_SET_OUTPUTS_SEQUENCE_START					57
#define DEV_ENABLE_OUT_1												58
#define DEV_ENABLE_OUT_7												59
#define DEV_EN_VCC_250													60
#define DEV_EN_VCC_150													61
#define DEV_EN_VCC_75														62
#define DEV_EN_VCC_7_5													63
//------PYRO SQUIB SETTINGS-----------------
#define PYRO_SQUIB_PIR_SET_TIME									64
#define PYRO_SQUIB_PIR_1_SET_CURRENT						65
#define PYRO_SQUIB_PIR_2_SET_CURRENT						67
#define PYRO_SQUIB_PIR_3_SET_CURRENT						69
#define PYRO_SQUIB_PIR_4_SET_CURRENT						71
#define PYRO_SQUIB_PIR_SET_MASK									73
#define PYRO_SQUIB_PIR_START										74
#define PYRO_SQUIB_PIR_1_CALIBR_CURRENT_K				75
#define PYRO_SQUIB_PIR_1_CALIBR_CURRENT_B				77
#define PYRO_SQUIB_PIR_2_CALIBR_CURRENT_K				79
#define PYRO_SQUIB_PIR_2_CALIBR_CURRENT_B				81
#define PYRO_SQUIB_PIR_3_CALIBR_CURRENT_K				83
#define PYRO_SQUIB_PIR_3_CALIBR_CURRENT_B				85
#define PYRO_SQUIB_PIR_4_CALIBR_CURRENT_K				87
#define PYRO_SQUIB_PIR_4_CALIBR_CURRENT_B				89
#define DEV_ENABLE_AIR													91
//------------------------------------------------
#define DEV_RESET_CONTROLLER										92
//------------------------------------------------
#define DEV_PROC_TIME_HOUR											93
#define DEV_PROC_TIME_MINUTE										94
#define DEV_PROC_TIME_SECOND										95
#define DEV_SOUND_ALARM													96
//------------------------------------------------
#define DEV_LOAD_RESISTOR_VALUE_1								97
#define DEV_LOAD_RESISTOR_VALUE_2								99
#define DEV_LOAD_RESISTOR_VALUE_3								101
#define DEV_LOAD_RESISTOR_VALUE_4								103
#define DEV_LOAD_RESISTOR_VALUE_5								105
#define DEV_LOAD_RESISTOR_VALUE_6								107
#define DEV_LOAD_RESISTOR_VALUE_7								109
#define DEV_LOAD_RESISTOR_VALUE_8								111
#define DEV_LOAD_RESISTOR_VALUE_9								113
#define DEV_LOAD_RESISTOR_VALUE_10							115
#define DEV_LOAD_RESISTOR_VALUE_11							117
#define DEV_LOAD_RESISTOR_VALUE_12							119
#define DEV_LOAD_RESISTOR_VALUE_13							121
#define DEV_LOAD_RESISTOR_VALUE_14							123



#endif

