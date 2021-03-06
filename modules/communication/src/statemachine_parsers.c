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
static const parser_t parser_lookup[18] = {
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
    ParseGetDeviceInfo                /* 17:  Cmd_GetBootloaderVersion        */
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
    if (command > 17)
        return NULL;
    else
        return parser_lookup[command];
}
