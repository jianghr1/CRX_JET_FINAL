#ifndef TMC_H_
#define TMC_H_

//=======================================================================================
// Include
//=======================================================================================
#include "stm32f4xx_hal.h"
#include "stdint.h"
#include "stdbool.h"
//=======================================================================================
// User Settings - Setup
//=======================================================================================
#define TMC_UART_TX_TIMEOUT						20

//=======================================================================================
// User Settings - Default Register Values in decimal system

// These values should only be changed if you know what you are doing.
// In most cases, the available functions are enough to configure most used register values.

//=======================================================================================

/*---------------------------------------------------------------------------------------
	GCONF Register Default Values
-----------------------------------------------------------------------------------------*/
// I_scale_analog (Reset default=1)
// 0: Use internal reference derived from 5VOUT
// 1: Use voltage supplied to VREF as current reference
#define TMC_GCONF_I_SCALE_ANALOG_DEFAULT		1 // [Default: 1]

// internal_Rsense (Reset default: OTP)
// 0: Operation with external sense resistors
// 1: Internal sense resistors. Use current supplied into
//    VREF as reference for internal sense resistor. 
//    VREF pin internally is driven to GND in this mode.
#define TMC_GCONF_INTERNAL_RSENSE_DEFAULT		0 // [Default: 0]

// en_SpreadCycle (Reset default: OTP)
// 0: StealthChop PWM mode enabled (depending on velocity thresholds). 
//    Initially switch from off to on state while in stand still, only.
// 1: SpreadCycle mode enabled. A high level on the pin SPREAD inverts this flag to
//    switch between both chopper modes.
#define TMC_GCONF_EN_SPREADCYCLE_DEFAULT		0 // [Default: 0]

// shaft
// 1: Inverse motor direction
#define TMC_GCONF_SHAFT_DIR_DEFAULT				0 // [Default: 0]

// index_otpw
// 0: INDEX shows the first micro step position of sequencer
// 1: INDEX pin outputs over-temperature prewarning flag (otpw) instead
#define TMC_GCONF_INDEX_OTPW_DEFAULT			0 // [Default: 0]

// index_step
// 0: INDEX output as selected by index_otpw
// 1: INDEX output shows step pulses from internal pulse generator (toggle upon each step)
#define TMC_GCONF_INDEX_STEP_DEFAULT			1 // [Default: 0]

// pdn_disable
// 0: PDN_UART controls standstill current reduction
// 1: PDN_UART input function disabled. Set this bit, when using the UART interface!
#define TMC_GCONF_PDN_DISABLE_DEFAULT			1 // [Default: 1]

// mstep_reg_select
// 0: Micro-step resolution selected by pins MS1, MS2
// 1: Micro-step resolution selected by MSTEP register
#define TMC_GCONF_MSTEP_REG_DEFAULT				1 // [Default: 1]

// multistep_filt (Reset default=1)
// 0: No filtering of STEP pulses
// 1: Software pulse generator optimization enabled when full-step frequency > 750Hz (roughly). 
// TSTEP shows filtered step time values when active.
#define TMC_GCONF_MULTISTEP_FILT_DEFAULT		1 // [Default: 1]

// test_mode
// 0: Normal operation
// 1: Enable analog test output on pin ENN (pull down resistor off), ENN treated as enabled.
//    IHOLD[1..0] selects the function of DCO:  0�2: T120, DAC, VDDH
//    Attention: Not for user, set to 0 for normal operation!
#define TMC_GCONF_TEST_MODE_DEFAULT				0 // [Default: 0]


/*---------------------------------------------------------------------------------------
	CHOPCONF Register Default Values
-----------------------------------------------------------------------------------------*/
// TOFF - bits[0:3] - TOFF off time and driver enable
// Off time setting controls duration of slow decay phase
// Sets the slow decay time (off time). This setting also limits the maximum chopper frequency.
// For operation with StealthChop, this parameter is not used, but it is required to enable the motor. 
// In case of operation with StealthChop only, any setting is OK.
// Setting this parameter to zero completely disables all driver transistors and the motor can free-wheel.
// 0...15 - (1 will work with minimum blank time of 24 clocks)
#define TMC_CHOPCONF_TOFF_DEFAULT				3 // [Default: 3] in StealthChop mode

// HSTRT - bits[4:6] - hysteresis start value added to HEND
// (Default: OTP, resp. 0 in StealthChop mode)
// 0...7 -> HSTRT=1�8
#define TMC_CHOPCONF_HSTRT_DEFAULT				0 // [Default: 0]

// HEND - bits[7:10] - hysteresis low value OFFSET sine wave offset
// (Default: OTP, resp. 5 in StealthChop mode)
// 0...2	->	-3�-1: negative HEND
// 3		->	0: zero HEND
// 4...15	->	1�12: positive HEND
#define TMC_CHOPCONF_HEND_DEFAULT				5 // [Default: 5]

