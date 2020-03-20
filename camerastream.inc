#  @author Kritesh tripathi

# This file is the camerastream recipe.

SUMMARY = "An application that takes video feed from camera and displays on screen"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Pylon SDK
PYLONSDK_VERSION = "5.2.0.13457-arm64"
SRC_URI += "https://www.baslerweb.com/fp-1535524589/media/downloads/software/pylon_software/pylon-${PYLONSDK_VERSION}.tar.gz;name=pylonSDK "
SRC_URI[pylonSDK.md5sum] = "7311821e501c81db81e9e2d2eb9fa3d6"
SRC_URI[pylonSDK.sha256sum] = "c8de551c053e9a5094da3bed1ba3b2627f74c65dcf97d99be8b4b02a64e6b3a6"

S = "${WORKDIR}"

CXXFLAGS += " -I${STAGING_INCDIR}/drm/ "

do_compile () {
	tar zxf pylon-${PYLONSDK_VERSION}/pylonSDK-${PYLONSDK_VERSION}.tar.gz -C .
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 camstream_* ${D}${bindir}
	install -d ${D}${libdir}
	find pylon5/lib64 -type f -exec install -Dm 755 "{}" "${D}${libdir}/{}" \;
}

pkg_postinst_${PN} () {
	test -f $D/etc/profile && echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/usr/lib/pylon5/lib64" >> $D/etc/profile
}

FILES_${PN} += "${libdir}/*"

RDEPENDS_${PN}_append += " bash cairo "

INSANE_SKIP_${PN} = "already-stripped"
