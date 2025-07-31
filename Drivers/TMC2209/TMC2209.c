/*___________________________________________________________________________________________________

Title:
	TMC2209.c
	
File version: 1.2
Project version: 1.2

Description:
	Library for the TMC2209 stepper motor driver.

Author:
 	Liviu Istrate
	istrateliviu24@yahoo.com
	www.programming-electronics-diy.xyz

Donate:
	Software development takes time and effort so if you find this useful consider a small donation at:
	https://www.paypal.com/donate/?hosted_button_id=L5ZL3SAN6NABY
_____________________________________________________________________________________________________*/


/* ----------------------------- LICENSE - GNU GPL v3 -----------------------------------------------

* This license must be included in any redistribution.

* Copyright (C) 2023 Liviu Istrate, www.programming-electronics-diy.xyz (istrateliviu24@yahoo.com)

* Project URL: https://www.programming-electronics-diy.xyz/2023/12/library-for-tmc2209-driver-avr.html

* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program. If not, see <https://www.gnu.org/licenses/>.

--------------------------------- END OF LICENSE --------------------------------------------------*/

#include "TMC2209.h"
#include "main.h"
#include "Comm.h"
#include "cmsis_os.h"

//=======================================================================================
// Functions
//=======================================================================================
static uint8_t TMC_readRegister(TMC* self, uint8_t reg_addr);
static uint8_t TMC_writeRegister(TMC* self, uint8_t reg_addr, uint32_t data);
uint8_t _swuart_calcCRC(uint8_t* datagram, uint8_t datagramLength);
static uint32_t _TMC_read32bit(void);


extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;


extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim8;

replyDatagram_t replyDatagram;

TMC_TIM tmc_timers[3] = {
	/* TIMER 1 Setting */
	{
		.htim=&htim1,
		.target_speed=1024,
		.acceleration=256,
		.deceleration=256,
	},
	/* TIMER 4 Setting */
	{
		.htim=&htim4,
		.target_speed=1024,
		.acceleration=256,
		.deceleration=256,
	},
	/* TIMER 8 Setting */
	{
		.htim=&htim8,
		.target_speed=1024,
		.acceleration=256,
		.deceleration=256,
	},
};

TMC tmc_drivers[8] = {
	/* MX Settings */
	{
		.address=0,
		.huart=&huart1,
		.tim_ptr=&tmc_timers[TMC_TIMER1],
		.tim_channel=0
	},
	/* VAC Settings */
	{
		.address=1,
		.huart=&huart1,
		.tim_ptr=&tmc_timers[TMC_TIMER8],
		.tim_channel=0
	},
	/* MZ1 Settings */
	{
		.address=2,
		.huart=&huart1,
		.tim_ptr=&tmc_timers[TMC_TIMER8],
		.tim_channel=1
	},
	/* MZ2 Settings */
	{
		.address=3,
		.huart=&huart1,
		.tim_ptr=&tmc_timers[TMC_TIMER8],
		.tim_channel=3
	},
	/* QJ Settings */
	{
		.address=0,
		.huart=&huart3,
		.tim_ptr=&tmc_timers[TMC_TIMER4],
		.tim_channel=1
	},
	/* FY Settings */
	{
		.address=1,
		.huart=&huart3,
		.tim_ptr=&tmc_timers[TMC_TIMER4],
		.tim_channel=0
	},
	/* MS2 Settings */
	{
		.address=2,
		.huart=&huart3,
		.tim_ptr=&tmc_timers[TMC_TIMER4],
		.tim_channel=3
	},
	/* MS1 Settings */
	{
		.address=3,
		.huart=&huart3,
		.tim_ptr=&tmc_timers[TMC_TIMER4],
		.tim_channel=2
	}
};

