AM_CFLAGS = -Wall -O3 -std=gnu11 -pedantic
ACLOCAL_AMFLAGS = -I m4

dist_doc_DATA = LICENSE README.md

lib_LTLIBRARIES = libe131.la
libe131_la_SOURCES = src/e131.c
libe131_la_LDFLAGS = -export-symbols-regex '^[eE]131_[^_]'
include_HEADERS = src/e131.h

# see: https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
libe131_la_LDFLAGS += -version-info 1:4:0
