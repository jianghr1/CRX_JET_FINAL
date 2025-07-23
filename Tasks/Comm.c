#include "Comm.h"
#include <stdarg.h>
#include "usbd_cdc_if.h"
#include "cmsis_os.h"
#include "TMC2209.h"
#define COMM_QUEUE_SIZE 32

GlobalState_t currentState;
GMCommand_t* currentIntCommandPtr;
GlobalInfo_t globalInfo;
TriggerHandler_t triggerHandler;

GMCommand_t commIntQueue[COMM_QUEUE_SIZE];
uint32_t commIntQueueHead;
uint32_t commIntQueueTail;

void Comm_Init_Queue(void) {
	commIntQueueHead = 0;
	commIntQueueTail = 0;
}

GMCommand_t* Comm_Fetch_Queue(void) {
	GMCommand_t* ret = 0;
	if (commIntQueueHead != commIntQueueTail)
	{
		ret = commIntQueue + commIntQueueHead;
		commIntQueueHead = (commIntQueueHead + 1) % COMM_QUEUE_SIZE;
	}
	return ret;
}

GMCommand_t* Comm_Put_Queue(void) {
	if ((commIntQueueTail + 1) % COMM_QUEUE_SIZE == commIntQueueHead)
	{
		return 0;
	}
	return commIntQueue + commIntQueueTail;
}

void Comm_Put_Queue_CPLT(void) {
	commIntQueueTail = (commIntQueueTail + 1) % COMM_QUEUE_SIZE;
}

uint8_t usb_printf(const char *format, ...) {
  extern USBD_HandleTypeDef hUsbDeviceHS;
  extern uint8_t UserTxBufferHS[];
  va_list args;
  uint32_t length;
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
  va_start(args, format);
  length = vsnprintf((char *)UserTxBufferHS, APP_TX_DATA_SIZE, (char *)format, args);
  va_end(args);
  uint32_t transmit_start_time = HAL_GetTick();
  while (hcdc->TxState != 0) {
    if (HAL_GetTick() - transmit_start_time > 10) {
      return (USBD_FAIL);
    }
    osDelay(1);
  }
  return CDC_Transmit_HS(UserTxBufferHS, length);
}

void EmergencyStop(GlobalState_t issue) {
	extern TIM_HandleTypeDef htim2;
	extern TIM_HandleTypeDef htim9;
	extern osThreadId_t defaultTaskHandle;
	extern osThreadId_t motorTaskHandle;
	extern osThreadId_t pumpTaskHandle;
	extern osThreadId_t vacTaskHandle;
	extern osThreadId_t headerTaskHandle;
	extern osThreadId_t jettingTaskHandle;
	// First: Set ESTOP State
	currentState = issue;
	// Second: Trigger Low Level Thread
	osThreadFlagsSet(motorTaskHandle, ALL_EMG_STOP);
	osThreadFlagsSet(pumpTaskHandle, ALL_EMG_STOP);
	osThreadFlagsSet(vacTaskHandle, ALL_EMG_STOP);
	osThreadFlagsSet(headerTaskHandle, ALL_EMG_STOP);
	osThreadFlagsSet(jettingTaskHandle, ALL_EMG_STOP);
	// Forth: Trigger High Level Thread
	osThreadFlagsSet(defaultTaskHandle, ALL_EMG_STOP);
	// Finally: Disable All Device
	// A: Valves
	HAL_GPIO_WritePin(MS1_CTL_GPIO_Port , MS1_CTL_Pin , GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MS2_CTL_GPIO_Port , MS2_CTL_Pin , GPIO_PIN_RESET);
	HAL_GPIO_WritePin(VAC1_CTL_GPIO_Port, VAC1_CTL_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(VAC2_CTL_GPIO_Port, VAC2_CTL_Pin, GPIO_PIN_RESET);
	// B: UV Lights
	HAL_GPIO_WritePin(UVF_CTL_GPIO_Port , UVF_CTL_Pin , GPIO_PIN_RESET);
	htim2.Instance->CCR2 = 0;
	// C: Heater
	htim9.Instance->CCR1 = 0;
	// D: Motors
	TMC_softEnable(TMC_MX , false);
	TMC_softEnable(TMC_MZ1, false);
	TMC_softEnable(TMC_MZ2, false);
	// E: Pumps
	TMC_softEnable(TMC_VAC, false);
	TMC_softEnable(TMC_MS1, false);
	TMC_softEnable(TMC_MS2, false);
	TMC_softEnable(TMC_QJ , false);
	TMC_softEnable(TMC_FY , false);
}
