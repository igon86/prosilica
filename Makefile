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

## shared libs
SOLIB   = $(EXTRA_LIB) -L$(BIN_DIR) -lPvAPI
LIB         = -Bstatic $(LIB_DIR)/$(CVER)/libImagelib.a -Bdynamic $(LTIFF) $(SOLIB) 

## final compilation flags
CFLAGS  = $(OPT) $(FLAGS) -Wall -I$(INC_DIR) -D_REENTRANT $(EXTRA)

snap:
	$(CC) $(RPATH) $(TARGET) $(CFLAGS)  mysnap.cpp psnap.cpp snap.cpp -o mySnap $(LIB)

post:
	g++ -ltiff -o post tiffPostElaboration.cpp

stream:
	$(CC) $(RPATH) $(TARGET) $(CFLAGS) testStream.cpp mysnap.cpp psnap.cpp tiffPostElaboration.cpp -o stream $(LIB)