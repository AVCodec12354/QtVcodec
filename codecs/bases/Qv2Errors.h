#ifndef QV2ERRORS_H
#define QV2ERRORS_H

#include <string>

/**
 * @brief Qv2 Status codes for the Qt Video Core v2 system.
 * These codes are designed to be generic across different codecs.
 */
enum Qv2Status : int {
    QV2_OK                    = 0,      ///< Success
    QV2_ERR_GENERIC           = -1,     ///< Generic error
    
    // Resource & Argument Errors
    QV2_ERR_INVALID_ARG       = -101,   ///< Invalid argument provided
    QV2_ERR_NO_MEMORY         = -102,   ///< Memory allocation failed
    QV2_ERR_NOT_INITIALIZED   = -103,   ///< Component not initialized
    QV2_ERR_ALREADY_EXISTS    = -104,   ///< Resource already exists
    QV2_ERR_NOT_FOUND         = -105,   ///< Resource not found
    
    // Capability & Format Errors
    QV2_ERR_UNSUPPORTED       = -201,   ///< Operation or format not supported
    QV2_ERR_BAD_VALUE         = -202,   ///< Parameter out of range
    QV2_ERR_BAD_FORMAT        = -203,   ///< Invalid color or pixel format
    
    // Bitstream & Processing Errors
    QV2_ERR_MALFORMED         = -301,   ///< Malformed bitstream data
    QV2_ERR_BUFFER_OVERFLOW   = -302,   ///< Buffer too small for output
    QV2_ERR_EOS               = -303,   ///< End of Stream reached
    QV2_ERR_TIMEOUT           = -304,   ///< Operation timed out
    
    // Internal & Hardware Errors
    QV2_ERR_HW_FAILURE        = -401,   ///< Underlying hardware failed
    QV2_ERR_INTERNAL          = -402    ///< Internal library error
};

/**
 * @brief Utility function to convert status to human-readable string.
 */
inline std::string Qv2StatusToString(int status) {
    switch (status) {
        case QV2_OK:                  return "OK";
        case QV2_ERR_GENERIC:         return "Generic Error";
        case QV2_ERR_INVALID_ARG:     return "Invalid Argument";
        case QV2_ERR_NO_MEMORY:       return "Out of Memory";
        case QV2_ERR_NOT_INITIALIZED: return "Not Initialized";
        case QV2_ERR_NOT_FOUND:       return "Not Found";
        case QV2_ERR_UNSUPPORTED:     return "Unsupported";
        case QV2_ERR_BAD_FORMAT:      return "Bad Format";
        case QV2_ERR_MALFORMED:       return "Malformed Bitstream";
        case QV2_ERR_BUFFER_OVERFLOW: return "Buffer Overflow";
        case QV2_ERR_EOS:               return "End of Stream";
        case QV2_ERR_HW_FAILURE:      return "Hardware Failure";
        case QV2_ERR_INTERNAL:        return "Internal Error";
        default:                      return "Unknown Error (" + std::to_string(status) + ")";
    }
}

#endif // QV2ERRORS_H
