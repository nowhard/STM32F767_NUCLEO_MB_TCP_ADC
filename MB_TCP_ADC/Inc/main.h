/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define DATA4_Pin GPIO_PIN_4
#define DATA4_GPIO_Port GPIOE
#define DATA6_Pin GPIO_PIN_5
#define DATA6_GPIO_Port GPIOE
#define DATA7_Pin GPIO_PIN_6
#define DATA7_GPIO_Port GPIOE
#define User_Blue_Button_Pin GPIO_PIN_13
#define User_Blue_Button_GPIO_Port GPIOC
#define ADR0_Pin GPIO_PIN_0
#define ADR0_GPIO_Port GPIOF
#define ADR1_Pin GPIO_PIN_1
#define ADR1_GPIO_Port GPIOF
#define ADR2_Pin GPIO_PIN_2
#define ADR2_GPIO_Port GPIOF
#define STROB_Pin GPIO_PIN_5
#define STROB_GPIO_Port GPIOF
#define CLK_Pin GPIO_PIN_7
#define CLK_GPIO_Port GPIOF
#define ADR3_Pin GPIO_PIN_8
#define ADR3_GPIO_Port GPIOF
#define DATALED_Pin GPIO_PIN_9
#define DATALED_GPIO_Port GPIOF
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH
#define RMII_MDC_Pin GPIO_PIN_1
#define RMII_MDC_GPIO_Port GPIOC
#define SYNC_ADC_Pin GPIO_PIN_2
#define SYNC_ADC_GPIO_Port GPIOC
#define GPIO0_ADC_Pin GPIO_PIN_3
#define GPIO0_ADC_GPIO_Port GPIOC
#define SIG_100kHz_Pin GPIO_PIN_0
#define SIG_100kHz_GPIO_Port GPIOA
#define RMII_REF_CLK_Pin GPIO_PIN_1
#define RMII_REF_CLK_GPIO_Port GPIOA
#define RMII_MDIO_Pin GPIO_PIN_2
#define RMII_MDIO_GPIO_Port GPIOA
#define U_SCLK_Pin GPIO_PIN_5
#define U_SCLK_GPIO_Port GPIOA
#define RMII_CRS_DV_Pin GPIO_PIN_7
#define RMII_CRS_DV_GPIO_Port GPIOA
#define RMII_RXD0_Pin GPIO_PIN_4
#define RMII_RXD0_GPIO_Port GPIOC
#define RMII_RXD1_Pin GPIO_PIN_5
#define RMII_RXD1_GPIO_Port GPIOC
#define ENABLE_OUT_1_Pin GPIO_PIN_1
#define ENABLE_OUT_1_GPIO_Port GPIOB
#define FAULT_OUT_1_Pin GPIO_PIN_2
#define FAULT_OUT_1_GPIO_Port GPIOB
#define TEST_250_Pin GPIO_PIN_11
#define TEST_250_GPIO_Port GPIOF
#define FAULT_OUT_7_Pin GPIO_PIN_12
#define FAULT_OUT_7_GPIO_Port GPIOF
#define FAULT_250A_Pin GPIO_PIN_13
#define FAULT_250A_GPIO_Port GPIOF
#define FAULT_75A_Pin GPIO_PIN_14
#define FAULT_75A_GPIO_Port GPIOF
#define TEST_150_Pin GPIO_PIN_15
#define TEST_150_GPIO_Port GPIOF
#define ADR6_Pin GPIO_PIN_0
#define ADR6_GPIO_Port GPIOG
#define OE_Pin GPIO_PIN_7
#define OE_GPIO_Port GPIOE
#define TEST_75_Pin GPIO_PIN_9
#define TEST_75_GPIO_Port GPIOE
#define TEST_7_5_Pin GPIO_PIN_11
#define TEST_7_5_GPIO_Port GPIOE
#define RMII_TXD1_Pin GPIO_PIN_13
#define RMII_TXD1_GPIO_Port GPIOB
#define USART_TX_Pin GPIO_PIN_14
#define USART_TX_GPIO_Port GPIOB
#define USART_RX_Pin GPIO_PIN_15
#define USART_RX_GPIO_Port GPIOB
#define STLK_RX_Pin GPIO_PIN_8
#define STLK_RX_GPIO_Port GPIOD
#define STLK_TX_Pin GPIO_PIN_9
#define STLK_TX_GPIO_Port GPIOD
#define EN_VCC_75_Pin GPIO_PIN_10
#define EN_VCC_75_GPIO_Port GPIOD
#define FAULT_7_5A_Pin GPIO_PIN_13
#define FAULT_7_5A_GPIO_Port GPIOD
#define FAULT_150A_Pin GPIO_PIN_14
#define FAULT_150A_GPIO_Port GPIOD
#define ENABLE_AIR_Pin GPIO_PIN_2
#define ENABLE_AIR_GPIO_Port GPIOG
#define AIR_CS_Pin GPIO_PIN_3
#define AIR_CS_GPIO_Port GPIOG
#define EN_VCC_7_5_Pin GPIO_PIN_4
#define EN_VCC_7_5_GPIO_Port GPIOG
#define EN_VCC_150_Pin GPIO_PIN_5
#define EN_VCC_150_GPIO_Port GPIOG
#define USB_PowerSwitchOn_Pin GPIO_PIN_6
#define USB_PowerSwitchOn_GPIO_Port GPIOG
#define USB_OverCurrent_Pin GPIO_PIN_7
#define USB_OverCurrent_GPIO_Port GPIOG
#define EN_VCC_250_Pin GPIO_PIN_8
#define EN_VCC_250_GPIO_Port GPIOG
#define DATA0_Pin GPIO_PIN_6
#define DATA0_GPIO_Port GPIOC
#define DATA1_Pin GPIO_PIN_7
#define DATA1_GPIO_Port GPIOC
#define DATA2_Pin GPIO_PIN_8
#define DATA2_GPIO_Port GPIOC
#define DATA3_Pin GPIO_PIN_9
#define DATA3_GPIO_Port GPIOC
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define BUSE_SYNC_Pin GPIO_PIN_15
#define BUSE_SYNC_GPIO_Port GPIOA
#define AIR_SCLK_Pin GPIO_PIN_10
#define AIR_SCLK_GPIO_Port GPIOC
#define AIR_MISO_Pin GPIO_PIN_11
#define AIR_MISO_GPIO_Port GPIOC
#define BUSE_SIG_Pin GPIO_PIN_12
#define BUSE_SIG_GPIO_Port GPIOC
#define ADR5_Pin GPIO_PIN_0
#define ADR5_GPIO_Port GPIOD
#define ADR4_Pin GPIO_PIN_1
#define ADR4_GPIO_Port GPIOD
#define GPIO1_ADC_Pin GPIO_PIN_4
#define GPIO1_ADC_GPIO_Port GPIOD
#define GPIO2_ADC_Pin GPIO_PIN_5
#define GPIO2_ADC_GPIO_Port GPIOD
#define GPIO3_ADC_Pin GPIO_PIN_6
#define GPIO3_ADC_GPIO_Port GPIOD
#define GPIO4_ADC_Pin GPIO_PIN_7
#define GPIO4_ADC_GPIO_Port GPIOD
#define ADR7_Pin GPIO_PIN_9
#define ADR7_GPIO_Port GPIOG
#define U_CS_Pin GPIO_PIN_10
#define U_CS_GPIO_Port GPIOG
#define RMII_TX_EN_Pin GPIO_PIN_11
#define RMII_TX_EN_GPIO_Port GPIOG
#define U_MISO_Pin GPIO_PIN_12
#define U_MISO_GPIO_Port GPIOG
#define RMII_TXD0_Pin GPIO_PIN_13
#define RMII_TXD0_GPIO_Port GPIOG
#define ENABLE_OUT_7_Pin GPIO_PIN_14
#define ENABLE_OUT_7_GPIO_Port GPIOG
#define DIN_ADC_Pin GPIO_PIN_15
#define DIN_ADC_GPIO_Port GPIOG
#define DATA5_Pin GPIO_PIN_6
#define DATA5_GPIO_Port GPIOB
#define SIG_SYNC_TIM_Pin GPIO_PIN_9
#define SIG_SYNC_TIM_GPIO_Port GPIOB
#define SIG_SYNC_TIM_EXTI_IRQn EXTI9_5_IRQn
/* USER CODE BEGIN Private defines */


