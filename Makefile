DEBUG          := YES

CC     := gcc
CXX    := g++
LD     := g++
AR     := ar rc
RANLIB := ranlib

DEBUG_CXXFLAGS   := -std=c++11 -Wall -Wno-format -g -DDEBUG
RELEASE_CXXFLAGS := -std=c++11 -Wall -Wno-unknown-pragmas -Wno-format -O3

LIBS		 :=

DEBUG_LDFLAGS    := -g
RELEASE_LDFLAGS  :=

ifeq (YES, ${DEBUG})
   CXXFLAGS     := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
else
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
endif

INCS :=

OUTPUT := teklatemplategen

all: ${OUTPUT}

SRCS := teklatemplategen.cpp

OBJS := $(addsuffix .o,$(basename ${SRCS}))

${OUTPUT}: ${OBJS}
	${LD} -o $@ ${OBJS} ${LIBS} ${LDFLAGS}

%.o : %.cpp
	${CXX} -c ${CXXFLAGS} ${INCS} $< -o $@

dist:
	bash makedistlinux

clean:
	-rm -f core ${OBJS} ${OUTPUT}

teklatemplategen.o: teklatemplategen.cpp

