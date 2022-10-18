########################################################################
#                                                                      
#              --- CAEN VME Bridge interface makefile  ---                   
#                                                                      
#                                                                      
#   Created  :                                            
#                                                                      
#   Hyunmin Yang                                                 
#   Modified :                                            
#                                                                      
#		Kang Byungmin
#                                                                      
########################################################################

EXE	= daq_test.exe

CC	=	g++ -std=c++11

COPTS	=	-fPIC -DLINUX -Wall 
#COPTS	=	-g -fPIC -DLINUX -Wall 

FLAGS	=	-Wall -s $(COPTS)
#FLAGS	=	-Wall

# RTLIBS    = $(shell root-config --libs)
# RTINCLUDE = $(shell root-config --cflags)

INCLUDE	= -I. -I/usr/include/ -I./src -I./include 

CCFLAGS = $(FLAGS) $(INCLUDE) $(RTINCLUDE)
SOURCEDIR	= ./src/
OBJDIR	= ./.objs/


LIBS	= -lCAENVME -lc -lm -lstdc++ $(RTLIBS)


OBJS	= daq_test.o CAENVMEBridge.o CAENVMEV1290.o CAENVMEV792.o CAENVMEV895.o
#########################################################################

all	:	$(EXE)

clean	:
		/bin/rm -f $(OBJS) $(EXE)

$(EXE)	:	$(OBJS)
		/bin/rm -f $(EXE)
		$(CC) $(CCFLAGS) -o $(EXE) $(OBJS) $(LIBS)

%.o :	$(SOURCEDIR)/%.c
		$(CC) $(CCFLAGS) $(INCLUDE) -c -o $@ $<
%.o	:	%.c
		$(CC) $(CCFLAGS) $(INCLUDE) -c -o $@ $<

$(OBJDIR)/%.o: %.c
	$(CC) $(CCFLAGS) $(INCLUDE) -c -o $@ $<


