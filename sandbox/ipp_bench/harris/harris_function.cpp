#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <cmath>
#include <string.h>
#define isl_min(x,y) ((x) < (y) ? (x) : (y))
#define isl_max(x,y) ((x) > (y) ? (x) : (y))
#define isl_floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
void  refHarris(int  C, int  R, float * img, float *& harris)
{
  harris = (float *) (malloc((sizeof(float ) * ((2 + R) * (2 + C)))));
  memset(harris, 0, (sizeof(float ) * ((2 + R) * (2 + C))));
  #pragma omp parallel for schedule(static)
  for (int  _T_i0 = -1; (_T_i0 <= (R / 32)); _T_i0 = (_T_i0 + 1))
  {
    float * Iy;
    Iy = (float *) (malloc((sizeof(float ) * 8772)));
    memset(Iy, 0, (sizeof(float ) * 8772));
    float * Ix;
    Ix = (float *) (malloc((sizeof(float ) * 8772)));
    memset(Ix, 0, (sizeof(float ) * 8772));
    for (int  _T_i1 = -1; (_T_i1 <= (C / 256)); _T_i1 = (_T_i1 + 1))
    {
      int  _ct0 = ((R < ((32 * _T_i0) + 33))? R: ((32 * _T_i0) + 33));
      int  _ct1 = ((1 > (32 * _T_i0))? 1: (32 * _T_i0));
      for (int  _i0 = _ct1; (_i0 <= _ct0); _i0 = (_i0 + 1))
      {
        int  _ct2 = ((C < ((256 * _T_i1) + 257))? C: ((256 * _T_i1) + 257));
        int  _ct3 = ((1 > (256 * _T_i1))? 1: (256 * _T_i1));
        #pragma ivdep
        for (int  _i1 = _ct3; (_i1 <= _ct2); _i1 = (_i1 + 1))
        {
          Iy[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))] = ((((((img[(((-1 + _i0) * (C + 2)) + (-1 + _i1))] * -0.0833333333333f) + (img[(((-1 + _i0) * (C + 2)) + (1 + _i1))] * 0.0833333333333f)) + (img[((_i0 * (C + 2)) + (-1 + _i1))] * -0.166666666667f)) + (img[((_i0 * (C + 2)) + (1 + _i1))] * 0.166666666667f)) + (img[(((1 + _i0) * (C + 2)) + (-1 + _i1))] * -0.0833333333333f)) + (img[(((1 + _i0) * (C + 2)) + (1 + _i1))] * 0.0833333333333f));
        }
      }
      int  _ct4 = ((R < ((32 * _T_i0) + 33))? R: ((32 * _T_i0) + 33));
      int  _ct5 = ((1 > (32 * _T_i0))? 1: (32 * _T_i0));
      for (int  _i0 = _ct5; (_i0 <= _ct4); _i0 = (_i0 + 1))
      {
        int  _ct6 = ((C < ((256 * _T_i1) + 257))? C: ((256 * _T_i1) + 257));
        int  _ct7 = ((1 > (256 * _T_i1))? 1: (256 * _T_i1));
        #pragma ivdep
        for (int  _i1 = _ct7; (_i1 <= _ct6); _i1 = (_i1 + 1))
        {
          Ix[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))] = ((((((img[(((-1 + _i0) * (C + 2)) + (-1 + _i1))] * -0.0833333333333f) + (img[(((1 + _i0) * (C + 2)) + (-1 + _i1))] * 0.0833333333333f)) + (img[(((-1 + _i0) * (C + 2)) + _i1)] * -0.166666666667f)) + (img[(((1 + _i0) * (C + 2)) + _i1)] * 0.166666666667f)) + (img[(((-1 + _i0) * (C + 2)) + (1 + _i1))] * -0.0833333333333f)) + (img[(((1 + _i0) * (C + 2)) + (1 + _i1))] * 0.0833333333333f));
        }
      }
      int  _ct8 = (((R - 1) < ((32 * _T_i0) + 32))? (R - 1): ((32 * _T_i0) + 32));
      int  _ct9 = ((2 > ((32 * _T_i0) + 1))? 2: ((32 * _T_i0) + 1));
      for (int  _i0 = _ct9; (_i0 <= _ct8); _i0 = (_i0 + 1))
      {
        int  _ct10 = (((C - 1) < ((256 * _T_i1) + 256))? (C - 1): ((256 * _T_i1) + 256));
        int  _ct11 = ((2 > ((256 * _T_i1) + 1))? 2: ((256 * _T_i1) + 1));
        #pragma ivdep
        for (int  _i1 = _ct11; (_i1 <= _ct10); _i1 = (_i1 + 1))
        {
          harris[((_i0 * (2 + C)) + _i1)] = ((((((((((((Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))]) + (Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Ix[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))] * Ix[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Ix[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) * (((((((((Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))]) + (Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Iy[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Iy[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))] * Iy[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))])) + (Iy[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))]))) - ((((((((((Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))]) + (Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))] * Iy[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) * (((((((((Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))]) + (Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))] * Iy[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])))) - ((0.04f * ((((((((((Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))]) + (Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Ix[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))] * Ix[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Ix[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (((((((((Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))]) + (Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Iy[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Iy[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))] * Iy[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))])) + (Iy[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])))) * ((((((((((Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))]) + (Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Ix[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Ix[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))] * Ix[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Ix[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Ix[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (((((((((Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))]) + (Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[(((-1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Iy[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[((((-32 * _T_i0) + _i0) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Iy[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))] * Iy[((((-32 * _T_i0) + _i0) * 258) + ((-256 * _T_i1) + _i1))])) + (Iy[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[((((-32 * _T_i0) + _i0) * 258) + (1 + ((-256 * _T_i1) + _i1)))])) + (Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (-1 + ((-256 * _T_i1) + _i1)))])) + (Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + ((-256 * _T_i1) + _i1))])) + (Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))] * Iy[(((1 + ((-32 * _T_i0) + _i0)) * 258) + (1 + ((-256 * _T_i1) + _i1)))])))));
        }
      }
    }
    free(Iy);
    free(Ix);
  }
}
