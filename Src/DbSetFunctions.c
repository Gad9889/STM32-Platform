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
 * @return void
 */
void setPedalParameters(uint8_t* data)
{
    pMainDB->vcu_node->keep_alive[PEDALNODE] = 1; // Set the pedal node aliv
    uint16_t gas_value = 0;
    uint16_t brake_value = 0;
    int16_t steering_wheel_angle = 0;
    uint16_t BIOPS = 0;

    memcpy(&gas_value,&data[0], sizeof(uint16_t));
    memcpy(&brake_value,&data[2], sizeof(uint16_t));
    memcpy(&steering_wheel_angle,&data[4], sizeof(int16_t));
    memcpy(&BIOPS,&data[6], sizeof(uint16_t));

    gas_value = (gas_value > MAX_VALUE_APPS) ? MAX_VALUE_APPS : gas_value;
    brake_value = (brake_value > MAX_VALUE_BPPS) ? MAX_VALUE_BPPS : brake_value;
    steering_wheel_angle = (steering_wheel_angle > MAX_VALUE_SW) ? MAX_VALUE_SW : (steering_wheel_angle < MIN_VALUE_SW) ? MIN_VALUE_SW : steering_wheel_angle;
    BIOPS = (BIOPS > MAX_VALUE_BIOPS) ? MAX_VALUE_BIOPS :  BIOPS ;

    pMainDB->pedal_node->gas_value = gas_value;
    pMainDB->pedal_node->brake_value = brake_value;
    pMainDB->pedal_node->steering_wheel_angle = steering_wheel_angle;
    pMainDB->pedal_node->BIOPS = BIOPS;

}


void setDBParameters(uint8_t* data)
{
    pMainDB->vcu_node->keep_alive[DBNODE] = 1; // Set the database node alive
    if(pMainDB->dashboard_node->R2D == 0)
    {
        memcpy(&pMainDB->dashboard_node->R2D,&data[2], sizeof(uint16_t));
    }
    
}
void setInv1Av1Parameters(uint8_t* data)
{
    pMainDB->vcu_node->keep_alive[INV1] = 1; // Set the inverter 1 alive

    pMainDB->vcu_node->inverters[0].AMK_Status.AMK_bReserve = 0xbb;
    pMainDB->vcu_node->inverters[0].AMK_Status.AMK_bSystemReady = data[1] & 0x01;
    pMainDB->vcu_node->inverters[0].AMK_Status.AMK_bError = (data[1] >> 1) & 0x01;
    pMainDB->vcu_node->inverters[0].AMK_Status.AMK_bWarn = (data[1] >> 2) & 0x01;
    pMainDB->vcu_node->inverters[0].AMK_Status.AMK_bQuitDCon = (data[1] >> 3) & 0x01;
    pMainDB->vcu_node->inverters[0].AMK_Status.AMK_bDcOn = (data[1] >> 4) & 0x01;
    pMainDB->vcu_node->inverters[0].AMK_Status.AMK_bQuitInverterOn = (data[1] >> 5) & 0x01;
    pMainDB->vcu_node->inverters[0].AMK_Status.AMK_bInverterOn = (data[1] >> 6) & 0x01;
    pMainDB->vcu_node->inverters[0].AMK_Status.AMK_bDerating = (data[1] >> 7) & 0x01;
    memcpy(&pMainDB->vcu_node->inverters[0].actual_speed,&data[2], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->inverters[0].torque_current,&data[4], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->inverters[0].magnetizing_current,&data[6], sizeof(uint16_t));
}
void setInv1Av2Parameters(uint8_t* data)
{
    
    pMainDB->vcu_node->keep_alive[INV1] = 1; // Set the inverter 1 alive

    memcpy(&pMainDB->vcu_node->inverters[0].motor_temperature,&data[0], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->inverters[0].plate_temperature,&data[2], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->error_group.inv1_error,&data[4], sizeof(uint16_t));
}

void setInv2Av1Parameters(uint8_t* data)
{
    pMainDB->vcu_node->keep_alive[INV2] = 1; // Set the inverter 2 alive
    pMainDB->vcu_node->inverters[1].AMK_Status.AMK_bReserve = 0xbb;
    pMainDB->vcu_node->inverters[1].AMK_Status.AMK_bSystemReady = data[1] & 0x01;
    pMainDB->vcu_node->inverters[1].AMK_Status.AMK_bError = (data[1] >> 1) & 0x01;
    pMainDB->vcu_node->inverters[1].AMK_Status.AMK_bWarn = (data[1] >> 2) & 0x01;
    pMainDB->vcu_node->inverters[1].AMK_Status.AMK_bQuitDCon = (data[1] >> 3) & 0x01;
    pMainDB->vcu_node->inverters[1].AMK_Status.AMK_bDcOn = (data[1] >> 4) & 0x01;
    pMainDB->vcu_node->inverters[1].AMK_Status.AMK_bQuitInverterOn = (data[1] >> 5) & 0x01;
    pMainDB->vcu_node->inverters[1].AMK_Status.AMK_bInverterOn = (data[1] >> 6) & 0x01;
    pMainDB->vcu_node->inverters[1].AMK_Status.AMK_bDerating = (data[1] >> 7) & 0x01;
    memcpy(&pMainDB->vcu_node->inverters[1].actual_speed,&data[2], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->inverters[1].torque_current,&data[4], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->inverters[1].magnetizing_current,&data[6], sizeof(uint16_t));
}
void setInv2Av2Parameters(uint8_t* data)
{
    pMainDB->vcu_node->keep_alive[INV2] = 1; // Set the inverter 2 alive

    memcpy(&pMainDB->vcu_node->inverters[1].motor_temperature,&data[0], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->inverters[1].plate_temperature,&data[2], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->error_group.inv2_error,&data[4], sizeof(uint16_t));
}

