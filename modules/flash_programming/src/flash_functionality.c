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

/**
 * @brief                   Gets the end sector, so that base_sector to, and
 *                          including, end_sector can fit size bytes.
 *
 * @param[in] base_sector   Base sector ID.
 * @param[in] size          Number of bytes of area needed.
 * @return                  The last sector needed. Returns -1 if it cannot fit.
 */
uint32_t FlashGetSector(uint32_t base_sector, uint32_t size)
{
    /* Check the parameters */
    osalDbgCheck(IS_FLASH_SECTOR(base_sector));

    /* Sector sizes for STM32F405 */
    static ROMCONST uint32_t sector_sizes[] = {16*1024, 16*1024, 16*1024,
                                               16*1024, 64*1024, 128*1024,
                                               128*1024, 128*1024, 128*1024,
                                               128*1024, 128*1024};

    uint32_t i = base_sector / FLASH_Sector_1, sum = 0;

    /* Sum sectors until requested size has been achieved. */
    for (; i < sizeof(sector_sizes)/sizeof(sector_sizes[0]); i++)
    {
        sum += sector_sizes[i];
        if (sum >= size)
            return i * FLASH_Sector_1;
    }

    return (uint32_t)-1;
}

/**
 * @brief                   Erases as many sectors as needed, starting from
 *                          base_sector, to fit size bytes there.
 *
 * @param[in] base_sector   Base sector ID.
 * @param[in] size          Number of bytes of area needed.
 * @return                  The flash status.
 */
FLASH_Status FlashEraseFromSector(uint32_t base_sector, uint32_t size)
{
    /* Check the parameters */
    osalDbgCheck(IS_FLASH_SECTOR(base_sector));

    /* Get the end sector. */
    uint32_t i, end_sector = FlashGetSector(base_sector, size);
    FLASH_Status status = FLASH_COMPLETE;

    /* Check for errors. */
    if (end_sector == (uint32_t)-1)
        return FLASH_ERROR_OPERATION;

    FLASH_Unlock();

    /* Erase the needed number of sectors, including end_sector. */
    for (i = base_sector; i <= end_sector; i += FLASH_Sector_1)
    {
        /* Erasing at 2.7V to 3.6V */
        status = FLASH_EraseSector(i, VoltageRange_3);

        /* Check for errors. */
        if (status != FLASH_COMPLETE)
        {
            FLASH_Lock();
            return status;
        }
    }

    FLASH_Lock();
    return status;
}
