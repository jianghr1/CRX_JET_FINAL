#include "Print.h"
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "Comm.h"
#include "Jetting.h"
#include "Init.h"
#include "math.h"
#include "TMC2209.h"

#define MOTOR_X_MM_TO_ESTEP 2.502f
extern osThreadId_t defaultTaskHandle;
extern osThreadId_t motorTaskHandle;
extern osThreadId_t pumpTaskHandle;
extern osThreadId_t vacTaskHandle;
extern osThreadId_t headerTaskHandle;
extern osThreadId_t jettingTaskHandle;

#define CHECK_STATE if ((currentState & 0xF) == GlobalStateEStop || (currentState & 0xF) == GlobalStateError) return

#define AD_gap 12.3613
#define AC_gap 11.811
#define AB_gap 0.5503

#define MS1_CH1_BUFSIZE   1
#define MS1_CH2_BUFSIZE  16
#define MS2_CH1_BUFSIZE 160
#define MS2_CH2_BUFSIZE 160

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

uint8_t MS1_CH1_List[MS1_CH1_BUFSIZE][42];
uint8_t MS1_CH2_List[MS1_CH2_BUFSIZE][42];
uint8_t MS2_CH1_List[MS2_CH1_BUFSIZE][42];
uint8_t MS2_CH2_List[MS2_CH2_BUFSIZE][42];

uint32_t idx;

uint16_t rcr_overwrite = 0;

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
		if (rx[idx] >= '0' && rx[idx] <= '9') {
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
		if (rx[idx] >= '0' && rx[idx] <= '9') {
			*out = *out + (rx[idx] & 0xF) * mul;
			mul *= 0.1f;
			++idx;
		} else {
			return;
		}
	}
}

