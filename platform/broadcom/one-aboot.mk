# sonic broadcom one image installer

SONIC_ONE_ABOOT_IMAGE = sonic-aboot-broadcom.swi
$(SONIC_ONE_ABOOT_IMAGE)_MACHINE = broadcom
$(SONIC_ONE_ABOOT_IMAGE)_DEPENDENT_MACHINE = broadcom-dnx
$(SONIC_ONE_ABOOT_IMAGE)_IMAGE_TYPE = aboot
ifeq ($(INCLUDE_GBSYNCD), y)
$(SONIC_ONE_ABOOT_IMAGE)_INSTALLS += $(PHY_CREDO)
endif
$(SONIC_ONE_ABOOT_IMAGE)_INSTALLS += $(FLASHROM)
$(SONIC_ONE_ABOOT_IMAGE)_INSTALLS += $(SYSTEMD_SONIC_GENERATOR)
$(SONIC_ONE_ABOOT_IMAGE)_LAZY_BUILD_INSTALLS = $(BRCM_OPENNSL_KERNEL) $(BRCM_DNX_OPENNSL_KERNEL)
#$(SONIC_ONE_ABOOT_IMAGE)_INSTALLS += $(ARISTA_PLATFORM_MODULE_PYTHON3) \
#                                     $(ARISTA_PLATFORM_MODULE_DRIVERS) \
#                                     $(ARISTA_PLATFORM_MODULE_LIBS) \
#                                     $(ARISTA_PLATFORM_MODULE)
#ifeq ($(INSTALL_DEBUG_TOOLS),y)
#$(SONIC_ONE_ABOOT_IMAGE)_DOCKERS += $(SONIC_INSTALL_DOCKER_DBG_IMAGES)
#$(SONIC_ONE_ABOOT_IMAGE)_DOCKERS += $(filter-out $(patsubst %-$(DBG_IMAGE_MARK).gz,%.gz, $(SONIC_INSTALL_DOCKER_DBG_IMAGES)), $(SONIC_INSTALL_DOCKER_IMAGES))
#else
#$(SONIC_ONE_ABOOT_IMAGE)_DOCKERS += $(SONIC_INSTALL_DOCKER_IMAGES)
#endif
SONIC_INSTALLERS += $(SONIC_ONE_ABOOT_IMAGE)
