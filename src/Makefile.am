bin_PROGRAMS = clive
EXTRA_LIBRARIES = libmd5.a
noinst_LIBRARIES = @suplib@

clive_SOURCES = net.c clive.c util.c lj_postevent.c edit.c configure.c \
lj_login.c lj_getfriends.c lj_editfriends.c lj_getevents.c lj_getcomments.c \
lj_getchallenge.c hash.c
clive_LDADD = @suplib@

libmd5_a_SOURCES = md5.c md5hl.c

clive_CFLAGS = -pipe -g -ggdb -g3 -W -Wall -Wimplicit -Wreturn-type -Wunused -Wswitch -Wcomment -Wtrigraphs -Wformat -Wchar-subscripts -Wparentheses -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Winline -Wuninitialized -fshort-enums -funroll-loops -finline-functions -fexpensive-optimizations -fsigned-char
