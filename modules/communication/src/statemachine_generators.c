/* *
 *
 * All the generators for the different serial messages.
 * 
 * */

#include "ch.h"
#include "hal.h"
#include "myusb.h"
#include "version_information.h"
#include "serialmanager.h"
#include "crc.h"
#include "statemachine_parsers.h"
#include "statemachine_generators.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

static bool GenerateACK(circular_buffer_t *Cbuff);
static bool GeneratePing(circular_buffer_t *Cbuff);
static bool GenerateGetRunningMode(circular_buffer_t *Cbuff);
static bool GenerateGetDeviceInfo(circular_buffer_t *Cbuff);
static uint32_t myStrlen(const uint8_t *str, const uint32_t max_length);

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables and types.                                         */
/*===========================================================================*/


/**
 * The message generation lookup table
 */
static const generator_t generator_lookup[18] = {
    NULL,                             /* 0:   Cmd_None                        */
    GenerateACK,                      /* 1:   Cmd_ACK                         */
    GeneratePing,                     /* 2:   Cmd_Ping                        */
    NULL,                             /* 3:   Cmd_DebugMessage                */
    GenerateGetRunningMode,           /* 4:   Cmd_GetRunningMode              */
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
    GenerateGetDeviceInfo             /* 17:  Cmd_GetDeviceInfo               */
};

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/**
 * @brief               Generates a message with no data part.
 * 
 * @param[in] command   Command to generate message for.
 * @param[in] Cbuff     Pointer to the circular buffer to put the data in.
 * @return              HAL_FAILED if the message didn't fit or HAL_SUCCESS 
 *                      if it did fit.
 */
static bool GenerateHeaderOnlyCommand(KFly_Command command, 
                                      circular_buffer_t *Cbuff)
{
    int32_t count = 0;
    uint8_t crc8;

    /* Write the stating SYNC (without doubling it) */
    CircularBuffer_WriteSYNCNoIncrement(Cbuff, &count, &crc8, NULL); 

    /* Add all the data to the message */
    CircularBuffer_WriteNoIncrement(Cbuff, command, &count, &crc8, NULL); 
    CircularBuffer_WriteNoIncrement(Cbuff, 0,       &count, &crc8, NULL); 
    CircularBuffer_WriteNoIncrement(Cbuff, crc8,    &count, NULL,  NULL);

    /* Check if the message fit inside the buffer */
    return CircularBuffer_Increment(Cbuff, count); 
}

/**
 * @brief                   Generates a message with data and CRC16 part. 
 * 
 * @param[in] command       Command to generate message for.
 * @param[in] data          Pointer to where the data is located.
 * @param[in] data_count    Number of data bytes.
 * @param[out] Cbuff        Pointer to the circular buffer to put the data in.
 * @return                  HAL_FAILED if the message didn't fit or HAL_SUCCESS
 *                          if it did fit.
 */
static bool GenerateGenericCommand(KFly_Command command, 
                                   uint8_t *data, 
                                   const uint32_t data_count, 
                                   circular_buffer_t *Cbuff)
{
    int32_t count = 0;
    uint32_t i;
    uint8_t crc8;
    uint16_t crc16;

    /* Check if the "best case" won't fit in the buffer which is
     * data_count + header + CRC16 = data_count + 6 bytes */
    if (CircularBuffer_SpaceLeft(Cbuff) < (data_count + 6))
        return HAL_FAILED;

    /* Add the header */
    /* Write the starting SYNC (without doubling it) */
    CircularBuffer_WriteSYNCNoIncrement(Cbuff, &count, &crc8, &crc16); 

    /* Add all of the header to the message */
    CircularBuffer_WriteNoIncrement(Cbuff, command,    &count, &crc8, &crc16); 
    CircularBuffer_WriteNoIncrement(Cbuff, data_count, &count, &crc8, &crc16); 
    CircularBuffer_WriteNoIncrement(Cbuff, crc8,       &count, NULL,  &crc16);

    /* Add the data to the message */
    for (i = 0; i < data_count; i++)
        CircularBuffer_WriteNoIncrement(Cbuff, data[i], &count, NULL, &crc16); 

    /* Add the CRC16 */
    CircularBuffer_WriteNoIncrement(Cbuff, (uint8_t)(crc16 >> 8), &count, NULL, NULL);
    CircularBuffer_WriteNoIncrement(Cbuff, (uint8_t)(crc16),      &count, NULL, NULL);

    /* Check if the message fit inside the buffer */
    return CircularBuffer_Increment(Cbuff, count);
}


