# makefile of psnap code

#configuration file
include ./conf

.PHONY : clean sample install snap post stream

clean:
	rm -f $(EXE)
	rm -f *~
	rm -f testFrame
	rm -f mySnap
	rm -rf ./Image/*

snap:
	g++ -O3 -fno-strict-aliasing -fexceptions -I/usr/include -D_x86 -D_OSX -Wall -I./Prosilica/inc-pc -D_REENTRANT  mysnap.cpp psnap.cpp snap.cpp -o mySnap -Bstatic $(LIB_DIR)/libImagelib.a -Bdynamic -lpthread -lz -ltiff -L./Prosilica/bin-pc/x86 -lPvAPI

post:
	g++ -ltiff -o post tiffPostElaboration.cpp

stream:
	g++ -O3 -fno-strict-aliasing -fexceptions -I/usr/include -D_x86 -D_OSX -Wall -I./Prosilica/inc-pc -D_REENTRANT testStream.cpp mysnap.cpp psnap.cpp tiffPostElaboration.cpp -o stream -Bstatic $(LIB_DIR)/libImagelib.a -Bdynamic -lpthread -lz -ltiff -L./Prosilica/bin-pc/x86 -lPvAPI
