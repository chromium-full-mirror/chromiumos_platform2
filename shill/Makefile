#
# TODO(b/371112401): Add license information
#

include $(TOPDIR)/rules.mk

PKG_NAME:=shill-net
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/shill-net
  SECTION:=gwifi
  CATEGORY:=Nest WiFi Mesh Solutions
  SUBMENU:=Libs
  DEPENDS:=+libchrome +libbrillo
  TITLE:=libshill-net library for IPAddress and netlink
endef

LIBSHILLNET_CFLAGS:=["-O2", "-pipe", "-std=gnu11", "-I$(TOOLCHAIN_DIR)/include"]
LIBSHILLNET_CPPFLAGS:=["-DUSE_RTTI_FOR_TYPE_TAGS"]
LIBSHILLNET_CXXFLAGS:=["-Os","-pipe","-Wunused-function","-Wno-error", \
  "-std=c++17","-fno-exceptions","-fno-unwind-tables", \
  "-fno-asynchronous-unwind-tables","-Wno-implicit-fallthrough", \
  "-fpermissive","-fPIC","-DOS_CHROMEOS=1","-I$(STAGING_DIR)/usr/include", \
  "-I$(BUILD_DIR)/libchrome","-I$(BUILD_DIR)/libbrillo"]
LIBSHILLNET_LDFLAGS:=["-Wl,-O2","-Wl,--as-needed","-Wl,-Bdynamic"]

USE_FLAGS:={fuzzer=false cros_host=true test=false profiling=false \
  tcmalloc=false}

GN_ARGS:=platform_subdir="shill/net" OS="linux" \
	 platform2_root="$(BUILD_DIR)" \
	 sysroot="$(STAGING_DIR)" \
	 pkg_config="$(STAGING_DIR_HOST)/bin/pkg-config" \
	 cc="$(TARGET_CC)" ar="$(TARGET_AR)" build_root="$(PKG_BUILD_DIR)/"\
	 cxx="$(TARGET_CXX)" \
	 libdir="$(STAGING_DIR_ROOT)/usr/lib" sysroot="$(STAGING_DIR)" \
         external_cxxflags=$(LIBSHILLNET_CXXFLAGS) \
	 external_cflags=$(LIBSHILLNET_CFLAGS) \
	 external_cppflags=$(LIBSHILLNET_CPPFLAGS) \
	 external_ldflags=$(LIBSHILLNET_LDFLAGS) \
	 use=$(USE_FLAGS)

PLATFORM2_DIR=$(TOPDIR)/package/gwifi/external/platform2

define Build/Prepare
	$(CP) $(PLATFORM2_DIR)/shill/ $(BUILD_DIR)/
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
	  -C $(PKG_BUILD_DIR)/out/Default shill/net:libshill-net
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DIR) $(1)/usr/include/shill/net
	$(CP) $(BUILD_DIR)/shill/net/*.h $(1)/usr/include/shill/net
	$(CP) $(BUILD_DIR)/shill/net/libshill-net.pc $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_BUILD_DIR)/out/Default/lib/libshill-net.so $(1)/usr/lib
endef

$(eval $(call BuildPackage,shill-net))
