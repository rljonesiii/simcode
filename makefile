
SYSINCLUDES = -I/usr/include
MYINCLUDES = -I.
STDLIBS     = -lm -lfl

SOURCES = rngs.c rvgs.c rvms.c sort.c sssim.c

OBJS = $(SOURCES:.c=.o) 

ALLCCS = $(SOURCES) 

ALLHHS = $(ALLCCS:.c=.h)

ALLOBJS = $(OBJS) 

# MOREFLAGS       = -g
LINKFLAGS 	= $(PLAT_LINK) $(LINKINCLUDES)
CXX 		= g++
CC		= g++
CPPFLAGS 	= $(MOREFLAGS) -Wall $(SYSINCLUDES) $(MYINCLUDES) $(PLAT_FLAGS)
EXES		= sssim

all:	$(EXES)

sssim : $(ALLOBJS)
	$(CXX) $(LINKFLAGS) $(ALLOBJS) -o sssim $(STDLIBS) $(PLAT_LIBS)

depend:
	cd $(SOURCEDIR); g++ $(CPPFLAGS) -MM $(ALLCCS) > ./make.depend

clean :
	rm -f *.o $(EXES) errlog.dat 

# DO NOT DELETE THIS LINE -- make depend depends on it.
