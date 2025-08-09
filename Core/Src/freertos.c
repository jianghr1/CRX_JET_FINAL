/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Comm.h"
#include "Motor.h"
#include "Header.h"
#include "Jetting.h"
#include "Pump.h"
#include "Vac.h"
#include "Init.h"
#include "Clean.h"
#include "Print.h"
#include "TMC2209.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi4;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for motorTask */
osThreadId_t motorTaskHandle;
const osThreadAttr_t motorTask_attributes = {
  .name = "motorTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for pumpTask */
osThreadId_t pumpTaskHandle;
const osThreadAttr_t pumpTask_attributes = {
  .name = "pumpTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for vacTask */
osThreadId_t vacTaskHandle;
const osThreadAttr_t vacTask_attributes = {
  .name = "vacTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for jettingTask */
osThreadId_t jettingTaskHandle;
const osThreadAttr_t jettingTask_attributes = {
  .name = "jettingTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for headerTask */
osThreadId_t headerTaskHandle;
const osThreadAttr_t headerTask_attributes = {
  .name = "headerTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for sensorTask */
osThreadId_t sensorTaskHandle;
const osThreadAttr_t sensorTask_attributes = {
  .name = "sensorTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for MUart1Mutex */
osMutexId_t MUart1MutexHandle;
const osMutexAttr_t MUart1Mutex_attributes = {
  .name = "MUart1Mutex"
};
/* Definitions for MTim8Mutex */
osMutexId_t MTim8MutexHandle;
const osMutexAttr_t MTim8Mutex_attributes = {
  .name = "MTim8Mutex"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
extern void StartMotorTask(void *argument);
extern void StartPumpTask(void *argument);
extern void StartVacTask(void *argument);
extern void StartJettingTask(void *argument);
extern void StartHeaderTask(void *argument);
extern void StartSensorTask(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of MUart1Mutex */
  MUart1MutexHandle = osMutexNew(&MUart1Mutex_attributes);

  /* creation of MTim8Mutex */
  MTim8MutexHandle = osMutexNew(&MTim8Mutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of motorTask */
  motorTaskHandle = osThreadNew(StartMotorTask, NULL, &motorTask_attributes);

  /* creation of pumpTask */
  pumpTaskHandle = osThreadNew(StartPumpTask, NULL, &pumpTask_attributes);

  /* creation of vacTask */
  vacTaskHandle = osThreadNew(StartVacTask, NULL, &vacTask_attributes);

  /* creation of jettingTask */
  jettingTaskHandle = osThreadNew(StartJettingTask, NULL, &jettingTask_attributes);

  /* creation of headerTask */
  headerTaskHandle = osThreadNew(StartHeaderTask, NULL, &headerTask_attributes);

  /* creation of sensorTask */
  sensorTaskHandle = osThreadNew(StartSensorTask, NULL, &sensorTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
	uint32_t flag;
	/* Global Init */
  // 37V
	HAL_GPIO_WritePin(EN37V_GPIO_Port, EN37V_Pin, GPIO_PIN_SET);
	osDelay(500);
	uint8_t VoltageR=170;
  HAL_GPIO_WritePin(VSEL_A_GPIO_Port, VSEL_A_Pin, 0);
	HAL_GPIO_WritePin(VSEL_B_GPIO_Port, VSEL_B_Pin, 0);
	HAL_GPIO_WritePin(VSEL_C_GPIO_Port, VSEL_C_Pin, 0);
	HAL_GPIO_WritePin(VSEL_D_GPIO_Port, VSEL_D_Pin, 0);
	HAL_SPI_Transmit(&hspi1, &VoltageR, 1, 10);
	HAL_SPI_Transmit(&hspi4, &VoltageR, 1, 10);
	HAL_GPIO_WritePin(VSEL_A_GPIO_Port, VSEL_A_Pin, 1);
	HAL_GPIO_WritePin(VSEL_B_GPIO_Port, VSEL_B_Pin, 1);
	HAL_GPIO_WritePin(VSEL_C_GPIO_Port, VSEL_C_Pin, 1);
	HAL_GPIO_WritePin(VSEL_D_GPIO_Port, VSEL_D_Pin, 1);
	
	triggerHandler.MX  = TMC_MX;
	triggerHandler.MZ1 = TMC_MZ1;
	triggerHandler.MZ2 = TMC_MZ2;
	InitTask();
	HAL_GPIO_WritePin(LEDG_GPIO_Port, LEDG_Pin, 1);
  /* Infinite loop */
  for(;;)
  {
		// Wait For CDC Command
    flag = osThreadFlagsWait(ALL_NEW_TASK|ALL_EMG_STOP, osFlagsWaitAny, 20);
		// Emergemcy Stop Handling
		if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
			HAL_GPIO_WritePin(LEDG_GPIO_Port, LEDR_Pin, 0);
		} else {
			HAL_GPIO_WritePin(LEDG_GPIO_Port, LEDR_Pin, 1);
		}
		currentIntCommandPtr = Comm_Fetch_Queue();
		if (currentIntCommandPtr == 0)
		{
			HAL_GPIO_WritePin(LEDG_GPIO_Port, LEDB_Pin, 0);
			continue;
    } else {
			HAL_GPIO_WritePin(LEDG_GPIO_Port, LEDB_Pin, 1);
			if (currentState == GlobalStateEStop || currentState == GlobalStateError && currentIntCommandPtr->code != M171) {
				usb_printf("[Comm][Error] Emergency Stop or Error State, Command Ignored");
				continue;
			}
		}
		if (currentIntCommandPtr->code >= 0 && currentIntCommandPtr->code <= 3)
		{
			// Dispatch To Pump Thread
			osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
		} else if (currentIntCommandPtr->code >= 4 && currentIntCommandPtr->code <= 6)
		{
			// Dispatch To Motor Thread
			osThreadFlagsSet(vacTaskHandle, ALL_NEW_TASK);
		} else if (currentIntCommandPtr->code >= 7 && currentIntCommandPtr->code <= 8)
		{
			// Dispatch To Pump Thread
			osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
		} else if (currentIntCommandPtr->code >=10 && currentIntCommandPtr->code <=19)
		{
			// Dispatch To Motor Thread
			osThreadFlagsSet(motorTaskHandle, ALL_NEW_TASK);
		} else if (currentIntCommandPtr->code >=20 && currentIntCommandPtr->code <=29)
		{
			// Dispatch To Header Thread
			osThreadFlagsSet(headerTaskHandle, ALL_NEW_TASK);
		} else if (currentIntCommandPtr->code >=30 && currentIntCommandPtr->code <=49)
		{
			// Dispatch To Pump Thread
			osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
		} else if (currentIntCommandPtr->code >=50 && currentIntCommandPtr->code <=70)
		{
			// Query
			switch(currentIntCommandPtr->code)
			{
				case M150: {
					usb_printf("%d\n", globalInfo.trigger_state.bits.MX);
					break;
				}
				case M151: {
					usb_printf("%d\n", globalInfo.trigger_state.bits.MZ1);
					break;
				}
				case M152: {
					usb_printf("%d\n", globalInfo.trigger_state.bits.MZ2);
					break;
				}
				case M160: {
					usb_printf("%d\n", globalInfo.trigger_state.bits.YW1);
					break;
				}
				case M161: {
					usb_printf("%d\n", globalInfo.trigger_state.bits.YW2);
					break;
				}
				case M162: {
					usb_printf("%d\n", globalInfo.vac_pressure);
					break;
				}
				case M163: {
					usb_printf("%d\n", (int)(globalInfo.x_encoder_pos * 10));
					break;
				}
				case M164: {
					usb_printf("%d\n", globalInfo.temperature);
					break;
				}
				case M165: {
					usb_printf("%d\n", globalInfo.trigger_state.bits.SW1);
					break;
				}
				case M166: {
					usb_printf("%d\n", globalInfo.trigger_state.bits.SW2);
					break;
				}
				case M167: {
					usb_printf("%d\n", globalInfo.trigger_state.bits.SW3);
					break;
				}
				case M168: {
					usb_printf("TBD\n");
					break;
				}
				case M169: {
					usb_printf("TBD\n");
					break;
				}
				case M170: {
					usb_printf("%d\n", (int)currentState);
					break;
				}
				default:
					usb_printf("ERROR\n");
			}
			osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
		} else if (currentIntCommandPtr->code == M171)
		{
			//
			switch(currentIntCommandPtr->param1) {
				case GlobalStateInit: {
					if (currentState != GlobalStatePrint && currentState != GlobalStateClean) {
						usb_printf("OK");
						InitTask();
					} else {
						usb_printf("ERROR");
					}
					break;
				}
				case GlobalStatePOff: {
					if (currentState == GlobalStateIdle) {
						usb_printf("OK");
						// TODO
					} else {
						usb_printf("ERROR");
					}
					break;
				}
				case GlobalStateClean: {
					if (currentState == GlobalStateIdle) {
						usb_printf("OK");
						CleanTask(currentIntCommandPtr->param2);
					} else {
						usb_printf("ERROR");
					}
					break;
				}
				case GlobalStateEStop: {
					usb_printf("OK");
					break;
				}
				default:
					usb_printf("ERROR");
			}
			osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
		} else if (currentIntCommandPtr->code == M172) {
			if (currentState == GlobalStateIdle) {
				usb_printf("OK");
				PrintTaskPrepare();
				PrintTask();
			} else if (currentState == GlobalStatePause) {
				usb_printf("OK");
				PrintTask();
			} else {
				usb_printf("ERROR");
			}
		} else if (currentIntCommandPtr->code == M180) {
			ReadFileList();
			osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
		}
		osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

