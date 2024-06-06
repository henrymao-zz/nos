LIBSAI_BRONCOS_VERSION = 3.11
LIBSAI_BRONCOS_BRANCH_NAME = REL_3.11
LIBSAI_BRONCOS_URL_PREFIX = "https://sonicstorage.blob.core.windows.net/public/sai/bcmpai/$(LIBSAI_BRONCOS_BRANCH_NAME)/$(LIBSAI_BRONCOS_VERSION)"
LIBSAI_BRONCOS = libsaibroncos_$(LIBSAI_BRONCOS_VERSION)_amd64.deb
$(LIBSAI_BRONCOS)_URL = "$(LIBSAI_BRONCOS_URL_PREFIX)/$(LIBSAI_BRONCOS)"

ifneq ($($(LIBSAI_BRONCOS)_URL),)

DOCKER_GBSYNCD_BRONCOS_STEM = docker-gbsyncd-broncos
DOCKER_GBSYNCD_BRONCOS = $(DOCKER_GBSYNCD_BRONCOS_STEM).gz
DOCKER_GBSYNCD_BRONCOS_DBG = $(DOCKER_GBSYNCD_BRONCOS_STEM)-$(DBG_IMAGE_MARK).gz

$(DOCKER_GBSYNCD_BRONCOS)_FILES += $(SUPERVISOR_PROC_EXIT_LISTENER_SCRIPT)

$(DOCKER_GBSYNCD_BRONCOS)_LOAD_DOCKERS += $(DOCKER_CONFIG_ENGINE_JAMMY)

$(DOCKER_GBSYNCD_BRONCOS)_DBG_DEPENDS += $($(DOCKER_CONFIG_ENGINE_JAMMY)_DBG_DEPENDS)

$(DOCKER_GBSYNCD_BRONCOS)_DBG_IMAGE_PACKAGES = $($(DOCKER_CONFIG_ENGINE_JAMMY)_DBG_IMAGE_PACKAGES)

SONIC_DOCKER_IMAGES += $(DOCKER_GBSYNCD_BRONCOS)
SONIC_JAMMY_DOCKERS += $(DOCKER_GBSYNCD_BRONCOS)
SONIC_INSTALL_DOCKER_IMAGES += $(DOCKER_GBSYNCD_BRONCOS)

SONIC_DOCKER_DBG_IMAGES += $(DOCKER_GBSYNCD_BRONCOS_DBG)
SONIC_JAMMY_DBG_DOCKERS += $(DOCKER_GBSYNCD_BRONCOS_DBG)
SONIC_INSTALL_DOCKER_DBG_IMAGES += $(DOCKER_GBSYNCD_BRONCOS_DBG)

$(DOCKER_GBSYNCD_BRONCOS)_CONTAINER_NAME = gbsyncd
$(DOCKER_GBSYNCD_BRONCOS)_RUN_OPT += --privileged -t
$(DOCKER_GBSYNCD_BRONCOS)_RUN_OPT += -v /host/machine.conf:/etc/machine.conf
$(DOCKER_GBSYNCD_BRONCOS)_RUN_OPT += -v /etc/sonic:/etc/sonic:ro
$(DOCKER_GBSYNCD_BRONCOS)_RUN_OPT += -v /host/warmboot:/var/warmboot

SONIC_ONLINE_DEBS += $(LIBSAI_BRONCOS)

$(DOCKER_GBSYNCD_BRONCOS)_VERSION = 1.0.0
$(DOCKER_GBSYNCD_BRONCOS)_PACKAGE_NAME = gbsyncd-broncos
$(DOCKER_GBSYNCD_BRONCOS)_PATH = $(PLATFORM_PATH)/../components/docker-gbsyncd-broncos
$(DOCKER_GBSYNCD_BRONCOS)_DEPENDS += $(SYNCD) $(LIBSAI_BRONCOS)
endif
