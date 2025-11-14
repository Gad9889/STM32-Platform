/**
 * @file platform_status.h
 * @brief Platform-wide status codes and error handling
 * @author Ben Gurion Racing Team
 * @date 2025-11-14
 * 
 * This header defines standardized return codes for all platform functions,
 * replacing the VALID() macro with proper error propagation.
 */

#ifndef PLATFORM_STATUS_H
#define PLATFORM_STATUS_H

#include <stdint.h>

/**
 * @brief Platform operation status codes
 * 
 * All platform API functions should return plt_status_t to indicate success or
 * failure. This enables proper error handling and debugging.
 * 
 * @note Negative values indicate errors, zero indicates success
 */
typedef enum {
    PLT_OK                  = 0,    ///< Operation completed successfully
    PLT_ERROR               = -1,   ///< Generic error
    PLT_INVALID_PARAM       = -2,   ///< Invalid parameter passed to function
    PLT_NULL_POINTER        = -3,   ///< NULL pointer argument
    PLT_NO_MEMORY           = -4,   ///< Memory allocation failed
    PLT_NOT_INITIALIZED     = -5,   ///< Module not initialized
    PLT_ALREADY_INITIALIZED = -6,   ///< Module already initialized
    PLT_TIMEOUT             = -7,   ///< Operation timed out
    PLT_BUSY                = -8,   ///< Resource busy
    PLT_QUEUE_FULL          = -9,   ///< Queue is full
    PLT_QUEUE_EMPTY         = -10,  ///< Queue is empty
    PLT_HAL_ERROR           = -11,  ///< Underlying HAL error
    PLT_NOT_SUPPORTED       = -12,  ///< Feature not supported
    PLT_OVERFLOW            = -13,  ///< Buffer overflow detected
    PLT_UNDERFLOW           = -14,  ///< Buffer underflow detected
} plt_status_t;

/**
 * @brief Convert status code to human-readable string
 * 
 * @param[in] status Status code to convert
 * @return Constant string describing the status
 * 
 * @note The returned string is a constant and should not be modified
 * 
 * @example
 * ```c
 * plt_status_t status = plt_CanInit(32);
 * if (status != PLT_OK) {
 *     printf("Error: %s\n", plt_StatusToString(status));
 * }
 * ```
 */
const char* plt_StatusToString(plt_status_t status);

/**
 * @brief Check if status indicates success
 * 
 * @param[in] status Status code to check
 * @return 1 if successful, 0 if error
 */
static inline int plt_IsSuccess(plt_status_t status) {
    return (status == PLT_OK);
}

/**
 * @brief Check if status indicates error
 * 
 * @param[in] status Status code to check
 * @return 1 if error, 0 if successful
 */
static inline int plt_IsError(plt_status_t status) {
    return (status != PLT_OK);
}

/**
 * @brief Macro for parameter validation
 * 
 * Use this macro at the beginning of functions to validate parameters.
 * If the condition is false, the function returns PLT_INVALID_PARAM.
 * 
 * @param[in] condition Condition to check
 * 
 * @example
 * ```c
 * plt_status_t my_function(uint32_t *ptr, size_t size) {
 *     PLT_ASSERT_PARAM(ptr != NULL);
 *     PLT_ASSERT_PARAM(size > 0);
 *     // ... function implementation
 *     return PLT_OK;
 * }
 * ```
 */
#define PLT_ASSERT_PARAM(condition) \
    do { \
        if (!(condition)) { \
            return PLT_INVALID_PARAM; \
        } \
    } while(0)

/**
 * @brief Macro for NULL pointer validation
 * 
 * Use this macro to check if a pointer is NULL.
 * If NULL, the function returns PLT_NULL_POINTER.
 * 
 * @param[in] ptr Pointer to check
 * 
 * @example
 * ```c
 * plt_status_t process_data(uint8_t *data) {
 *     PLT_ASSERT_NOT_NULL(data);
 *     // ... process data
 *     return PLT_OK;
 * }
 * ```
 */
#define PLT_ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            return PLT_NULL_POINTER; \
        } \
    } while(0)

/**
 * @brief Macro to check and propagate HAL errors
 * 
 * Use this macro to call HAL functions and automatically convert
 * HAL_StatusTypeDef to plt_status_t.
 * 
 * @param[in] hal_call HAL function call
 * 
 * @example
 * ```c
 * plt_status_t can_start(CAN_HandleTypeDef *hcan) {
 *     PLT_CHECK_HAL(HAL_CAN_Start(hcan));
 *     PLT_CHECK_HAL(HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING));
 *     return PLT_OK;
 * }
 * ```
 */
#define PLT_CHECK_HAL(hal_call) \
    do { \
        HAL_StatusTypeDef hal_status = (hal_call); \
        if (hal_status != HAL_OK) { \
            return PLT_HAL_ERROR; \
        } \
    } while(0)

/**
 * @brief Macro to check and propagate platform errors
 * 
 * Use this macro to call other platform functions and propagate errors.
 * 
 * @param[in] plt_call Platform function call
 * 
 * @example
 * ```c
 * plt_status_t initialize_system(void) {
 *     PLT_CHECK(plt_CanInit(32));
 *     PLT_CHECK(plt_UartInit(16));
 *     return PLT_OK;
 * }
 * ```
 */
#define PLT_CHECK(plt_call) \
    do { \
        plt_status_t _status = (plt_call); \
        if (_status != PLT_OK) { \
            return _status; \
        } \
    } while(0)

/**
 * @brief Error context structure for debugging
 * 
 * This structure can be used to capture error context information
 * for debugging purposes.
 */
typedef struct {
    plt_status_t status;       ///< Error status code
    const char *function;      ///< Function where error occurred
    const char *file;          ///< File where error occurred
    uint32_t line;             ///< Line number where error occurred
    uint32_t timestamp;        ///< Timestamp when error occurred
} plt_error_context_t;

/**
 * @brief Macro to capture error context
 * 
 * @param[out] ctx Pointer to error context structure
 * @param[in] status_code Status code to record
 */
#define PLT_CAPTURE_ERROR(ctx, status_code) \
    do { \
        if ((ctx) != NULL) { \
            (ctx)->status = (status_code); \
            (ctx)->function = __func__; \
            (ctx)->file = __FILE__; \
            (ctx)->line = __LINE__; \
            (ctx)->timestamp = HAL_GetTick(); \
        } \
    } while(0)

#endif // PLATFORM_STATUS_H
