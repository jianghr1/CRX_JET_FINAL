#include "Comm.h"
#define COMM_QUEUE_SIZE 32

GlobalState_t currentState;
GMCommand_t* currentIntCommandPtr;
GlobalInfo_t globalInfo;

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
