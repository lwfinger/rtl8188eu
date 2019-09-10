SUMMARY = "Realtek rtl8188eu driver"
DESCRIPTION = "lwfinger's rtl8188eu driver that enables AP mode support"
AUTHOR = "Yevhen Fastiuk <yevfast@gmail.com>"
SECTION = "Connectivity"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

inherit module

S = "${WORKDIR}/git"

SRCREV = "${AUTOREV}"
SRC_URI = "git://github.com/lwfinger/rtl8188eu.git;branch=v4.1.8_9499"

RTLWIFI = "${base_libdir}/firmware/rtlwifi"

do_install_append() {
	install -d ${D}/${RTLWIFI}
	install -m 0755 ${S}/rtl8188eufw.bin ${D}/${RTLWIFI}/
}

FILES_${PN} += "${RTLWIFI}/*"
