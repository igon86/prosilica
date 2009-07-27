/* file: psnap.i */
%module psnap
%newobject  getFrame;
%{

/* Basic Includes */
#include "psnap.hpp"

%}

%include "psnap.hpp"
%include "cpointer.i"

%pointer_functions(long, longp);