void readLine(void) {
	uint8_t* MS1_CH1 = MS1_CH1_List[x%MS1_CH1_BUFSIZE];
	uint8_t* MS1_CH2 = MS1_CH2_List[x%MS1_CH2_BUFSIZE];
	uint8_t* MS2_CH1 = MS2_CH1_List[x%MS2_CH1_BUFSIZE];
	uint8_t* MS2_CH2 = MS2_CH2_List[x%MS2_CH2_BUFSIZE];

	MS2_CH2[0] = 0xA8; ++MS2_CH2;
	MS2_CH1[0] = 0xA9; ++MS2_CH1;
	MS1_CH2[0] = 0xAA; ++MS1_CH2;
	MS1_CH1[0] = 0xAB; ++MS1_CH1;
	
	for (y = 0; y < 40; y++) {
		MS1_CH1[y] = 0x00;
		MS1_CH2[y] = 0x00;
		MS2_CH1[y] = 0x00;
		MS2_CH2[y] = 0x00;
	}

	for (y = 0; y < size_y/2; y++, idx++) {
		if (idx == bytesRead)
		{
			f_res = f_read(&myFile, rx, sizeof(rx), &bytesRead);
			if (f_res != FR_OK) {
				usb_printf("[Print][Error] Failed to read file");
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
			MS1_CH1[y/8] |= 1<<(7-(y%8));
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
	decodeStr("\n");
}

void PrintTaskPrepare(void) {
	currentState = GlobalStatePrint;
	f_res = f_mount(&myFatFs, "0:", 1);
	if (f_res != FR_OK) {
		usb_printf("[Print][Error] Failed to mount filesystem\n");
		EmergencyStop(GlobalStateError);
		// Handle error
		return;
	}
	f_res = f_open(&myFile, globalInfo.fpath, FA_OPEN_EXISTING | FA_READ);
	if (f_res != FR_OK) {
		usb_printf("[Print][Error] Failed to open file\n");
		EmergencyStop(GlobalStateError);
		// Handle error
		return;
	}
	f_res = f_read(&myFile, rx, sizeof(rx), &bytesRead);
	if (f_res != FR_OK) {
		usb_printf("[Print][Error] Failed to read file\n");
		EmergencyStop(GlobalStateError);
		// Handle error
		return;
	}

	//Decode Header
	usb_printf("[Print][Info] Decoding Header\n");
	decodeHeader();
	usb_printf("[Print][Info] Initing\n");
	z = 0;
	//Init EveryThing
	InitTask();

	static GMCommand_t command;
}

void PrintTask(void) {
	currentState = GlobalStatePrint;
	static GMCommand_t command;
	// Heater Set To 70 degree
	usb_printf("[Print][Info] Heating\n");
	command.code = M120;
	command.param1 = 70;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(headerTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	for (; z < size_z; z++) {
		if (currentState & GlobalStatePauseReq) {
			usb_printf("[Print][Info] Reciev Pause Command!\n");
			usb_printf("OK\n");
			currentState = GlobalStatePause;
			return;
		}
		// Move To Next Position
		usb_printf("[Print][Info] Moving To Start\n");
		globalInfo.x_target_pos = zeropos_x;
		command.code = G110;
		command.param1 = 0;
		command.param2 = 50;
		command.param3 = 0;
		currentIntCommandPtr = &command;
		osThreadFlagsSet(motorTaskHandle, ALL_NEW_TASK);
		osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
		CHECK_STATE;
		osDelay(500);
		jettingInfo.threadId = defaultTaskHandle;
		TMC_setSpeed(TMC_MX, MOTOR_X_MM_TO_ESTEP * stepsize_x * 240);
		float rcr_per_step = TMC_MX->stepDivision * MOTOR_X_MM_TO_ESTEP * stepsize_x * 2;
		int32_t rcr_total = 0;
		rcr_overwrite = rcr_per_step;
		osThreadFlagsClear(MAIN_TASK_CPLT);
		TMC_move(TMC_MX, - stepsize_x * MOTOR_X_MM_TO_ESTEP * (size_x + AD_gap/stepsize_x));
		for (x = 0; x < size_x + AD_gap/stepsize_x; x++) {
			if (x==150) {
				CHECK_STATE;
			}
			int32_t posA = x;
			int32_t posB = x - AB_gap / stepsize_x;
			int32_t posC = x - AC_gap / stepsize_x;
			int32_t posD = x - AD_gap / stepsize_x;
			
			if (x < size_x) {
				readLine();
			}
			osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
			CHECK_STATE;
			if (posA > 0 && posA < size_x)
			{
				jettingInfo.data = (Jetting_t *)MS1_CH1_List[posA%MS1_CH1_BUFSIZE];
				osThreadFlagsSet(jettingTaskHandle, ALL_NEW_TASK);
				osThreadFlagsWait(JETTING_FPGA_REPLY|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
			}
			if (posB > 0 && posB < size_x)
			{
				jettingInfo.data = (Jetting_t *)MS1_CH1_List[posB%MS1_CH2_BUFSIZE];
				osThreadFlagsSet(jettingTaskHandle, ALL_NEW_TASK);
				osThreadFlagsWait(JETTING_FPGA_REPLY|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
				CHECK_STATE;
			}
			if (posC > 0 && posC < size_x)
			{
				jettingInfo.data = (Jetting_t *)MS1_CH1_List[posC%MS2_CH1_BUFSIZE];
				osThreadFlagsSet(jettingTaskHandle, ALL_NEW_TASK);
				osThreadFlagsWait(JETTING_FPGA_REPLY|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
				CHECK_STATE;
			}
			if (posD > 0 && posD < size_x)
			{
				jettingInfo.data = (Jetting_t *)MS1_CH1_List[posD%MS2_CH2_BUFSIZE];
				osThreadFlagsSet(jettingTaskHandle, ALL_NEW_TASK);
				osThreadFlagsWait(JETTING_FPGA_REPLY|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
				CHECK_STATE;
			}
			rcr_total = rcr_total + rcr_overwrite;
			rcr_overwrite = (rcr_per_step * (x + 2)) - rcr_total;
		}
		rcr_overwrite = 0;
	}
	
	f_res = f_close(&myFile);
	if (f_res != FR_OK) {
		usb_printf("[Print][Error] Failed to close file\n");
		EmergencyStop(GlobalStateError);
		// Handle error
		return;
	}
	f_res = f_mount(NULL, "0:", 1);
	if (f_res != FR_OK) {
		usb_printf("[Print][Error] Failed to unmount filesystem\n");
		EmergencyStop(GlobalStateError);
		// Handle error
		return;
	}
	currentState = GlobalStateIdle;
}


void ReadFileList(void) {
	f_res = f_mount(&myFatFs, "0:", 1);
	if (f_res != FR_OK) {
		usb_printf("[Print][Error] Failed to mount filesystem\n");
		// Handle error
		return;
	}
	f_res = f_opendir(&dir, "");
	if (f_res != FR_OK) {
		usb_printf("[Print][Error] Failed to openDir\n");
		// Handle error
		return;
	}
	f_res = f_readdir(&dir, &fno);
	if (f_res != FR_OK) {
		usb_printf("[Print][Error] Failed to ReadDir\n");
		return;
	}
	if (fno.fname[0] == 0) {
		usb_printf("[Print][Info] No files found\n");
		return;
	}
	usb_printf("%s", fno.fname);
	while(1) {
		f_res = f_readdir(&dir, &fno);
		if (f_res != FR_OK) {
			usb_printf("[Print][Error] Failed to ReadDir\n");
			return;
		}
		else if (fno.fname[0] == 0)
		{
			break;
		}
		else {
			usb_printf(", %s", fno.fname);
		}
	}
	usb_printf("\n");
	f_res = f_closedir(&dir);
	if (f_res != FR_OK) {
		usb_printf("[Print][Error] Failed to CloseDir\n");
		// Handle error
		return;
	}
	
	f_res = f_mount(NULL, "0:", 1);
	if (f_res != FR_OK) {
		usb_printf("[Print][Error] Failed to unmount filesystem\n");
		// Handle error
		return;
	}
}
