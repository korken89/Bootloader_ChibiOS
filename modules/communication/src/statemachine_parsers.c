/* *
 *
 * All the parsers for the different packages.
 * To expand the functionality of the serial communication is just to add
 * functions here and add them to the parser_lookup table.
 *
 * */

#include "ch.h"
#include "hal.h"
#include "myusb.h"
#include "version_information.h"
#include "statemachine_generators.h"
#include "serialmanager.h"
#include "crc.h"
#include "statemachine_parsers.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

static void ParsePing(parser_holder_t *pHolder);
static void ParseGetRunningMode(parser_holder_t *pHolder);
static void ParseGetDeviceInfo(parser_holder_t *pHolder);


/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief Lookup table for all the serial parsers.
 */
static const parser_t parser_lookup[128] = {
    NULL,                             /* 0:   Cmd_None                        */
    NULL,                             /* 1:   Cmd_ACK                         */
    ParsePing,                        /* 2:   Cmd_Ping                        */
    NULL,                             /* 3:   Cmd_DebugMessage                */
    ParseGetRunningMode,              /* 4:   Cmd_GetRunningMode              */
    NULL,                             /* 5:                                   */
    NULL,                             /* 6:                                   */
    NULL,                             /* 7:                                   */
    NULL,                             /* 8:                                   */
    NULL,                             /* 9:                                   */
    NULL,                             /* 10:  Cmd_PrepareWriteFirmware        */
    NULL,                             /* 11:  Cmd_WriteFirmwarePackage        */
    NULL,                             /* 12:  Cmd_WriteLastFirmwarePackage    */
    NULL,                             /* 13:  Cmd_ReadFirmwarePackage         */
    NULL,                             /* 14:  Cmd_ReadLastFirmwarePackage     */
    NULL,                             /* 15:  Cmd_NextPackage                 */
    NULL,                             /* 16:  Cmd_ExitBootloader              */
    ParseGetDeviceInfo,               /* 17:  Cmd_GetBootloaderVersion        */
    NULL,                             /* 18:                                  */
    NULL,                             /* 19:                                  */
    NULL,                             /* 20:                                  */
    NULL,                             /* 21:                                  */
    NULL,                             /* 22:                                  */
    NULL,                             /* 23:                                  */
    NULL,                             /* 24:                                  */
    NULL,                             /* 25:                                  */
    NULL,                             /* 26:                                  */
    NULL,                             /* 27:                                  */
    NULL,                             /* 28:                                  */
    NULL,                             /* 29:                                  */
    NULL,                             /* 30:                                  */
    NULL,                             /* 31:                                  */
    NULL,                             /* 32:                                  */
    NULL,                             /* 33:                                  */
    NULL,                             /* 34:                                  */
    NULL,                             /* 35:                                  */
    NULL,                             /* 36:                                  */
    NULL,                             /* 37:                                  */
    NULL,                             /* 38:  RESERVED                        */
    NULL,                             /* 39:                                  */
    NULL,                             /* 40:                                  */
    NULL,                             /* 41:                                  */
    NULL,                             /* 42:                                  */
    NULL,                             /* 43:                                  */
    NULL,                             /* 44:                                  */
    NULL,                             /* 45:                                  */
    NULL,                             /* 46:                                  */
    NULL,                             /* 47:                                  */
    NULL,                             /* 48:                                  */
    NULL,                             /* 49:                                  */
    NULL,                             /* 50:                                  */
    NULL,                             /* 51:                                  */
    NULL,                             /* 52:                                  */
    NULL,                             /* 53:                                  */
    NULL,                             /* 54:                                  */
    NULL,                             /* 55:                                  */
    NULL,                             /* 56:                                  */
    NULL,                             /* 57:                                  */
    NULL,                             /* 58:                                  */
    NULL,                             /* 59:                                  */
    NULL,                             /* 60:                                  */
    NULL,                             /* 61:                                  */
    NULL,                             /* 62:                                  */
    NULL,                             /* 63:                                  */
    NULL,                             /* 64:                                  */
    NULL,                             /* 65:                                  */
    NULL,                             /* 66:                                  */
    NULL,                             /* 67:                                  */
    NULL,                             /* 68:                                  */
    NULL,                             /* 69:                                  */
    NULL,                             /* 70:                                  */
    NULL,                             /* 71:                                  */
    NULL,                             /* 72:                                  */
    NULL,                             /* 73:                                  */
    NULL,                             /* 74:                                  */
    NULL,                             /* 75:                                  */
    NULL,                             /* 76:                                  */
    NULL,                             /* 77:                                  */
    NULL,                             /* 78:                                  */
    NULL,                             /* 79:                                  */
    NULL,                             /* 80:                                  */
    NULL,                             /* 81:                                  */
    NULL,                             /* 82:                                  */
    NULL,                             /* 83:                                  */
    NULL,                             /* 84:                                  */
    NULL,                             /* 85:                                  */
    NULL,                             /* 86:                                  */
    NULL,                             /* 87:                                  */
    NULL,                             /* 88:                                  */
    NULL,                             /* 89:                                  */
    NULL,                             /* 90:                                  */
    NULL,                             /* 91:                                  */
    NULL,                             /* 92:                                  */
    NULL,                             /* 93:                                  */
    NULL,                             /* 94:                                  */
    NULL,                             /* 95:                                  */
    NULL,                             /* 96:                                  */
    NULL,                             /* 97:                                  */
    NULL,                             /* 98:                                  */
    NULL,                             /* 99:                                  */
    NULL,                             /* 100:                                 */
    NULL,                             /* 101:                                 */
    NULL,                             /* 102:                                 */
    NULL,                             /* 103:                                 */
    NULL,                             /* 104:                                 */
    NULL,                             /* 105:                                 */
    NULL,                             /* 106:                                 */
    NULL,                             /* 107:                                 */
    NULL,                             /* 108:                                 */
    NULL,                             /* 109:                                 */
    NULL,                             /* 110:                                 */
    NULL,                             /* 111:                                 */
    NULL,                             /* 112:                                 */
    NULL,                             /* 113:                                 */
    NULL,                             /* 114:                                 */
    NULL,                             /* 115:                                 */
    NULL,                             /* 116:                                 */
    NULL,                             /* 117:                                 */
    NULL,                             /* 118:                                 */
    NULL,                             /* 119:                                 */
    NULL,                             /* 120:                                 */
    NULL,                             /* 121:                                 */
    NULL,                             /* 122:                                 */
    NULL,                             /* 123:                                 */
    NULL,                             /* 124:                                 */
    NULL,                             /* 125:                                 */
    NULL,                             /* 126:                                 */
    NULL                              /* 127:                                 */
};

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/**
 * @brief                   A generic function to save data. Locks the RTOS
 *                          while saving.
 * 
 * @param[in] save_location Pointer to the location where the data shall be
 *                          saved.
 * @param[in] buffer        Pointer to the buffer where to save from.
 * @param[in] data_length   Number of bytes to save
 */
