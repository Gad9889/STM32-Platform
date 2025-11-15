/**
 * @file callbacks.c
 * @brief Application callback stub implementation
 * 
 * @note This file is a compatibility stub. The old callback system using plt_*
 *       functions was removed in Platform v2.0.0 refactor. Applications should
 *       migrate to the new direct HAL API:
 *       - Platform_begin() for initialization
 *       - P_CAN, P_UART, P_SPI, P_ADC, P_PWM for peripheral access
 *       - Implement HAL callbacks directly in user code
 * 
 * @deprecated Legacy implementation removed - kept as stub for compilation
 */

#include "callbacks.h"

/* =============================== Global Variables =============================== */

// Compatibility stubs for legacy application code
uint8_t KL_Nodes[3] = {0};
uint8_t FSM_stage = 0;

/* ========================== Legacy Function Stubs ============================ */

/**
 * @deprecated All legacy callback functions removed in Platform v2.0.0
 * 
 * Migration Guide:
 * ----------------
 * Old API (removed):
 *   - PlatformInit()
 *   - SetCallbacks()
 *   - plt_CanInit(), plt_UartInit(), etc.
 * 
 * New API (use instead):
 *   - Platform_begin(&handlers) - Initialize platform with HAL handles
 *   - P_CAN->send()/receive()   - Direct CAN access
 *   - P_UART->printf()/read()   - Direct UART access
 *   - P_SPI->transfer()         - Direct SPI access
 *   - P_ADC->read()             - Direct ADC access
 *   - P_PWM->set()              - Direct PWM access
 * 
 * See README.md and examples/ for v2.0.0 usage patterns.
 */
