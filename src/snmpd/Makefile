SHELL = /bin/bash
.ONESHELL:
.SHELLFLAGS += -e

MAIN_TARGET = libsnmp-base_$(SNMPD_VERSION_FULL)_all.deb
ifneq (,$(findstring 5.9,$(SNMPD_VERSION)))
DERIVED_TARGETS = snmptrapd_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).deb \
		  snmp_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).deb \
		  snmpd_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).deb \
		  snmp-dbgsym_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).$(DBG_DEB) \
		  snmpd-dbgsym_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).$(DBG_DEB) \
		  libsnmp40t64_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).deb \
		  libsnmp40t64-dbgsym_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).$(DBG_DEB) \
		  libsnmp-dev_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).deb \
		  libsnmp-perl_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).deb \
		  tkmib_$(SNMPD_VERSION_FULL)_all.deb
SNMPD_MAKE_JOBS_NUM = $(SONIC_CONFIG_MAKE_JOBS)
else
DERIVED_TARGETS = snmptrapd_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).deb \
		  snmp_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).deb \
		  snmpd_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).deb \
		  snmp-dbgsym_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).$(DBG_DEB) \
		  snmpd-dbgsym_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).$(DBG_DEB) \
		  libsnmp30_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).deb \
		  libsnmp30-dbg_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).$(DBG_DEB) \
		  libsnmp-dev_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).deb \
		  libsnmp-perl_$(SNMPD_VERSION_FULL)_$(CONFIGURED_ARCH).deb \
		  tkmib_$(SNMPD_VERSION_FULL)_all.deb

SNMPD_MAKE_JOBS_NUM = 1
endif

$(addprefix $(DEST)/, $(MAIN_TARGET)): $(DEST)/% :
	rm -rf net-snmp-$(SNMPD_VERSION)

	# download debian net-snmp
ifneq ($(BLDENV),noble)
	dget -u https://sonicstorage.blob.core.windows.net/debian/pool/main/n/net-snmp/net-snmp_$(SNMPD_VERSION_FULL).dsc
else 
	dget -u http://archive.ubuntu.com/ubuntu/pool/main/n/net-snmp/net-snmp_5.9.4+dfsg-1.1ubuntu3.dsc
endif

	pushd net-snmp-$(SNMPD_VERSION)
	git init
	git add -f *
	git commit -m "unmodified snmpd source"

	# Apply patches
ifneq ($(BLDENV),noble)
	stg init
	stg import -s ../patch-$(SNMPD_VERSION)/series
endif

ifneq ($(CROSS_BUILD_ENVIRON), y)
	dpkg-buildpackage -rfakeroot -b -d -us -uc -j$(SNMPD_MAKE_JOBS_NUM) --admindir $(SONIC_DPKG_ADMINDIR)
else
	PYTHONPATH=/usr/lib/python2.7/plat-arm-linux-gnueabihf/ dpkg-buildpackage -rfakeroot -b -d -us -uc -a$(CONFIGURED_ARCH) -Pcross,nocheck -j$(SNMPD_MAKE_JOBS_NUM) --admindir $(SONIC_DPKG_ADMINDIR)
endif
	popd

	mv $(DERIVED_TARGETS) $* $(DEST)/

$(addprefix $(DEST)/, $(DERIVED_TARGETS)): $(DEST)/% : $(DEST)/$(MAIN_TARGET)
