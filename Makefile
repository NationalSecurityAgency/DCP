
CP        = cp
MV        = mv -f
RM        = rm -f

vpath %.ggo src
vpath %.c src
vpath %.h include

SRCS      = $(notdir $(wildcard src/*.c))
OBJS      = $(patsubst %.c, %.o, $(SRCS)) cmdline.o

VERSION   = 1.0.6

CC        = gcc
CFLAGS    = -g -O2
CPPFLAGS  = 
LDFLAGS   =  -lz -ldb -ljansson -lcrypto 

COMPILE   = $(CC) $(CFLAGS) $(CPPFLAGS) -I include -DVERSION=$(VERSION)

GENGETOPT = gengetopt
GOPTFLAGS = --unamed-opts -a cmdline_info -F cmdline -i src/option_parser.ggo --set-version=1.06

TARGET    = dcp

###########################################################
#
#                       Building
#
###########################################################

all: $(TARGET)

$(TARGET): cmdline.o $(OBJS)
	$(COMPILE)    -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(COMPILE) -c -o $@ $^

cmdline.o: cmdline.c
	$(COMPILE) -c -o $@ src/$^

cmdline.c: option_parser.ggo
	$(GENGETOPT) $(GOPTFLAGS)
	$(MV) cmdline.c src/
	$(MV) cmdline.h include/

clean:
	$(RM) $(OBJS) $(TARGET)

distclean: clean
	$(RM) src/cmdline.c include/cmdline.h
	$(RM) include/config.h

###########################################################
#
#                       Installation
#
###########################################################

PREFIX   = /usr/local
BINDIR   = ${exec_prefix}/bin
DOCDIR   = ${datarootdir}/doc/${PACKAGE_TARNAME}
DATAROOT = ${prefix}/share

install:
	$(CP) -u $(TARGET) $(PREFIX)$(BINDIR)

uninstall:
	$(RM) $(PREFIX)$(BINDIR)/$(TARGET)