static uint8_t TMC_readRegister(TMC* self, uint8_t reg_addr){
	uint32_t timeout = TMC_UART_TX_TIMEOUT;
	static uint8_t datagram[DATAGRAM_READ_REQUEST_SIZE];
	// In case of a communication error, the request will resent x times
	uint8_t retry = 2;
	RETRY:
	
	// Sync byte
	datagram[0] = 0x05; // 0b0000 0101
	
	// Device address (0...3)
	datagram[1] = self->address;
	
	// 7-bit register address and RW bit
	datagram[2] = reg_addr;
	
	// CRC
	datagram[3] = _swuart_calcCRC(datagram, DATAGRAM_READ_REQUEST_SIZE);
	
	if (HAL_HalfDuplex_EnableTransmitter(self->huart) != HAL_OK)
	{
		goto COMM_ERROR;
	}
	osDelay(1);
	if (HAL_UART_Transmit(self->huart, datagram, DATAGRAM_READ_REQUEST_SIZE, timeout) != HAL_OK)
	{
		goto COMM_ERROR;
	}
	
	if (HAL_HalfDuplex_EnableReceiver(self->huart) != HAL_OK)
	{
		goto COMM_ERROR;
	}
	
	if (HAL_UART_Receive(self->huart, replyDatagram.bytes, DATAGRAM_READ_RESPONSE_SIZE, timeout) != HAL_OK)
	{
		goto COMM_ERROR;
	}
	if(_swuart_calcCRC(replyDatagram.bytes, DATAGRAM_READ_RESPONSE_SIZE) != replyDatagram.reply.crc)
	{
		goto COMM_ERROR;
	}
	
	self->comm_failure = false;
	return 0;
	
	COMM_ERROR:
		if(retry--) goto RETRY;
		self->comm_failure = true;
		EmergencyStop(GlobalStateError);
		return 1;
}


static uint8_t TMC_writeRegister(TMC* self, uint8_t reg_addr, uint32_t data){
	uint32_t timeout = TMC_UART_TX_TIMEOUT;
	static uint8_t datagram[DATAGRAM_WRITE_REQUEST_SIZE];
		
	// In case of a communication error, the request will resent x times
	uint8_t retry = 2;
	uint8_t write_counter_buf;
	
	RETRY:
		
	// Read interface transmission counter.
	// This register becomes incremented with each successful UART interface write access.
	// Read out to check the serial transmission for lost data. Read accesses do not change the content.
	// The counter wraps around from 255 to 0.
	if(TMC_readRegister(self, TMC_REG_IFCNT)) goto COMM_ERROR;
	write_counter_buf = replyDatagram.reply.byte0;
	
	// Sync byte
	datagram[0] = 0x05; // 0b0000 0101
	
	// Device address (0...3)
	datagram[1] = self->address;
	
	// 7-bit register address and RW bit
	datagram[2] = (reg_addr | 0x80);
	
	// 32 bit data
	datagram[3] = data >> 24; // 4th byte
	datagram[4] = data >> 16; // 3rd byte
	datagram[5] = data >> 8; // 2nd byte
	datagram[6] = data; // 1st byte
	
	// CRC
	datagram[7] = _swuart_calcCRC(datagram, DATAGRAM_WRITE_REQUEST_SIZE);
	
	if (HAL_HalfDuplex_EnableTransmitter(self->huart) != HAL_OK)
	{
		goto COMM_ERROR;
	}
	
	if (HAL_UART_Transmit(self->huart, datagram, DATAGRAM_WRITE_REQUEST_SIZE, timeout) != HAL_OK)
	{
		goto COMM_ERROR;
	}
	
	if (HAL_HalfDuplex_EnableReceiver(self->huart) != HAL_OK)
	{
		goto COMM_ERROR;
	}
	
	self->comm_failure = false;
	
	if(TMC_readRegister(self, TMC_REG_IFCNT)) goto COMM_ERROR;
	
	if(replyDatagram.reply.byte0 == write_counter_buf + 1){
		return 0;
	}else if(replyDatagram.reply.byte0 == 0 && write_counter_buf == 255){
		return 0;
	}
	
	COMM_ERROR:
		if(retry--) goto RETRY;
		self->comm_failure = true;
		EmergencyStop(GlobalStateError);
		return 1;
}


