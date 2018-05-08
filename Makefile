CURRENT_DIR=$(shell pwd)

PREQUALTESTDIR=$(CURRENT_DIR)

WHAINCDIR=$(CURRENT_DIR)/include/wha
WHALIBINCDIR=$(CURRENT_DIR)/include/whadlib

GTESTINCDIR=$(CURRENT_DIR)/include/

GCC = g++ -std=c++98
DEFS =
CFLAGS = -O4 -c $(DEFS) -w -Werror=format -Wno-psabi

INC = -I$(CURRENT_DIR) -I$(GTESTINCDIR) -I$(WHAINCDIR) -I/usr/include/alsa -I$(WHALIBINCDIR)

TSLDFLAGS=-L$(BOOST_DIR)/out/lib
TSLDLIBS=-lboost_thread -lboost_system -lboost_chrono

LDFLAGS =  -L. -L/usr/local/lib/ -L/usr/lib/x86_64-linux-gnu $(TSLDFLAGS)
LDLIBS = -lpthread -lm -lasound $(TSLDLIBS)

OBJS = DelegatesRaspPi.o gtest-all.o ALSA.o timeSyncTest.o TimeSynchronizer2.o preQualTest.o
OBJS_DEL = DelegatesRaspPi.o gtest-all.o ALSA.o preQualTest.o

all: preQualTest

DelegatesRaspPi.o: DelegatesRaspPi.cpp $(wildcard $(WHAINCDIR)/*.h) $(wildcard $(PREQUALTESTDIR)/*.h)
	$(GCC) $(CFLAGS) $(WALL) $(INC) $< $(GLIB) $(LIB) -o DelegatesRaspPi.o

preQualTest.o:	preQualTest.cpp $(wildcard $(GTESTINCDIR)/*.h) $(wildcard $(WHAINCDIR)/*.h)
	$(GCC) $(CFLAGS) $(WALL) $(INC) $< $(GLIB) $(LIB) -o preQualTest.o

gtest-all.o:	gtest-all.cc $(wildcard $(GTESTINCDIR)/gtest/*.h)
	$(GCC) $(CFLAGS) $(WALL) $(INC) $< $(GLIB) $(LIB) -o gtest-all.o

ALSA.o: ALSA.cpp
	$(GCC) $(CFLAGS) $(WALL) $(INC) $< $(GLIB) $(LIB) -o ALSA.o

preQualTest: $(OBJS)
	$(GCC) $(LDFLAGS) -o preQualTest $(OBJS) $(LDLIBS)

clean:
	rm -f $(OBJS_DEL)
	rm -rf preQualTest