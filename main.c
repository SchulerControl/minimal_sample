#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "tuxono_sdk/tuxono.h"


int main(int argc, char *argv[]) 
{
	int result = -1;
	
	TUXONO_MEASURE_CONFIG measure_config;
	TUXONO_EX_DIGOUT_CONFIG ex_digout_config;
	TUXONO_MEASURE_DATA measure_data;
	TUXONO_PWM_CONFIG pwm_config;
	
	memset(&measure_config, 0, sizeof(TUXONO_MEASURE_CONFIG));
	memset(&ex_digout_config, 0, sizeof(TUXONO_EX_DIGOUT_CONFIG));
	memset(&measure_data, 0, sizeof(TUXONO_MEASURE_DATA));
	memset(&pwm_config, 0, sizeof(TUXONO_PWM_CONFIG));
	
	
	
	////////////////////////////////////////////////////////////////////
	// Initialize communication with hardware
	////////////////////////////////////////////////////////////////////
	
	if ((result = tuxono_InitCommunication()) != TUXONO_OK) {
		printf("Could not init communication! Error code: %d\n", result);
		return -1;
	}



	////////////////////////////////////////////////////////////////////
	// Setup analog measurement channels
	////////////////////////////////////////////////////////////////////
	
	// Retrieve currently active (default after startup) configuration
	if ((result = tuxono_GetMeasureConfig(&measure_config)) != TUXONO_OK) {
		printf("Could not get measurement configuration! Error code: %d\n", result);
		tuxono_CloseCommunication();
		return -1;
	}
	
	// Change configuration for slow channel 0 (PT1000 reference channel)
	measure_config.SlowChannelSetup[0]		= CHANNEL_IMPEDANCE;		// Measure impedance (PT1000)
	measure_config.ucADCSlowFilterLen[0]	= AVERAGING_LEN_64;			// Averaging with 64 samples (for use as reference)
	measure_config.ADCSlowGain[0]			= AMPLIFIER_GAIN_1_2;		// Measurement range +- 5 V
	measure_config.fDacMux16Values[0]		= 1.0;						// Measurement current = voltage * 4 / 1000 ohm (Tuxono S)
																		// Measurement current = voltage * 1 / 1000 ohm (Tuxono L)
	
	// Change configuration for slow channel 1 (PT1000 actual measurement channel)
	measure_config.SlowChannelSetup[1]		= CHANNEL_IMPEDANCE;		// Measure impedance (PT1000)
	measure_config.ucADCSlowFilterLen[1]	= AVERAGING_LEN_1;			// No averaging
	measure_config.ADCSlowGain[1]			= AMPLIFIER_GAIN_1_2;		// Measurement range +- 5 V
	measure_config.fDacMux16Values[1]		= 1.0;						// Measurement current = voltage * 4 / 1000 ohm (Tuxono S)
																		// Measurement current = voltage * 1 / 1000 ohm (Tuxono L)
																		
	// Store updated configuration
	if ((result = tuxono_SetMeasureConfig(&measure_config)) != TUXONO_OK) {
		printf("Could not set measurement configuration! Error code: %d\n", result);
		tuxono_CloseCommunication();
		return -1;
	}
					
		
												
	////////////////////////////////////////////////////////////////////
	// Setup PWM output channels
	////////////////////////////////////////////////////////////////////
	
	// Retrieve currently active (default after startup) configuration
	if ((result = tuxono_GetPWMConfig(&pwm_config)) != TUXONO_OK) {
		printf("Could not get PWM configuration! Error code: %d\n", result);
		tuxono_CloseCommunication();
		return -1;
	}
	
	// Adjust PWM configuration
	pwm_config.ClockFrequency	= PWM_FREQ_125k;						// Set the clock frequency for all PWM channels
	pwm_config.usPeriodeClocks	= 3000;									// Set the number of clocks in one PWM period for all channels
	pwm_config.ucActivePWM		= 0x70;									// Enable PWM out of channels: OUT_OPTO_MOS1, 2 and 3 --> Bits 5, 6 and 7
	
	pwm_config.usPWMValue[4]	= 0;									// Set output value: OUT_OPTO_MOS1 PWM off
	pwm_config.usPWMValue[5]	= pwm_config.usPeriodeClocks / 3;		// Set output value: OUT_OPTO_MOS2 PWM 1/3 high active
	pwm_config.usPWMValue[6]	= pwm_config.usPeriodeClocks;			// Set output value: OUT_OPTO_MOS3 PWM full on
	
		
	// Store updated configuration
	if ((result = tuxono_SetPWMConfig(&pwm_config)) != TUXONO_OK) {
		printf("Could not set PWM configuration! Error code: %d\n", result);
		tuxono_CloseCommunication();
		return -1;
	}

	
	
	
	////////////////////////////////////////////////////////////////////
	// Loop infinitely 
	////////////////////////////////////////////////////////////////////
	
	struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 250 * 1000 * 1000;	// 250 ms
    int counter = 0;
    
	do {
		////////////////////////////////////////////////////////////////////
		// Toggle LEDs
		////////////////////////////////////////////////////////////////////
	
		// Get ex digital out configuration
		if ((result = tuxono_GetExDigitalOutConfig(&ex_digout_config)) != TUXONO_OK) {
			printf("Could not get ex digital out configuration! Error code: %d\n", result);
			tuxono_CloseCommunication();
			return -1;
		}
		
		// Toggle LED 'Live' and '18' (Tuxono-S LED 6) status
		ex_digout_config.LEDLive	= !ex_digout_config.LEDLive;
		ex_digout_config.LED18		= !ex_digout_config.LED18;
		
		// Store updated configuration
		if ((result = tuxono_SetExDigitalOutConfig(&ex_digout_config)) != TUXONO_OK) {
			printf("Could not set ex digital out configuration! Error code: %d\n", result);
			tuxono_CloseCommunication();
			return -1;
		}
		
		
		
		////////////////////////////////////////////////////////////////////
		// Get and print measurement info
		////////////////////////////////////////////////////////////////////
		
		memset(&measure_data, 0, sizeof(TUXONO_MEASURE_DATA));
		
		// Get measurement data
		if ((result = tuxono_GetMeasureData(&measure_data)) != TUXONO_OK) {
			printf("Could not get measurement data! Error code: %d\n", result);
			tuxono_CloseCommunication();
			return -1;
		}
		
		// Printf measurement data every second
		if (counter % 4 == 0) {
			printf("Channel 0 (reference): %2.6f - channel 1: %2.6f\n", measure_data.fADCSlowValues[0], measure_data.fADCSlowValues[1]);
		}
		
		
		
		// Sleep a bit... (250 ms)
		nanosleep(&ts, NULL);
		counter++;
	} 
	while (1);
	
	// Close communication
	tuxono_CloseCommunication();
	return 0;
}
