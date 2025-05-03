#ifndef ADC_H
#define ADC_H
/* =============================== Includes ======================================= */
#include "platform.h"

#ifdef HAL_ADC_MODULE_ENABLED
#ifdef HAL_CAN_MODULE_ENABLED //because we need the CAN message to send the ADC data
/* ========================== Function Declarations ============================ */
void plt_AdcInit();
void plt_AdcProcessData(uint16_t *UF_Buffer, uint16_t Size);

/* =============================== Defines =============================== */
#define ADC1_NUM_SENSORS  3
#define ADC1_SAMPLES_PER_SENSOR 50  // Buffer size per sensor
#define ADC1_TOTAL_BUFFER_SIZE (ADC1_NUM_SENSORS * ADC1_SAMPLES_PER_SENSOR)
#define ADC2_NUM_SENSORS  3
#define ADC2_SAMPLES_PER_SENSOR 50  // Buffer size per sensor
#define ADC2_TOTAL_BUFFER_SIZE (ADC2_NUM_SENSORS * ADC2_SAMPLES_PER_SENSOR)
#define ADC3_NUM_SENSORS  3
#define ADC3_SAMPLES_PER_SENSOR 50  // Buffer size per sensor
#define ADC3_TOTAL_BUFFER_SIZE (ADC3_NUM_SENSORS * ADC3_SAMPLES_PER_SENSOR)

#define ADC1_MAX_VALUE 4095 // Maximum value for 12-bit ADC
#define ADC2_MAX_VALUE 4095 // Maximum value for 12-bit ADC
#define ADC3_MAX_VALUE 4095 // Maximum value for 12-bit ADC
#define ADC1_MIN_VALUE 0 // Minimum value for 12-bit ADC
#define ADC2_MIN_VALUE 0 // Minimum value for 12-bit ADC
#define ADC3_MIN_VALUE 0 // Minimum value for 12-bit ADC
#endif
#endif
#endif // ADC_H