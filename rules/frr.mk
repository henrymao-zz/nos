# FRRouting (frr) package

FRR_VERSION = 8.5.4
FRR_SUBVERSION = 0
FRR_BRANCH = frr/8.5
FRR_TAG = frr-8.5.4
export FRR_VERSION FRR_SUBVERSION FRR_BRANCH FRR_TAG

#gbp:error: Either 'EMAIL' or 'DEBEMAIL' must be set in the environment for 'dch' to work
EMAIL=ubuntu@ubuntu.com
export EMAIL

FRR = frr_$(FRR_VERSION)-sonic-$(FRR_SUBVERSION)_$(CONFIGURED_ARCH).deb
$(FRR)_DEPENDS += $(LIBSNMP_DEV) $(LIBYANG2) $(LIBYANG2_DEV)
$(FRR)_RDEPENDS += $(LIBYANG2)
$(FRR)_UNINSTALLS = $(LIBYANG2_DEV) $(LIBYANG2)
$(FRR)_SRC_PATH = $(SRC_PATH)/sonic-frr
# SONIC_MAKE_DEBS += $(FRR)

FRR_PYTHONTOOLS = frr-pythontools_$(FRR_VERSION)-sonic-$(FRR_SUBVERSION)_all.deb
$(eval $(call add_extra_package,$(FRR),$(FRR_PYTHONTOOLS)))

FRR_DBG = frr-dbgsym_$(FRR_VERSION)-sonic-$(FRR_SUBVERSION)_$(CONFIGURED_ARCH).$(DBG_DEB)
$(eval $(call add_extra_package,$(FRR),$(FRR_DBG)))

FRR_SNMP = frr-snmp_$(FRR_VERSION)-sonic-$(FRR_SUBVERSION)_$(CONFIGURED_ARCH).deb
$(eval $(call add_extra_package,$(FRR),$(FRR_SNMP)))

FRR_SNMP_DBG = frr-snmp-dbgsym_$(FRR_VERSION)-sonic-$(FRR_SUBVERSION)_$(CONFIGURED_ARCH).$(DBG_DEB)
$(eval $(call add_extra_package,$(FRR),$(FRR_SNMP_DBG)))

export FRR FRR_PYTHONTOOLS FRR_DBG FRR_SNMP FRR_SNMP_DBG

# The .c, .cpp, .h & .hpp files under src/{$DBG_SRC_ARCHIVE list}
# are archived into debug one image to facilitate debugging.
#
DBG_SRC_ARCHIVE += sonic-frr
