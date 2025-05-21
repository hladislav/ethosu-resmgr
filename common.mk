#
# Copyright 2025 Ladislav Hano 
#
# SPDX-License-Identifier: Apache-2.0
#


ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=ethos-u
endef

# Driver source

# EXTRA_SRCVPATH += $(EXTRA_SRCVPATH_$(SECTION))
EXTRA_SRCVPATH += $(PROJECT_ROOT)

EXTRA_INCVPATH += $(PROJECT_ROOT)/include
EXTRA_INCVPATH += $(PROJECT_ROOT)/upublic
EXTRA_INCVPATH += $(PROJECT_ROOT)

PUBLIC_INCVPATH += $(PROJECT_ROOT)/upublic
PUBLIC_INCVPATH += $(PROJECT_ROOT)

ifndef USEFILE
USEFILE=$(PROJECT_ROOT)/ethosu.use
endif

# RPMSG-lite lib

ifdef SHARED_LIB
EXTRA_LIBVPATH += $(BSP_PATH)/rpmsg-lite-qnx/aarch64/so-le
endif
EXTRA_LIBVPATH += $(BSP_PATH)/rpmsg-lite-qnx/aarch64/a-le

EXTRA_INCVPATH += $(BSP_PATH)/rpmsg-lite-qnx/src/public

LIBS += rpmsg_lite-imx

# Messaging unit

EXTRA_INCVPATH+=$(BSP_PATH)/install/usr/include/soc/nxp/imx9/common
EXTRA_INCVPATH+=$(BSP_PATH)/install/usr/include/soc/nxp/imx8/common

EXTRA_INCVPATH+=$(BSP_PATH)/install/usr/include

EXTRA_LIBVPATH+=$(BSP_PATH)/install/aarch64le/lib
EXTRA_LIBVPATH+=$(BSP_PATH)/src/lib/hw_vendor/nxp/imx9/mu/aarch64/a-le

LIBS += drvr mu-mx9

NAME=ethosu-resmgr
include $(MKFILES_ROOT)/qmacros.mk
-include $(PROJECT_ROOT)/roots.mk
-include $(PROJECT_ROOT)/$(SECTION)/pinfo.mk

include $(MKFILES_ROOT)/qtargets.mk


