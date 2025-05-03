
#ifndef DATABASE_H
#define DATABASE_H

/* =============================== Includes ======================================= */
#include "main.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* =============================== Structs ======================================= */
/**
 * @brief Inverter struct.
 * @note This struct is used to store the inverter paramets for the database layer
 */

 typedef struct{
    uint8_t status;
    uint8_t diagnostic_number;
    int16_t speed; //rpm
    int16_t torque; //0.1% Mn change to meaningful value
    int16_t motor_temperature; //0.1 degree C change to meaningful value
    int16_t plate_temperature; //0.1 degree C change to meaningful value
    int16_t igbt_temperature; //0.1 degree C change to meaningful value
    int32_t current_feedback_U; //not sure about the unit
    int32_t current_feedback_V; //not sure about the unit
    int32_t current_feedback_W; //not sure about the unit
    int16_t dc_bus_voltage; //not sure about the unit
    int16_t dc_bus_voltage_monitoring; //not sure about the unit
    int32_t actual_power; //not sure about the unit
    int16_t actual_magnetizing_current; //not sure about the unit
    
    struct {
        uint8_t status_word;
        int16_t torque_setpoint; //0.1% Mn change to meaningful value
        int16_t positive_torque_limit; //0.1% Mn change to meaningful value
        int16_t negative_torque_limit; //0.1% Mn change to meaningful value
    }setpoints;
} inverter_t;


/**
 * @brief Steering motor struct.
 * @note This struct is used to store the Steering motor paramets for the database layer
 */

typedef struct{
    float position;
    struct{
        float position_setpoint;
        float left_limit;
        float right_limit;
    }setpoints;
    struct{
        float kp;
        float ki;
        float kd;
    }pid;
} steering_motor_t; //FIXME - TBD


/**
 * @brief Pedal node struct.
 * @note This struct is used to store the Pedal node paramets for the database layer
 */

typedef struct{

    uint16_t throttle;
    uint16_t brake;
    uint16_t steering_wheel_angle;

}pedal_node_t;


/**
 * @brief Sub-VCU node struct.
 * @note This struct is used to store the Sub-VCU node paramets for the database layer
 */

typedef struct{
    uint8_t ASMS;
    uint8_t water_temp;

    struct Pneumatic_Pumps
    {

    }pneumatic_pumps;

    struct EBS
        {

        }ebs;

    struct IMU{
        float acceleration[3];     // STUB - TBD
        float angular_velocity[3]; // STUB - TBD
    } imu;

}sub_node_t;


/**
 * @brief VCU node struct.
 * @note This struct is used to store the VCU node paramets for the database layer
 */

typedef struct{
    inverter_t inverters[4];
    float sd_state;
    float back_oil_pressure;
    uint8_t ASMS;

}vcu_node_t;


/**
 * @brief Dashboard node struct.
 * @note This struct is used to store the Dashboard node paramets for the database layer
 */

typedef struct{
    uint8_t AMI[8];//check if it
    steering_motor_t steering_motor;

    float wheel_angle;

    struct BMS{
        int16_t cell_voltage[12]; // STUB - TBD
    } bms;

    struct RES{

    }res;

}dashboard_node_t;


/**
 * @brief DB struct.
 * @note This struct is used to store the pointers to all nodes in the DB
 */

typedef struct {

    pedal_node_t* pedal_node;
    sub_node_t* sub_node;
    vcu_node_t*	vcu_node;
    dashboard_node_t* dashboard_node;

} database_t;


/* ========================== Messages ID's =============================== */
#define PEDAL_ID   0x30
#define DASHBOARD_ID   0x40
#define SUB_ID   0x20
#define VCU_ID    0x10
#define Internal_ADC 0x77

/* =============================== Global Defines =============================== */
#define Stage1 1 
#define Stage2 2 
#define StageError 3    

/* ========================== Function Declarations =============================== */

database_t* db_AllocateMemory();
void db_FreeMemory(database_t* db_ptr);
database_t* db_Init();
database_t* db_GetDBPointer();
#endif // DATABASE_H




