#include "ch.h"
#include "hal.h"
#include "myusb.h"
#include "flash_statemachine.h"
#include "chprintf.h"
/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables and types.                                         */
/*===========================================================================*/
char command_buffer[FLASH_CMD_BUFFER_SIZE];
uint32_t pos = 0;
flash_state_t state = FLASH_GET_CMD;
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
static int32_t simple_atoi(char *str, uint32_t len)
{
    int32_t i, scale = 1, sum = 0;
    for (i = len-1; i >= 0; i--)
    {
        sum += scale*((int32_t)str[i] - (int32_t)'0');
        scale *= 10;
    }

    return sum;
}

static int32_t getNum(char *str, uint32_t len)
{
    uint32_t pos = 0, start;

    /* Check for errors */
    if (len == 0)
        return -1;

    /* Skip all spaces */
    while (str[pos] == ' ' && pos < len)
        pos++;

    /* Check for errors */
    if (pos >= len)
        return -1;

    start = pos;
    /* Check so all the last characters are numbers */
    while (pos < len)
    {
        if (str[pos] > '9' || str[pos] < '0')
            return -1;
        else
            pos++;
    }

    /* Convert characters to number */
    return simple_atoi(&str[start], len - start);
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

flash_state_t ParseCommand(uint8_t data)
{
    int32_t size;

    if (pos < FLASH_CMD_BUFFER_SIZE && data != '\n')
    {
        command_buffer[pos++] = (char)data;
    }
    else if (pos > 0 && pos < FLASH_CMD_BUFFER_SIZE && data == '\n')
    {
        if (strcmp_ms("HELP", command_buffer, 4))
        {
            USBSendData((uint8_t *)"Help!\n", 6, TIME_INFINITE);
        }
        else if (strcmp_ms("INFO", command_buffer, 4))
        {
            USBSendData((uint8_t *)"Info!\n", 6, TIME_INFINITE);
        }
        else if (strcmp_ms("WRITE", command_buffer, 5))
        {
            size = getNum(&command_buffer[5], pos - 5);
            chprintf(USBStream(), "WRITE: %i\n", size);
        }
        else if (strcmp_ms("ERASE", command_buffer, 5))
        {
            USBSendData((uint8_t *)"Erase!\n", 7, TIME_INFINITE);
        }
        else if (strcmp_ms("USERAPP", command_buffer, 7))
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


