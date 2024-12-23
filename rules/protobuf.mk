# protobuf package
# Protobuf 3.21.12 has been released in bookworm, So we only need to build it
# in the bullseye environment.
ifeq ($(BLDENV),bullseye)

    PROTOBUF_VERSION = 3.21.12
    PROTOBUF_VERSION_FULL = $(PROTOBUF_VERSION)-8.2build1

    export PROTOBUF_VERSION
    export PROTOBUF_VERSION_FULL

    PROTOBUF = libprotobuf32t64_$(PROTOBUF_VERSION_FULL)_$(CONFIGURED_ARCH).deb
    $(PROTOBUF)_SRC_PATH = $(SRC_PATH)/protobuf
    # SONIC_MAKE_DEBS += $(PROTOBUF)

    PROTOBUF_DEV = libprotobuf-dev_$(PROTOBUF_VERSION_FULL)_$(CONFIGURED_ARCH).deb
    $(PROTOBUF_DEV)_DEPENDS = $(PROTOBUF) $(PROTOBUF_LITE)
    $(eval $(call add_derived_package,$(PROTOBUF),$(PROTOBUF_DEV)))

    PROTOBUF_LITE = libprotobuf-lite32t64_$(PROTOBUF_VERSION_FULL)_$(CONFIGURED_ARCH).deb
    $(eval $(call add_derived_package,$(PROTOBUF),$(PROTOBUF_LITE)))

    PROTOC32 = libprotoc32t64_$(PROTOBUF_VERSION_FULL)_$(CONFIGURED_ARCH).deb
    $(PROTOC32)_RDEPENDS = $(PROTOBUF) $(PROTOBUF_LITE)
    $(eval $(call add_derived_package,$(PROTOBUF),$(PROTOC32)))

    PROTOC_DEV = libprotoc-dev_$(PROTOBUF_VERSION_FULL)_$(CONFIGURED_ARCH).deb
    $(PROTOC_DEV)_DEPENDS = $(PROTOBUF) $(PROTOBUF_LITE) $(PROTOC32)
    $(eval $(call add_derived_package,$(PROTOBUF),$(PROTOC_DEV)))

    PROTOBUF_COMPILER = protobuf-compiler_$(PROTOBUF_VERSION_FULL)_$(CONFIGURED_ARCH).deb
    $(PROTOBUF_COMPILER)_DEPENDS = $(PROTOC32)
    $(PROTOBUF_COMPILER)_RDEPENDS = $(PROTOC32)
    $(eval $(call add_derived_package,$(PROTOBUF),$(PROTOBUF_COMPILER)))

    PYTHON3_PROTOBUF = python3-protobuf_$(PROTOBUF_VERSION_FULL)_$(CONFIGURED_ARCH).deb
    $(PYTHON3_PROTOBUF)_DEPENDS = $(PROTOBUF_DEV) $(PROTOBUF)
    $(PYTHON3_PROTOBUF)_RDEPENDS = $(PROTOBUF)
    $(eval $(call add_derived_package,$(PROTOBUF),$(PYTHON3_PROTOBUF)))

    RUBY_PROTOBUF = ruby-google-protobuf_$(PROTOBUF_VERSION_FULL)_$(CONFIGURED_ARCH).deb
    $(RUBY_PROTOBUF)_DEPENDS = $(PROTOBUF_DEV) $(PROTOBUF)
    $(RUBY_PROTOBUF)_RDEPENDS = $(PROTOBUF)
    $(eval $(call add_derived_package,$(PROTOBUF),$(RUBY_PROTOBUF)))

endif
