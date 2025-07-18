#include "Comm.h"
#define COMM_QUEUE_SIZE 32

GMCommand_t currentInternalCommand;

GMCommand_t commIntQueue[COMM_QUEUE_SIZE];
size_t commIntQueueHead;
size_t commIntQueueTail;

void Comm_Init_Queue() {
	commIntQueueHead = 0;
	commIntQueueTail = 0;
}

GMCommand_t* Comm_Fetch_Queue() {
	GMCommand_t* ret = 0;
	if (commIntQueueHead != commIntQueueTail)
	{
		ret = commIntQueue + commIntQueueHead;
		commIntQueueHead = (commIntQueueHead + 1) % COMM_QUEUE_SIZE;
	}
	return ret;
}

GMCommand_t* Comm_Put_Queue() {
	if ((commIntQueueTail + 1) % COMM_QUEUE_SIZE == commIntQueueHead)
	{
		return 0;
	}
	return commIntQueue + commIntQueueTail;
}

void Comm_Put_Queue_CPLT() {
	commIntQueueTail = (commIntQueueTail + 1) % COMM_QUEUE_SIZE;
}