/*-----------------------------------------------------------------------------
	An 8 bit CRC polynomial is used for checking both read and write access. 
	It allows detection of up to eight single bit errors. The CRC8-ATM 
	polynomial with an initial value of zero is applied LSB to MSB,	
	including the sync- and addressing byte.

	datagram			array containing the datagram bytes
	datagramLength		array length
	
	returns the crc
------------------------------------------------------------------------------*/
uint8_t _swuart_calcCRC(uint8_t* datagram, uint8_t datagramLength){
	uint8_t i, j;
	uint8_t currentByte;
	uint8_t crc = 0;
	
	for(i = 0; i < (datagramLength - 1); i++){ // Execute for all bytes of a message
		currentByte = datagram[i]; // Retrieve a byte to be sent from Array
		
		for(j = 0; j < 8; j++){
			if((crc >> 7) ^ (currentByte & 0x01)){ // update CRC based result of XOR operation
				crc = (crc << 1) ^ 0x07;
			}else{
				crc = (crc << 1);
			}
			currentByte = currentByte >> 1;
		} // for CRC bit
	} // for message byte
	
	return crc;
}


static uint32_t _TMC_read32bit(void){
	uint32_t data = replyDatagram.reply.byte0;
	data |= replyDatagram.reply.byte1 << 8;
	data |= (uint32_t)replyDatagram.reply.byte2 << 16;
	data |= (uint32_t)replyDatagram.reply.byte3 << 24;
	return data;
}

/*-------------------------------------------------------------------------------------------
	* Used by addDriver function
	* Set the UART with the given Baud rate
	* Set register default values
	* Disable PDN_UART that controls standstill current reduction
	  because now this pin is being used for UART communication.
	* Select micro-step resolution using MSTEP register instead of MS1 and MS2 pins
	  because in UART communication they are used to set the device address.
	
	self						pointer to a structure
								Ex:
								TMC2209 motor_x;
								function(&motor_x);
	
	returns 1 in case of an error
--------------------------------------------------------------------------------------------*/
uint8_t TMC_init(TMC* self, uint16_t mres){
	
	// GCONF Register default values
	self->global_config.gconf.I_scale_analog =	TMC_GCONF_I_SCALE_ANALOG_DEFAULT;
	self->global_config.gconf.internal_Rsense =	TMC_GCONF_INTERNAL_RSENSE_DEFAULT;
	self->global_config.gconf.en_SpreadCycle =	TMC_GCONF_EN_SPREADCYCLE_DEFAULT;
	self->global_config.gconf.shaft =				TMC_GCONF_SHAFT_DIR_DEFAULT;
	self->global_config.gconf.index_otpw =		TMC_GCONF_INDEX_OTPW_DEFAULT;
	self->global_config.gconf.index_step =		TMC_GCONF_INDEX_STEP_DEFAULT;
	self->global_config.gconf.pdn_disable =		TMC_GCONF_PDN_DISABLE_DEFAULT;
	self->global_config.gconf.mstep_reg_select =	TMC_GCONF_MSTEP_REG_DEFAULT;
	self->global_config.gconf.multistep_filt =	TMC_GCONF_MULTISTEP_FILT_DEFAULT;
	self->global_config.gconf.test_mode =			TMC_GCONF_TEST_MODE_DEFAULT;
	
	// Write the updated GCONF register back to the driver.
	// Sets comm_failure if the UART fails to send or receive the request.
	if(TMC_writeRegister(self, TMC_REG_GCONF, self->global_config.bytes)) return 1;
	
	// CHOPCONF default values
	uint32_t chopconf = TMC_CHOPCONF_TOFF_DEFAULT;
	chopconf |= TMC_CHOPCONF_HSTRT_DEFAULT << TMC_CHOPCONF_HSTRT_OFFSET;
	chopconf |= TMC_CHOPCONF_HEND_DEFAULT << TMC_CHOPCONF_HEND_OFFSET;
	chopconf |= (uint32_t)TMC_CHOPCONF_TBL_DEFAULT << TMC_CHOPCONF_TBL_OFFSET;
	chopconf |= (uint32_t)TMC_CHOPCONF_VSENSE_DEFAULT << TMC_CHOPCONF_VSENSE_OFFSET;
	chopconf |= (uint32_t)TMC_CHOPCONF_INTPOL_DEFAULT << TMC_CHOPCONF_INTPOL_OFFSET;
	if(TMC_writeRegister(self, TMC_REG_CHOPCONF, chopconf)) return 1;
	
	// PWMCONF default values
	uint32_t pwmconf = TMC_PWMCONF_PWM_OFS_DEFAULT;
	pwmconf |= (uint32_t)TMC_PWMCONF_PWM_GRAD_DEFAULT << TMC_PWMCONF_PWM_GRAD_OFFSET;
	pwmconf |= (uint32_t)TMC_PWMCONF_PWM_FREQ_DEFAULT << TMC_PWMCONF_PWM_FREQ_OFFSET;
	pwmconf |= (uint32_t)TMC_PWMCONF_PWM_AUTOSCALE_DEFAULT << TMC_PWMCONF_PWM_AUTOSCALE_OFFSET;
	pwmconf |= (uint32_t)TMC_PWMCONF_PWM_AUTOGRAD_DEFAULT << TMC_PWMCONF_PWM_AUTOGRAD_OFFSET;
	pwmconf |= (uint32_t)TMC_PWMCONF_FREEWHEEL_DEFAULT << TMC_PWMCONF_FREEWHEEL_OFFSET;
	pwmconf |= (uint32_t)TMC_PWMCONF_PWM_REG_DEFAULT << TMC_PWMCONF_PWM_REG_OFFSET;
	pwmconf |= (uint32_t)TMC_PWMCONF_PWM_LIM_DEFAULT << TMC_PWMCONF_PWM_LIM_OFFSET;
	if(TMC_writeRegister(self, TMC_REG_PWMCONF, pwmconf)) return 1;
	TMC_setMicrostepResolution(self, mres);
	return 0;
}


