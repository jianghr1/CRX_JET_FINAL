#include "Print.h"
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "Comm.h"
#include "Jetting.h"
#include "Init.h"
#include "math.h"

extern osThreadId_t motorTaskHandle;
extern osThreadId_t pumpTaskHandle;
extern osThreadId_t vacTaskHandle;
extern osThreadId_t headerTaskHandle;
extern osThreadId_t jettingTaskHandle;

#define CHECK_STATE if ((currentState & 0xF) == GlobalStateEStop || (currentState & 0xF) == GlobalStateError) return

#define AD_gap 12.3613
#define AC_gap 11.811
#define AB_gap 0.5503

uint8_t rx[BLOCKSIZE];

FATFS myFatFs;
FIL myFile;
FRESULT f_res;
uint32_t bytesRead;
FILINFO fno;
DIR dir;

float zeropos_x;
float stepsize_x;
float stepsize_z;
uint32_t size_x;
uint32_t size_y;
uint32_t size_z;

uint32_t x, y, z;

uint8_t MS1_CH1_List[42];
uint8_t MS1_CH2_List[16][42];
uint8_t MS2_CH1_List[64][42];
uint8_t MS2_CH2_List[64][42];

uint32_t idx;

void decodeStr(char* ref) {
	uint32_t ref_idx = 0;
	while (idx < bytesRead && ref[ref_idx]!=0) {
		if (ref[ref_idx++] != rx[idx++]) {
			ref_idx = 0;
		}
	}
}

void decodeUInt(uint32_t* out) {
	*out = 0;
	while (idx < bytesRead && (rx[idx] < '0' || rx[idx] > '9')) ++idx;
	while (idx < bytesRead && (rx[idx] >='0' && rx[idx] <='9')) {
		*out = *out * 10 + (rx[idx] - '0');
		++idx;
	}
}

void decodeFloat(float* out) {
	*out = 0.0f;
	float mul = 0.1f;
	while (idx < bytesRead && (rx[idx] < '0' || rx[idx] > '9')) ++idx;
	while (idx < bytesRead) {
		if (rx[idx] > '0' && rx[idx] < '9') {
			*out = *out * 10 + (rx[idx] - '0');
		} else if (rx[idx] == '.') {
			break;
		} else {
			return;
		}
	}
	while (idx < bytesRead) {
		if (rx[idx] > '0' && rx[idx] < '9') {
			*out = *out + (rx[idx] & 0xF) * mul;
			mul *= 0.1f;
		} else {
			return;
		}
	}
}

void readLine(void) {
	uint8_t* CHA_R = MS1_CH1_List;
	uint8_t* CHB_R = MS1_CH2_List[x%16];
	uint8_t* CHC_R = MS2_CH1_List[x%64];
	uint8_t* CHD_R = MS2_CH2_List[x%64];

	CHA_R[0] = 0xA8; ++CHA_R;
	CHB_R[0] = 0xA9; ++CHB_R;
	CHC_R[0] = 0xAA; ++CHC_R;
	CHD_R[0] = 0xAB; ++CHD_R;
	
	for (y = 0; y < size_y/2; y++) {
		if (idx == bytesRead)
		{
			f_res = f_read(&myFile, rx, sizeof(rx), &bytesRead);
			if (f_res != FR_OK) {
				usb_printf("Failed to read file");
				EmergencyStop(GlobalStateError);
				// Handle error
				return;
			}
			idx = 0;
		}
		if ((rx[idx] >> 4) == 0) {
			// MS1
			CHA_R[y/8] |= 1<<(7-(y%8));
			CHC_R[y/8] &= ~(1<<(7-(y%8)));
		} else {
			// MS2
			CHC_R[y/4] |= 1<<(7-(y%8));
			CHA_R[y/8] &= ~(1<<(7-(y%8)));
		}
		
		if ((rx[idx] & 0xF) == 0) {
			// MS1
			CHB_R[y/8] |= 1<<(7-(y%8));
			CHD_R[y/8] &= ~(1<<(7-(y%8)));
		} else {
			// MS2
			CHD_R[y/8] |= 1<<(7-(y%8));
			CHB_R[y/8] &= ~(1<<(7-(y%8)));
		}
	}
	if (y % 8 != 0)
	{
		CHA_R[y/8] &= 0xFF << (8-(y%8));
		CHB_R[y/8] &= 0xFF << (8-(y%8));
		CHC_R[y/8] &= 0xFF << (8-(y%8));
		CHD_R[y/8] &= 0xFF << (8-(y%8));
		y += 8 - (y%8);
	}
	for (; y < 320; y+=8)
	{
		CHA_R[y/8] = 0;
		CHB_R[y/8] = 0;
		CHC_R[y/8] = 0;
		CHD_R[y/8] = 0;
	}
}

void decodeHeader(void) {

	idx = 0;
	decodeStr("zeropos x");
	decodeFloat(&zeropos_x);
	decodeStr("stepsize x");
	decodeFloat(&stepsize_x);
	decodeStr("stepsize z");
	decodeFloat(&stepsize_z);
	decodeStr("size x");
	decodeUInt(&size_x);
	decodeStr("size y");
	decodeUInt(&size_y);
	decodeStr("size z");
	decodeUInt(&size_z);
	decodeStr("end header");
}

