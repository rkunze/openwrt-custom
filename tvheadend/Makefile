#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tvheadend
PKG_VERSION:=3.9-2249-g920cb35
PKG_SOURCE_VERSION:=40a2e93c8ade6b823e47cfa0b2a6264668cdd2dc
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/rkunze/tvheadend.git
#PKG_SOURCE:=$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=https://github.com/tvheadend/tvheadend/archive/
#PKG_MD5SUM:=86d1be0ad6e02bd2aecd3d529a026797

PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

ifneq ($(PACKAGE_sundtek-dvb-driver),n)
 TARGET_LDFLAGS +=-lmediaclient
endif

define Package/tvheadend
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=Tvheadend is a TV streaming server for Linux
  DEPENDS:=$(ICONV_DEPENDS) +libcurl +libopenssl +librt +zlib +TVHEADEND_AVAHI_SUPPORT:libavahi-client +PACKAGE_sundtek-dvb-driver:sundtek-dvb-driver
  URL:=https://tvheadend.org/
  USERID:=tvheadend:video
endef

define Package/tvheadend/description
  Tvheadend is a TV streaming server for Linux supporting DVB, ATSC, IPTV,
  and Analog video (V4L) as input sources.
  Can be used as a backend to Showtime, XBMC and various other clients.
endef

define Package/tvheadend/config
  menu "Configuration"
  depends on PACKAGE_tvheadend
  source "$(SOURCE)/Config.in"
  endmenu
endef

define Package/tvheadend/postinst
	#!/bin/sh
	chown $(USERID) /etc/tvheadend
endef

ECHO := echo
define Build/Compile
  $(INSTALL_DIR) $(PKG_BUILD_DIR)/support/rpm/
	$(ECHO) '$(PKG_VERSION)' > $(PKG_BUILD_DIR)/support/rpm/version
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/$(MAKE_PATH) $(MAKE_FLAGS)
endef

ifeq ($(CONFIG_TVHEADEND_CWC_SUPPORT),)
  CONFIGURE_ARGS += --disable-cwc
endif

ifeq ($(CONFIG_TVHEADEND_V4L_SUPPORT),)
  CONFIGURE_ARGS += --disable-v4l
endif

ifeq ($(CONFIG_TVHEADEND_LINUXDVB_SUPPORT),)
  CONFIGURE_ARGS += --disable-linuxdvb
endif

ifeq ($(CONFIG_TVHEADEND_DVBSCAN_SUPPORT),)
  CONFIGURE_ARGS += --disable-dvbscan
endif

ifeq ($(CONFIG_TVHEADEND_AVAHI_SUPPORT),)
  CONFIGURE_ARGS += --disable-avahi
else
  CONFIGURE_ARGS += --enable-avahi
endif

CONFIGURE_ARGS += \
	--release \
	--enable-bundle \
	--disable-libav \
	--disable-dbus_1
	

define Package/tvheadend/install
	$(INSTALL_DIR) $(1)/etc/tvheadend
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/tvheadend.init $(1)/etc/init.d/tvheadend
	$(INSTALL_DIR) $(1)/lib/upgrade/keep.d
	$(INSTALL_DATA) ./files/tvheadend.upgrade $(1)/lib/upgrade/keep.d/tvheadend
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build.linux/tvheadend $(1)/usr/bin/
endef

$(eval $(call BuildPackage,tvheadend))
