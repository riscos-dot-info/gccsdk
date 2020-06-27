--- qb/config.params.sh.orig	2020-03-14 13:19:26.479962062 +0000
+++ qb/config.params.sh	2020-03-14 13:30:50.381938525 +0000
@@ -2,29 +2,29 @@
 HAVE_ASSETS_DIR=           # Assets install directory
 HAVE_BIN_DIR=              # Binary install directory (Deprecated)
 HAVE_MAN_DIR=              # Manpage install directory (Deprecated)
-HAVE_GDI=yes               # GDI support (Win32-only)
+HAVE_GDI=no                # GDI support (Win32-only)
 HAVE_OPENGLES_LIBS=        # Link flags for custom GLES library
 HAVE_OPENGLES_CFLAGS=      # C-flags for custom GLES library
-HAVE_CACA=auto             # Libcaca support
-HAVE_SIXEL=auto            # Libsixel support
+HAVE_CACA=no               # Libcaca support
+HAVE_SIXEL=no              # Libsixel support
 HAVE_DEBUG=no              # Enable a debug build
 HAVE_LIBRETRODB=yes        # Libretrodb support
 HAVE_MENU=yes              # Enable menu drivers
 HAVE_MENU_WIDGETS=yes      # Enable menu widgets
 HAVE_RGUI=auto             # RGUI menu
-HAVE_MATERIALUI=auto       # MaterialUI menu
-HAVE_XMB=auto              # XMB menu
-HAVE_OZONE=auto            # Ozone menu
+HAVE_MATERIALUI=no         # MaterialUI menu
+HAVE_XMB=no                # XMB menu
+HAVE_OZONE=no              # Ozone menu
 HAVE_STRIPES=no            # Stripes menu
 HAVE_RUNAHEAD=yes          # Runahead support
-HAVE_DSOUND=auto           # DirectSound support
-HAVE_XAUDIO=auto           # XAudio support
-HAVE_WASAPI=auto           # WASAPI support
-HAVE_WINMM=auto            # WinMM support
+HAVE_DSOUND=no             # DirectSound support
+HAVE_XAUDIO=no             # XAudio support
+HAVE_WASAPI=no             # WASAPI support
+HAVE_WINMM=no              # WinMM support
 HAVE_CC_RESAMPLER=yes      # CC Resampler
 HAVE_SSL=auto              # SSL/mbedtls support
 C89_SSL=no
-HAVE_BUILTINMBEDTLS=auto   # Bake in the mbedtls library
+HAVE_BUILTINMBEDTLS=no     # Bake in the mbedtls library
 C89_BUILTINMBEDTLS=no
 CXX_BUILTINMBEDTLS=no
 HAVE_OVERLAY=yes           # Overlay support
@@ -32,13 +32,13 @@
 HAVE_DYNAMIC=yes           # Dynamic loading of libretro library
 HAVE_SDL=auto              # SDL support
 C89_SDL=no
-HAVE_SDL2=auto             # SDL2 support (disables SDL 1.x)
+HAVE_SDL2=no               # SDL2 support (disables SDL 1.x)
 C89_SDL2=no
-HAVE_LIBUSB=auto           # Libusb HID support
+HAVE_LIBUSB=no             # Libusb HID support
 C89_LIBUSB=no
 HAVE_DBUS=no               # dbus support
-HAVE_SYSTEMD=auto          # Systemd support
-HAVE_UDEV=auto             # Udev/Evdev gamepad support
+HAVE_SYSTEMD=no            # Systemd support
+HAVE_UDEV=no               # Udev/Evdev gamepad support
 HAVE_THREADS=auto          # Threading support
 HAVE_THREAD_STORAGE=auto   # Thread Local Storage support
 HAVE_FFMPEG=auto           # FFmpeg support
@@ -50,67 +50,67 @@
 HAVE_NETWORKGAMEPAD=auto   # Networked game pad (plus baked-in core)
 C89_NETWORKGAMEPAD=no
 HAVE_NETPLAYDISCOVERY=yes  # Add netplay discovery (room creation, etc.)
-HAVE_MINIUPNPC=auto        # Mini UPnP client library (for NAT traversal)
-HAVE_BUILTINMINIUPNPC=auto # Bake in Mini UPnP client library (for NAT traversal)
+HAVE_MINIUPNPC=no          # Mini UPnP client library (for NAT traversal)
+HAVE_BUILTINMINIUPNPC=no   # Bake in Mini UPnP client library (for NAT traversal)
 C89_BUILTINMINIUPNPC=no
 HAVE_D3D8=no               # Direct3D 8 support
