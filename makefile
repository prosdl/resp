#-----------------------------------------------------------------------
#    makefile for resp
#    -----------------
#
#    $Id: makefile,v 1.25 2003/06/01 09:13:26 rosendahl Exp $
#
#-----------------------------------------------------------------------

CC	= g++
LD	= g++
LDFLAGS  = 
CFLAGS	= -O3  -Wall -ansi -fno-gcse  -fno-rtti -fno-check-new
DEFS = -DLINUX -DUSE_TRANSPOSITION_TABLE -DUSE_PAWNHASH -DUSE_ASM_MSBLSB \
       -DUSE_READLINE


OBJS  =	console.o StringTools.o basic_stuff.o bitboard.o board.o\
	book.o controller.o eval.o hash.o respoptions.o \
	move.o pgnparse.o phash.o respmath.o search.o\
	xboard.o notation.o recognizer.o kedb.o incmovgen.o \
	gameinfo.o   

# --------------------------
#  Pattern rule definition
# --------------------------
%.o : 
	$(CC) $(CFLAGS) $(DEFS) -c $*.cpp

# -----------
#   Targets
# -----------
.PHONY: all dependencies depend buildnum install
all: resp

help:
	@echo "" 
	@echo "make ................... compile release version"
	@echo "make profile ........... compile profiling version"
	@echo "make debug ............. compile debug version"
	@echo "make clean ............. remove object files"
	@echo "make maintainer-clean .. remove almost everything"
	@echo "make dependencies ...... generate dependencies"
	@echo "make doxygen ........... generate documentation"
	@echo ""

profile:
	$(MAKE) CFLAGS='-O3 -march=i686 -pg -fno-gcse -mpreferred-stack-boundary=2 -fforce-mem' \
		LDFLAGS='-pg'

debug:
	$(MAKE) CFLAGS='-O2 -march=i686 -fomit-frame-pointer -Wall -ansi -g'

clean:	
	rm -f *.o

maintainer-clean:
	rm -f *.o *~
	rm -rf ../src_doc/html

dependencies:
	$(CC) -DLINUX -MM  *.cpp >depend    

resp: $(OBJS) buildnum version.o
	$(LD) $(LDFLAGS) $(OBJS) version.o -o resp -lreadline

doxygen:
	doxygen doxy.conf

buildnum:
	./upd_bn.sh && ./upd_bhist.sh

install: 
	cp resp ../bin/resp-`./get_version.sh`

# -----------------------------------------------------------------


# dependencies
-include depend
