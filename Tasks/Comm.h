#ifndef _COMM_H
#define _COMM_H

#include <stdint.h>

typedef struct GMCommand GMCommand_t;
typedef void(*GMHandler)(GMCommand_t*);

typedef enum {
	M100 =  0,
	M101 =  1,
	M102 =  2,
	M103 =  3,
	M104 =  4,
	M105 =  5,
	M106 =  6,
	G110 = 10,
	G111 = 11,
	G112 = 12,
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
	int32_t  param3;
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

extern GlobalState_t currentState;
extern GMCommand_t* currentIntCommandPtr;
void Comm_Init_Queue(); 
GMCommand_t* Comm_Fetch_Queue();
GMCommand_t* Comm_Put_Queue();
void Comm_Put_Queue_CPLT();

#endif//_COMM_H
