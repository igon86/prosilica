# makefile of psnap code

#configuration file
include ./conf

.PHONY : clean sample install snap post stream crop live

clean:
	rm -f $(EXE)
	rm -f *~
	rm -f testFrame
	rm -f mySnap
	rm -rf ./Image/*

## shared libs
SOLIB   = $(EXTRA_LIB) -L$(BIN_DIR) -lPvAPI
LIB         = -Bstatic $(LIB_DIR)/$(CVER)/libImagelib.a -Bdynamic $(SOLIB)

## final compilation flags
CFLAGS  = $(OPT) $(FLAGS) -I$(INC_DIR) -D_REENTRANT $(EXTRA)

snap:
	$(CC) $(RPATH) $(TARGET) $(CFLAGS)  mysnap.cpp psnap.cpp snap.cpp -o mySnap $(LIB)

post:
	g++ -Wall -ltiff -o post tiffPostElaboration.cpp post.cpp

stream:
	$(CC) $(RPATH) $(TARGET) $(CFLAGS) $(LTIFF) testStream.cpp mysnap.cpp psnap.cpp tiffPostElaboration.cpp -o stream $(LIB)

crop:
	$(CC) $(RPATH) $(TARGET) -I/opt/local/var/macports/software/gsl/1.12_0/opt/local/include/ $(CFLAGS) $(LTIFF) $(MAT) testCrop.cpp fit.cpp tiffPostElaboration.cpp -o crop $(LIB) -lgsl -lgslcblas -L/opt/local/var/macports/software/gsl/1.12_0/opt/local/lib

live:
	$(CC) $(RPATH) $(TARGET) -I/opt/local/var/macports/software/gsl/1.12_0/opt/local/include/ $(CFLAGS) $(LTIFF) $(MAT) -DDEBUG streamFit.cpp fit.cpp mysnap.cpp psnap.cpp tiffPostElaboration.cpp -o live $(LIB) -lgsl -lgslcblas -L/opt/local/var/macports/software/gsl/1.12_0/opt/local/lib

newlive:
	$(CC) $(RPATH) $(TARGET) -DDEBUG -I/opt/local/var/macports/software/gsl/1.12_0/opt/local/include/ $(CFLAGS) $(LTIFF) $(MAT) streamCpy.cpp fit.cpp mysnap.cpp psnap.cpp tiffPostElaboration.cpp -o live $(LIB) -lgsl -lgslcblas -L/opt/local/var/macports/software/gsl/1.12_0/opt/local/lib
