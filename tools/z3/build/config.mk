PREFIX=/usr
CC=gcc
CXX=g++
CXXFLAGS= -D_MP_INTERNAL -DNDEBUG -D_EXTERNAL_RELEASE -D_AMD64_ -D_USE_THREAD_LOCAL  -std=c++11 -fvisibility=hidden -c -mfpmath=sse -msse -msse2 -fopenmp -O3 -D_LINUX_ -fPIC
CFLAGS= -D_MP_INTERNAL -DNDEBUG -D_EXTERNAL_RELEASE -D_AMD64_ -D_USE_THREAD_LOCAL   -fvisibility=hidden -c -mfpmath=sse -msse -msse2 -fopenmp -O3 -D_LINUX_ -fPIC
EXAMP_DEBUG_FLAG=
CXX_OUT_FLAG=-o 
C_OUT_FLAG=-o 
OBJ_EXT=.o
LIB_EXT=.a
AR=ar
AR_FLAGS=rcs
AR_OUTFLAG=
EXE_EXT=
LINK=g++
LINK_FLAGS=
LINK_OUT_FLAG=-o 
LINK_EXTRA_FLAGS=-lpthread  -fopenmp -lrt
SO_EXT=.so
SLINK=g++
SLINK_FLAGS=-shared
SLINK_EXTRA_FLAGS= -fopenmp -lrt -Wl,-soname,libz3.so
SLINK_OUT_FLAG=-o 
OS_DEFINES=-D_LINUX_
