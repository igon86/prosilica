#ifndef __FIT__H
#define __FIT__H

#DEFINE GAUSSIAN 'G'
#DEFINE AIRY 'A'

typedef struct fit {
  /** function type*/
  char type;
  /** fit parameters */
  double x_0;
  double y_0;
  double sigma_x;
  double sigma_y;
  double a;
  double b;
  double c;
} fit_t;

void iteration(const unsigned char* data,fit_t* results);

#endif