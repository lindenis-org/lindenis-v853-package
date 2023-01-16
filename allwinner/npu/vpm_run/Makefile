include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/package.mk

PKG_NAME:=vpm_run
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

define Package/$(PKG_NAME)
  SUBMENU:=NPU
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=vpm model runner 
  DEPENDS:=+viplite-driver
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/src
	$(CP) -fr ./src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		TARGET="$(PKG_NAME)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vpm_run $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
