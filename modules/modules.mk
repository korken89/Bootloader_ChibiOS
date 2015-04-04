# Modules directory location from Makefile
MODULE_DIR = ./modules

# Imported source files and paths from modules
include $(MODULE_DIR)/communication/communication.mk
include $(MODULE_DIR)/crc/crc.mk
include $(MODULE_DIR)/flash_programming/flash_programming.mk
include $(MODULE_DIR)/usb/usb.mk
include $(MODULE_DIR)/version_information/version_information.mk

# List of all the module related files.
MODULES_SRC = $(COMMUNICATION_SRCS) \
              $(CONTROL_SRCS) \
              $(CRC_SRCS) \
              $(FLASHPROG_SRCS) \
              $(USB_SRCS) \
              $(VERSIONINFO_SRCS)

# Required include directories
MODULES_INC = $(COMMUNICATION_INC) \
              $(CONTROL_INC) \
              $(CRC_INC) \
              $(FLASHPROG_INC) \
              $(USB_INC) \
              $(VERSIONINFO_INC)