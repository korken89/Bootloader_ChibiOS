#include "ch.h"
#include "hal.h"
#include "system_init.h"
#include "bootloader.h"
#include "flash_functionality.h"


/**
 * @brief Placeholder for error messages.
 */
volatile assert_errors _assert_errors;

time_measurement_t tm;
rtcnt_t tm_delta;



int main(void)
{
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

    FlashEraseFromSector(FLASH_Sector_2, 1024*250);

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
