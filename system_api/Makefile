#
# TODO(b/371112401): Add license information
#

include $(TOPDIR)/rules.mk

PKG_NAME:=system_api
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/system_api
  SECTION:=gwifi
  CATEGORY:=Nest WiFi Mesh Solutions
  SUBMENU:=Libs
  TITLE:=Chrome OS system API (D-Bus service names, etc.)
endef

PLATFORM2_DIR:=$(TOPDIR)/package/gwifi/external/platform2
HEADERS_DIRS:=anomaly_detector arc-data-snapshotd attestation authpolicy biod \
  bluetooth bootlockbox cecservice cfm chunneld cros-disks cros_healthd \
  cryptohome debugd  dlcservice dlp drivefs hammerd hermes hps ip_peripheral \
  kerberos login_manager lorgnette minios missive ocr oobe_config \
  os_install_service patchpanel pciguard permission_broker power_manager \
  resource_manager rmad runtime_probe seneschal shill smbfs smbprovider \
  system_proxy tpm_manager typecd u2f update_engine usbguard vm_applications \
  vm_cicerone vm_concierge vm_disk_management vm_plugin_dispatcher \
  vm_sk_forwarding wilco_dtc_supportd

define Build/Prepare
	$(CP) $(PLATFORM2_DIR)/system_api/ $(BUILD_DIR)/
	$(CP) $(PLATFORM2_DIR)/common-mk/ $(BUILD_DIR)/
	$(CP) $(PLATFORM2_DIR)/.gn $(BUILD_DIR)/
endef

define Build/Compile
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/chromeos/dbus
	$(CP) $(PKG_BUILD_DIR)/dbus/service_constants.h $(1)/usr/include/chromeos/dbus
	$(foreach header_dir,$(HEADERS_DIRS), \
		$(INSTALL_DIR) $(1)/usr/include/chromeos/$(header_dir); \
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/dbus/$(header_dir)/dbus-constants.h \
		    $(1)/usr/include/chromeos/$(header_dir)/;)
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
