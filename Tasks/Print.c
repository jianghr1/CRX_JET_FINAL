#include "Print.h"
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "Comm.h"
#include "Jetting.h"
#include "Init.h"

extern osThreadId_t motorTaskHandle;
extern osThreadId_t pumpTaskHandle;
extern osThreadId_t vacTaskHandle;
extern osThreadId_t headerTaskHandle;
extern osThreadId_t jettingTaskHandle;

#define CHECK_STATE if ((currentState & 0xF) == GlobalStateEStop || (currentState & 0xF) == GlobalStateError) return


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
			++idx;
		} else if (rx[idx] == '.') {
			++idx;
			break;
		} else {
			return;
		}
	}
	while (idx < bytesRead) {
		if (rx[idx] > '0' && rx[idx] < '9') {
			*out = *out + (rx[idx] & 0xF) * mul;
			mul *= 0.1f;
			++idx;
		} else {
			return;
		}
	}
}

void readLine(void) {
	uint8_t* MS1_CH1 = MS1_CH1_List;
	uint8_t* MS1_CH2 = MS1_CH2_List[x%16];
	uint8_t* MS2_CH1 = MS2_CH1_List[x%64];
	uint8_t* MS2_CH2 = MS2_CH2_List[x%64];

	MS2_CH2[0] = 0xA8; ++MS2_CH2;
	MS2_CH1[0] = 0xA9; ++MS2_CH1;
	MS1_CH2[0] = 0xAA; ++MS1_CH2;
	MS1_CH1[0] = 0xAB; ++MS1_CH1;
	
	for (y = 0; y < 40; y++) {
		MS1_CH1[y] = 0;
		MS1_CH2[y] = 0;
		MS2_CH1[y] = 0;
		MS2_CH2[y] = 0;
	}

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
			MS2_CH1[y/8] |= 1<<(7-(y%8));
		} else {
			// MS2
			MS1_CH1[y/4] |= 1<<(7-(y%8));
		}
		
		if ((rx[idx] & 0xF) == 0) {
			// MS1
			MS2_CH2[y/8] |= 1<<(7-(y%8));
		} else {
			// MS2
			MS1_CH2[y/8] |= 1<<(7-(y%8));
		}
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

		for (x = 0; x < size_x; x++) {
			readLine();
			CHECK_STATE;
			// currently, We only Consider Channel A
			jettingInfo.data = (Jetting_t *)MS1_CH1_List;
			// jettingInfo.threadId = TBD
			osThreadFlagsSet(jettingTaskHandle, ALL_NEW_TASK);
			osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
			CHECK_STATE;
			
			// Move To Next Position
			command.code = G110;
			command.param1 = 1;
			command.param2 = 30;
			command.param3 = stepsize_x;
			currentIntCommandPtr = &command;
			osThreadFlagsSet(headerTaskHandle, ALL_NEW_TASK);
			osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
			CHECK_STATE;
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