/*----------NET SETTINGS-------------------------*/

#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   109
#define IP_ADDR3   150
   
/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   109
#define GW_ADDR3   8 

#define SERVER_IP_ADDR0   192
#define SERVER_IP_ADDR1   168
#define SERVER_IP_ADDR2   109
#define SERVER_IP_ADDR3   140

#define SERVER_PORT		  1000


/*----------ADC SETTINGS-------------------------*/
#define ADC_CHN_NUM 		6	//всего каналов ацп 
#define ADC_UDP_CHN_NUM	3

//ADC DCMI
#define ADC_DCMI_HW_CHN_NUM	8
#define ADC_DCMI_BUF_LEN 		32000
#define ADC_DCMI_RESULT_BUF_LEN (ADC_DCMI_BUF_LEN/4)
#define ADC_DCMI_NUM_BITS		16


//ADC SPI
#define SPI_ADC_FREQ_DIV	4
#define SPI_ADC_BUF_LEN		(((ADC_DCMI_BUF_LEN/sizeof(uint16_t))/ADC_DCMI_HW_CHN_NUM)/(SPI_ADC_FREQ_DIV))


//ADC PYRO
#define ADC_PYRO_SAMPLING_PERIOD	2// ms
#define ADC_PYRO_CHN_NUM	8
#define ADC_PYRO_FIFO_LEN	1024
/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