-HAVE_D3D9=auto             # Direct3D 9 support
+HAVE_D3D9=no               # Direct3D 9 support
 C89_D3D9=no
-HAVE_D3D10=yes             # Direct3D 10 support
+HAVE_D3D10=no              # Direct3D 10 support
 C89_D3D10=no
-HAVE_D3D11=yes             # Direct3D 11 support
+HAVE_D3D11=no              # Direct3D 11 support
 C89_D3D11=no
-HAVE_D3D12=yes             # Direct3D 12 support
+HAVE_D3D12=no              # Direct3D 12 support
 C89_D3D12=no
-HAVE_D3DX=auto             # Direct3DX support
-HAVE_DINPUT=auto           # Dinput support
-HAVE_OPENGL=auto           # OpenGL 2.0 support
-HAVE_OPENGL_CORE=yes       # Modern OpenGL driver support (GLES3+/GL3.2 core+), requires OpenGL.
+HAVE_D3DX=no               # Direct3DX support
+HAVE_DINPUT=no             # Dinput support
+HAVE_OPENGL=no             # OpenGL 2.0 support
+HAVE_OPENGL_CORE=no        # Modern OpenGL driver support (GLES3+/GL3.2 core+), requires OpenGL.
 C89_OPENGL_CORE=no
-HAVE_OPENGL1=yes           # OpenGL 1.1 support
+HAVE_OPENGL1=no            # OpenGL 1.1 support
 HAVE_MALI_FBDEV=no         # Mali fbdev context support
 HAVE_VIVANTE_FBDEV=no      # Vivante fbdev context support
 HAVE_OPENDINGUX_FBDEV=no   # Opendingux fbdev context support
 HAVE_OPENGLES=no           # Use GLESv2 instead of desktop GL
 HAVE_OPENGLES3=no          # OpenGLES3 support
-HAVE_X11=auto              # everything X11.
-HAVE_XRANDR=auto           # Xrandr support.
+HAVE_X11=no                # everything X11.
+HAVE_XRANDR=no             # Xrandr support.
 HAVE_OMAP=no               # OMAP video support
-HAVE_XINERAMA=auto         # Xinerama support.
-HAVE_KMS=auto              # KMS context support
+HAVE_XINERAMA=no           # Xinerama support.
+HAVE_KMS=no                # KMS context support
 HAVE_PLAIN_DRM=no          # Plain DRM video support
 HAVE_EXYNOS=no             # Exynos video support
 HAVE_DISPMANX=no           # Dispmanx video support
 HAVE_SUNXI=no              # Sunxi video support
-HAVE_WAYLAND=auto          # Wayland support
+HAVE_WAYLAND=no            # Wayland support
 C89_WAYLAND=no
 CXX_WAYLAND=no
-HAVE_EGL=auto              # EGL context support
-HAVE_VG=auto               # OpenVG support
-HAVE_CG=auto               # Cg shader support
+HAVE_EGL=no                # EGL context support
+HAVE_VG=no                 # OpenVG support
+HAVE_CG=no                 # Cg shader support
 HAVE_HLSL=no               # HLSL9 shader support (for Direct3D9)
 HAVE_BUILTINZLIB=auto      # Bake in zlib
 HAVE_ZLIB=auto             # zlib support (ZIP extract, PNG decoding/encoding)
-HAVE_ALSA=auto             # ALSA support
+HAVE_ALSA=no               # ALSA support
 C89_ALSA=no
-HAVE_RPILED=auto           # RPI led support
-HAVE_TINYALSA=auto         # TinyALSA support
-HAVE_AUDIOIO=auto          # AudioIO support
+HAVE_RPILED=no             # RPI led support
+HAVE_TINYALSA=no           # TinyALSA support
+HAVE_AUDIOIO=no            # AudioIO support
 HAVE_OSS=auto              # OSS support