/*-----------------------------------------------------------------------------------------------
	Returns IC version. 0x21 = first version of the IC.
-----------------------------------------------------------------------------------------------*/
uint8_t TMC_readVersion(TMC* self){
	TMC_readRegister(self, TMC_REG_IOIN);
	return replyDatagram.reply.byte3;
}


/*-----------------------------------------------------------------------------------------------
	Returns the 3 status bits of the GSTAT register and clears them on the device.
	Use the associated functions to check which flag is set.
-----------------------------------------------------------------------------------------------*/
uint8_t TMC_readStatus(TMC* driver){
	driver->gstat = 0; // reset local flags
	
	// Send a read request to the device and store the received data in replyDatagram.
	// Sets comm_failure if the UART fails to send or receive the request.
	if(TMC_readRegister(driver, TMC_REG_GSTAT)) return 0x80;
	
	// Update local status flags
	driver->gstat = replyDatagram.reply.byte0;
	
	// Clear flags on the device
	if(driver->gstat) TMC_writeRegister(driver, TMC_REG_GSTAT, 0x7);
	
	return driver->gstat;
}


/*-----------------------------------------------------------------------------------------------
	Returns 1 if the device has been reset since the last read. The initialization function
	should be used again to reconfigure the registers.
-----------------------------------------------------------------------------------------------*/
uint8_t TMC_statusIsReset(TMC* driver){
	return driver->gstat & TMC_REG_GSTAT_RESET;
}

/*-----------------------------------------------------------------------------------------------
	Returns 1 if the driver has been shut down due to over temperature or short circuit detection
	since the last read access. Read DRV_STATUS for details. The flag can only be cleared when all
	error conditions are cleared.
-----------------------------------------------------------------------------------------------*/
uint8_t TMC_statusDriverError(TMC* driver){
	return driver->gstat & TMC_REG_GSTAT_DRV_ERR;
}

/*-----------------------------------------------------------------------------------------------
	Returns 1 if an under voltage on the charge pump has been detected.
	The driver is disabled in this case. This flag is not latched and thus 
	does not need to be cleared.
-----------------------------------------------------------------------------------------------*/
uint8_t TMC_statusChargePumpUV(TMC* driver){
	return driver->gstat & TMC_REG_GSTAT_UV_CP;
}


/*-----------------------------------------------------------------------------------------------
	Returns 1 if the communication failure flag is set. This flag is set when CRC fails, 
	wrong number of bytes is received or write confirmation is not received.
	The flag is cleared automatically by read/write functions.
-----------------------------------------------------------------------------------------------*/
uint8_t TMC_deviceConnectionLost(TMC* driver){
	return driver->comm_failure;
}


