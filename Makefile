# Simple minimalist makefile for Frame Toy

# source requirements: standard C++23 at minimal implementation of llvm ver.17
# beware of default for llvm17 provided by llvm-config is -std=c++17, which is unsuitable
# so the CXXFLAGS are set manually down there

# change platform/target configuration here:
LLVMVER=llvm17
CPLUSSTDVER=-std=c++23
LLVMINSTALLPATH=/usr/local/
OPT=-O2 

# the rest is automated. almost.
LLVM=${LLVMINSTALLPATH}${LLVMVER}
LLVMCONFIG=${LLVM}/bin/llvm-config
CLANG=${LLVM}/bin/clang
CPP=${LLVM}/bin/clang-cpp
CPLUS=${LLVM}/bin/clang++
LINKER=${LLVM}/bin/ld.lld

CXXFLAGS=-I${LLVM}/include ${CPLUSSTDVER} ${OPT} -fno-exceptions -funwind-tables -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS

LDFLAGS!=${LLVMCONFIG} --ldflags

# final executable 
ft:	ft.cpp
	${CPLUS} ${CXXFLAGS} ${LDFLAGS} ft.cpp -o ft

# syntax check produces assembly. sometimes, we want to see that
syntax: ft.cpp
	${CPLUS} ${CXXFLAGS} ft.cpp -S
