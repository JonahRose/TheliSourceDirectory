void spline(float x[],float y[],int n,float yp1,float ypn,float y2[]);
void splint(float xa[],float ya[],float y2a[],int n,float x,float *y);
void splie2(float x1a[], float x2a[], float **ya, int m, int n, float **y2a);
void splin2(float x1a[], float x2a[], float **ya, float **y2a, int m, int n,float x1, float x2, float *y);
void polint(float xa[],float ya[],int n,float x,float *y,float *dy);
float interp_d2(int nx, int ny, float **data, float x, float y);
