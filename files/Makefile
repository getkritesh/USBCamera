# camerastream Makefile
#
# @author Kritesh tripathi
# 

APP_X11 = camstream_x11
APP_X11_DMA = camstream_x11_dma
APP_FBDEV = camstream_fbdev

APP_FBDEV_OBJS = XGLSLCompile.o Core_DRM.o dma_buf.o \
			camutils.o camstream.o camOpenGL.o pylonCam.o

APP_OBJS = $(APP_FBDEV_OBJS) XEGLIntf.o \
			XLinuxCamera.o XLinuxTarget.o XCamera.o

X11_LIBS = -lX11 -lxcb -lXau -lXdmcp -lXfixes -lXext -lXdamage -lexpat \
			-lglapi -lX11-xcb -lxcb-glx -lxcb-dri2 -lXxf86vm
OPENGL_LIBS = -ldrm -lexpat -lEGL -lGLESv2
PYLON5_LIBS = -lpylonbase -lpylonutility -lGenApi_gcc_v3_1_Basler_pylon \
				-lGCBase_gcc_v3_1_Basler_pylon

LIBDIR = pylon5/lib64
INCDIR = pylon5/include/

CXXFLAGS += -O2 -I$(INCDIR)
LDFLAGS += -L$(LIBDIR)

X11_DEFINE := -DX11_WINDOW
DMABUFFER := -DDMA_BUF_IMPORT

all: build

build:
	@$(MAKE) build_X11
	@$(MAKE) build_X11_DMA

build_X11:
	@rm -f $(APP_X11)
	$(eval CXXFLAGS += $(X11_DEFINE))
	@$(MAKE) $(APP_X11)
	@$(MAKE) clean

build_X11_DMA:
	@rm -f $(APP_X11_DMA)
	$(eval CXXFLAGS += $(X11_DEFINE))
	$(eval CXXFLAGS += $(DMABUFFER))
	@$(MAKE) $(APP_X11_DMA)
	@$(MAKE) clean

build_fbdev:
	@rm -f $(APP_FBDEV)
	$(eval CXXFLAGS += $(DMABUFFER))
	@$(MAKE) $(APP_FBDEV)
	@$(MAKE) clean

$(APP_X11) $(APP_X11_DMA): $(APP_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(APP_OBJS) $(LDLIBS) -lm -lpthread -ldl \
		$(X11_LIBS) $(OPENGL_LIBS) $(PYLON5_LIBS) -lcairo

$(APP_FBDEV): $(APP_FBDEV_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(APP_FBDEV_OBJS) $(LDLIBS) -lm -lpthread -ldl \
		$(OPENGL_LIBS) $(PYLON5_LIBS) -lcairo

clean:
	@rm -f *.o


