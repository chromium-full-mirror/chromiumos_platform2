# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include $(TOPDIR)/rules.mk

PKG_NAME:=libbrillo
PKG_VERSION:=1.0
PKG_RELEASE:=1

SOURCE_DIR:=$(TOPDIR)/package/gwifi/platform2/libbrillo
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=BSD-3

include $(INCLUDE_DIR)/package.mk

define Package/libbrillo/Default
  SECTION:=gwifi
  CATEGORY:=Nest WiFi Mesh Solutions
  TITLE:=ChromiumOS's libbrillo library
  SUBMENU:=libbrillo libraries
endef

define Package/libbrillo/description
libbrillo is a shared library meant to hold common utility
code that we deem useful for platform projects.
endef

define Package/libbrillo
$(call Package/libbrillo/Default)
  TITLE:=Shared libbrillo for target
endef

define Package/libbrillo-binder
$(call Package/libbrillo/Default)
  TITLE:=Shared binder library for target
endef

define Package/libbrillo-dbus
$(call Package/libbrillo/Default)
  TITLE:=Shared dbus library for target
endef

define Package/libbrillo-minijail
$(call Package/libbrillo/Default)
  TITLE:=Shared minijail library for target
endef

define Package/libbrillo-stream
$(call Package/libbrillo/Default)
  TITLE:=Shared stream library for target
endef

define Package/libbrillo-http
$(call Package/libbrillo/Default)
  TITLE:=Shared http library for target
endef

define Package/libbrillo-policy
$(call Package/libbrillo/Default)
  TITLE:=Shared policy library for target
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/libbrillo/install
endef

define Package/libbrillo-binder/install
endef

define Package/libbrillo-dbus/install
endef

define Package/libbrillo-minijail/install
endef

define Package/libbrillo-stream/install
endef

define Package/libbrillo-http/install
endef

define Package/libbrillo-policy/install
endef

$(eval $(call BuildPackage,libbrillo))
$(eval $(call BuildPackage,libbrillo-binder))
$(eval $(call BuildPackage,libbrillo-dbus))
$(eval $(call BuildPackage,libbrillo-minijail))
$(eval $(call BuildPackage,libbrillo-stream))
$(eval $(call BuildPackage,libbrillo-http))
$(eval $(call BuildPackage,libbrillo-policy))
