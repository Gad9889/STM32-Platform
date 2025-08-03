
#ifndef DBSETFUNCTIONS_H
#define DBSETFUNCTIONS_H

/* =============================== Includes ======================================= */
#include "database.h"

/* ========================== Function Declarations =============================== */
void DbSetFunctionsInit();
void setPedalParameters(uint8_t* data);


void setVCUParameters(uint8_t* data);
void setDBParameters(uint8_t* data);
void setInv1Av1Parameters(uint8_t* data);
void setInv1Av2Parameters(uint8_t* data);
void setInv2Av1Parameters(uint8_t* data);
void setInv2Av2Parameters(uint8_t* data);
void setInv3Av1Parameters(uint8_t* data);
void setInv3Av2Parameters(uint8_t* data);
void setInv4Av1Parameters(uint8_t* data);
void setInv4Av2Parameters(uint8_t* data);
void setStage0Parameters(uint8_t* data);
void setStage1Parameters(uint8_t* data);
void setStage2Parameters(uint8_t* data);
void setStage3Parameters(uint8_t* data);
void setBmsParameters(uint8_t* data);
void setResParameters(uint8_t* data);

/* ==========================  Defines =============================== */
#define MAX_VALUE_APPS 100
#define MIN_VALUE_APPS  0
#define MAX_VALUE_BPPS 100
#define MIN_VALUE_BPPS 0
#define MAX_VALUE_SW 100
#define MIN_VALUE_SW -100
#define MAX_VALUE_BIOPS 100
#define MIN_VALUE_BIOPS 0

#endif // DBSETFUNCTIONS_H