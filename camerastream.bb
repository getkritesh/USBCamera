#  @author Kritesh tripathi

# This file is the camerastream recipe.

SRC_URI = " \
	file://XEGLIntf.h \
	file://XGLSLCompile.h \
	file://XLinuxTarget.h \
	file://XCVector.h \
	file://Xfbdev_window.h \
	file://XLinuxCamera.h \
	file://XCamera.h \
	file://XEGLIntf.cpp \
	file://XGLSLCompile.cpp \
	file://XLinuxCamera.cpp \
	file://XLinuxTarget.cpp \
	file://XCamera.cpp \
	file://Core_egl_zero_copy.h \
	file://dma_buf.cpp \
	file://Core_DRM.h \
	file://Core_DRM.cpp \
	file://camstream.cpp \
	file://camOpenGL.h \
	file://camOpenGL.cpp \
	file://camera.h \
	file://pylonCam.h \
	file://pylonCam.cpp \
	file://sharedQueue.h \
	file://camutils.h \
	file://camutils.cpp \
	file://Makefile \
	"

require camerastream.inc

DEPENDS = " \
	libmali-xlnx \
	libx11 \
	libxext \
	libxfixes \
	libxdamage \
	libdrm \
	expat \
	libxxf86vm \
	mesa-gl \
	cairo \
	"

do_compile_append () {
	oe_runmake
}
