include $(TOPDIR)/rules.mk

PKG_NAME:=viplite-driver
PKG_VERSION:=vip-1.8.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=viplite-driver npu-runtime-test
  DEPENDS:=+libstdcpp
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -fr ./sdk_release $(PKG_BUILD_DIR)/
	$(CP) -fr ./algo_aw $(PKG_BUILD_DIR)/
	mkdir -p $(PKG_BUILD_DIR)/src
	$(CP) -fr ./src/* $(PKG_BUILD_DIR)/src
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sdk_release/libVIPlite.so $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sdk_release/libVIPuser.so $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/algo_aw/awnn_det.h $(1)/usr/include/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/algo_aw/cross_utils.h $(1)/usr/include/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/algo_aw/libawnn_det.a $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sdk_release/sdk/*.h $(1)/usr/include
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sdk_release/libVIPlite.so $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sdk_release/libVIPuser.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
