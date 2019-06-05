
CP        = cp
MV        = mv -f

vpath %.c src
vpath %.h include

SRCS      = $(notdir $(wildcard src/*.c))
OBJS      = $(patsubst %.c, %.o, $(SRCS)) cmdline.o

VERSION   = "\"1.6\""

CC        = gcc
CFLAGS    = -g -O2
CPPFLAGS  =

COMPILE   = $(CC) $(CFLAGS) $(CPPFLAGS) -I include -DVERSION=$(VERSION) -fPIC

LD        = gcc
LDFLAGS   =  -lcrypto -ljansson -ldb -pie

GENGETOPT = gengetopt
GOPTFLAGS = --unamed-opts -a cmdline_info -F cmdline -i option_parser.ggo --set-version=1.06

PROGRAM  = dcp

all: $(PROGRAM)

$(PROGRAM): OptionsParser $(OBJS)
	$(COMPILE)    -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(COMPILE) -c -o $@ $^

OptionsParser: cmdline.o

cmdline.o: cmdline.c
	$(COMPILE) -c -o $@ src/$^

cmdline.c: option_parser.ggo
	$(GENGETOPT) $(GOPTFLAGS)
	$(MV) cmdline.c src/
	$(MV) cmdline.h include/

clean:
	$(RM) $(OBJS) $(PROGRAM)

distclean: clean
	$(RM) src/cmdline.c include/cmdline.h
	$(RM) include/config.h
