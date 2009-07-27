# makefile of psnap code

#configuration file
include ./conf

# files created by the Makefile
EXE	= psnap.o psnap.py _psnap.so psnap_wrap.cxx psnap_wrap.o

OBJ	= ./psnap.o ./psnap_wrap.o

clean:
	rm -f $(EXE)
	rm -f *~

sample:
	swig -python -classic -c++ $(DFLAGS) -I./ psnap.i
	$(CC) $(OPT) $(DFLAGS) -fPIC -I$(PYTHON) -I$(INC_DIR) -c ./psnap.cpp psnap_wrap.cxx
	$(CC) -shared $(OBJ) -I$(INC_DIR) -I$(PYTHON) -L$(LIB_DIR) $(LIB) -o _psnap.so $(LIB_DIR)/libImagelib.a

install:
	mv -f -t Python $(EXE) 
