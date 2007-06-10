#ifndef QDP_GENERIC_BLAS_VCAXPBY3
#define QDP_GENERIC_BLAS_VCAXPBY3

namespace QDP {
inline
void vcaxpby3(REAL* Out, REAL* ap, REAL* xp, REAL* bp, REAL *yp, int n_3vec)
{
  register double a_r;
  register double a_i;
  register double b_r;
  register double b_i;

  register double x0r;
  register double x0i;
  
  register double x1r;
  register double x1i;
  
  register double x2r;
  register double x2i;
  
  register double y0r;
  register double y0i;
  
  register double y1r;
  register double y1i;
  
  register double y2r;
  register double y2i;
  
  register double z0r;
  register double z0i;
  
  register double z1r;
  register double z1i;
  
  register double z2r;
  register double z2i;
  
  a_r =(double)(*ap);
  a_i =(double)*(ap+1);

  b_r =(double)(*bp);
  b_i =(double)*(bp+1);

  register int index_x = 0;
  register int index_y = 0;
  register int index_z = 0;
  
  register int counter;

  if( n_3vec > 0 ) { 
    // Prefetch whole vectors
    x0r = (double)xp[index_x++];    
    x0i = (double)xp[index_x++];
    y0r = (double)yp[index_y++];
    y0i = (double)yp[index_y++];

    x1r = (double)xp[index_x++];    
    x1i = (double)xp[index_x++];
    y1r = (double)yp[index_y++];
    y1i = (double)yp[index_y++];
 
    x2r = (double)xp[index_x++];    
    x2i = (double)xp[index_x++];
    y2r = (double)yp[index_y++];
    y2i = (double)yp[index_y++];

    for( counter = 0; counter < n_3vec-1; counter++) {
     
      z0r = a_r * x0r;    
      z0i = a_i * x0r;
      x0r = (double)xp[index_x++];    
      z0r += b_r * y0r;
      z0i += b_i * y0r;
      y0r = (double)yp[index_y++];

      z0r -= a_i * x0i;
      z0i += a_r * x0i;
      x0i = (double)xp[index_x++];
      z0r -= b_i * y0i;
      z0i += b_r * y0i;
      y0i = (double)yp[index_y++];
      Out[index_z++] = (REAL)z0r;
      Out[index_z++] = (REAL)z0i;

      z1r = a_r * x1r;    
      z1i = a_i * x1r;
      x1r = (double)xp[index_x++];    
      z1r += b_r * y1r;
      z1i += b_i * y1r;
      y1r = (double)yp[index_y++];

      z1r -= a_i * x1i;
      z1i += a_r * x1i;
      x1i = (double)xp[index_x++];
      z1r -= b_i * y1i;
      z1i += b_r * y1i;
      y1i = (double)yp[index_y++];
      Out[index_z++] = (REAL)z1r;
      Out[index_z++] = (REAL)z1i;

      z2r = a_r * x2r;    
      z2i = a_i * x2r;
      x2r = (double)xp[index_x++];    
      z2r += b_r * y2r;
      z2i += b_i * y2r;
      y2r = (double)yp[index_y++];

      z2r -= a_i * x2i;
      z2i += a_r * x2i;
      x2i = (double)xp[index_x++];
      z2r -= b_i * y2i;
      z2i += b_r * y2i;
      y2i = (double)yp[index_y++];
      Out[index_z++] = (REAL)z2r;
      Out[index_z++] = (REAL)z2i;
    }

    z0r = a_r * x0r;    
    z0i = a_i * x0r;      
    z0r += b_r * y0r;
    z0i += b_i * y0r;
    
    z0r -= a_i * x0i;
    z0i += a_r * x0i;
    z0r -= b_i * y0i;
    Out[index_z++] = (REAL)z0r;
    z0i += b_r * y0i;
    Out[index_z++] = (REAL)z0i;
    
    z1r = a_r * x1r;    
    z1i = a_i * x1r;
    z1r += b_r * y1r;
    z1i += b_i * y1r;
    
    z1r -= a_i * x1i;
    z1i += a_r * x1i;
    z1r -= b_i * y1i;
    Out[index_z++] = (REAL)z1r;
    z1i += b_r * y1i;
    Out[index_z++] = (REAL)z1i;
    
    z2r = a_r * x2r;    
    z2i = a_i * x2r;
    z2r += b_r * y2r;
    z2i += b_i * y2r;
    
    z2r -= a_i * x2i;
    z2i += a_r * x2i;
    z2r -= b_i * y2i;
    Out[index_z++] = (REAL)z2r;
    z2i += b_r * y2i;
    Out[index_z++] = (REAL)z2i;
  }
}


} // namespace QDP;

#endif // guard