uint8_t TMC_readPwmScaleSum(TMC* self){
	TMC_readRegister(self, TMC_REG_PWM_SCALE);
	return replyDatagram.reply.byte0;
}


int16_t TMC_readPwmScaleAuto(TMC* self){
	TMC_readRegister(self, TMC_REG_PWM_SCALE);
	
	int16_t data = replyDatagram.reply.byte2;
	data |= (replyDatagram.reply.byte3 & 1) << 8;
	return data;
}


uint8_t TMC_readPwmOfsAuto(TMC* self){
	TMC_readRegister(self, TMC_REG_PWM_AUTO);
	return replyDatagram.reply.byte0;
}


uint8_t TMC_readPwmGradAuto(TMC* self){
	TMC_readRegister(self, TMC_REG_PWM_AUTO);
	return replyDatagram.reply.byte2;
}


uint16_t TMC_readStallGuard(TMC* self){
	TMC_readRegister(self, TMC_REG_SG_RESULT);
	
	// Bits 9 and 0 will always	show 0.
	// Scaling to 10 bit is for compatibility to StallGuard2.
	uint16_t data = replyDatagram.reply.byte0 >> 1;
	data |= replyDatagram.reply.byte1 << 8;
	return data;
}


uint32_t TMC_readTstep(TMC* self){
	TMC_readRegister(self, TMC_REG_TSTEP);
	return _TMC_read32bit();
}


uint32_t TMC_readDrvStat(TMC* self){
	// Send a read request to the device and store the received data in replyDatagram.
	// Sets comm_failure if the UART fails to send or receive the request.
	if(TMC_readRegister(self, TMC_REG_DRV_STATUS)) return 0;
	
	// Update local status register
	self->drv_stat = _TMC_read32bit();
	
	return self->drv_stat;
}


bool TMC_checkStandstil(TMC* self){
	return self->drv_stat >> TMC_DRV_ST_STANDSTILL;	
}


bool TMC_checkChopper(TMC* self){
	return self->drv_stat >> TMC_DRV_ST_CHOPPER;	
}


// Irun and Ihold
uint8_t TMC_checkCS(TMC* self){
	return (self->drv_stat >> TMC_DRV_ST_CS_ACTUAL) & 0x1F;
}


uint8_t TMC_checkTempFlags(TMC* self){
	return (self->drv_stat >> TMC_DRV_ST_TEMP) & 0x0F;
}


uint8_t TMC_checkOpenLoad(TMC* self){
	return (self->drv_stat >> TMC_DRV_ST_OPEN_LOAD) & 0x03;
}


uint8_t TMC_checkLowSideShort(TMC* self){
	return (self->drv_stat >> TMC_DRV_ST_LOW_SIDE_SHORT) & 0x03;	
}


uint8_t TMC_checkShortToGnd(TMC* self){
	return (self->drv_stat >> TMC_DRV_ST_SHORT_TO_GND) & 0x03;
}


uint8_t TMC_checkOverTemp(TMC* self){
	return (self->drv_stat >> TMC_DRV_ST_OVER_TEMP_LIMIT) & 0x01;
}


uint8_t TMC_checkOverTempPreWarning(TMC* self){
	return self->drv_stat & 0x01;
}


void TMC_setMicrostepResolution(TMC* self, uint16_t mstep_res){
	if(TMC_readRegister(self, TMC_REG_CHOPCONF)) return;
	
	uint32_t data = _TMC_read32bit(); // read and buffer register
	data &= ~((uint32_t)0x0F << TMC_CHOPCONF_MRES_OFFSET); // clear 4 bits
	data |= ((uint32_t)mstep_res << TMC_CHOPCONF_MRES_OFFSET); // set 4 bits
	
	TMC_writeRegister(self, TMC_REG_CHOPCONF, data);
	
	if(mstep_res){
		// Set micro step resolution to: 128, 64, 32, 16, 8, 4, 2, 1
		self->stepDivision = 0x80 >> (mstep_res - 1);
	}else{
		self->stepDivision = 256;
	}
	
}


