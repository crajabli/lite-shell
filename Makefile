#
# Simple Makefile
# Mike Lam, James Madison University, August 2016
#
# This makefile builds a simple application that contains a main module
# (specified by the EXE variable) and a predefined list of additional modules
# (specified by the MODS variable). If there are any external library
# dependencies (e.g., the math library, "-lm"), list them in the LIBS variable.
# If there are any precompiled object files, list them in the OBJS variable.
#
# By default, this makefile will build the project with debugging symbols and
# without optimization. To change this, edit or remove the "-g" and "-O0"
# options in CFLAGS and LDFLAGS accordingly.
#
# By default, this makefile build the application using the GNU C compiler,
# adhering to the C99 standard with all warnings enabled.


# application-specific settings and run target

EXE=dukesh
MODS=main.o process.o shell.o builtins.o hash.o
OBJS=
LIBS=

default: build $(EXE)

build:
	mkdir build

test: build $(EXE)
	make -C tests test

style: $(EXE)
	make -C tests style

# compiler/linker settings

CC=gcc
CFLAGS=-g -O0 -Wall -std=c99 -pedantic -D_POSIX_C_SOURCE=200809L
LDFLAGS=-g -O0


# build targets

BUILD=$(addprefix build/, $(MODS))

$(EXE): build/main.o $(BUILD) $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXE) $^ $(LIBS)
	make -C utils

build/%.o: src/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -rf $(EXE) build
	make -C utils clean
	make -C tests clean

.PHONY: default clean

