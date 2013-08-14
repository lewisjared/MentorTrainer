/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "usbcfg.h"
#include "string.h"

/* Virtual serial port over USB.*/
SerialUSBDriver SDU1;

#define ADC_NUM_CHANNELS   6
#define ADC_BUF_DEPTH      1


static adcsample_t samples[ADC_NUM_CHANNELS * ADC_BUF_DEPTH];

static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {

  (void)adcp;
  (void)err;
}

static const ADCConversionGroup adcgrpcfg1 = {
  FALSE,
  ADC_NUM_CHANNELS,
  NULL,
  adcerrorcallback,
  0,                        /* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  0,
  ADC_SMPR2_SMP_AN0(ADC_SAMPLE_3) | ADC_SMPR2_SMP_AN1(ADC_SAMPLE_3) |
  ADC_SMPR2_SMP_AN2(ADC_SAMPLE_3) | ADC_SMPR2_SMP_AN3(ADC_SAMPLE_3) |
  ADC_SMPR2_SMP_AN4(ADC_SAMPLE_3) | ADC_SMPR2_SMP_AN5(ADC_SAMPLE_3),                        /* SMPR2 */
  ADC_SQR1_NUM_CH(ADC_NUM_CHANNELS),
  0,                        /* SQR2 */
  ADC_SQR3_SQ6_N(ADC_CHANNEL_IN5)   | ADC_SQR3_SQ5_N(ADC_CHANNEL_IN4) |
  ADC_SQR3_SQ4_N(ADC_CHANNEL_IN3)   | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN2) |
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN1)   | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN0)
};



/*===========================================================================*/
/* Initialization and main thread.                                           */
/*===========================================================================*/

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  palSetGroupMode(GPIOA, PAL_PORT_BIT(0) | PAL_PORT_BIT(1) |
		  	  	  	  	  PAL_PORT_BIT(2) | PAL_PORT_BIT(3) |
		  	  	  	  	  PAL_PORT_BIT(4) | PAL_PORT_BIT(5),
                  0, PAL_MODE_INPUT_ANALOG);

  adcStart(&ADCD1, NULL);

  /*
   * Initializes a serial-over-USB CDC driver.
   */
  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1000);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);


  /*
   * Normal main() thread activity, in this demo it just performs
   * a shell respawn upon its termination.
   */
  while (TRUE) {
	  systime_t time = chTimeNow();

	  //Sample ADCs
	  adcConvert(&ADCD1, &adcgrpcfg1, samples, ADC_BUF_DEPTH);
	  chSequentialStreamWrite(&SDU1, samples, ADC_NUM_CHANNELS*sizeof(adcsample_t));

	  chThdSleepUntil(time + MS2ST(1000));
  }
}
