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

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for motorTask */
osThreadId_t motorTaskHandle;
const osThreadAttr_t motorTask_attributes = {
  .name = "motorTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for pumpTask */
osThreadId_t pumpTaskHandle;
const osThreadAttr_t pumpTask_attributes = {
  .name = "pumpTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for vacTask */
osThreadId_t vacTaskHandle;
const osThreadAttr_t vacTask_attributes = {
  .name = "vacTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for jettingTask */
osThreadId_t jettingTaskHandle;
const osThreadAttr_t jettingTask_attributes = {
  .name = "jettingTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for headerTask */
osThreadId_t headerTaskHandle;
const osThreadAttr_t headerTask_attributes = {
  .name = "headerTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
extern void StartMotorTask(void *argument);
extern void StartPumpTask(void *argument);
extern void StartVacTask(void *argument);
extern void StarJettingTask(void *argument);
extern void StartHeaderTask(void *argument);

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
  jettingTaskHandle = osThreadNew(StarJettingTask, NULL, &jettingTask_attributes);

  /* creation of headerTask */
  headerTaskHandle = osThreadNew(StartHeaderTask, NULL, &headerTask_attributes);

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
  /* Infinite loop */
  for(;;)
  {
		// Wait For CDC Command
    osThreadFlagsWait(ALL_NEW_TASK|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
		// Emergemcy Stop Handling
		if (currentState == GlobalStateEStop)
		{
			//TODO
			continue;
		}
		currentIntCommandPtr = Comm_Fetch_Queue();
		if (currentIntCommandPtr == 0)
		{
			// This Should Not Happen Unless EStop
			continue;
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
		} else if (currentIntCommandPtr->code >=10 && currentIntCommandPtr->code <=20)
		{
			// Dispatch To Motor Thread
			osThreadFlagsSet(motorTaskHandle, ALL_NEW_TASK);
		} else if (currentIntCommandPtr->code >=20 && currentIntCommandPtr->code <=30)
		{
			// Dispatch To Motor Thread
			osThreadFlagsSet(motorTaskHandle, ALL_NEW_TASK);
		} else if (currentIntCommandPtr->code >=30 && currentIntCommandPtr->code <=41)
		{
			// Dispatch To Pump Thread
			osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
		} else if (currentIntCommandPtr->code >=50 && currentIntCommandPtr->code <=70)
		{
			// Query
		} else if (currentIntCommandPtr->code == M171)
		{
			// Need to 
		}
		osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
		
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

