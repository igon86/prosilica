# makefile of psnap code

#configuration file
include ./conf

# files created by the Makefile
EXE	= psnap.o psnap.py _psnap.so psnap_wrap.cxx psnap_wrap.o

OBJ	= ./psnap.o ./psnap_wrap.o


.PHONY : clean sample install snap post stream

clean:
	rm -f $(EXE)
	rm -f *~
	rm -f testFrame
	rm -f mySnap
	rm -rf ./Image/*

sample:
	swig -python -classic -c++ $(DFLAGS) -I./ psnap.i
	$(CC) $(OPT) $(DFLAGS) -fPIC -I$(PYTHON) -I$(INC_DIR) -c ./psnap.cpp psnap_wrap.cxx
	$(CC) -shared $(OBJ) -I$(INC_DIR) -I$(PYTHON) -L$(LIB_DIR) -L$(BIN_DIR) -L$(PYTHON) -I/usr/bin/python2.5 $(LIB) -o _psnap.so $(LIB_DIR)/libImagelib.a

install:
	mv -f -t Python $(EXE)

snap:
	g++ -O3 -fno-strict-aliasing -fexceptions -I/usr/include -D_x86 -D_OSX -Wall -I./Prosilica/inc-pc -D_REENTRANT  mysnap.cpp psnap.cpp snap.cpp -o mySnap -Bstatic $(LIB_DIR)/libImagelib.a -Bdynamic -lpthread -lz -ltiff -L./Prosilica/bin-pc/x86 -lPvAPI

post:
	g++ -ltiff -o post tiffPostElaboration.cpp

stream:
	g++ -O3 -fno-strict-aliasing -fexceptions -I/usr/include -D_x86 -D_OSX -Wall -I./Prosilica/inc-pc -D_REENTRANT testStream.cpp mysnap.cpp psnap.cpp tiffPostElaboration.cpp -o stream -Bstatic $(LIB_DIR)/libImagelib.a -Bdynamic -lpthread -lz -ltiff -L./Prosilica/bin-pc/x86 -lPvAPI
