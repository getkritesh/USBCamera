#  @author Kritesh tripathi

# This file is the camerastream fbdev recipe.

SRC_URI = " \
    file://XGLSLCompile.cpp \
    file://XGLSLCompile.h \
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
	libdrm \
	cairo \
	"

do_compile_append () {
	oe_runmake build_fbdev
}
