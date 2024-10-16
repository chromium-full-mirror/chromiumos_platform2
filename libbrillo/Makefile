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
LIBBRILLO_CPPFLAGS:=["-D_LARGEFILE_SOURCE","-D_LARGEFILE64_SOURCE","-DNDEBUG", \
  "-DUSE_RTTI_FOR_TYPE_TAGS"]
LIBBRILLO_CXXFLAGS:=["-Os","-pipe","-Wunused-function","-Wno-error", \
  "-std=c++17","-fno-exceptions","-fno-unwind-tables", \
  "-fno-asynchronous-unwind-tables","-Wno-implicit-fallthrough", \
  "-fpermissive","-fPIC","-DOS_CHROMEOS=1",\
  "-I$(BUILD_DIR)/lvm2-normal/LVM2.2.03.16/tools/", \
  "-I$(STAGING_DIR)/usr/include","-D_FILE_OFFSET_BITS=64",\
  "-D_LARGEFILE_SOURCE","-D_LARGEFILE64_SOURCE","-DNDEBUG"]
LIBBRILLO_LDFLAGS:=["-Wl,-O2","-Wl,--as-needed","-Wl,-Bdynamic", \
  "-L$(STAGING_DIR_ROOT)/usr/lib"]

USE_FLAGS:={arm=$(ARM_ARCH) abi_x86_64=$(X86_ARCH) amd64=$(X86_ARCH) \
  fuzzer=false cros_host=false test=false dbus=true device_mapper=false \
  udev=false profiling=false tcmalloc=false}

GN_ARGS:=platform_subdir="libbrillo" OS="linux" \
	 platform2_root="$(BUILD_DIR)" \
	 libdir="$(STAGING_DIR_ROOT)/usr/lib" \
	 libbase_ver="860220" \
	 sysroot="$(STAGING_DIR_ROOT)" \
         build_root="$(PKG_BUILD_DIR)/"\
	 pkg_config="$(STAGING_DIR_HOST)/bin/pkg-config" \
	 cc="$(TARGET_CC)" ar="$(TARGET_AR)" cxx="$(TARGET_CXX)" \
	 enable_exceptions=false \
         external_cxxflags=$(LIBBRILLO_CXXFLAGS) \
	 external_cflags=$(LIBBRILLO_CFLAGS) \
	 external_cppflags=$(LIBBRILLO_CPPFLAGS) \
	 external_ldflags=$(LIBBRILLO_LDFLAGS) \
	 use=$(USE_FLAGS)

PLATFORM2_DIR=$(TOPDIR)/package/gwifi/external/platform2
HEADERS_DIRS:=brillo brillo/blkdev_utils brillo/crossystem brillo/daemons \
  brillo/dbus brillo/errors brillo/files brillo/glib brillo/grpc brillo/http \
  brillo/message_loops brillo/minijail brillo/namespaces brillo/process \
  brillo/streams brillo/strings brillo/timezone brillo/udev

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
	$(CP) $(PKG_BUILD_DIR)/out/Default/obj/libbrillo/*.pc* $(1)/usr/lib/pkgconfig
	$(foreach header_dir,$(HEADERS_DIRS), \
		$(INSTALL_DIR) $(1)/usr/include/$(header_dir); \
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(header_dir)/*.h $(1)/usr/include/$(header_dir)/;)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/out/Default/lib/*.so $(1)/usr/lib
endef

$(eval $(call BuildPackage,libbrillo))
