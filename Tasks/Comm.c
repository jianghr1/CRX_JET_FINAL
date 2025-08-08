#include "Comm.h"
#include <stdarg.h>
#include <ctype.h>
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

uint8_t print_buffer[256];
uint8_t decode_buffer[256];

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim9;
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi4;
extern UART_HandleTypeDef huart8;
extern osThreadId_t defaultTaskHandle;
extern osThreadId_t motorTaskHandle;
extern osThreadId_t pumpTaskHandle;
extern osThreadId_t vacTaskHandle;
extern osThreadId_t headerTaskHandle;
extern osThreadId_t jettingTaskHandle;

void Comm_Init_Queue(void) {
	commIntQueueHead = 0;
	commIntQueueTail = 0;
	__HAL_UART_ENABLE_IT(&huart8, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&huart8, decode_buffer, 256);
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
  uint32_t length;
	while(huart8.gState != HAL_UART_STATE_READY) {
		osDelay(1);
	}
  va_list args;
  va_start(args, format);
  length = vsnprintf((char *)print_buffer, APP_TX_DATA_SIZE, (char *)format, args);
  va_end(args);
	HAL_UART_Transmit_DMA(&huart8, print_buffer, length);
}

void EmergencyStop(GlobalState_t issue) {
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
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
	// Heater
	HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_RESET);
	HAL_TIM_PWM_Stop(&htim9, TIM_CHANNEL_1);
	// Motors
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_2);
	HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_4);
	// Pumps
	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_2);
	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_4);
}

void GlobalInit() {
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
	// Motors
	htim1.Instance->CCR1 = 0;
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	htim8.Instance->CCR1 = 0;
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
	htim8.Instance->CCR2 = 0;
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
	htim8.Instance->CCR4 = 0;
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
	// Pumps
	htim4.Instance->CCR1 = 0;
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	htim4.Instance->CCR2 = 0;
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
	htim4.Instance->CCR3 = 0;
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	htim4.Instance->CCR4 = 0;
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
	// Power
	osDelay(1500);
	HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_SET);
	// Heat
	htim2.Instance->CCR1 = 0;
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	// Light
	htim9.Instance->CCR1 = 0;
	HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
	//TMC Init
	osDelay(100);
	TMC_init(TMC_MX , MRES_64);
	TMC_init(TMC_MZ1, MRES_16);
	TMC_init(TMC_MZ2, MRES_16);
	TMC_init(TMC_VAC, MRES_16);
	TMC_init(TMC_MS1, MRES_16);
	TMC_init(TMC_MS2, MRES_16);
	TMC_init(TMC_FY , MRES_16);
	TMC_init(TMC_QJ , MRES_16);
}

typedef enum {
	GMCode_GM,
	GMCode_X1,
	GMCode_X2,
	GMCode_X3,
	GMCode_Param1,
	GMCode_Param2,
	GMCode_Param3,
	GMCode_FPATH,
	GMCode_Jump,
} DecoderState_t;

void DecodeNumParam(GMCommand_t *procCommand) {
	if (procCommand->code == M105) {
		procCommand->numParams = 1;
	} else if (procCommand->code == M106) {
		procCommand->numParams = 1;
	} else if (procCommand->code == M120) {
		procCommand->numParams = 1;
	} else if (procCommand->code >= 30  && procCommand->code <=41) {
		procCommand->numParams = 1;
	} else if (procCommand->code == M122) {
		procCommand->numParams = 2;
	} else if (procCommand->code == M171) {
		procCommand->numParams = 2;
	} else if (procCommand->code >= 50 && procCommand->code <=70) {
		procCommand->numParams = 0;
	} else {
		procCommand->numParams = 3;
	}
}

