#ifndef _JETTING_H
#define _JETTING_H
#include "cmsis_os.h"

typedef struct {
    uint8_t channel:2;
    uint8_t SOF:6;
    uint8_t data[40];
    uint8_t crc8;
} Jetting_t;

typedef struct {
    Jetting_t* data;
    uint32_t status;
    osThreadId_t threadId;
} JettingInfo_t;

extern JettingInfo_t jettingInfo;

void StartJettingTask(void *argument);
#endif//_JETTING_H
