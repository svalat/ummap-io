
BUILDDIR = ../build
BINDIR   = $(BUILDDIR)/bin
INCDIR   = $(BUILDDIR)/include
LIBDIR   = $(BUILDDIR)/lib
OBJDIR   = $(BUILDDIR)/obj
LIBPATH  = $(LIBDIR)/libummapio.a
CLIBS    = -lummapio
CFLAGS   = -DDEBUG_PRINT=$(or $(DEBUG_PRINT),0) -O2 -I. -L$(LIBDIR) -g
CXXFLAGS = -DDEBUG_PRINT=$(or $(DEBUG_PRINT),0) -O2 -I. -L$(LIBDIR) -g

ifeq (,$(findstring Cray,$(shell cc -V 2>&1)))
    CLIBS  += -pthread -lrt
    CFLAGS += -Wall -Wno-unused-label -Wno-unused-function -std=c99
	CXXFLAGS += -Wall -Wno-unused-label -Wno-unused-function
endif

all: setup $(LIBPATH) $(BINDIR)/example.out $(BINDIR)/bench.out

$(BINDIR)/bench.out: $(LIBPATH) bench/main.cpp
	$(CXX) $(CXXFLAGS) bench/main.cpp -o $(BINDIR)/bench.out $(CLIBS) -O0

$(BINDIR)/example.out: $(LIBPATH) example/main.c
	$(CC) $(CFLAGS) example/main.c -o $(BINDIR)/example.out $(CLIBS)

$(LIBPATH): $(OBJDIR)/ummap.o $(OBJDIR)/ummap_util.o \
			$(OBJDIR)/ummap_policy.o $(OBJDIR)/futex.o
	ar -cq $(LIBPATH) $(OBJDIR)/*.o

$(OBJDIR)/ummap.o: ummap.* common.h
	$(CC) $(CFLAGS) -c ummap.c -o $(OBJDIR)/ummap.o

$(OBJDIR)/ummap_util.o: ummap_util.* common.h
	$(CC) $(CFLAGS) -c ummap_util.c -o $(OBJDIR)/ummap_util.o

$(OBJDIR)/ummap_policy.o: ummap_policy.* common.h
	$(CC) $(CFLAGS) -c ummap_policy.c -o $(OBJDIR)/ummap_policy.o

$(OBJDIR)/futex.o: futex.* common.h
	$(CC) $(CFLAGS) -c futex.c -o $(OBJDIR)/futex.o

setup:
	@mkdir -p $(BINDIR) $(INCDIR) $(LIBDIR) $(OBJDIR) 2> /dev/null
	@cp ummap.h $(INCDIR) 2> /dev/null

clean:
	@rm -rf $(BUILDDIR)

rebuild: clean all

