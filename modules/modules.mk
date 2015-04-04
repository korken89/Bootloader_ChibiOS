# Modules directory location from Makefile
MODULE_DIR = ./modules

# Imported source files and paths from modules
include $(MODULE_DIR)/communication/communication.mk
include $(MODULE_DIR)/crc/crc.mk
include $(MODULE_DIR)/st_flash/st_flash.mk
include $(MODULE_DIR)/usb/usb.mk
include $(MODULE_DIR)/version_information/version_information.mk

# List of all the module related files.
MODULES_SRC = $(COMMUNICATION_SRCS) \
              $(CONTROL_SRCS) \
              $(CRC_SRCS) \
              $(STFLASH_SRCS) \
              $(USB_SRCS) \
              $(VERSIONINFO_SRCS)

# Required include directories
MODULES_INC = $(COMMUNICATION_INC) \
              $(CONTROL_INC) \
              $(CRC_INC) \
              $(STFLASH_INC) \
              $(USB_INC) \
              $(VERSIONINFO_INC)