-HAVE_RSOUND=auto           # RSound support
-HAVE_ROAR=auto             # RoarAudio support
-HAVE_AL=auto               # OpenAL support
-HAVE_JACK=auto             # JACK support
-HAVE_COREAUDIO=auto        # CoreAudio support
-HAVE_PULSE=auto            # PulseAudio support
+HAVE_RSOUND=no             # RSound support
+HAVE_ROAR=no               # RoarAudio support
+HAVE_AL=no                 # OpenAL support
+HAVE_JACK=no               # JACK support
+HAVE_COREAUDIO=no          # CoreAudio support
+HAVE_PULSE=no              # PulseAudio support
 C89_PULSE=no
 HAVE_FREETYPE=auto         # FreeType support
 HAVE_STB_FONT=yes          # stb_truetype font support
 HAVE_STB_IMAGE=yes         # stb image loading support
 HAVE_STB_VORBIS=yes        # stb vorbis support
 HAVE_IBXM=yes        	   # IBXM support
-HAVE_XVIDEO=auto           # XVideo support
-HAVE_V4L2=auto             # Video4linux2 support
+HAVE_XVIDEO=no             # XVideo support
+HAVE_V4L2=no               # Video4linux2 support
 HAVE_NEON=no               # ARM NEON optimizations
 HAVE_SSE=no                # x86 SSE optimizations (SSE, SSE2)
 HAVE_FLOATHARD=no          # Force hard float ABI (for ARM)
@@ -123,10 +123,10 @@
 HAVE_BUILTINFLAC=auto      # Bake in flac support
 HAVE_UPDATE_ASSETS=yes     # Disable downloading assets with online updater
 HAVE_PRESERVE_DYLIB=no     # Enable dlclose() for Valgrind support
-HAVE_PARPORT=auto          # Parallel port joypad support
+HAVE_PARPORT=no            # Parallel port joypad support
 HAVE_IMAGEVIEWER=yes       # Built-in image viewer support.
 HAVE_MMAP=auto             # MMAP support
-HAVE_QT=auto               # Qt companion support
+HAVE_QT=no                 # Qt companion support
 C89_QT=no
 HAVE_XSHM=no               # XShm video driver support
 HAVE_CHEEVOS=yes           # Retro Achievements
@@ -134,30 +134,30 @@
 HAVE_DISCORD=yes           # Discord Integration
 C89_DISCORD=no
 HAVE_TRANSLATE=yes         # OCR and Translation Server Integration
-HAVE_SHADERPIPELINE=yes    # Additional shader-based pipelines
+HAVE_SHADERPIPELINE=no     # Additional shader-based pipelines
 C89_SHADERPIPELINE=no
-HAVE_VULKAN=auto           # Vulkan support
-HAVE_VULKAN_DISPLAY=yes    # Vulkan KHR display backend support
+HAVE_VULKAN=no             # Vulkan support
+HAVE_VULKAN_DISPLAY=no     # Vulkan KHR display backend support
 C89_VULKAN=no
 HAVE_RPNG=yes              # RPNG support
 HAVE_RBMP=yes              # RBMP support
 HAVE_RJPEG=yes             # RJPEG support
 HAVE_RTGA=yes              # RTGA support
-HAVE_HID=yes               # Low-level HID (Human Interface Device) support
+HAVE_HID=no                 # Low-level HID (Human Interface Device) support
 HAVE_AUDIOMIXER=yes        # Audio Mixer
 HAVE_LANGEXTRA=yes         # Multi-language support
 HAVE_OSMESA=no             # Off-screen Mesa rendering
-HAVE_VIDEOPROCESSOR=auto   # Enable video processor core
-HAVE_VIDEOCORE=auto        # Broadcom Videocore 4 support
+HAVE_VIDEOPROCESSOR=no     # Enable video processor core
+HAVE_VIDEOCORE=no          # Broadcom Videocore 4 support
 HAVE_DRMINGW=no            # DrMingw exception handler
 HAVE_EASTEREGG=yes         # Easter egg
 HAVE_CDROM=auto            # CD-ROM support
-HAVE_GLSL=yes              # GLSL shaders support
-HAVE_SLANG=auto            # slang support
+HAVE_GLSL=no               # GLSL shaders support
+HAVE_SLANG=no              # slang support
 C89_SLANG=no
-HAVE_GLSLANG=auto          # glslang support (requires C++11)
+HAVE_GLSLANG=no            # glslang support (requires C++11)
 C89_GLSLANG=no
-HAVE_SPIRV_CROSS=auto      # SPIRV-Cross support (requires C++11)
+HAVE_SPIRV_CROSS=no        # SPIRV-Cross support (requires C++11)
 C89_SPIRV_CROSS=no
 HAVE_METAL=no              # Metal support (macOS-only)
 C89_METAL=no