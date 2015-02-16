/* *
 *
 * OS layer for Serial Communication.
 * Handles package coding/decoding.
 *
 * */

#include "ch.h"
#include "hal.h"
#include "myusb.h"
#include "statemachine.h"
#include "statemachine_generators.h"
#include "crc.h"
#include "serialmanager.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

#define START_TRANSMISSION_EVENT            EVENT_MASK(0)

static bool USBTransmitCircularBuffer(circular_buffer_t *Cbuff);

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Holder of the necessary information for the data pump threads.
 */
typedef struct
{
    /**
     * @brief   Pointer to the USB data pump thread.
     */
    thread_t *ptrUSBDataPump;
    /**
     * @brief   USB data pump circular transmit buffer.
     */
    circular_buffer_t USBTransmitBuffer;
    /**
     * @brief   Pointer to the AUX1 data pump thread.
     */
    thread_t *ptrAUX1DataPump;
    /**
     * @brief   AUX1 data pump circular transmit buffer.
     */
    circular_buffer_t AUX1TransmitBuffer;
    /**
     * @brief   Pointer to the AUX2 data pump thread.
     */
    thread_t *ptrAUX2DataPump;
    /**
     * @brief   AUX2 data pump circular transmit buffer.
     */
    circular_buffer_t AUX2TransmitBuffer;
    /**
     * @brief   Pointer to the AUX3 data pump thread.
     */
    thread_t *ptrAUX3DataPump;
    /**
     * @brief   AUX3 data pump circular transmit buffer.
     */
    circular_buffer_t AUX3TransmitBuffer;
    /**
     * @brief   Pointer to the AUX4 data pump thread.
     */
    thread_t *ptrAUX4DataPump;
    /**
     * @brief   AUX4 data pump circular transmit buffer.
     */
    circular_buffer_t AUX4TransmitBuffer;
} Serial_Datapump_Holder;

/* Instance of the data pump holder structure */
Serial_Datapump_Holder data_pumps = {
    .ptrUSBDataPump = NULL,
    .ptrAUX1DataPump = NULL,
    .ptrAUX2DataPump = NULL,
    .ptrAUX3DataPump = NULL,
    .ptrAUX4DataPump = NULL
};

/*===================================================*/
/* Working area for the data pump                    */
/* and data decode threads.                          */
/*===================================================*/

THD_WORKING_AREA(waUSBSerialManagerTask, 128);
THD_WORKING_AREA(waUSBDataPumpTask, 128);

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/*===================================================*/
/* USB Communication threads.                        */
/*===================================================*/

/**
 * @brief           The USB Serial Manager task will handle incoming
 *                  data and direct it for decode and processing.
 *             
 * @param[in] arg   Input argument (unused).
 */
__attribute__((noreturn))
static THD_FUNCTION(USBSerialManagerTask, arg)
{
    (void)arg;

    /* Name for debug */
    chRegSetThreadName("USB Serial Manager");

    /* Data structure for communication */
    static parser_holder_t data_holder;

    /* Buffer for parsing serial USB commands */
    CCM_MEMORY static uint8_t USB_in_buffer[SERIAL_RECIEVE_BUFFER_SIZE]; 

    /* Initialize data structure */
    vInitStatemachineDataHolder(&data_holder, PORT_USB, USB_in_buffer);

    while(1)
        vStatemachineDataEntry(USBReadByte(TIME_INFINITE),
                               &data_holder);
}

/**
 * @brief           Transmits the content of the USB circular buffer over USB.
 *  
 * @param[in] arg   Input argument (unused).
 */
__attribute__((noreturn))
static THD_FUNCTION(USBDataPumpTask, arg)
{
    (void)arg;

    /* Name for debug */
    chRegSetThreadName("USB Data Pump");

    /* Buffer for transmitting serial USB commands */
    CCM_MEMORY static uint8_t USB_out_buffer[SERIAL_TRANSMIT_BUFFER_SIZE]; 

    /* Initialize the USB transmit circular buffer */
    CircularBuffer_Init(&data_pumps.USBTransmitBuffer,
                        USB_out_buffer,
                        SERIAL_TRANSMIT_BUFFER_SIZE);
    CircularBuffer_InitMutex(&data_pumps.USBTransmitBuffer);

    /* Put the USB data pump thread into the list of available data pumps */
    data_pumps.ptrUSBDataPump = chThdGetSelfX();

    while(1)
    {
        /* Wait for a start transmission event */
        chEvtWaitAny(START_TRANSMISSION_EVENT);

        /* We will only get here is a request to send data has been received */
        USBTransmitCircularBuffer(&data_pumps.USBTransmitBuffer);
    }
}