void setInv3Av1Parameters(uint8_t* data)
{
    pMainDB->vcu_node->keep_alive[INV3] = 1; // Set the inverter 3 alive

    pMainDB->vcu_node->inverters[2].AMK_Status.AMK_bReserve = 0xbb;
    pMainDB->vcu_node->inverters[2].AMK_Status.AMK_bSystemReady = data[1] & 0x01;
    pMainDB->vcu_node->inverters[2].AMK_Status.AMK_bError = (data[1] >> 1) & 0x01;
    pMainDB->vcu_node->inverters[2].AMK_Status.AMK_bWarn = (data[1] >> 2) & 0x01;
    pMainDB->vcu_node->inverters[2].AMK_Status.AMK_bQuitDCon = (data[1] >> 3) & 0x01;
    pMainDB->vcu_node->inverters[2].AMK_Status.AMK_bDcOn = (data[1] >> 4) & 0x01;
    pMainDB->vcu_node->inverters[2].AMK_Status.AMK_bQuitInverterOn = (data[1] >> 5) & 0x01;
    pMainDB->vcu_node->inverters[2].AMK_Status.AMK_bInverterOn = (data[1] >> 6) & 0x01;
    pMainDB->vcu_node->inverters[2].AMK_Status.AMK_bDerating = (data[1] >> 7) & 0x01;
    memcpy(&pMainDB->vcu_node->inverters[2].actual_speed,&data[2], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->inverters[2].torque_current,&data[4], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->inverters[2].magnetizing_current,&data[6], sizeof(uint16_t));
}
void setInv3Av2Parameters(uint8_t* data)
{
    pMainDB->vcu_node->keep_alive[INV3] = 1; // Set the inverter 3 alive

    memcpy(&pMainDB->vcu_node->inverters[2].motor_temperature,&data[0], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->inverters[2].plate_temperature,&data[2], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->error_group.inv3_error,&data[4], sizeof(uint16_t));
}

void setInv4Av1Parameters(uint8_t* data)
{
    pMainDB->vcu_node->keep_alive[INV4] = 1; // Set the inverter 4 alive
    pMainDB->vcu_node->inverters[3].AMK_Status.AMK_bReserve = 0xbb;
    pMainDB->vcu_node->inverters[3].AMK_Status.AMK_bSystemReady = data[1] & 0x01;
    pMainDB->vcu_node->inverters[3].AMK_Status.AMK_bError = (data[1] >> 1) & 0x01;
    pMainDB->vcu_node->inverters[3].AMK_Status.AMK_bWarn = (data[1] >> 2) & 0x01;
    pMainDB->vcu_node->inverters[3].AMK_Status.AMK_bQuitDCon = (data[1] >> 3) & 0x01;
    pMainDB->vcu_node->inverters[3].AMK_Status.AMK_bDcOn = (data[1] >> 4) & 0x01;
    pMainDB->vcu_node->inverters[3].AMK_Status.AMK_bQuitInverterOn = (data[1] >> 5) & 0x01;
    pMainDB->vcu_node->inverters[3].AMK_Status.AMK_bInverterOn = (data[1] >> 6) & 0x01;
    pMainDB->vcu_node->inverters[3].AMK_Status.AMK_bDerating = (data[1] >> 7) & 0x01;
    memcpy(&pMainDB->vcu_node->inverters[3].actual_speed,&data[2], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->inverters[3].torque_current,&data[4], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->inverters[3].magnetizing_current,&data[6], sizeof(uint16_t));
}

void setInv4Av2Parameters(uint8_t* data)
{
    pMainDB->vcu_node->keep_alive[INV4] = 1; // Set the inverter 4 alive
    
    memcpy(&pMainDB->vcu_node->inverters[3].motor_temperature,&data[0], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->inverters[3].plate_temperature,&data[2], sizeof(uint16_t));
    memcpy(&pMainDB->vcu_node->error_group.inv4_error,&data[4], sizeof(uint16_t));
}

// ! meanwhile, these functions are not implemented yet maybe not relvante to vcu


void setBmsParameters(uint8_t* data)
{

}

void setResParameters(uint8_t* data)
{

}


void setStage0Parameters(uint8_t* data)
{

}

void setStage1Parameters(uint8_t* data)
{

}

void setStage2Parameters(uint8_t* data)
{

}

void setStage3Parameters(uint8_t* data)
{

}


void setVCUParameters(uint8_t* data)
{

}