/**
 * @brief               Generates an ACK.
 * 
 * @param[out] Cbuff    Pointer to the circular buffer to put the data in.
 * @return              HAL_FAILED if the message didn't fit or HAL_SUCCESS
 *                      if it did fit.
 */
static bool GenerateACK(circular_buffer_t *Cbuff)
{
    return GenerateHeaderOnlyCommand(Cmd_ACK, Cbuff);   /* Return status */
}

/**
 * @brief               Generates a Ping.
 * 
 * @param[out] Cbuff    Pointer to the circular buffer to put the data in.
 * @return              HAL_FAILED if the message didn't fit or HAL_SUCCESS
 *                      if it did fit.
 */
static bool GeneratePing(circular_buffer_t *Cbuff)
{
    return GenerateHeaderOnlyCommand(Cmd_Ping, Cbuff);  /* Return status */
}

/**
 * @brief               Generates the message for the current running mode.
 * 
 * @param[out] Cbuff    Pointer to the circular buffer to put the data in.
 * @return              HAL_FAILED if the message didn't fit or HAL_SUCCESS
 *                      if it did fit.
 */
static bool GenerateGetRunningMode(circular_buffer_t *Cbuff)
{
    return GenerateGenericCommand(Cmd_GetRunningMode, (uint8_t *)"B", 1, Cbuff);
}

/**
 * @brief               Generates the message for the the ID of the system.
 * 
 * @param[out] Cbuff    Pointer to the circular buffer to put the data in.
 * @return              HAL_FAILED if the message didn't fit or HAL_SUCCESS
 *                      if it did fit.
 */
static bool GenerateGetDeviceInfo(circular_buffer_t *Cbuff)
{
    uint8_t *device_id, *text_fw, *text_bl, *text_usr;
    uint32_t length_fw, length_bl, length_usr, data_count, i = 0;
    uint8_t crc8;
    uint16_t crc16;
    int32_t count = 0;

    /* The strings are at know location */
    device_id = (uint8_t *)ptrGetUniqueID();
    text_bl = (uint8_t *)ptrGetBootloaderVersion();
    text_fw = (uint8_t *)ptrGetFirmwareVersion();
    text_usr = ptrGetUserIDString();

    /* Find the length of the string */
    length_bl = myStrlen(text_bl, VERSION_MAX_SIZE);
    length_fw = myStrlen(text_fw, VERSION_MAX_SIZE);
    length_usr = myStrlen(text_usr, USER_ID_MAX_SIZE);

    /* The 3 comes from the 3 null bytes */
    data_count = UNIQUE_ID_SIZE + length_bl + length_fw + length_usr + 3;

    /* Check if the "best case" won't fit in the buffer */
    if (CircularBuffer_SpaceLeft(Cbuff) < (data_count + 6))
        return HAL_FAILED;

    /* Add the header */
    /* Write the starting SYNC (without doubling it) */
    CircularBuffer_WriteSYNCNoIncrement(                Cbuff, &count, &crc8, 
                                                                       &crc16); 

    /* Add all of the header to the message */
    CircularBuffer_WriteNoIncrement(Cbuff, Cmd_GetDeviceInfo, &count, &crc8, 
                                                                       &crc16); 
    CircularBuffer_WriteNoIncrement(Cbuff, data_count,        &count, &crc8,
                                                                       &crc16); 
    CircularBuffer_WriteNoIncrement(Cbuff, crc8,              &count, NULL,  
                                                                       &crc16);

    /* Get the Device ID */
    for (i = 0; i < UNIQUE_ID_SIZE; i++) 
        CircularBuffer_WriteNoIncrement(Cbuff, device_id[i], &count, NULL, 
                                                                       &crc16);

    /* Get the Bootloader Version string */
    for (i = 0; i < length_bl; i++) 
        CircularBuffer_WriteNoIncrement(Cbuff, text_bl[i], &count, NULL, 
                                                                       &crc16);

    CircularBuffer_WriteNoIncrement(Cbuff, 0x00, &count, NULL, 
                                                                       &crc16);

    /* Get the Firmware Version string */
    for (i = 0; i < length_fw; i++) 
        CircularBuffer_WriteNoIncrement(Cbuff, text_fw[i], &count, NULL, 
                                                                       &crc16);

    CircularBuffer_WriteNoIncrement(Cbuff, 0x00, &count, NULL, 
                                                                       &crc16);

    /* Get the User string */
    for (i = 0; i < length_usr; i++) 
        CircularBuffer_WriteNoIncrement(Cbuff, text_usr[i], &count, NULL, 
                                                                       &crc16);

    CircularBuffer_WriteNoIncrement(Cbuff, 0x00, &count, NULL, 
                                                                       &crc16);

    /* Add the CRC16 */
    CircularBuffer_WriteNoIncrement(Cbuff, (uint8_t)(crc16 >> 8), &count, NULL, 
                                                                          NULL);
    CircularBuffer_WriteNoIncrement(Cbuff, (uint8_t)(crc16),      &count, NULL, 
                                                                          NULL);

    /* Check if the message fit inside the buffer */
    return CircularBuffer_Increment(Cbuff, count);
}