void TMC_setSpeed(TMC* self, float erps){
	self->tim_ptr->target_speed = erps * self->stepDivision;
	if (self->tim_ptr->target_speed > 0){
		uint32_t arr = TMC_TIMER_IFREQ / self->tim_ptr->target_speed;
		self->tim_ptr->htim->Instance->ARR = (arr > 0xFFFF) ? 0xFFFF : arr;
	}
}

void TMC_setAcceleration(TMC* self, float acceleration){
	self->tim_ptr->acceleration = acceleration * self->stepDivision;
}

void TMC_setDeceleration(TMC* self, float deceleration){
	self->tim_ptr->deceleration = deceleration * self->stepDivision;
}


/*-----------------------------------------------------------------------------------------------
	Inverse motor direction.

	driver				pointer to a structure
						Ex: 
						TMC2209 motor_x;
						function(&motor_x);
						
	dir					direction
						DIRECTION_CW = 1 = clockwise
						DIRECTION_CCW = -1 = counter-clockwise
-----------------------------------------------------------------------------------------------*/
void TMC_setDirection(TMC* driver, int8_t dir){
	// DIRECTION_CW and DIRECTION_CCW are defined as 1 and -1 for faster increment
	// so this is to adapt to the stepper driver that requires 0 or 1
	if(dir == DIRECTION_CW) dir = 0;
	else if(dir == DIRECTION_CCW) dir = 1;
	driver->global_config.gconf.shaft = dir;
	TMC_writeRegister(driver, TMC_REG_GCONF, driver->global_config.bytes);
	driver->tim_ptr->steps[driver->tim_channel] = 0;
	driver->tim_ptr->current_speed = 0;
}


/*-----------------------------------------------------------------------------------------------
	Switch between StealthChop and SpreadCycle.

	driver				pointer to a structure
						Ex: 
						TMC2209 motor_x;
						function(&motor_x);
						
	chopper				one of the following two macros: StealthChop or SpreadCycle
-----------------------------------------------------------------------------------------------*/
void TMC_setChopper(TMC* driver, bool chopper){
	driver->global_config.gconf.en_SpreadCycle = chopper;
	TMC_writeRegister(driver, TMC_REG_GCONF, driver->global_config.bytes);
}


/*-----------------------------------------------------------------------------------------------
	Writes TPWMTHRS register. When the velocity exceeds the limit set by TPWMTHRS, 
	the driver switches to SpreadCycle. 
	Set TPWMTHRS zero if you want to work with StealthChop only. 
	RPM is converted to TSTEP.
	
	rpm_threshold:	velocity threshold in RPM. When the motor velocity exceeds this threshold, 
					the driver switches to SpreadCycle.
					
	Return:			calculated TSTEP value written to the register.
-----------------------------------------------------------------------------------------------*/
void TMC_spreadCycleThreshold(TMC* driver, uint32_t tstep){
	TMC_writeRegister(driver, TMC_REG_TPWMTHRS, tstep);
}


/*-----------------------------------------------------------------------------------------------
	Used to set the motor run and standstill current.
						
	irun				IRUN (Reset default=31). Motor run current (0=1/32 � 31=32/32).
						Hint: Choose sense resistors in a way, that normal IRUN is 16 to 31 
						for best micro-step performance.
						
	ihold				IHOLD (Reset default: OTP). Standstill current (0=1/32 � 31=32/32).
						In combination with StealthChop mode, setting IHOLD=0 allows to choose 
						freewheeling or coil short circuit (passive braking) for motor stand still.
						
	ihold_delay			IHOLDDELAY (Reset default: OTP). Controls the number of clock cycles for motor
						power down after standstill is detected (stst=1) and TPOWERDOWN has expired. 
						The smooth transition avoids a motor jerk upon power down.
						0: instant power down
						1..15: Delay per current reduction step in multiple of 2^18 clocks
-----------------------------------------------------------------------------------------------*/
void TMC_setIrunIhold(TMC* driver, uint8_t irun, uint8_t ihold, uint8_t ihold_delay){
	uint32_t data = (uint32_t)ihold_delay << 16;
	data |= (irun << 8);
	data |= ihold;
	TMC_writeRegister(driver, TMC_REG_IHOLD_IRUN, data);
}


