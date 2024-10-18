#
# TODO(b/371112401): Add license information
#

include $(TOPDIR)/rules.mk

PKG_NAME:=webserver
PKG_RELEASE:=1
PKG_BUILD_DEPENDS:=chromeos-dbus-bindings

include $(INCLUDE_DIR)/package.mk

define Package/webserver
  TITLE:= HTTP sever interface library
  SECTION:=gwifi
  CATEGORY:=Nest WiFi Mesh Solutions
  SUBMENU:=Libs
  DEPENDS:=+libchrome +libbrillo +libmicrohttpd +libopenssl +minijail
endef

define Package/$(PKG_NAME)/description
  HTTP sever interface library
endef

ifeq ($(ARCH),arm)
        ARM_ARCH=true
        X86_ARCH=false
else
        X86_ARCH=true
        ARM_ARCH=false
endif

WEBSERVER_CFLAGS:=["-O2", "-pipe", "-std=gnu11", "-I$(TOOLCHAIN_DIR)/include"]
WEBSERVER_CPPFLAGS:=["-D_LARGEFILE_SOURCE","-D_LARGEFILE64_SOURCE","-DNDEBUG", \
  "-DUSE_RTTI_FOR_TYPE_TAGS"]
WEBSERVER_CXXFLAGS:=["-Os","-pipe","-Wunused-function","-Wno-error", \
  "-std=c++17","-fno-exceptions","-fno-unwind-tables", \
  "-fno-asynchronous-unwind-tables","-Wno-implicit-fallthrough", \
  "-fpermissive","-fPIC","-DOS_CHROMEOS=1","-I$(STAGING_DIR)/usr/include"]
WEBSERVER_LDFLAGS:=["-Wl,-O2","-Wl,--as-needed","-Wl,-Bdynamic", \
  "-L$(STAGING_DIR_ROOT)/usr/lib"]

USE_FLAGS:={arm=$(ARM_ARCH) abi_x86_64=$(X86_ARCH) amd64=$(X86_ARCH) \
  test=false cros_host=false profiling=false tcmalloc=false}

GN_ARGS:=platform_subdir="webserver" target_os="prplos" OS="linux" \
	 platform2_root="$(BUILD_DIR)" \
	 sysroot="$(STAGING_DIR_ROOT)" build_root="$(PKG_BUILD_DIR)/" \
	 pkg_config="$(STAGING_DIR_HOST)/bin/pkg-config" \
	 cc="$(TARGET_CC)" ar="$(TARGET_AR)" \
	 cxx="$(TARGET_CXX)" \
	 libdir="$(STAGING_DIR_ROOT)/usr/lib" \
         external_cxxflags=$(WEBSERVER_CXXFLAGS) \
	 external_cflags=$(WEBSERVER_CFLAGS) \
	 external_cppflags=$(WEBSERVER_CPPFLAGS) \
	 external_ldflags=$(WEBSERVER_LDFLAGS) \
	 use=$(USE_FLAGS)

PLATFORM2_DIR:=$(TOPDIR)/package/gwifi/external/platform2
LIBCHROME_PATH:=$(TOPDIR)/package/gwifi/external/libchrome
LIBCHROME_VER:=$(shell cat $(LIBCHROME_PATH)/BASE_VER)

define Build/Prepare
	$(CP) $(PLATFORM2_DIR)/webserver/ $(BUILD_DIR)/
	$(CP) $(PLATFORM2_DIR)/common-mk/ $(BUILD_DIR)/
	$(CP) $(PLATFORM2_DIR)/.gn $(BUILD_DIR)/
	$(call Build/Prepare/Default)
endef

define Build/Configure
	cd $(PKG_BUILD_DIR) && $(STAGING_DIR_HOSTPKG)/bin/gn gen \
	  '--root=$(BUILD_DIR)' '--args=$(GN_ARGS)' $(PKG_BUILD_DIR)/out/Default
endef

define Build/Compile
	cd $(PKG_BUILD_DIR) && $(STAGING_DIR_HOST)/bin/ninja \
	  -C $(PKG_BUILD_DIR)/out/Default webserver:all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/libwebserv
	$(INSTALL_DIR) $(1)/usr/include/webservd
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_BUILD_DIR)/libwebserv/*.h $(1)/usr/include/libwebserv
	$(CP) $(PKG_BUILD_DIR)/out/Default/gen/include/dbus_bindings/*.h $(1)/usr/include/libwebserv
	$(CP) $(PKG_BUILD_DIR)/out/Default/gen/include/webservd/*.h $(1)/usr/include/webservd
	cd $(PKG_BUILD_DIR) && ./libwebserv/preinstall.sh $(PKG_BUILD_DIR)/out/Default/ $(LIBCHROME_VER)
	$(CP) $(PKG_BUILD_DIR)/out/Default/lib/libwebserv.pc $(1)/usr/lib/pkgconfig
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/dbus-1/system.d
	$(CP) $(PKG_BUILD_DIR)/out/Default/lib/libwebserv.so $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/webservd/etc/dbus-1/org.chromium.WebServer.conf $(1)/etc/dbus-1/system.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/out/Default/webservd $(1)/usr/sbin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
