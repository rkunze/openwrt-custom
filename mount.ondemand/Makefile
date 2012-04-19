# 
# Copyright (C) 2012 richard.kunze@web.de
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mount.ondemand
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/mount.ondemand
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=A minimal autofs based automounter
  DEPENDS:=+kmod-fs-autofs4
endef

define Package/mount.ondemand/description
  A minimal autofs implementation for a single mount point, realized
  as a mount helper program.

  usage: mount.ondemand SOURCE TARGET [-dfvsn] [-o OPT[,OPTS...]]
  or     mount -t ondemand [-fvsn] [-o OPT[,OPTS...]] SOURCE TARGET

  options:
     -d Debug, run autofs handler in foreground
     -f Fake mount, do not start autofs handler
     -v Verbose
     -s Passed through to the actual mount
     -n Passed through to the actual mount
  special -o arguments:
    fstype=TYPE Passed to the actual mount as "-t TYPE"
    timeout=SEC Expire timeout in seconds
    helper=CMD  (optional) program that is run on mount/umount
    other -o arguments are passed to the actual mount
  Helper programm calls:
    before mount: CMD prepare SOURCE TARGET
    after mount:  CMD mounted SOURCE TARGET
    after umount: CMD cleanup SOURCE TARGET
endef


define Package/mount.ondemand/install
	$(INSTALL_DIR) $(1)/sbin
	$(CP) $(PKG_BUILD_DIR)/mount.ondemand $(1)/sbin/
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

$(eval $(call BuildPackage,mount.ondemand))
