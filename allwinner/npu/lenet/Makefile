include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/package.mk

PKG_NAME:=lenet
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

define Package/$(PKG_NAME)
  SUBMENU:=NPU
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=lenet demo
  DEPENDS:=+libjpeg-turbo +viplite-driver
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_LDFLAGS+=-L$(STAGING_DIR)/usr/lib/
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
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) 	$(1)/usr/bin/
endef

define Package/$(PKG_NAME)-model
  SUBMENU:=NPU
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=lenet demo model (70 KB)
  DEPENDS:=lenet
endef

define Package/$(PKG_NAME)-model/install
	$(INSTALL_DIR) $(1)/etc/models
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/lenet_model.nb $(1)/etc/models/lenet_model.nb
	$(INSTALL_DIR) $(1)/etc/data
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/lenet_input_0.dat $(1)/etc/data/lenet_input_0.dat
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-model))