void DecodeCDC(uint8_t *data, uint32_t len) {
	static DecoderState_t state;
	static GMCommand_t* procCommand;
	uint32_t i = 0;
	uint32_t j = 0;
	while (i < len) {
		uint8_t ch = data[i];
		switch(state) {
			case GMCode_GM: {
				if (ch == 'M' || ch == 'G') {
					state = GMCode_X1;
				}
				break;
			}
			case GMCode_X1: {
				if (ch == '1') {
					state = GMCode_X2;
					procCommand = Comm_Put_Queue();
					procCommand->code = 0;
					procCommand->commandSource = 1;
				} else {
					state = GMCode_GM;
				}
				break;
			}
			case GMCode_X2: {
				if (ch >= '0' && ch <= '9') {
					state = GMCode_X3;
					procCommand->code = (ch - '0') * 10;
				} else {
					state = GMCode_GM;
				}
				break;
			}
			case GMCode_X3: {
				if (ch >= '0' && ch <= '9') {
					procCommand->code += (ch - '0');
					if (procCommand->code == M172) {
						j = 0;
						state = GMCode_FPATH;
					} else if (procCommand->code == M171) {
						state = GMCode_Jump;
					} else {
						DecodeNumParam(procCommand);
						if (procCommand->numParams == 0) {
							state = GMCode_GM;
							Comm_Put_Queue_CPLT();
						} else {
							procCommand->param1 = 0;
							state = GMCode_Param1;
						}
					}
				} else {
					state = GMCode_GM;
				}
				break;
			}
			case GMCode_Param1: {
				while (i < len && (data[i] < '0' || data[i] > '9')) ++i;
				while (i < len && (data[i] >= '0' && data[i] <= '9')) {
					procCommand->param1 = procCommand->param1 * 10 + data[i] - '0';
					++i;
				}
				if (i == len) {
					break;
				} else {
					if (procCommand->numParams > 1) {
						procCommand->param2 = 0;
						state = GMCode_Param2;
					} else {
						state = GMCode_GM;
						Comm_Put_Queue_CPLT();
						break;
					}
				}
			}
			case GMCode_Param2: {
				while (i < len && (data[i] < '0' || data[i] > '9')) ++i;
				while (i < len && (data[i] >= '0' && data[i] <= '9')) {
					procCommand->param2 = procCommand->param2 * 10 + data[i] - '0';
					++i;
				}
				if (i == len) {
					break;
				} else {
					if (procCommand->numParams > 2) {
						procCommand->param3 = 0.0f;
						state = GMCode_Param3;
					} else {
						state = GMCode_GM;
						Comm_Put_Queue_CPLT();
						break;
					}
				}
			}
			case GMCode_Param3: {
				while (i < len && (data[i] < '0' || data[i] > '9')) ++i;
				while (i < len && (data[i] >= '0' && data[i] <= '9')) {
					procCommand->param3 = procCommand->param3 * 10 + data[i] - '0';
					++i;
				}
				if (i < len) {
					state = GMCode_GM;
					Comm_Put_Queue_CPLT();
				}
				break;
			}
			case GMCode_FPATH: {
				while (i < len && !isgraph(data[i])) ++i;
				for (; i < len && isgraph(data[i]); ++i, ++j) {
					globalInfo.fpath[j] = data[i];
				}
				if (i < len) {
					state = GMCode_GM;
					Comm_Put_Queue_CPLT();
				}
				break;
			}
			case GMCode_Jump: {
				while (i < len && (data[i] < '0' || data[i] > '9')) ++i;
				switch (data[i]) {
					case '3': {
						currentState |= GlobalStatePauseReq;
					}
					case '0': 
					case '1': 
					case '2':
					case '5':
					case '6': {
						procCommand->param1 = data[i] - '0';
						Comm_Put_Queue_CPLT();
						break;
					}
					case '7': {
						EmergencyStop(GlobalStateEStop);
						break;
					}
					case '4': {
						procCommand->param1 = 4;
						while (i < len && !isgraph(data[i])) ++i;
						procCommand->param2 = data[i] - '0';
						Comm_Put_Queue_CPLT();
						break;
					}
				}
			}
		}
		++i;
	}
}
