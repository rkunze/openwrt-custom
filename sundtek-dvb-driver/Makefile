# 
# Copyright (C) 2013, 2014 richard.kunze@web.de
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sundtek-dvb-driver
PKG_VERSION:=20150907
PKG_RELEASE:=1
PKG_SOURCE:=installer.tar.gz
PKG_SOURCE_URL:=http://www.sundtek.de/media/netinst/openwrtarm4/
PKG_MD5SUM:=6ae5e12004327bb28320f349bc718fb7

PKG_ROOT_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/
PKG_BUILD_DIR:=$(PKG_ROOT_DIR)/build

include $(INCLUDE_DIR)/package.mk

define Package/sundtek-dvb-driver
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=Proprietary userspace driver for Sundtek DVB devices
  URL:=http://www.sundtek.de
  DEPENDS:=@arm @USE_UCLIBC +libpthread +librt
endef

define Package/sundtek-dvb-driver/description
  Packages the Sundtek driver blob in an OpenWRT friendly way.  
  For now, only ARM with uclibc is supported
endef

define Package/sundtek-dvb-driver/install
	chmod u+rw $(PKG_ROOT_DIR)/opt/bin/mediasrv
	$(RSTRIP) $(PKG_ROOT_DIR)
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_ROOT_DIR)/opt/lib/libmediaclient.so $(1)/lib
	$(CP) $(PKG_ROOT_DIR)/opt/bin/mediaclient $(1)/bin
	$(CP) $(PKG_ROOT_DIR)/opt/bin/mediasrv $(1)/sbin
	$(INSTALL_BIN) ./files/sundtek-dvb-driver.init $(1)/etc/init.d/sundtek-dvb
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_ROOT_DIR)/opt/lib/libmediaclient.so $(1)/lib
endef

define Build/Configure
endef

define Build/Compile
endef

$(eval $(call BuildPackage,sundtek-dvb-driver))