static void GenericSaveData(uint8_t *save_location,
                            uint8_t *buffer,
                            uint32_t data_length)
{
    uint32_t i;

    /* Lock while saving the data */
    osalSysLock();

    /* Save the string */
    for (i = 0; i < data_length; i++) 
            save_location[i] = buffer[i];

    osalSysUnlock();
}

/**
 * @brief               Parses a Ping command.
 * 
 * @param[in] pHolder   Message holder containing information
 *                      about the transmission. 
 */
static void ParsePing(parser_holder_t *pHolder)
{
    GenerateMessage(Cmd_Ping, pHolder->Port);
}

/**
 * @brief               Parses a GetRunningMode command.
 * 
 * @param[in] pHolder   Message holder containing information
 *                      about the transmission.
 */
static void ParseGetRunningMode(parser_holder_t *pHolder)
{
    GenerateMessage(Cmd_GetRunningMode, pHolder->Port);
}


/**
 * @brief               Parses a GetDeviceInfo command.
 * 
 * @param[in] pHolder   Message holder containing information
 *                      about the transmission.
 */
static void ParseGetDeviceInfo(parser_holder_t *pHolder)
{
    GenerateMessage(Cmd_GetDeviceInfo, pHolder->Port);
}

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief               Return the parser associated to the command.
 * 
 * @param[in] command   Command to get the parser for.
 * @return              Pointer to the associated parser.
 */
parser_t GetParser(KFly_Command command)
{
    return parser_lookup[command];
}
