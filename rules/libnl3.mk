# libnl3

ifneq ($(BLDENV),noble)

LIBNL3_VERSION_BASE = 3.5.0
LIBNL3_VERSION = $(LIBNL3_VERSION_BASE)-1

export LIBNL3_VERSION_BASE
export LIBNL3_VERSION

LIBNL3 = libnl-3-200_$(LIBNL3_VERSION)_$(CONFIGURED_ARCH).deb
$(LIBNL3)_SRC_PATH = $(SRC_PATH)/libnl3
# SONIC_MAKE_DEBS += $(LIBNL3)

LIBNL3_DEV = libnl-3-dev_$(LIBNL3_VERSION)_$(CONFIGURED_ARCH).deb
$(eval $(call add_derived_package,$(LIBNL3),$(LIBNL3_DEV)))

LIBNL_GENL3 = libnl-genl-3-200_$(LIBNL3_VERSION)_$(CONFIGURED_ARCH).deb
$(LIBNL_GENL3)_RDEPENDS += $(LIBNL3)
$(eval $(call add_derived_package,$(LIBNL3),$(LIBNL_GENL3)))

LIBNL_GENL3_DEV = libnl-genl-3-dev_$(LIBNL3_VERSION)_$(CONFIGURED_ARCH).deb
$(LIBNL_GENL3_DEV)_DEPENDS += $(LIBNL_GENL3) $(LIBNL3_DEV)
$(eval $(call add_derived_package,$(LIBNL3),$(LIBNL_GENL3_DEV)))

LIBNL_ROUTE3 = libnl-route-3-200_$(LIBNL3_VERSION)_$(CONFIGURED_ARCH).deb
$(LIBNL_ROUTE3)_RDEPENDS += $(LIBNL3)
$(eval $(call add_derived_package,$(LIBNL3),$(LIBNL_ROUTE3)))

LIBNL_ROUTE3_DEV = libnl-route-3-dev_$(LIBNL3_VERSION)_$(CONFIGURED_ARCH).deb
$(LIBNL_ROUTE3_DEV)_DEPENDS += $(LIBNL_ROUTE3) $(LIBNL3_DEV)
$(eval $(call add_derived_package,$(LIBNL3),$(LIBNL_ROUTE3_DEV)))

LIBNL_NF3 = libnl-nf-3-200_$(LIBNL3_VERSION)_$(CONFIGURED_ARCH).deb
$(LIBNL_NF3)_DEPENDS += $(LIBNL_ROUTE3_DEV)
$(LIBNL_NF3)_RDEPENDS += $(LIBNL_ROUTE3)
$(eval $(call add_derived_package,$(LIBNL3),$(LIBNL_NF3)))

LIBNL_NF3_DEV = libnl-nf-3-dev_$(LIBNL3_VERSION)_$(CONFIGURED_ARCH).deb
$(LIBNL_NF3_DEV)_DEPENDS += $(LIBNL_NF3)
$(eval $(call add_derived_package,$(LIBNL3),$(LIBNL_NF3_DEV)))

LIBNL_CLI = libnl-cli-3-200_$(LIBNL3_VERSION)_$(CONFIGURED_ARCH).deb
$(LIBNL_CLI)_DEPENDS += $(LIBNL_GENL3_DEV) $(LIBNL_NF3_DEV) $(LIBNL_ROUTE3_DEV)
$(LIBNL_CLI)_RDEPENDS += $(LIBNL_GENL3) $(LIBNL_NF3) $(LIBNL_ROUTE3)
$(eval $(call add_derived_package,$(LIBNL3),$(LIBNL_CLI)))

LIBNL_CLI_DEV = libnl-cli-3-dev_$(LIBNL3_VERSION)_$(CONFIGURED_ARCH).deb
$(LIBNL_CLI_DEV)_DEPENDS += $(LIBNL_CLI) $(LIBNL_GENL3_DEV) $(LIBNL_NF3_DEV) $(LIBNL_ROUTE3_DEV)
$(eval $(call add_derived_package,$(LIBNL3),$(LIBNL_CLI_DEV)))

# The .c, .cpp, .h & .hpp files under src/{$DBG_SRC_ARCHIVE list}
# are archived into debug one image to facilitate debugging.
#
DBG_SRC_ARCHIVE += libnl3

endif
