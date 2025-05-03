#include "adc.h"

#ifdef HAL_ADC_MODULE_ENABLED

//! Make sure that you change the ADC defines in adc.h if you change the number of sensors or samples per sensor

ADC_HandleTypeDef *pAdc1;
ADC_HandleTypeDef *pAdc2;
ADC_HandleTypeDef *pAdc3;

static handler_set_t* pHandlers = NULL; // Pointer to the handler set form the platform layer
static plt_callbacks_t* pCallbacks = NULL; // Pointer to the callback function pointers from the platform layer
static Queue_t* pCanRxQueue = NULL; // Pointer to the CAN RX queue from the platform layer
static can_message_t msg = {0}; 
//static Queue_t* pUartRxQueue = NULL; // Pointer to the UART RX queue from the platform layer
//static uart_message_t uart_msg = {0};




uint16_t ADC1_UF_Buffer[ADC1_TOTAL_BUFFER_SIZE];  // ADC Data Buffer
uint16_t ADC1_AVG_Samples[ADC1_NUM_SENSORS];  // Stores the averaged sensor values

uint16_t ADC2_UF_Buffer[ADC2_TOTAL_BUFFER_SIZE];  // ADC Data Buffer
uint16_t ADC2_AVG_Samples[ADC2_NUM_SENSORS];  // Stores the averaged sensor values

uint16_t ADC3_UF_Buffer[ADC3_TOTAL_BUFFER_SIZE];  // ADC Data Buffer
uint16_t ADC3_AVG_Samples[ADC3_NUM_SENSORS];  // Stores the averaged sensor values



void plt_AdcInit() 
{
    pHandlers = plt_GetHandlersPointer(); // Get the platform layer handlers pointer
    pCallbacks = plt_GetCallbacksPointer(); // Get the platform layer Callbacks pointer
    pCanRxQueue = plt_GetCanRxQueue(); // Get the CAN RX queue pointer
    
    // Initialize the ADC1 peripheral
    if (pHandlers->hadc1 != NULL) 
    {   
        
        pAdc1 = pHandlers->hadc1;
        HAL_ADC_Start_DMA(pAdc1, (uint32_t*)ADC1_UF_Buffer, ADC1_TOTAL_BUFFER_SIZE);

    }
    
    // Initialize the ADC2 peripheral
    if (pHandlers->hadc2 != NULL) 
    {
        pAdc2 = pHandlers->hadc2;
        HAL_ADC_Start_DMA(pAdc2, (uint32_t*)ADC2_UF_Buffer, ADC2_TOTAL_BUFFER_SIZE);
    }

    // Initialize the ADC3 peripheral
    if (pHandlers->hadc3 != NULL) 
    {
        pAdc3 = pHandlers->hadc3;
        HAL_ADC_Start_DMA(pAdc3, (uint32_t*)ADC3_UF_Buffer, ADC3_TOTAL_BUFFER_SIZE);
    }

    msg.id = Internal_ADC; // Set the message ID for the CAN message
}


/**
 * @brief Process the ADC data and store it in DB using the CAN-RxQueue
 * @param UF_Buffer Pointer to the Unfiltered ADC data buffer
 * @param Size Size of the Unfiltered buffer
 * TODO: add error check on the values
 * TODO : this setup need to be checked that it not taking to long to process the data
 * TODO : try to figureout if we can change the number of sensors to 3 fixed make the code more easy and efficent.
 * TODO : check if we can use DMA circular mode to remove the DMA_Start and Stop calls
 * 
 */