// TBL - bits[15:16] - blank time select
// Set comparator blank time to 16, 24, 32 or 40 clocks
// Hint: %00 or %01 is recommended for most applications. (Default: OTP)
// Comparator blank time. This time needs to safely cover the switching event and the duration of the
// ringing on the sense resistor. For most applications, a setting of 1 or 2 is good. For highly
// capacitive loads, a setting of 2 or 3 will be required.
#define TMC_CHOPCONF_TBL_DEFAULT				2 // [Default: 2]. A wrong value will result in a high pitch motor noise.

// vsense - bit[17] - sense resistor voltage based current scaling
// 0: Low sensitivity, high sense resistor voltage
// 1: High sensitivity, low sense resistor voltage
#define TMC_CHOPCONF_VSENSE_DEFAULT				0 // [Default: 0]

// intpol - bit[28] - interpolation to 256 micro-steps
// 1: The actual micro-step resolution (MRES) becomes
// extrapolated to 256 micro-steps for smoothest motor operation. (Default: 1)
#define TMC_CHOPCONF_INTPOL_DEFAULT				1 // [Default: 1]

/*---------------------------------------------------------------------------------------
	PWMCONF Register Default Values
-----------------------------------------------------------------------------------------*/
// PWM_OFS - bits[0:7] - User defined amplitude (offset). Reset default=36
#define TMC_PWMCONF_PWM_OFS_DEFAULT				36 // [Default: 36]

// PWM_GRAD - bits[8:15] - User defined amplitude gradient
#define TMC_PWMCONF_PWM_GRAD_DEFAULT			20 // [Default: 20]

// pwm_freq - bits[16:17] - PWM frequency
#define TMC_PWMCONF_PWM_FREQ_DEFAULT			1 // [Default: 1] 35.1kHz with 12MHz internal clock

// pwm_autoscale - bit[18] - PWM automatic amplitude scaling. Reset default=1
#define TMC_PWMCONF_PWM_AUTOSCALE_DEFAULT		1 // [Default: 1]

// pwm_autograd - bit[19] - PWM automatic gradient adaptation. Reset default=1
#define TMC_PWMCONF_PWM_AUTOGRAD_DEFAULT		1 // [Default: 1]

// freewheel - bits[20:21] - Allows different standstill modes. Reset default=0
#define TMC_PWMCONF_FREEWHEEL_DEFAULT			0 // [Default: 0]

// PWM_REG - bits[24:27] - Regulation loop gradient
#define TMC_PWMCONF_PWM_REG_DEFAULT				8 // [Default: 8]

// PWM_LIM - bits[28:31] - PWM automatic scale 
// amplitude limit when switching on. (Default = 12)
#define TMC_PWMCONF_PWM_LIM_DEFAULT				4 // [Default: 12]


//=======================================================================================
// Macros
//=======================================================================================
#define TMC_FCLK							12000000.0
#define TMC_TCLK							(1.0 / TMC_FCLK)
#define TMC_STEP_GEN_DELAY					0.001

// Number of bytes for datagrams
#define DATAGRAM_READ_REQUEST_SIZE			4
#define DATAGRAM_READ_RESPONSE_SIZE			8
#define DATAGRAM_WRITE_REQUEST_SIZE			8

// GCONF bit
#define STEALTH_CHOP						0
#define SPREAD_CYCLE						1

// Chopper mode returned by DRV_STATUS
#define SPREAD_CYCLE_MODE					0
#define STEALTH_CHOP_MODE					1

// Freewheel bits
#define NORMAL_OPERATION					0x00
#define FREEWHEELING						0x01
#define COIL_SHORTED_LS						0x02
#define COIL_SHORTED_HS						0x03

// MRES (micro step resolution)
#define MRES_256							0x00
#define MRES_128							0x01
#define MRES_64								0x02
#define MRES_32								0x03
#define MRES_16								0x04
#define MRES_8								0x05
#define MRES_4								0x06
#define MRES_2								0x07
#define MRES_1								0x08

// CoolStep
// seup - Current increment steps per measured StallGuard2 value
#define SEUP_1								0
#define SEUP_2								1
#define SEUP_4								2
#define SEUP_8								3

// sedn - current down step speed. For each x StallGuard4 values, decrease the current by one.
#define SEDN_32								0
#define SEDN_8								1
#define SEDN_2								2
#define SEDN_1								3

// seimin - minimum current for smart current control
#define SEIMIN_1_2							0 // 1/2 of current setting (IRUN)
#define SEIMIN_1_4							1 // 1/4 of current setting (IRUN)

//=======================================================================================
// Register Addresses and Bit Offsets
//=======================================================================================
// GCONF - Global configuration flags (RW, 10 bits)
#define TMC_REG_GCONF						0x00

