#include "DbSetFunctions.h"

/* =============================== Global Variables =============================== */
static database_t* pMainDB = NULL;

/* ========================== Function Definitions ============================ */
/**
 * @brief Initialize the database set functions
 * @note This function initializes the database set functions by getting the pointer to the main database set
 */
void DbSetFunctionsInit()
{
    pMainDB = db_GetDBPointer();
}

/**
 * @brief Set the pedal parameters
 * @note This function sets the pedal parameters by converting the data received from a message
 *       to the appropriate scale and assigning it to the corresponding fields in the pedal node
 * @param data Pointer to the data received from the CAN message
 * TODO : implement the function to set the pedal parameters
 * @return void
 */
void setPedalParameters(uint8_t* data)
{
   
     memcpy(&pMainDB->pedal_node->brake,&data[0], sizeof(uint16_t));
     memcpy(&pMainDB->pedal_node->steering_wheel_angle,&data[2], sizeof(uint16_t)); 
     memcpy(&pMainDB->pedal_node->throttle,&data[4], sizeof(uint16_t)); 
    
}

