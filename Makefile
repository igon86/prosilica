# makefile of psnap code

#configuration file
include ./conf

# files created by the Makefile
EXE	= psnap.o psnap.py _psnap.so psnap_wrap.cxx psnap_wrap.o

OBJ	= ./psnap.o ./psnap_wrap.o

clean:
	rm -f $(EXE)
	rm -f *~
	rm -f testFrame
	rm -f mySnap
	rm -f ./Image/*

sample:
	swig -python -classic -c++ $(DFLAGS) -I./ psnap.i
	$(CC) $(OPT) $(DFLAGS) -fPIC -I$(PYTHON) -I$(INC_DIR) -c ./psnap.cpp psnap_wrap.cxx
	$(CC) -shared $(OBJ) -I$(INC_DIR) -I$(PYTHON) -L$(LIB_DIR) -L$(BIN_DIR) -L$(PYTHON) -I/usr/bin/python2.5 $(LIB) -o _psnap.so $(LIB_DIR)/libImagelib.a

install:
	mv -f -t Python $(EXE) 

mine:
	g++  -O3 -fno-strict-aliasing -fexceptions -I/usr/include -D_x86 -D_OSX -Wall -I./Prosilica/inc-pc -D_REENTRANT  testFrame.cpp psnap.cpp -o testFrame  -Bdynamic -L./Prosilica/bin-pc/x86 -lPvAPI

snap:
	g++  -O3 -fno-strict-aliasing -fexceptions -I/usr/include -D_x86 -D_OSX -Wall -I./Prosilica/inc-pc -D_REENTRANT  mysnap.cpp psnap.cpp -o mySnap -Bstatic $(LIB_DIR)/libImagelib.a -Bdynamic -lpthread -lz -ltiff -L./Prosilica/bin-pc/x86 -lPvAPI

