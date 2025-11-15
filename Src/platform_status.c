/**
 * @file platform_status.c
 * @brief Implementation of platform status utilities
 */

#include "platform_status.h"

const char* plt_StatusToString(plt_status_t status) {
    switch (status) {
        case PLT_OK:
            return "Success";
        case PLT_ERROR:
            return "Generic error";
        case PLT_INVALID_PARAM:
            return "Invalid parameter";
        case PLT_NULL_POINTER:
            return "NULL pointer";
        case PLT_NO_MEMORY:
            return "Memory allocation failed";
        case PLT_NOT_INITIALIZED:
            return "Not initialized";
        case PLT_ALREADY_INITIALIZED:
            return "Already initialized";
        case PLT_TIMEOUT:
            return "Operation timed out";
        case PLT_BUSY:
            return "Resource busy";
        case PLT_QUEUE_FULL:
            return "Queue is full";
        case PLT_QUEUE_EMPTY:
            return "Queue is empty";
        case PLT_HAL_ERROR:
            return "HAL error";
        case PLT_NOT_SUPPORTED:
            return "Feature not supported";
        case PLT_OVERFLOW:
            return "Buffer overflow";
        case PLT_UNDERFLOW:
            return "Buffer underflow";
        default:
            return "Unknown error";
    }
}

// Alias for compatibility with new API
const char* plt_GetStatusString(plt_status_t status) {
    return plt_StatusToString(status);
}
