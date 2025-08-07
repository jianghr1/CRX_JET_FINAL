#ifndef _Print_H
#define _Print_H

#include "stdint.h"

extern uint16_t rcr_overwrite;
void PrintTask(void);
void PrintTaskPrepare(void);
void ReadFileList(void);
#endif//_Print_H
