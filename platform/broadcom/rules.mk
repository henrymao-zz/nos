include $(PLATFORM_PATH)/sai-modules.mk
#include $(PLATFORM_PATH)/sai.mk
# TODO: re-enable once the following have been updated for bookworm:
# * Nokia
# * Juniper
# * Ragile
# * Ufispace
####include $(PLATFORM_PATH)/platform-modules-nokia.mk
#include $(PLATFORM_PATH)/platform-modules-dell.mk
####include $(PLATFORM_PATH)/platform-modules-arista.mk
#include $(PLATFORM_PATH)/platform-modules-ingrasys.mk
#include $(PLATFORM_PATH)/platform-modules-accton.mk
###include $(PLATFORM_PATH)/platform-modules-alphanetworks.mk
#include $(PLATFORM_PATH)/platform-modules-inventec.mk
###include $(PLATFORM_PATH)/platform-modules-cel.mk
#include $(PLATFORM_PATH)/platform-modules-delta.mk
###include $(PLATFORM_PATH)/platform-modules-quanta.mk
##include $(PLATFORM_PATH)/platform-modules-mitac.mk
#include $(PLATFORM_PATH)/platform-modules-juniper.mk
#include $(PLATFORM_PATH)/platform-modules-brcm-xlr-gts.mk
#include $(PLATFORM_PATH)/platform-modules-ruijie.mk
#include $(PLATFORM_PATH)/platform-modules-ragile.mk
#include $(PLATFORM_PATH)/platform-modules-tencent.mk
#include $(PLATFORM_PATH)/platform-modules-ufispace.mk
#include $(PLATFORM_PATH)/platform-modules-micas.mk
#include $(PLATFORM_PATH)/docker-syncd-brcm.mk
#include $(PLATFORM_PATH)/docker-syncd-brcm-rpc.mk
#include $(PLATFORM_PATH)/docker-saiserver-brcm.mk

include $(PLATFORM_PATH)/one-image.mk
include $(PLATFORM_PATH)/raw-image.mk
include $(PLATFORM_PATH)/one-aboot.mk
#include $(PLATFORM_PATH)/libsaithrift-dev.mk
#include $(PLATFORM_PATH)/docker-syncd-brcm-dnx.mk
#include $(PLATFORM_PATH)/docker-syncd-brcm-dnx-rpc.mk
ifeq ($(INCLUDE_GBSYNCD), y)
#include $(PLATFORM_PATH)/../components/docker-gbsyncd-credo.mk
#include $(PLATFORM_PATH)/../components/docker-gbsyncd-broncos.mk
endif

BCMCMD = bcmcmd
$(BCMCMD)_URL = "https://sonicstorage.blob.core.windows.net/public/20190307/bcmcmd"

DSSERVE = dsserve
$(DSSERVE)_URL = "https://sonicstorage.blob.core.windows.net/public/20190307/dsserve"

SONIC_ONLINE_FILES += $(BCMCMD) $(DSSERVE)

SONIC_ALL += $(SONIC_ONE_IMAGE) $(SONIC_ONE_ABOOT_IMAGE)