// GSTAT � Global status flags (RW, 3 bits)
#define TMC_REG_GSTAT						0x01
#define TMC_REG_GSTAT_RESET					0x01 // bit 1
#define TMC_REG_GSTAT_DRV_ERR				0x02 // bit 2
#define TMC_REG_GSTAT_UV_CP					0x04 // bit 3

// Interface transmission counter (R, 8 bits)
#define TMC_REG_IFCNT						0x02

// Input pins and IC version
#define TMC_REG_IOIN						0x06

// IHOLD_IRUN � Driver current control (W, 5 + 5 + 4)
#define TMC_REG_IHOLD_IRUN					0x10

// TPOWERDOWN (Reset default=20, W, 8 bits)
#define TMC_REG_TPOWERDOWN					0x11

// TSTEP (R, 20 bits)
#define TMC_REG_TSTEP						0x12

// TPWMTHRS (W, 20 bits)
#define TMC_REG_TPWMTHRS					0x13

// VACTUAL (W, 24 bits)
#define TMC_REG_VACTUAL						0x22

// MSCNT (R, 10 bits)
#define TMC_REG_MSCNT						0x6A

// CHOPCONF (RW, 32 bits)
#define TMC_REG_CHOPCONF					0x6C
#define TMC_CHOPCONF_HSTRT_OFFSET			4
#define TMC_CHOPCONF_HEND_OFFSET			7
#define TMC_CHOPCONF_TBL_OFFSET				15
#define TMC_CHOPCONF_VSENSE_OFFSET			17
#define TMC_CHOPCONF_MRES_OFFSET			24
#define TMC_CHOPCONF_INTPOL_OFFSET			28

// DRV_STATUS (R, 32 bits)
#define TMC_REG_DRV_STATUS					0x6F
#define TMC_DRV_ST_STANDSTILL				31
#define TMC_DRV_ST_CHOPPER					30
#define TMC_DRV_ST_CS_ACTUAL				16
#define TMC_DRV_ST_TEMP						8
#define TMC_DRV_ST_OPEN_LOAD				6
#define TMC_DRV_ST_LOW_SIDE_SHORT			4
#define TMC_DRV_ST_SHORT_TO_GND				2
#define TMC_DRV_ST_OVER_TEMP_LIMIT			1
#define TMC_DRV_ST_OVER_TEMP_PREWARNING		0

// PWMCONF (RW, 22 bits)
#define TMC_REG_PWMCONF						0x70
#define TMC_PWMCONF_PWM_OFS_OFFSET			0
#define TMC_PWMCONF_PWM_GRAD_OFFSET			8
#define TMC_PWMCONF_PWM_FREQ_OFFSET			16
#define TMC_PWMCONF_PWM_AUTOSCALE_OFFSET	18
#define TMC_PWMCONF_PWM_AUTOGRAD_OFFSET		19
#define TMC_PWMCONF_FREEWHEEL_OFFSET		20
#define TMC_PWMCONF_PWM_REG_OFFSET			24
#define TMC_PWMCONF_PWM_LIM_OFFSET			28

// PWM_SCALE (R, 9+8 bits)
#define TMC_REG_PWM_SCALE					0x71

// PWM_AUTO (R, 8+8 bits)
#define TMC_REG_PWM_AUTO					0x72

// TCOOLTHRS (W, 20 bits)
#define TMC_REG_TCOOLTHRS					0x14

// SGTHRS (W, 8 bits)
#define TMC_REG_SGTHRS						0x40

// SG_RESULT (R, 10 bits)
#define TMC_REG_SG_RESULT					0x41

// COOLCONF (W, 16 bits)
#define TMC_REG_COOLCONF					0x42

//=======================================================================================
// Structures
//=======================================================================================
// All registers become reset to 0 upon power up, unless otherwise noted.
// GCONF - Global configuration flags (RW, 10 bits)
typedef union {
	struct {
		// BIT 0
		uint32_t I_scale_analog : 1;
	
		// BIT 1
		uint32_t internal_Rsense : 1;
	
		// BIT 2
		uint32_t en_SpreadCycle : 1;
	
		// BIT 3
		uint32_t shaft : 1;
	
		// BIT 4
		uint32_t index_otpw : 1;
	
		// BIT 5
		uint32_t index_step : 1;
	
		// BIT 6
		uint32_t pdn_disable : 1;
	
		// BIT 7
		uint32_t mstep_reg_select : 1;
	
		// BIT 8
		uint32_t multistep_filt : 1;
	
		// BIT 9
		uint32_t test_mode : 1;
		
		uint32_t reserved : 22;
	}gconf;
	
	uint32_t bytes;
} GCONF;

