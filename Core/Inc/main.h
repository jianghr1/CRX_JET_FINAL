/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ALL_NEW_TASK 4
#define MAIN_TASK_CPLT 4
#define JETTING_FPGA_REPLY 16
#define ALL_EMG_STOP 1
#define JETTING_UART_CPLT 8
#define HTR_CTL_TIM htim10
#define HTR_CTL_CHN TIM_CHANNEL_1
#define VSEL_B_Pin GPIO_PIN_3
#define VSEL_B_GPIO_Port GPIOE
#define VSEL_C_Pin GPIO_PIN_4
#define VSEL_C_GPIO_Port GPIOE
#define VSEL_D_Pin GPIO_PIN_5
#define VSEL_D_GPIO_Port GPIOE
#define TH_FB_Pin GPIO_PIN_1
#define TH_FB_GPIO_Port GPIOC
#define MX_TRIG_Pin GPIO_PIN_1
#define MX_TRIG_GPIO_Port GPIOA
#define MX_TRIG_EXTI_IRQn EXTI1_IRQn
#define MZ1_TRIG_Pin GPIO_PIN_2
#define MZ1_TRIG_GPIO_Port GPIOA
#define MZ1_TRIG_EXTI_IRQn EXTI2_IRQn
#define MZ2_TRIG_Pin GPIO_PIN_3
#define MZ2_TRIG_GPIO_Port GPIOA
#define MZ2_TRIG_EXTI_IRQn EXTI3_IRQn
#define MS1_YW_Pin GPIO_PIN_4
#define MS1_YW_GPIO_Port GPIOC
#define MS1_YW_EXTI_IRQn EXTI4_IRQn
#define MS2_YW_Pin GPIO_PIN_5
#define MS2_YW_GPIO_Port GPIOC
#define MS2_YW_EXTI_IRQn EXTI9_5_IRQn
#define USB_DET1_Pin GPIO_PIN_0
#define USB_DET1_GPIO_Port GPIOB
#define EN37V_Pin GPIO_PIN_1
#define EN37V_GPIO_Port GPIOB
#define N_CRC_FAIL_Pin GPIO_PIN_9
#define N_CRC_FAIL_GPIO_Port GPIOE
#define N_CRC_FAIL_EXTI_IRQn EXTI9_5_IRQn
#define JETTING_Pin GPIO_PIN_10
#define JETTING_GPIO_Port GPIOE
#define JETTING_EXTI_IRQn EXTI15_10_IRQn
#define SW3_Pin GPIO_PIN_13
#define SW3_GPIO_Port GPIOE
#define SW2_Pin GPIO_PIN_14
#define SW2_GPIO_Port GPIOE
#define SW1_Pin GPIO_PIN_15
#define SW1_GPIO_Port GPIOE
#define USB_DET2_Pin GPIO_PIN_12
#define USB_DET2_GPIO_Port GPIOB
#define MFY_Pin GPIO_PIN_12
#define MFY_GPIO_Port GPIOD
#define MQJ_Pin GPIO_PIN_13
#define MQJ_GPIO_Port GPIOD
#define MMS1_Pin GPIO_PIN_14
#define MMS1_GPIO_Port GPIOD
#define MMS2_Pin GPIO_PIN_15
#define MMS2_GPIO_Port GPIOD
#define MVAC_Pin GPIO_PIN_6
#define MVAC_GPIO_Port GPIOC
#define MMZ1_Pin GPIO_PIN_7
#define MMZ1_GPIO_Port GPIOC
#define MMZ2_Pin GPIO_PIN_9
#define MMZ2_GPIO_Port GPIOC
#define MMX_Pin GPIO_PIN_8
#define MMX_GPIO_Port GPIOA
#define LEDR_Pin GPIO_PIN_10
#define LEDR_GPIO_Port GPIOA
#define LEDG_Pin GPIO_PIN_11
#define LEDG_GPIO_Port GPIOA
#define LEDB_Pin GPIO_PIN_12
#define LEDB_GPIO_Port GPIOA
#define MOTOR_EN_Pin GPIO_PIN_10
#define MOTOR_EN_GPIO_Port GPIOC
#define VCOM_EN_Pin GPIO_PIN_11
#define VCOM_EN_GPIO_Port GPIOC
#define SDIO_CD_Pin GPIO_PIN_0
#define SDIO_CD_GPIO_Port GPIOD
#define VAC1_CTL_Pin GPIO_PIN_3
#define VAC1_CTL_GPIO_Port GPIOD
#define MS1_CTL_Pin GPIO_PIN_4
#define MS1_CTL_GPIO_Port GPIOD
#define VAC2_CTL_Pin GPIO_PIN_5
#define VAC2_CTL_GPIO_Port GPIOD
#define MS2_CTL_Pin GPIO_PIN_6
#define MS2_CTL_GPIO_Port GPIOD
#define UVF_CTL_Pin GPIO_PIN_7
#define UVF_CTL_GPIO_Port GPIOD
#define UVL_CTL_Pin GPIO_PIN_3
#define UVL_CTL_GPIO_Port GPIOB
#define HTR_CTL_Pin GPIO_PIN_8
#define HTR_CTL_GPIO_Port GPIOB
#define VSEL_A_Pin GPIO_PIN_9
#define VSEL_A_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
