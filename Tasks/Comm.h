#ifndef _COMM_H
#define _COMM_H

#include <stdint.h>

typedef struct GMCommand GMCommand_t;
typedef struct TMC_t TMC;

typedef enum {
	M100 =  0,
	M101 =  1,
	M102 =  2,
	M103 =  3,
	M104 =  4,
	M105 =  5,
	M106 =  6,
	M107 =  7,
	M108 =  8,
	G110 = 10,
	G111 = 11,
	G112 = 12,
	G113 = 13, // Z move sycn
	G114 = 14, // X zeroing
	G115 = 15, // Z zeroing
	M120 = 20,
	M121 = 21,
	M122 = 22,
	M130 = 30,
	M131 = 31,
	M132 = 32,
	M133 = 33,
	M140 = 40,
	M141 = 41,
	M150 = 50,
	M151 = 51,
	M152 = 52,
	M160 = 60,
	M161 = 61,
	M162 = 62,
	M163 = 63,
	M164 = 64,
	M165 = 65,
	M166 = 66,
	M167 = 67,
	M168 = 68,
	M169 = 69,
	M170 = 70,
	M171 = 71,
	M172 = 72,
	M180 = 80,
} GMCode;

struct GMCommand {
	uint32_t cid;
	GMCode   code;
	int32_t  param1;
	int32_t  param2;
	float    param3;
	uint8_t  num_params;
	uint8_t  command_source;
};

typedef enum {
	GlobalStateInit  = 0,
	GlobalStateIdle  = 1,
	GlobalStatePrint = 2,
	GlobalStatePause = 3,
	GlobalStateClean = 4,
	GlobalStatePOff  = 5,
	GlobalStateError = 6,
	GlobalStateEStop = 7,
} GlobalState_t;

typedef union {
	struct {
		uint8_t MX :1;
		uint8_t MZ1:1;
		uint8_t MZ2:1;
		uint8_t YW1:1;
		uint8_t YW2:1;
		uint8_t SW1:1;
		uint8_t SW2:1;
		uint8_t SW3:1;
	} bits;
	uint8_t data;
}TriggerState_t;

typedef struct {
	int32_t vac_pressure;
	int16_t temperature;
	int16_t targetTemperature;
	int16_t x_encoder_pos;
	uint8_t motor_state;
	TriggerState_t trigger_state;
} GlobalInfo_t;

typedef struct {
	TMC* MX;
	TMC* MZ1;
	TMC* MZ2;
	TMC* YW1;
	TMC* YW2;
} TriggerHandler_t;

extern GlobalState_t currentState;
extern GMCommand_t* currentIntCommandPtr;
extern GlobalInfo_t globalInfo;
extern TriggerHandler_t triggerHandler;

void Comm_Init_Queue(void); 
GMCommand_t* Comm_Fetch_Queue(void);
GMCommand_t* Comm_Put_Queue(void);
void Comm_Put_Queue_CPLT(void);

uint8_t usb_printf(const char *format, ...);

void EmergencyStop(GlobalState_t issue);

#endif//_COMM_H