typedef union{
	struct{
		uint64_t sync : 4;
		uint64_t reserved : 4;
		uint64_t serial_address : 8;
		uint64_t register_address : 7;
		uint64_t rw : 1;
		uint64_t byte3 : 8;
		uint64_t byte2 : 8;
		uint64_t byte1 : 8;
		uint64_t byte0 : 8;
		uint64_t crc : 8;
	}reply;
	
	uint8_t bytes[8];
} replyDatagram_t;

typedef struct {
	TIM_HandleTypeDef* htim;
	int32_t target_speed;
	int32_t current_speed;
	int32_t acceleration;
	int32_t deceleration;
	uint16_t HARR;
	uint32_t steps[4];
} TMC_TIM;

typedef struct {
	GCONF global_config;				// Global device configurations
	uint32_t drv_stat;
	UART_HandleTypeDef* huart;
	uint32_t stepDivision;
	TMC_TIM* tim_ptr;
	uint8_t address;					// Address of the driver [0 to 3]
	uint8_t gstat;						// GSTAT register flags
	uint8_t comm_failure;
	uint8_t tim_channel;
	float current_pos;
} TMC;

extern TMC tmc_drivers[8];
extern TMC_TIM tmc_timers[3];

//=======================================================================================
// Settings
//=======================================================================================

#define TMC_USE_HARDWARE_ENABLE_PIN 0
#define DIRECTION_CW 1
#define DIRECTION_CCW -1
#define TMC_TIMER_IFREQ 1000000
#define TMC_TIMER1 0
#define TMC_TIMER4 1
#define TMC_TIMER8 2

#define TMC_MX  (&tmc_drivers[0])
#define TMC_VAC (&tmc_drivers[1])
#define TMC_MZ1 (&tmc_drivers[2])
#define TMC_MZ2 (&tmc_drivers[3])
#define TMC_QJ  (&tmc_drivers[4])
#define TMC_FY  (&tmc_drivers[5])
#define TMC_MS2 (&tmc_drivers[6])
#define TMC_MS1 (&tmc_drivers[7])
//=======================================================================================
// Function Prototypes
//=======================================================================================

// Initialization functions
uint8_t TMC_init(TMC* self, uint16_t mres);

// Read values
uint8_t TMC_readVersion(TMC* self);
uint8_t TMC_readStatus(TMC* driver);
uint8_t TMC_statusIsReset(TMC* driver);
uint8_t TMC_statusDriverError(TMC* driver);
uint8_t TMC_statusChargePumpUV(TMC* driver);
uint8_t TMC_deviceConnectionLost(TMC* driver);
uint8_t TMC_readPwmScaleSum(TMC* self);
int16_t TMC_readPwmScaleAuto(TMC* self);
uint8_t TMC_readPwmOfsAuto(TMC* self);
uint8_t TMC_readPwmGradAuto(TMC* self);
uint16_t TMC_readStallGuard(TMC* self);
uint32_t TMC_readTstep(TMC* self);
uint32_t TMC_readDrvStat(TMC* self);
bool TMC_checkStandstil(TMC* self);
bool TMC_checkChopper(TMC* self);
uint8_t TMC_checkCS(TMC* self);
uint8_t TMC_checkTempFlags(TMC* self);
uint8_t TMC_checkOpenLoad(TMC* self);
uint8_t TMC_checkLowSideShort(TMC* self);
uint8_t TMC_checkShortToGnd(TMC* self);
uint8_t TMC_checkOverTemp(TMC* self);
uint8_t TMC_checkOverTempPreWarning(TMC* self);

// Set values
void TMC_setMicrostepResolution(TMC* self, uint16_t mstep_res);
void TMC_setSpeed(TMC* self, float erps);
void TMC_setAcceleration(TMC* self, float acceleration);
void TMC_setDeceleration(TMC* self, float deceleration);
void TMC_setDirection(TMC* self, int8_t dir);
void TMC_setChopper(TMC* self, bool chopper);
void TMC_spreadCycleThreshold(TMC* driver, uint32_t tstep);
void TMC_setIrunIhold(TMC* self, uint8_t irun, uint8_t ihold, uint8_t ihold_delay);
void TMC_setTPowerDown(TMC* self, uint8_t value);
void TMC_setStallGuard(TMC* self, uint8_t value);
void TMC_coolStepThreshold(TMC* self, uint32_t tstep);
void TMC_setCoolStep(TMC* self, uint16_t semin, uint8_t semax, uint8_t seup, uint8_t sedn, uint8_t seimin);
void TMC_softEnable(TMC* self, bool enable);

// Motion
void TMC_stop(TMC* self);
void TMC_moveTo(TMC* self, float absolute);
void TMC_move(TMC* self, float relative);
void TMC_wait_motor_stop(TMC* self);

// Configuration
void TMC_stealthChopAT(TMC* self, uint8_t irun);
void TMC_setStandstillMode(TMC* self, uint8_t mode);

#endif /* TMC_H_ */