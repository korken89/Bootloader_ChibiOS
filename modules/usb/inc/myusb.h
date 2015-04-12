#ifndef __MYUSB_H
#define __MYUSB_H

/* Defines */
#define USBD1_DATA_REQUEST_EP           1
#define USBD1_DATA_AVAILABLE_EP         1
#define USBD1_INTERRUPT_REQUEST_EP      2

/* Typedefs */

/* Global variables */
extern SerialUSBDriver SDU1;
extern const USBConfig usbcfg;
extern const SerialUSBConfig serusbcfg;

/* Macros */
#define USBStream()     (BaseSequentialStream *)&SDU1

/* Inline functions */

/* Global functions */
bool isUSBActive(void);
size_t USBSendData(uint8_t *data, size_t size, systime_t timeout);
size_t USBReadByte(systime_t timeout);

#endif
