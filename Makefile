VERSION=0.1
PROGRAM=kixterm

## Silent by default
V =
ifeq ($(strip $(V)),)
        E = @echo
        Q = @
else
        E = @\#
        Q =
endif
export E Q


CC=gcc
DEBUG =-ggdb
CFLAGS=-Wall -Wno-pointer-sign $(DEBUG)
INSTALL=install
PKGCONFIG=pkg-config
NULL=

# Locations
# Change 'prefix' variable to suit your needs
RUN_IN_PLACE=1
EXTRA_FLAGS =
prefix=/opt/partha/kixterm
BINDIR=$(prefix)/bin
DATADIR=$(prefix)/share
UIDIR=$(prefix)/ui
ICONDIR=$(prefix)/icons
INCLUDEDIR =
LIBDIR = lib
SRCDIR = src
CWD =  $(shell pwd)

# Resources
ifeq ($(RUN_IN_PLACE), 1)
ui_dir = $(CWD)/ui
icon_dir = $(CWD)/icons
EXTRA_FLAGS += -DRUN_IN_PLACE
else
ui_dir = $(UIDIR)
icon_dir = $(ICONDIR)
endif

# Platform
UNAME := $(shell $(CC) -dumpmachine 2>&1 | grep -E -o "linux|darwin")

ifeq ($(UNAME), linux)
        OSSUPPORT = linux
        OSSUPPORT_CFLAGS = -DLINUX
else ifeq ($(UNAME), darwin)
        OSSUPPORT = darwin
        OSSUPPORT_CFLAGS = -DDARWIN
endif

# Dependencies
LIBXCB = $(shell $(PKGCONFIG) --libs xcb)
LIBXCBICCCM = $(shell $(PKGCONFIG) --libs xcb-icccm)
LIBXCBAUX = $(shell $(PKGCONFIG) --libs xcb-aux)
LIBXCBKEYSYMS = $(shell $(PKGCONFIG) --libs xcb-keysyms)
LIBXDGBASE = $(shell $(PKGCONFIG) --libs libxdg-basedir)
LIBXCBEWMH = $(shell $(PKGCONFIG) --libs xcb-ewmh)
LIBPANGO = $(shell $(PKGCONFIG) --libs pango)
LIBCAIRO = $(shell $(PKGCONFIG) --libs cairo)
LIBPANGOCAIRO = $(shell $(PKGCONFIG) --libs pangocairo)

XCBCFLAGS = $(shell $(PKGCONFIG) --cflags xcb)
XCBICCCMCFLAGS = $(shell $(PKGCONFIG) --cflags xcb-icccm)
XCBAUXCFLAGS = $(shell $(PKGCONFIG) --cflags xcb-aux)
XCBKEYSYMSCFLAGS = $(shell $(PKGCONFIG) --cflags xcb-keysyms)
XDGBASECFLAGS = $(shell $(PKGCONFIG) --cflags libxdg-basedir)
XCBEWMHCFLAGS = $(shell $(PKGCONFIG) --cflags xcb-ewmh)
PANGOCFLAGS = $(shell $(PKGCONFIG) --cflags pango)
CAIROCFLAGS = $(shell $(PKGCONFIG) --cflags cairo)
PANGOCAIROCFLAGS = $(shell $(PKGCONFIG) --cflags pangocairo)

LIBS = \
	$(LIBXCB) \
	$(LIBXCBICCCM) \
	$(LIBXCBAUX) \
	$(LIBXCBKEYSYMS) \
	$(LIBXDGBASE) \
	$(LIBXCBEWMH) \
	$(LIBPANGO) \
	$(LIBCAIRO) \
	$(LIBPANGOCAIRO) \
	-lutil \
	$(NULL)
LDFLAGS =
EXTRA_FLAGS += \
	$(XCBCFLAGS) \
	$(XCBICCCMCFLAGS) \
	$(XCBAUXCFLAGS) \
	$(XCBKEYSYMSCFLAGS) \
	$(XDGBASECFLAGS) \
	$(OSSUPPORT_CFLAGS) \
	$(XCBEWMHCFLAGS) \
	$(PANGOCFLAGS) \
	$(CAIROCFLAGS) \
	$(PANGOCAIROCFLAGS) \
	-DVERSION_STRING='"$(VERSION_STRING)"' \
	$(NULL)

# Object Files
OBJS = \
	main.o \
	kt-util.o \
	kt-app.o \
	kt-window.o \
	kt-prefs.o \
	kt-color.o \
	kt-font.o \
	kt-terminal.o \
	kt-pty.o \
	kt-buffer.o \
	$(NULL)

HEADERS = \
	kixterm.h \
	kt-util.h \
	kt-app.h \
	kt-window.h \
	kt-prefs.h \
	kt-color.h \
	kt-font.h \
	kt-terminal.h \
	kt-pty.h \
	kt-buffer.h \
	$(NULL)

DEPS = $(wildcard .dep/*.dep)

all: $(PROGRAM)

$(PROGRAM): $(OBJS) $(HDRS)
	$(E) '             LD' $@
	$(Q)$(CC) $(LDFLAGS) -o $(PROGRAM) $(OBJS) $(LIBS)

%.o: %.c
	$(E) '             CC' $<
	$(Q)mkdir -p .dep
	$(Q)$(CC) $(CFLAGS) $(EXTRA_FLAGS) -MD -MF .dep/$@.dep -c -o $@ $<


clean:
	$(E) '   RM $(OBJS) $(PROGRAM)'
	$(Q)rm -f $(OBJS) *~ $(PROGRAM) po/*~
	$(Q)rm -rf .dep

.PHONY: check-syntax
check-syntax:
	gcc -Wall -Wextra -pedantic -fsyntax-only -Wno-variadic-macros -std=c99 $(HEADERS) $(CHK_SOURCES)

-include $(DEPS)