/**
 * @brief               Transmits a circular buffer over the USB interface.
 *             
 * @param[in] Cbuff     Circular buffer to transmit.
 * @return              Returns HAL_FAILED if it did not succeed to transmit
 *                      the buffer, else HAL_SUCCESS is returned.
 */
static bool USBTransmitCircularBuffer(circular_buffer_t *Cbuff)
{
    uint8_t *read_pointer;
    uint32_t read_size;

    if ((isUSBActive() == true) && (Cbuff != NULL))
    {
        /* Read out the number of bytes to send and the pointer to the
           first byte */
        read_pointer = CircularBuffer_GetReadPointer(Cbuff, &read_size);

        /* Claim the USB bus during the entire transfer */
        USBClaim();
        while (read_size > 0)
        {
            /* Send the data from the circular buffer */
            USBSendData(read_pointer, read_size, TIME_INFINITE);

            /* Increment the circular buffer tail */
            CircularBuffer_IncrementTail(Cbuff, read_size);

            /* Get the read size again in case new data is available or if
               we reached the end of the buffer (to make sure the entire
               buffer is sent) */
            read_pointer = CircularBuffer_GetReadPointer(Cbuff, &read_size);

            /* If the USB has been removed during the transfer: abort */
            if (isUSBActive() == false)
                return HAL_FAILED;
        }
        /* Release the USB bus */
        USBRelease();

        /* Transfer finished successfully */
        return HAL_SUCCESS;
    }
    else /* Some error occurred */
        return HAL_FAILED;
}

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Initializes communication.
 */
void vSerialManagerInit(void)
{
    /* Initialize the USB mutex */
    USBMutexInit();


    /* Start the USB communication tasks */

    chThdCreateStatic(waUSBSerialManagerTask,
                      sizeof(waUSBSerialManagerTask),
                      NORMALPRIO,
                      USBSerialManagerTask,
                      NULL);

    chThdCreateStatic(waUSBDataPumpTask,
                      sizeof(waUSBDataPumpTask),
                      NORMALPRIO,
                      USBDataPumpTask,
                      NULL);
}

/**
 * @brief               Return the circular buffer of corresponding 
 *                      communication port.
 *             
 * @param[in] port      Port parameter.
 * @return              Returns the pointer to the corresponding port's 
 *                      circular buffer.
 */
circular_buffer_t *SerialManager_GetCircularBufferFromPort(External_Port port)
{
    if (port == PORT_USB)
        return &data_pumps.USBTransmitBuffer;

    else if (port == PORT_AUX1)
        return &data_pumps.AUX1TransmitBuffer;

    else if (port == PORT_AUX2)
        return &data_pumps.AUX2TransmitBuffer;

    else if (port == PORT_AUX3)
        return &data_pumps.AUX3TransmitBuffer;

    else if (port == PORT_AUX4)
        return &data_pumps.AUX4TransmitBuffer;

    else
        return NULL;
}

/**
 * @brief               Signal the data pump thread to start transmission.
 *             
 * @param[in] port      Port parameter.
 */
void SerialManager_StartTransmission(External_Port port)
{
    if ((port == PORT_USB) && (data_pumps.ptrUSBDataPump != NULL))
        chEvtSignal(data_pumps.ptrUSBDataPump, START_TRANSMISSION_EVENT);

    else if ((port == PORT_AUX1) && (data_pumps.ptrAUX1DataPump != NULL))
        chEvtSignal(data_pumps.ptrAUX1DataPump, START_TRANSMISSION_EVENT);

    else if ((port == PORT_AUX2) && (data_pumps.ptrAUX2DataPump != NULL))
        chEvtSignal(data_pumps.ptrAUX2DataPump, START_TRANSMISSION_EVENT);

    else if ((port == PORT_AUX3) && (data_pumps.ptrAUX3DataPump != NULL))
        chEvtSignal(data_pumps.ptrAUX3DataPump, START_TRANSMISSION_EVENT);

    else if ((port == PORT_AUX4) && (data_pumps.ptrAUX4DataPump != NULL))
        chEvtSignal(data_pumps.ptrAUX4DataPump, START_TRANSMISSION_EVENT);
}
