#include "ch.h"
#include "hal.h"
#include "system_init.h"
#include "bootloader.h"
#include "flash_statemachine.h"
#include "myusb.h"


/**
 * @brief Placeholder for error messages.
 */
volatile assert_errors _assert_errors;

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

    /* Test the VBUS line... */
    if (palReadPad(GPIOA, GPIOA_VBUS_FS) != 1)
    {

    }

    /*
     *
     * Initialize all drivers and modules.
     *
     */
    vSystemInit();


    /*
     *
     * Idle task loop.
     *
     */
    while(bSystemShutdownRequested() == false)
    {
        if (isUSBActive())
        {
            FlashStateMachine(USBReadByte(TIME_INFINITE));
        }
        else
        {
            chThdSleepMilliseconds(200);
        }
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