void PrintTaskPrepare(void) {
	f_res = f_mount(&myFatFs, "0:", 1);
	if (f_res != FR_OK) {
		usb_printf("Failed to mount filesystem");
		EmergencyStop(GlobalStateError);
		// Handle error
		return;
	}
	f_res = f_open(&myFile, "amount.txt", FA_OPEN_EXISTING | FA_READ);
	if (f_res != FR_OK) {
		usb_printf("Failed to open file");
		EmergencyStop(GlobalStateError);
		// Handle error
		return;
	}
	f_res = f_read(&myFile, rx, sizeof(rx), &bytesRead);
	if (f_res != FR_OK) {
		usb_printf("Failed to read file");
		EmergencyStop(GlobalStateError);
		// Handle error
		return;
	}

	//Decode Header
	decodeHeader();
	z = 0;
	
	//Init EveryThing
	InitTask();

}

void PrintTask(void) {

	static GMCommand_t command;

	// Heater Set To 70 degree
	command.code = M120;
	command.param1 = 70;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(headerTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;

	for (; z < size_z; z++) {
		if (currentState == GlobalStatePause) {
			
		}

		for (x = 0; x < size_x + AD_gap/stepsize_x; x++) {
			float posA = x;
			float posB = x - AB_gap / stepsize_x;
			float posC = x - AC_gap / stepsize_x;
			float posD = x - AD_gap / stepsize_x;
			
			if (x < size_x) {
				readLine();
			}
			CHECK_STATE;
			// currently, We only Consider Channel A
			
			while ((int)posA < (x + 1)) {
				// Calculate next integer positions for each channel
				int32_t nextA = ceilf(posA);
				int32_t nextB = ceilf(posB);
				int32_t nextC = ceilf(posC);
				int32_t nextD = ceilf(posD);

				// Find minimum delta to next integer position
				float deltaA = (nextA >= 0) ? (nextA - posA) : 1;
				float deltaB = (nextB >= 0) ? (nextB - posB) : 1;
				float deltaC = (nextC >= 0) ? (nextC - posC) : 1;
				float deltaD = (nextD >= 0) ? (nextD - posD) : 1;

				float minDelta = deltaA;
				int channel = 0; // 0:A, 1:B, 2:C, 3:D
				if (deltaB < minDelta) { minDelta = deltaB; channel = 1; }
				if (deltaC < minDelta) { minDelta = deltaC; channel = 2; }
				if (deltaD < minDelta) { minDelta = deltaD; channel = 3; }

				// Move all channels by minDelta
				posA += minDelta;
				posB += minDelta;
				posC += minDelta;
				posD += minDelta;

				// jetting
				switch (channel) {
					case 0:
						if (nextA >= 0 && nextA < size_x) {
							jettingInfo.data = (Jetting_t *)MS1_CH1_List;
							// jettingInfo.threadId = TBD
							osThreadFlagsSet(jettingTaskHandle, ALL_NEW_TASK);
							osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
							CHECK_STATE;
						}	
						break;
					case 1:
						if (nextB >= 0 && nextB < size_x) {
							jettingInfo.data = (Jetting_t *)(MS1_CH2_List[nextB]);
							// jettingInfo.threadId = TBD
							osThreadFlagsSet(jettingTaskHandle, ALL_NEW_TASK);
							osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
							CHECK_STATE;
						}
						break;
					case 2:
						if (nextC >= 0 && nextC < size_x) {
							jettingInfo.data = (Jetting_t *)(MS2_CH1_List[nextC]);
							// jettingInfo.threadId = TBD
							osThreadFlagsSet(jettingTaskHandle, ALL_NEW_TASK);
							osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
							CHECK_STATE;
						}
						break;
					case 3:
						if (nextD >= 0 && nextD < size_x) {
							jettingInfo.data = (Jetting_t *)(MS2_CH2_List[nextD]);
							// jettingInfo.threadId = TBD
							osThreadFlagsSet(jettingTaskHandle, ALL_NEW_TASK);
							osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
							CHECK_STATE;
						}
						break;
				}

				// Move To Next Position
				command.code = G110;
				command.param1 = 1;
				command.param2 = 30;
				command.param3 = minDelta * stepsize_x;
				currentIntCommandPtr = &command;
				osThreadFlagsSet(headerTaskHandle, ALL_NEW_TASK);
				osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
				CHECK_STATE;
			}			
		}
	}
	f_res = f_close(&myFile);
	if (f_res != FR_OK) {
		usb_printf("Failed to close file");
		EmergencyStop(GlobalStateError);
		// Handle error
		return;
	}
	f_res = f_mount(NULL, "0:", 1);
	if (f_res != FR_OK) {
		usb_printf("Failed to unmount filesystem");
		EmergencyStop(GlobalStateError);
		// Handle error
		return;
	}
}


void ReadFileList(void) {
	f_res = f_mount(&myFatFs, "0:", 1);
	if (f_res != FR_OK) {
		usb_printf("Failed to mount filesystem");
		// Handle error
		return;
	}
	f_res = f_opendir(&dir, "");
	if (f_res != FR_OK) {
		usb_printf("Failed to openDir");
		// Handle error
		return;
	}
	while(1) {
		f_res = f_readdir(&dir, &fno);
		if (f_res != FR_OK) {
			usb_printf("Failed to ReadDir");
			return;
		}
		if (fno.fname[0] == 0)
		{
			break;
		}
	}
	f_res = f_closedir(&dir);
	if (f_res != FR_OK) {
		usb_printf("Failed to CloseDir");
		// Handle error
		return;
	}
	
	f_res = f_mount(NULL, "0:", 1);
	if (f_res != FR_OK) {
		usb_printf("Failed to unmount filesystem");
		// Handle error
		return;
	}
}
