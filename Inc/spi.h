#ifndef SPI_H
#define SPI_H
/* =============================== Includes ======================================= */
#include "platform.h"

#ifdef HAL_SPI_MODULE_ENABLED
/*========================= Function Declarations =========================*/
void plt_SpiInit(size_t rx_queue_size);
void plt_SpiSendMsg(spi_message_t* pData);
void plt_SpiProcessRxMsgs(void);

#endif

#endif    // SPI_H