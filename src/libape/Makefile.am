lib_LTLIBRARIES = libape.la
libape_la_SOURCES = mac.c macbind.cpp
libape_la_LDFLAGS = -version-info 0:0:0
libape_la_CFLAGS = $(DEPS_CFLAGS) -std=c99 -include config.h -fPIC -DPIC
libape_la_CXXFLAGS = $(MACLIB_CFLAGS) -include config.h
libape_la_LIBADD = $(DEPS_LIBS) $(MACLIB_LIBS)

#lib_LTLIBRARIES = libape.la libmacbind.la
#libape_la_SOURCES = mac.c
#libape_la_LDFLAGS = -version-info 0:0:0
#libape_la_CFLAGS = $(DEPS_CFLAGS) -std=c99 -include config.h
#libape_la_LIBADD = $(DEPS_LIBS) libmacbind.la
#libmacbind_la_SOURCES = macbind.cpp
#libmacbind_la_CXXFLAGS = $(MACLIB_CFLAGS) -include config.h -fPIC
#libmacbind_la_LIBADD = $(MACLIB_LIBS)
