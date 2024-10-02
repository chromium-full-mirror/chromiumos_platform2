#
# TODO(b/371112401): Add license information
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libbrillo
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libbrillo
  SECTION:=gwifi
  CATEGORY:=Nest WiFi Mesh Solutions
  SUBMENU:=Libs
  DEPENDS:=+protobuf +libstdcpp +libchrome +libdevmapper
  TITLE:=Base library for Chromium OS
endef

define Package/libbrillo/description
  Base library for Chromium OS
endef

ifeq ($(ARCH),arm)
        ARM_ARCH=true
        X86_ARCH=false
else
        X86_ARCH=true
        ARM_ARCH=false
endif

LIBBRILLO_CFLAGS:=["-O2", "-pipe", "-std=gnu11", "-I$(TOOLCHAIN_DIR)/include"]
LIBBRILLO_CPPFLAGS:=["-D_LARGEFILE_SOURCE","-D_LARGEFILE64_SOURCE","-DNDEBUG"]
LIBBRILLO_CXXFLAGS:=["-Os","-pipe","-Wunused-function","-Wno-error", \
  "-std=c++17","-fno-exceptions","-fno-unwind-tables", \
  "-fno-asynchronous-unwind-tables","-Wno-implicit-fallthrough", \
  "-fpermissive","-fPIC","-DOS_CHROMEOS=1","-I$(BUILD_DIR)/modp_b64", \
  "-I$(STAGING_DIR)/usr/include","-I$(STAGING_DIR)/usr/include/glib-2.0", \
  "-I$(STAGING_DIR)/usr/include/nspr","-I$(STAGING_DIR)/usr/include/nss", \
  "-I$(STAGING_DIR)/usr/include/dbus-1.0","-I$(BUILD_DIR)/libchrome", \
  "-I$(BUILD_DIR)/lvm2-normal/LVM2.2.03.16/tools/", \
  "-I$(STAGING_DIR)/usr/lib/dbus-1.0/include","-D_FILE_OFFSET_BITS=64",\
  "-D_LARGEFILE_SOURCE","-D_LARGEFILE64_SOURCE","-DNDEBUG"]
LIBBRILLO_LDFLAGS:=["-Wl,-O2","-Wl,--as-needed","-Wl,-Bdynamic"]

USE_FLAGS:={arm=$(ARM_ARCH) abi_x86_64=$(X86_ARCH) amd64=$(X86_ARCH) \
  fuzzer=false cros_host=true test=false dbus=true device_mapper=true \
  udev=false profiling=false tcmalloc=false}

GN_ARGS:=platform_subdir="libbrillo" OS="linux" \
	 platform2_root="$(BUILD_DIR)" \
	 sysroot="$(STAGING_DIR)" \
	 pkg_config="$(STAGING_DIR_HOST)/bin/pkg-config" \
	 cc="$(TARGET_CC)" ar="$(TARGET_AR)" build_root="$(PKG_BUILD_DIR)/"\
	 enable_exceptions=false \
	 libbase_ver="860220" \
	 cxx="$(TARGET_CXX)" \
	 libdir="$(STAGING_DIR_ROOT)/usr/lib" sysroot="$(STAGING_DIR)" \
         external_cxxflags=$(LIBBRILLO_CXXFLAGS) \
	 external_cflags=$(LIBBRILLO_CFLAGS) \
	 external_cppflags=$(LIBBRILLO_CPPFLAGS) \
	 external_ldflags=$(LIBBRILLO_LDFLAGS) \
	 use=$(USE_FLAGS)

PLATFORM2_DIR=$(TOPDIR)/package/gwifi/external/platform2
HEADER_DIRS:=brillo brillo/message_loops brillo/daemons brillo/blkdev_utils \
  brillo/dbus brillo/udev brillo/files brillo/grpc brillo/streams \
  brillo/process brillo/http brillo/errors brillo/strings

define Build/Prepare
	$(CP) $(PLATFORM2_DIR)/libbrillo/ $(BUILD_DIR)/
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
	  -C $(PKG_BUILD_DIR)/out/Default libbrillo:all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/out/Default/obj/libbrillo/*.pc* $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_BUILD_DIR)/out/Default/lib/*.so $(1)/usr/lib
	$(foreach header_dir,$(HEADERS_DIRS), \
		$(INSTALL_DIR) $(1)/usr/include/$(header_dir); \
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(header_dir)/*.h $(1)/usr/include/$(header_dir)/;)
endef

$(eval $(call BuildPackage,libbrillo))
