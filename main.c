#include "ch.h"
#include "hal.h"
#include "system_init.h"
#include "bootloader.h"
#include "modules/flash_programming/inc/stm32f4xx_flash.h"

/**
 * @brief Placeholder for error messages.
 */
volatile assert_errors _assert_errors;

time_measurement_t tm;
rtcnt_t tm_delta;

FLASH_Status EraseArea(uint32_t base_sector, uint32_t size)
{
    /* Check the parameters */
    osalDbgCheck(IS_FLASH_SECTOR(base_sector));
}

int main(void)
{
    FLASH_Status status;
    int i;

    /*
     * System initializations.
     * - HAL initialization, this also initializes the configured 
     *   device drivers and performs the board-specific initializations.
     * - Kernel initialization, the main() function becomes a thread
     *   and the RTOS is active.
     */
    halInit();
    chSysInit();

    /*
     *
     * Initialize all drivers and modules.
     *
     */
    vSystemInit();

    chTMObjectInit(&tm);
    chTMStartMeasurementX(&tm);


    FLASH_Unlock();

    status = FLASH_EraseSector(FLASH_Sector_2, VoltageRange_3); // 16k
    status = FLASH_EraseSector(FLASH_Sector_3, VoltageRange_3); // 16k
    status = FLASH_EraseSector(FLASH_Sector_4, VoltageRange_3); // 64k
    status = FLASH_EraseSector(FLASH_Sector_5, VoltageRange_3); // 128k

    for (i = 0; i < 256*150; i++)
      status = FLASH_ProgramWord(0x08008000 + i*4, 0xdeadbeef+i);

    FLASH_Lock();

    (void)status;

    chTMStopMeasurementX(&tm);
    tm_delta = RTC2MS(STM32_SYSCLK, tm.last);

    /*
     *
     * Idle task loop.
     *
     */
    while(bSystemShutdownRequested() == false)
    {
        palTogglePad(GPIOC, GPIOC_LED_USR);
        chThdSleepMilliseconds(200);
        //vSystemRequestShutdown(SYSTEM_SHUTDOWN_KEY);
    }

    /*
     *
     * Deinitialize all drivers and modules.
     *
     */
    vSystemDeinit();

   /*
    *
    * All threads, drivers, interrupts and SysTick are now disabled.
    * The main function is now just a "normal" function again.
    *
    */

    /*
     *
     * Start the DFU bootloader.
     * This can be replaced if a custom bootloader is available.
     *
     */
    //vBootloaderResetAndStartDFU();

    /* In case of error get stuck here */
    while (1);
}
