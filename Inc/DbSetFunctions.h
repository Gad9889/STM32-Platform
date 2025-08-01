
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
#endif // DBSETFUNCTIONS_H