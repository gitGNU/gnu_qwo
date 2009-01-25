INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS = \
  -DPKGDATADIR=\"$(pkgdatadir)\" \
  -DDATADIR=\""$(datadir)"\" -D_GNU_SOURCE

AM_CFLAGS = -Wall -pedantic -std=c99

# add the name of your application
bin_PROGRAMS = qwo

# add the sources to compile for your application
qwo_SOURCES = \
  qwo.c

MAINTAINERCLEANFILES  = config.h.in Makefile.in

if HAVE_HELP2MAN
man_MANS = qwo.1
endif

dist_man_MANS = $(man_MANS)
CLEANFILES = $(man_MANS)

qwo.1: qwo
	help2man -n "An input method for touchscreen" -s 1 -N -o qwo.1 ./qwo
