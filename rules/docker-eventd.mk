# docker image for eventd

DOCKER_EVENTD_STEM = docker-eventd
DOCKER_EVENTD = $(DOCKER_EVENTD_STEM).gz
DOCKER_EVENTD_DBG = $(DOCKER_EVENTD_STEM)-$(DBG_IMAGE_MARK).gz

$(DOCKER_EVENTD)_DEPENDS += $(SONIC_EVENTD)

$(DOCKER_EVENTD)_DBG_DEPENDS = $($(DOCKER_CONFIG_ENGINE_JAMMY)_DBG_DEPENDS)
$(DOCKER_EVENTD)_DBG_DEPENDS += $(SONIC_EVENTD_DBG) $(LIBSWSSCOMMON_DBG)

$(DOCKER_EVENTD)_DBG_IMAGE_PACKAGES = $($(DOCKER_CONFIG_ENGINE_JAMMY)_DBG_IMAGE_PACKAGES)

$(DOCKER_EVENTD)_LOAD_DOCKERS = $(DOCKER_CONFIG_ENGINE_JAMMY)

$(DOCKER_EVENTD)_PATH = $(DOCKERS_PATH)/$(DOCKER_EVENTD_STEM)

$(DOCKER_EVENTD)_INSTALL_PYTHON_WHEELS = $(SONIC_UTILITIES_PY3)
$(DOCKER_EVENTD)_INSTALL_DEBS = $(PYTHON3_SWSSCOMMON)

$(DOCKER_EVENTD)_VERSION = 1.0.0
$(DOCKER_EVENTD)_PACKAGE_NAME = eventd

SONIC_DOCKER_IMAGES += $(DOCKER_EVENTD)
ifeq ($(INCLUDE_SYSTEM_EVENTD), y)
SONIC_INSTALL_DOCKER_IMAGES += $(DOCKER_EVENTD)
endif

SONIC_DOCKER_DBG_IMAGES += $(DOCKER_EVENTD_DBG)
ifeq ($(INCLUDE_SYSTEM_EVENTD), y)
SONIC_INSTALL_DOCKER_DBG_IMAGES += $(DOCKER_EVENTD_DBG)
endif

$(DOCKER_EVENTD)_FILES += $(SUPERVISOR_PROC_EXIT_LISTENER_SCRIPT)
$(DOCKER_EVENTD)_CONTAINER_NAME = eventd
$(DOCKER_EVENTD)_RUN_OPT += -t
$(DOCKER_EVENTD)_RUN_OPT += -v /etc/sonic:/etc/sonic:ro
$(DOCKER_EVENTD)_RUN_OPT += -v /etc/timezone:/etc/timezone:ro 

SONIC_JAMMY_DOCKERS += $(DOCKER_EVENTD)
SONIC_JAMMY_DBG_DOCKERS += $(DOCKER_EVENTD_DBG)