/*-----------------------------------------------------------------------------------------------
	Sets the delay time from stand still (stst) detection to motor current power down. 
	Time range is about 0 to 5.6 seconds. 0�((2^8)-1) * 2^18 tCLK.
	Attention: A minimum setting of 2 is required to allow automatic tuning of StealthChop PWM_OFFS_AUTO.
	
	value = time(s) / (1 / 12MHz) / 2^18 // 1 / 12MHz is tclk
	time(s) = value * 2^18 * tclk
						
	value				0 to 255 (255 is 5.6 seconds). Reset default = 20
-----------------------------------------------------------------------------------------------*/
void TMC_setTPowerDown(TMC* driver, uint8_t value){
	TMC_writeRegister(driver, TMC_REG_TPOWERDOWN, value);
}


/*-----------------------------------------------------------------------------------------------
	Detection threshold for stall. The StallGuard value SG_RESULT becomes compared to the double 
	of this threshold.

	Note: Sensor-less homing/probing is not recommend for machines that use a lead screws due to high torque.

	value:		SGTHRS value (0...255). This value controls the StallGuard4 threshold level 
				for stall detection. It compensates for motor specific characteristics and 
				controls sensitivity. A higher value gives a higher sensitivity. 
				A higher value makes StallGuard4 more sensitive and requires less torque to 
				indicate a stall. The stall output becomes active if SG_RESULT fall below this value. 
				See datasheet chapter 11 for details.
-----------------------------------------------------------------------------------------------*/
void TMC_setStallGuard(TMC* self, uint8_t value){
	TMC_writeRegister(self, TMC_REG_SGTHRS, value);
}


/*-----------------------------------------------------------------------------------------------
	rpm_threshold:	velocity threshold in RPM. When the motor velocity exceeds this threshold, 
					the driver enables CoolStep and stall output. 
					See datasheet chapter 12 for details.
-----------------------------------------------------------------------------------------------*/
void TMC_coolStepThreshold(TMC* self, uint32_t tstep){
	TMC_writeRegister(self, TMC_REG_TCOOLTHRS, tstep);
}


void TMC_setCoolStep(TMC* self, uint16_t semin, uint8_t semax, uint8_t seup, uint8_t sedn, uint8_t seimin){
	semin = 1.0 / 16.0 * semin + 1.0;
	
	uint16_t coolstep = semin & 0x0F;
	coolstep |= (seup & 0x03) << 5;
	coolstep |= (semax & 0x0F) << 8;
	coolstep |= (sedn & 0x03) << 13;
	coolstep |= (seimin & 0x01) << 15;
	
	TMC_writeRegister(self, TMC_REG_COOLCONF, coolstep);
}


/*-----------------------------------------------------------------------------------------------
	To save micro controller pins, the enable pin can be connected to ground, 
	thus keeping the drive enabled and then disable the driver using this function. 
	Writes TOFF bits inside the CHOPCONF register.
	
	enable:		0 - disable the driver by setting TOFF to 0
				1 - restores TOFF off time with previous values
-----------------------------------------------------------------------------------------------*/
void TMC_softEnable(TMC* self, bool enable){
	self->tim_ptr->steps[self->tim_channel] = 0;
	if(TMC_readRegister(self, TMC_REG_CHOPCONF)) return;
	
	uint32_t data = _TMC_read32bit(); // read and buffer register
	
	if(enable){
		data |= (TMC_CHOPCONF_TOFF_DEFAULT);
	}else{
		data &= ~(0x0F); // clear first 4 lower bits
	}
	
	TMC_writeRegister(self, TMC_REG_CHOPCONF, data);
}

void TMC_reset(TMC* self){
	self->tim_ptr->steps[self->tim_channel] = 0;
	self->current_pos = 0;
}

void TMC_moveTo(TMC* self, float absolute){
	TMC_move(self, absolute - self->current_pos);
	TMC_wait_motor_stop(self);
}

