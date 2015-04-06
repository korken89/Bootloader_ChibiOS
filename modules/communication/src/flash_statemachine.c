#include "ch.h"
#include "hal.h"
#include "myusb.h"
#include "flash_statemachine.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables and types.                                         */
/*===========================================================================*/
uint8_t command_buffer[FLASH_CMD_BUFFER_SIZE];
uint32_t pos = 0;
flash_states_t state = FLASH_GET_CMD;
/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

static inline void byte2hex(uint8_t byte, uint8_t *hex)
{
    hex[0] = byte / 16;
    hex[1] = byte % 16;
}

static inline uint8_t hex2byte(uint8_t *hex)
{
    return hex[0] * 16 + hex[1];
}

static bool strcmp_ms(char *s1, char *s2, const uint32_t max_size)
{
    uint32_t i;

    for (i = 0; i < max_size; i++)
    {
        if (s1[i] != s2[i])
            return false;
    }

    return true;
}
/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/
void FlashStateMachine(uint8_t data)
{
    if ((data < 32 && data != '\n') || data > 126)
        return; /* Not ASCII data */

    if (state == FLASH_GET_CMD)
    {
        state = ParseCommand(data);
    }
    else if (state == FLASH_GET_DATA)
    {

    }
    else if (state == FLASH_EXIT)
    {

    }
}

flash_states_t ParseCommand(uint8_t data)
{
    if (pos < FLASH_CMD_BUFFER_SIZE && data != '\n')
    {
        command_buffer[pos++] = data;
    }
    else if (pos > 0 && pos < FLASH_CMD_BUFFER_SIZE && data == '\n')
    {
        if (strcmp_ms("HELP", (char *)command_buffer, 4))
        {
            USBSendData((uint8_t *)"Help!\n", 6, TIME_INFINITE);
        }
        else if (strcmp_ms("INFO", (char *)command_buffer, 4))
        {
            USBSendData((uint8_t *)"Info!\n", 6, TIME_INFINITE);
        }
        else if (strcmp_ms("WRITE", (char *)command_buffer, 5))
        {
            USBSendData((uint8_t *)"Write!\n", 7, TIME_INFINITE);
        }
        else if (strcmp_ms("ERASE", (char *)command_buffer, 5))
        {
            USBSendData((uint8_t *)"Erase!\n", 7, TIME_INFINITE);
        }
        else if (strcmp_ms("USERAPP", (char *)command_buffer, 7))
        {
            USBSendData((uint8_t *)"User app!\n", 10, TIME_INFINITE);
        }
        else
        {
            USBSendData((uint8_t *)"Unknown command!\n", 17, TIME_INFINITE);
        }

        pos = 0;
    }
    else if (pos >= FLASH_CMD_BUFFER_SIZE && data != '\n')
    {
        pos = 0;
        USBSendData((uint8_t *)"Command buffer overrun!\n", 25, TIME_INFINITE);
    }
    else
    {
        pos = 0;
        USBSendData((uint8_t *)"Unknown error!\n", 15, TIME_INFINITE);
    }

    return FLASH_GET_CMD;

}


