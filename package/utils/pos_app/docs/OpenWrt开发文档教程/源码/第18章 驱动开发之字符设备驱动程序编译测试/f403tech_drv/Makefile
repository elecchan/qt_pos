#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=f403tech_drv
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/f403tech_drv
  SUBMENU:=Other modules
  # DEPENDS:=@!LINUX_3_3
  TITLE:=Motor driver
  FILES:=$(PKG_BUILD_DIR)/f403tech_drv.ko
  # AUTOLOAD:=$(call AutoLoad,30,f403tech_drv,1)
  KCONFIG:=
endef

define KernelPackage/f403tech_drv/description
 This is a f403tech_drv drivers
 endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,f403tech_drv))