void TMC_move(TMC* self, float relative){
	if (relative == 0)
	{
		return;
	}
	else if (relative > 0 && self->global_config.gconf.shaft == 0)
	{
		self->tim_ptr->steps[self->tim_channel] += relative * self->stepDivision * 2;
		self->current_pos += relative;
	}
	else if (relative < 0 && self->global_config.gconf.shaft == 1)
	{
		self->tim_ptr->steps[self->tim_channel] += (-relative) * self->stepDivision * 2;
		self->current_pos += relative;
	}
	else
	{
		// Wait until stop
		TMC_wait_motor_stop(self);
		if (relative > 0)
		{
			TMC_setDirection(self, 1);
			self->tim_ptr->steps[self->tim_channel] += relative * self->stepDivision * 2;
			self->current_pos += relative;
		}
		else
		{
			TMC_setDirection(self, -1);
			self->tim_ptr->steps[self->tim_channel] += (-relative) * self->stepDivision * 2;
			self->current_pos += relative;
		}
	}
}

void TMC_wait_motor_stop(TMC* self){
	while(self->tim_ptr->steps[self->tim_channel])
	{
		osThreadFlagsWait(ALL_EMG_STOP, osFlagsWaitAny, 1);
	}
}

/*-----------------------------------------------------------------------------------------------
	Automatic Tuning (AT) of StealthChop following AT#1 step from the datasheet (chapter 6.1). 
	This function could be used before homing or any other movement that must be done for AT#2.

	The function does the following:
	- Sets IHOLD same as IRUN
	- Sets TPOWERDOWN to a larger value to prevent automatic power down before tuning is done
	- Sets SpreadCycle threshold to 0 to prevent changing to SpreadCycle at threshold velocity
	- Enables the driver using EN pin or by software
	- Makes 1 step to exit a potential stand still mode
	- Wait for 130ms
	
	irun:		IRUN current scale value used in your application
 -----------------------------------------------------------------------------------------------*/
void TMC_stealthChopAT(TMC* self, uint8_t irun){
	// Requires standstill at IRUN for > 130ms in order to
	// a) detect standstill b) wait > 128 chopper cycles at IRUN and c) regulate PWM_OFS_AUTO
	TMC_setIrunIhold(self, irun, irun, 14);
	
	// Extend power down time to maintain IRUN current
	TMC_setTPowerDown(self, 200);
	
	// Prevent changing to SpreadCycle at a certain velocity
	TMC_spreadCycleThreshold(self, 0);
	
	// Enable the driver
	#if TMC_USE_HARDWARE_ENABLE_PIN == 1
		self->stepper->EN_f(true);
	#else
		TMC_softEnable(self, true);
	#endif
	
	// Make 1 step to exit a potential stand still mode
	//self->stepper->STEP_f();
	
	// Wait for a minimum of 130ms
	osThreadFlagsWait(ALL_EMG_STOP, osFlagsWaitAny, 200);
}


/*-----------------------------------------------------------------------------------------------
	Sets freewheel bits in PWMCONF register. Only available with StealthChop enabled. 
	The freewheeling option makes the motor easy movable, while both coil short options realize 
	a passive brake. See datasheet chapter 6.7 for details.

	Notes:
	- IHOLD must be set to 0 first except when using normal operation (mode 00).
	- Open load flag will be set
	- Driver must be enabled

	mode:	stand still option when motor current setting is zero (I_HOLD=0).
			00: Normal operation
			01: Freewheeling
			10: Coil shorted using LS drivers
			11: Coil shorted using HS drivers

			Available macros:

			// Freewheel bits
			NORMAL_OPERATION		0x00
			FREEWHEELING			0x01
			COIL_SHORTED_LS			0x02
			COIL_SHORTED_HS			0x03
 -----------------------------------------------------------------------------------------------*/
void TMC_setStandstillMode(TMC* self, uint8_t mode){
	if(TMC_readRegister(self, TMC_REG_PWMCONF)) return;
	
	uint32_t data = _TMC_read32bit();
	data &= ~((uint32_t)0x03 << TMC_PWMCONF_FREEWHEEL_OFFSET); // clear bits
	data |= ((uint32_t)mode << TMC_PWMCONF_FREEWHEEL_OFFSET); // set bits
	
	TMC_writeRegister(self, TMC_REG_PWMCONF, data);
}
