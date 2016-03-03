lib_LTLIBRARIES = libusenet.la

AM_CXXFLAGS = $(INTI_CFLAGS) -I@srcdir@/include $(EXPAT_CFLAGS) $(SSL_CFLAGS) $(PTHREAD_CFLAGS)

libusenet_la_LDFLAGS = -version-info @LIBVER_INFO@ $(EXPAT_LIBS) $(SSL_LIBS) $(PTHREAD_LIBS)
libusenet_la_SOURCES = binparts.cpp  crc32.cpp  expatparse.cpp  nntpclient.cpp  nzb.cpp  nzbparse.cpp  sockstream.cpp  usenet.cpp  yenc.cpp
libusenet_la_LIBADD = $(INTI_LIBS)
pkginclude_HEADERS = include/libusenet/binParts.h  include/libusenet/crc32.h  include/libusenet/nntpclient.h  include/libusenet/nzb.h  include/libusenet/nzbparse.h  include/libusenet/sockstream  include/libusenet/usenet  include/libusenet/yenc.h include/libusenet/options.h