/**
 * @brief                   Calculates the length of a string but with
 *                          maximum length termination.
 * 
 * @param[in] str           Pointer to the string.
 * @param[in] max_length    Maximum length/timeout.
 * @return                  Returns the length of the string.
 */
uint32_t myStrlen(const uint8_t *str, const uint32_t max_length)
{
    const uint8_t *s;
    s = str;

    while ((*s != '\0') && ((uint32_t)(s - str) < max_length))
        s++;

    return (s - str);
}

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

 /**
  * @brief              Generate a message for the ports based on the
  *                     generators in the lookup table.
  * 
  * @param[in] command  The command to generate a message for.
  * @param[in] port     Which port to send the data.
  * @return             HAL_FAILED if the message didn't fit or HAL_SUCCESS
  *                     if it did fit.
  */
bool GenerateMessage(KFly_Command command, External_Port port)
{
    bool status;
    circular_buffer_t *Cbuff = NULL;

    Cbuff = SerialManager_GetCircularBufferFromPort(port);

    /* Check so the circular buffer address is valid and that
       we are inside the lookup table */
    if (Cbuff == NULL || command > 17)
        return HAL_FAILED;

    /* Check so there is an available Generator function for this command */
    if (generator_lookup[command] != NULL)
    {
        /* Claim the circular buffer for writing */
        CircularBuffer_Claim(Cbuff);
        {
            status = generator_lookup[command](Cbuff);
        }
        /* Release the circular buffer */
        CircularBuffer_Release(Cbuff);

        /* If it was successful then start the transmission */
        if (status == HAL_SUCCESS)
            SerialManager_StartTransmission(port);
    }
    else
        status = HAL_FAILED;
    
    return status;
}

 /**
  * @brief              Generate a message for the ports based on the
  *                     generators in the lookup table.
  * 
  * @param[in] command  The command to generate a custom message for.
  * @param[in] data     Pointer to the data to be sent.
  * @param[in] size     Size of the data to be sent.
  * @param[in] port     Which port to send the data.
  * @return             HAL_FAILED if the message didn't fit or HAL_SUCCESS
  *                     if it did fit.
  */
bool GenerateCustomMessage(KFly_Command command,
                           uint8_t *data,
                           uint16_t size,
                           External_Port port)
{
    bool status;
    circular_buffer_t *Cbuff = NULL;

    Cbuff = SerialManager_GetCircularBufferFromPort(port);

    /* Check so the circular buffer address is valid and that
           we are inside the lookup table */
    if (Cbuff == NULL || command > 17)
        return HAL_FAILED;

    /* Claim the circular buffer for writing */
    CircularBuffer_Claim(Cbuff);
    {
        status = GenerateGenericCommand(command,
                                        data,
                                        size,
                                        Cbuff);
    }
    /* Release the circular buffer */
    CircularBuffer_Release(Cbuff);

    /* If it was successful then start the transmission */
    if (status == HAL_SUCCESS)
        SerialManager_StartTransmission(port);
    
    return status;
}



/**
 * @brief               Generates a Debug Message.
 * 
 * @param[in] data      Pointer to the data.
 * @param[in] size      Length of the data.
 * @param[out] Cbuff    Pointer to the circular buffer to put the data in.
 * @return              HAL_FAILED if the message didn't fit or HAL_SUCCESS
 *                      if it did fit.
 */
bool GenerateDebugMessage(uint8_t *data,
                          uint32_t size, 
                          circular_buffer_t *Cbuff)
{
    if (size > 256)
        return HAL_FAILED;
    else
        return GenerateGenericCommand(Cmd_DebugMessage, data, size, Cbuff);
}
