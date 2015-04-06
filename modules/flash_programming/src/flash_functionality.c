/* *
 *
 * 
 *
 * */

#include "ch.h"
#include "hal.h"
#include "flash_functionality.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

uint32_t FlashGetSector(uint32_t base_sector, uint32_t size)
{
    /* Check the parameters */
    osalDbgCheck(IS_FLASH_SECTOR(base_sector));

    /* Sector sizes for STM32F405 */
    static const uint32_t sector_sizes[] = {16*1024, 16*1024, 16*1024, 16*1024,
                                            64*1024, 128*1024, 128*1024,
                                            128*1024, 128*1024, 128*1024,
                                            128*1024};

    uint32_t i = base_sector / FLASH_Sector_1, sum = 0;

    for (; i < sizeof(sector_sizes)/sizeof(sector_sizes[0]); i++)
    {
        sum += sector_sizes[i];
        if (sum >= size)
            return i * FLASH_Sector_1;
    }

    return (uint32_t)-1;
}

FLASH_Status FlashEraseFromSector(uint32_t base_sector, uint32_t size)
{
    /* Check the parameters */
    osalDbgCheck(IS_FLASH_SECTOR(base_sector));

    uint32_t i, end_sector = FlashGetSector(base_sector, size);
    FLASH_Status status = FLASH_COMPLETE;

    if (end_sector == (uint32_t)-1)
        return FLASH_ERROR_OPERATION;

    FLASH_Unlock();

    for (i = base_sector; i <= end_sector; i += FLASH_Sector_1)
    {
        status = FLASH_EraseSector(i, VoltageRange_3);

        if (status != FLASH_COMPLETE)
        {
            FLASH_Lock();
            return status;
        }
    }

    FLASH_Lock();
    return status;
}