void plt_AdcProcessData(uint16_t *UF_Buffer, uint16_t Size)
{
/* ---------- 1.  Pick the right metadata for the buffer we got ---------- */
uint16_t   numSensors;
uint16_t   samplesPerSensor;
uint16_t  *avgSamples;


if (UF_Buffer == ADC1_UF_Buffer) {        /* ADC-1 */
    numSensors       = ADC1_NUM_SENSORS;
    samplesPerSensor = ADC1_SAMPLES_PER_SENSOR;
    avgSamples       = ADC1_AVG_Samples;
    
} else if (UF_Buffer == ADC2_UF_Buffer) { /* ADC-2 */
    numSensors       = ADC2_NUM_SENSORS;
    samplesPerSensor = ADC2_SAMPLES_PER_SENSOR;
    avgSamples       = ADC2_AVG_Samples;
  
} else if (UF_Buffer == ADC3_UF_Buffer) { /* ADC-3 */
    numSensors       = ADC3_NUM_SENSORS;
    samplesPerSensor = ADC3_SAMPLES_PER_SENSOR;
    avgSamples       = ADC3_AVG_Samples;
   
} else {
    return;                               /* unknown buffer-ptr → ignore  */
}

/* ---------- 2.  Single-pass accumulation (fast, cache-friendly) -------- */
uint32_t sums[numSensors] = {0};     /* ADC1_NUM_SENSORS == 3       */
for (uint16_t i = 0; i < Size; ++i) {
    sums[i % numSensors] +=UF_Buffer[i]  ; // subtract the min value from the sensor value);
}

/* ---------- 3.  Convert to averages ------------------------------------ */
for (uint16_t i = 0; i < numSensors; ++i) {
    avgSamples[i] = (uint16_t)(sums[i] / samplesPerSensor);
}

/* copy the averages (6 bytes for 3×uint16_t) and clear any padding       */
memset(msg.data, 0, sizeof(msg.data));
memcpy(msg.data, avgSamples, numSensors * sizeof(uint16_t));
Queue_Push(pCanRxQueue, &msg);
}

/*
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) {
        HAL_ADC_Stop_DMA(pAdc1);
        plt_AdcProcessData(ADC1_UF_Buffer,ADC1_TOTAL_BUFFER_SIZE / 2);
        HAL_ADC_Start_DMA(pAdc1, (uint32_t*)ADC1_UF_Buffer, ADC1_TOTAL_BUFFER_SIZE);
    }

    if (hadc->Instance == ADC2) {
        HAL_ADC_Stop_DMA(pAdc2);
        plt_AdcProcessData(ADC2_UF_Buffer,ADC2_TOTAL_BUFFER_SIZE / 2);
        HAL_ADC_Start_DMA(pAdc2, (uint32_t*)ADC2_UF_Buffer, ADC2_TOTAL_BUFFER_SIZE);
    } 

    if (hadc->Instance == ADC3) {
        HAL_ADC_Stop_DMA(pAdc3);
        plt_AdcProcessData(ADC3_UF_Buffer,ADC3_TOTAL_BUFFER_SIZE / 2);
        HAL_ADC_Start_DMA(pAdc3, (uint32_t*)ADC3_UF_Buffer, ADC3_TOTAL_BUFFER_SIZE);
    }
}
*/


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance == ADC1)
  {
    HAL_ADC_Stop_DMA(pAdc1);
    // Process the ADC data for ADC1
    plt_AdcProcessData(ADC1_UF_Buffer, ADC1_TOTAL_BUFFER_SIZE);

    HAL_ADC_Start_DMA(pAdc1, (uint32_t*)ADC1_UF_Buffer, ADC1_TOTAL_BUFFER_SIZE);

  }
  else if(hadc->Instance == ADC2)
  {
    HAL_ADC_Stop_DMA(pAdc2);
    // Process the ADC data for ADC2
    plt_AdcProcessData(ADC2_UF_Buffer, ADC2_TOTAL_BUFFER_SIZE);

    HAL_ADC_Start_DMA(pAdc2, (uint32_t*)ADC2_UF_Buffer, ADC2_TOTAL_BUFFER_SIZE);
  }
  else if(hadc->Instance == ADC3)
  {
    HAL_ADC_Stop_DMA(pAdc3);
    // Process the ADC data for ADC3
    plt_AdcProcessData(ADC3_UF_Buffer, ADC3_TOTAL_BUFFER_SIZE);

    HAL_ADC_Start_DMA(pAdc3, (uint32_t*)ADC3_UF_Buffer, ADC3_TOTAL_BUFFER_SIZE);
  }
}

void plt_AdcSetMinMax(Adc_Module_t adc_module,uint16_t *min, uint16_t *max)
{
    switch (adc_module) {
        case Adc1:
            memcpy(ADC1_Mins, min, sizeof(ADC1_Mins));
            memcpy(ADC1_Maxs, max, sizeof(ADC1_Maxs));
            break;
        case Adc2:
            memcpy(ADC2_Mins, min, sizeof(ADC2_Mins));
            memcpy(ADC2_Maxs, max, sizeof(ADC2_Maxs));
            break;
        case Adc3:
            memcpy(ADC3_Mins, min, sizeof(ADC3_Mins));
            memcpy(ADC3_Maxs, max, sizeof(ADC3_Maxs));
            break;
        default:
            break;
    }
}
#endif
