#ifndef RAD
#define RAD 3.1415926535/180.
#endif

#ifndef FILEMAX
#define FILEMAX 4096
#endif

#ifndef PI
#define PI 3.1415926535
#endif

#ifndef LOW_ELEM
#define LOW_ELEM "WARNING: Insufficient elements in array! Assuming zero for result.\n"
#endif

#ifndef __STATISTICS_H
#define __STATISTICS_H

#include <typeinfo>
#include <cstring>
#include <limits>

using namespace std;

//****************************************************************************
// Template for (optionally thresholded) mean; 
//****************************************************************************
template<class T>
T mean(vector<T> const &data, const long dim, double threshold = -1.0E+12, 
       string add = "noadd")
{

  // Initialisation
  T sum = T();
  long i, n = 0;

  for (i=0; i<dim; ++i) {
    if (data[i] > threshold) {
      sum += data[i];
      n++;
    }
  }

  if (n != 0) {
    // Return the normal average unless the sum is requested
    if (add.compare("noadd") == 0) sum /= T(n);
    // else leave sum unchanged
  }
  else {
    //    cerr << LOW_ELEM;
    sum = T();
  }

  return sum;
}


//****************************************************************************
// Template for rms; last argument is an optional lower threshold
//****************************************************************************
template<class T>
T rms(vector<T> const &data, const long dim, double threshold = -1.0E+12)
{
  // Initialisation
  T sdev = T(), avg;
  long i, n = 0;
  
  avg = mean(data, dim, threshold);

  for (i=0; i<dim; ++i) {
    if (data[i] > threshold) {
      sdev += (avg - data[i]) * (avg - data[i]);
      n++;
    }
  }

  if (n > 1)
    return sqrt(sdev / T(n-1) );
  else {
    //    cerr << LOW_ELEM;
    return T();
  }
}


//***************************************************************
// Extract a quasi-random sample from a larger array
//***************************************************************
long get_array_subsample_size(long dim);

//***************************************************************
// Extract a quasi-random sample from a larger array
//***************************************************************
template<class T>
void get_array_subsample(vector<T> &data, vector<T> &datasub, long dim, 
			 long dim_sub)
{
  long i, j, t;
  
  // select the small array
  j = t = 0;
  for (i=0; i<dim; i++) {
    if (t==0 && j < dim_sub)
      datasub[j++] = data[i];
    if (t<dim_sub) t++;
    if (t == dim_sub) t = 0;
  }
}

//***************************************************************
// Extract a quasi-random sample from a larger array, using an
// additional constraint from a segmentation image
//***************************************************************
template<class T>
void get_array_subsample(vector<T> &data, vector<T> &datasub, vector<T> &seg, 
			 long dim, long dim_sub)
{
  long i, j, t;
  
  // select the small array
  j = t = 0;
  for (i=0; i<dim; i++) {
    if (t==0 && j < dim_sub && seg[i] == 0)
      datasub[j++] = data[i];
    if (t<dim_sub) t++;
    if (t == dim_sub) t = 0;
  }
}

//****************************************************************************
// Template for weighted mean (overloading function 'mean') 
//****************************************************************************
template<class T>
T mean(vector<T> const &data, const long dim, vector<T> const &weight, 
       double threshold = -1.0E+12)
{
  // Initialisation
  T sum = T(), wsum = T();
  long i;
  
  for (i=0; i<dim; ++i) {
    if (data[i] > threshold) {
      sum += data[i] * weight[i];
      wsum += weight[i];
    }
  }

  if (wsum > 0.)
    sum /= wsum;
  else {
    //    cerr << LOW_ELEM;
    sum = T();
  }

  return sum;
}


//****************************************************************************
// Filtered mean (obtain mean from center 'fraction');
// More stable than kappa-sigma-clipping for small arrays
//****************************************************************************
template<class T, class T2>
T mean_fraction(vector<T> const &data, const long dim, T2 fraction, double threshold = -1.0E+12)
{
  T avg = 0.0;
  long dim_out;
  // this is larger than what is needed, but it has to be initialised here
  // to avoid compiler warnings
  vector<T> data_out(dim,0); 

  select_central_fraction(data, dim, data_out, &dim_out, fraction);

  cout << "dim_out " << dim_out << endl;

  avg = mean(data_out, dim_out, threshold);
  
  return avg;
}


//***************************************************************
// Iterative mean calculation, using kappa-sigma clipping
//***************************************************************
template<class T>
T mean_iter(vector<T> const &data, const long dim, float kappa, int niter = 1, 
       double threshold = -1.0E+12, string reingest="noreingest")
{
  long i, j, k, dim2;
  T avg, sdev;

  avg  = mean(data, dim, threshold);
  sdev = rms(data, dim, threshold);
  
  vector<T> tmp(dim,0);
  vector<T> tmp2(dim,0);
  
  for (i=0; i<dim; i++) 
    tmp[i] = data[i];
  dim2 = dim;

  for (k=0; k<niter; k++) {

    // calculate sigma-clipped mean and rms
    j = 0;
    for (i=0; i<dim2; i++)  {
      if ( tmp[i] < avg + kappa * sdev &&
	   tmp[i] > avg - kappa * sdev &&
	   tmp[i] > threshold) {
	tmp2[j++] = tmp[i];
      }
    }
    avg  = mean(tmp2, j, threshold);
    sdev = rms(tmp2, j, threshold);
    
    // Default: Do not re-ingest data (otherwise skip the following instruction)
    if (reingest.compare("noreingest") == 0) {
      for (i=0; i<j; i++)  {
	tmp[i] = tmp2[i];
      }
      dim2 = j;
    }
  }
  
  if (sdev == 0.) avg = mean(data, dim);

  return avg;
}


//****************************************************************************
// Template for min 
//****************************************************************************
template<class T>
T min(vector<T> const &data, const long dim)
{
  long i;
  T minval = data[0];
  
  for (i=0; i<dim; i++) {
    if (data[i] < minval) minval = data[i];
  }
  
  return (minval);
}


//****************************************************************************
// Template for max
//****************************************************************************
template<class T>
T max(vector<T> const &data, const long dim)
{
  long i;
  T maxval = data[0];
  
  for (i=0; i<dim; i++) {
    if (data[i] > maxval) maxval = data[i];
  }
  
  return (maxval);
}


//****************************************************************************
// Template for min (no explicit upper boundary)
//****************************************************************************
template<class T>
T min(vector<T> const &data)
{
  T minval = data[0];
  
  for (unsigned long i=0; i<data.size(); i++) {
    if (data[i] < minval) minval = data[i];
  }
  
  return (minval);
}


//****************************************************************************
// Template for max (no explicit upper boundary)
//****************************************************************************
template<class T>
T max(vector<T> const &data)
{
  T maxval = data[0];
  
  for (unsigned long i=0; i<data.size(); i++) {
    if (data[i] > maxval) maxval = data[i];
  }
  
  return (maxval);
}


//****************************************************************************
// Template for minindex
//****************************************************************************
template<class T>
T minindex(vector<T> const &data, const long dim)
{
  long i, index = 0;
  T minval = data[0];

  for (i=0; i<dim; i++) {
    if (data[i] < minval) {
      minval = data[i];
      index = i;
    }
  }
  
  return (index);
}


//****************************************************************************
// Template for maxindex
//****************************************************************************
template<class T>
T maxindex(vector<T> const &data, const long dim)
{
  long i, index = 0;
  T maxval = data[0];

  for (i=0; i<dim; i++) {
    if (data[i] > maxval) {
      maxval = data[i];
      index = i;
    }
  }
  
  return (index);
}


//****************************************************************************
// Template for minmaxindex
//****************************************************************************
template<class T>
void minmaxindex(vector<T> const &data, vector<long> &index, const long dim, long &min_index, long &max_index)
{
  // Initialisation
  T minval = T();
  T maxval = T();
  long i;
  
  minval = maxval = data[index[0]];
  min_index = max_index = 0;

  for (i=0; i<dim; i++) {
    if (data[index[i]] < minval) {
      minval = data[index[i]];
      min_index = i;
    }
    else if (data[index[i]] > maxval) {
      maxval = data[index[i]];
      max_index = i;
    }
  }
}


//****************************************************************************
// Template for minmax
//****************************************************************************
template<class T>
void minmax(vector<T> const &data, vector<long> &index, const long dim, T &minval, T &maxval)
{
  // Initialisation
  long i;
  
  minval = maxval = data[index[0]];
  
  for (i=0; i<dim; i++) {
    if (data[index[i]] < minval) minval = data[index[i]];
    else if (data[index[i]] > maxval) maxval = data[index[i]];
  }
}


//****************************************************************************
// Template for minmaxindex
//****************************************************************************
template<class T>
long maxindex2(vector<T> const &data, vector<long> &index, const long dim)
{
  // Initialisation
  T maxval = T();
  long i, max_index;
  
  maxval = data[index[0]];
  max_index = 0;

  for (i=0; i<dim; i++) {
    if (data[index[i]] > maxval) {
      maxval = data[index[i]];
      max_index = i;
    }
  }
  return max_index;
}


//****************************************************************************
// Template for variance
//****************************************************************************
template<class T>
T variance(vector<T> const &data, const long dim)
{
  // Initialisation
  T variance = rms(data, dim);
  return variance * variance;
}


//****************************************************************************
// Template for crossvariance
//****************************************************************************
template<class T>
T crossvariance(vector<T> const &data1, vector<T> const &data2, const long dim)
{
  // Initialisation
  T xvar = T(), mean1, mean2;
  long i;
  
  mean1 = mean(data1, dim);
  mean2 = mean(data2, dim);

  for (i=0; i<dim; ++i) {
    xvar = xvar + (mean1 - data1[i]) * (mean2 - data2[i]);
  }

  if (dim > 1)
    return xvar / T(dim-1);
  else {
    //    cerr << LOW_ELEM;
    return T();
  }
}

//****************************************************************************
// Filtered rms (sort data; obtain rms from center 'fraction';
// Overloading rms()
//****************************************************************************
//float rms_fraction(const float *data, long dim, float fraction);


//****************************************************************************
// Template for linear regression; implied abscissa
//****************************************************************************
template<class T>
void linfit(vector<T> const &data, const long dim, vector<T> &result)
{
  long i;
  T sx, sxy;
  vector<T> abscissa(dim,0);
  
  // fill in the abscissa
  for (i=0; i<dim; i++) {
    abscissa[i] = T(i+1);
  }    
  
  // do the linfit
  sx  = variance(abscissa, dim);
  sxy = crossvariance(abscissa, data, dim);
  // the slope
  result[0] = sxy / sx;
  result[1] = mean(data, dim) - result[0] * mean(abscissa, dim);
}


//****************************************************************************
// Template for linear regression, arbitrary abscissa
//****************************************************************************
template<class T>
void linfit(vector<T> const &data, vector<T> const &abscissa, const long dim, vector<T> &result)
{
  T sx, sxy;
  
  // do the linfit
  sx  = variance(abscissa, dim);
  sxy = crossvariance(abscissa, data, dim);
  // the slope
  result[0] = sxy / sx;
  result[1] = mean(data, dim) - result[0] * mean(abscissa, dim);
}


//***************************************************************
// Template for median;
// Has three options for the last arguments: 
// -- normal (default): changes order in input data 
// -- conserve: preserves order in input data
// -- fast: selects quasi-random sparse subarray for speed
//***************************************************************
template<class T>
T median(vector<T> &data, const long dim, string conserve="normal")
{
  T medianval;
  long dimarr = dim;

  if (dim==0) {
    //    cerr << LOW_ELEM;
    return (0.);
  }

  // Sort the original data
  if (conserve.compare("normal") == 0) {
    //    sort(data.begin(), data.end());
    sort(data.begin(), data.begin()+dim);
    if (dimarr % 2) medianval = data[dimarr / 2];
    else {
      if (dimarr != 1) medianval = 0.5 * (data[dimarr / 2] + data[dimarr / 2 - 1]);
      if (dimarr == 1) medianval = data[0];
    }
    return medianval;
  }

  // Preserve the original data, sort a copy
  else if (conserve.compare("conserve") == 0) {
    vector<T> copy(data);
    sort(copy.begin(), copy.begin()+dim);
    if (dimarr % 2) medianval = copy[dimarr / 2];
    else {
      if (dimarr != 1) medianval = 0.5 * (copy[dimarr / 2] + copy[dimarr / 2 - 1]);
      if (dimarr == 1) medianval = copy[0];
    }
    return medianval;
  }

  // Extract a subarray for faster median calculation (preserves original data)
  else if (conserve.compare("fast") == 0) {
    dimarr = get_array_subsample_size(dim);
    vector<T> copy(dimarr,0);
    get_array_subsample(data, copy, dim, dimarr);
    sort(copy.begin(), copy.end());
    if (dimarr % 2) medianval = copy[dimarr / 2];
    else {
      if (dimarr != 1) medianval = 0.5 * (copy[dimarr / 2] + copy[dimarr / 2 - 1]);
      if (dimarr == 1) medianval = copy[0];
    }
    return medianval;
  }

  else {
    cerr << "ERROR: Invalid 3rd argument in median function.\n";
    cerr << "Must be skipped or set to \"conserve\"\n";
    exit (1);
  }
}


//***************************************************************
// Template for matrix multiplication.
// Stores the result in the elements of the second matrix
//***************************************************************
template<class T1, class T2>
void matrix_mult(T1 a11, T1 a12, T1 a21, T1 a22, 
		 T2 &b11, T2 &b12, T2 &b21, T2 &b22)
{
  T2 c11, c12, c21, c22;

  c11 = a11 * b11 + a12 * b21;
  c12 = a11 * b12 + a12 * b22;
  c21 = a21 * b11 + a22 * b21;
  c22 = a21 * b12 + a22 * b22;

  b11 = c11;
  b12 = c12;
  b21 = c21;
  b22 = c22;
}

//***************************************************************
// Collapse a 2D array; calculate mean/median row/columns
//***************************************************************
void collapse(vector<float> const &data, vector<float> &result, 
	      string direction, long n, long m, string mode, 
	      string norm="donothing");


//***************************************************************
// ds9-style z-scale
//***************************************************************
void get_zscale(vector<float> &data, int n, int m, vector<float> &zscale, 
		float shrink, float contrast);


//***************************************************************
// Iterative mean and rms calculation, using kappa-sigma clipping
//***************************************************************
template<class T>
void mean_rms(vector<T> &data, const long dim, float kappa, T &mean_ptr, T &rms_ptr, 
	      int niter = 1, double threshold = -1.0E+12, 
	      string reingest="noreingest")
{
  long i, j, k, dim2;

  mean_ptr = mean(data, dim);
  rms_ptr = rms(data, dim);
  
  vector<T> tmp(dim,0);
  vector<T> tmp2(dim,0);

  for (i=0; i<dim; i++) 
    tmp[i] = data[i];
  dim2 = dim;

  for (k=0; k<niter; k++) {

    // calculate sigma-clipped mean and rms
    j = 0;
    for (i=0; i<dim2; i++)  {
      if ( tmp[i] < mean_ptr + kappa * rms_ptr &&
	   tmp[i] > mean_ptr - kappa * rms_ptr &&
	   tmp[i] > threshold) {
	tmp2[j++] = tmp[i];
      }
    }
    mean_ptr = mean(tmp2, j, threshold);
    rms_ptr = rms(tmp2, j, threshold);
    
    // Default: Do not re-ingest data (otherwise skip the following instruction)
    if (reingest.compare("noreingest") == 0) {
      for (i=0; i<j; i++)  {
	tmp[i] = tmp2[i];
      }
      dim2 = j;
    }
  }
  
  if (rms_ptr == 0.) mean_ptr = mean(data, dim);
}


//****************************************************************
// Convolve a 1D array with a Gaussian 
//****************************************************************
template<class T1, class T2>
void smooth_array(vector<T1> &array, const long dim, T2 sigma)
{
  long i, j;
  T1 r, weight;
  vector<T1> tmp(dim,0);
  
  for (i=0; i<dim; i++) {
    for (j=0; j<dim; j++) {
      r = T1(i-j);
      weight = 1./sqrt(2.*PI*sigma*sigma) * exp(-r*r/(2.*sigma*sigma));
      tmp[i] += (array[j] * weight);
    }
  }
  
  for (i=0; i<dim; i++) {
    array[i] = tmp[i];
  }
}


//******************************************************
// Puts a grid with spacing 10 pixels over an image,
// obtains a sparse sample of pixels around each grid 
// point, calculates the median from them for each grid
// point, and returns the maximum median value found.
//******************************************************
template<class T>
T get_max_median(vector<T> &data, long n, long m)
{
  long i, j, xi, yi, u, k;
  T maxval;
  
  vector<T> tmp(36,0);
  vector<T> result(n*m/100,0);
  
  u = 0;
  for (j=10; j<m-10; j=j+10) {
    for (i=10; i<n-10; i=i+10) {
      k=0;
      for (yi=-5; yi<=5; yi+=2) {
	for (xi=-5; xi<=5; xi+=2) {
	  tmp[k++] = data[i+xi+n*(j+yi)];
	}
      }
      if (u<n*m*100)
	result[u++] = median(tmp, 36);
    }
  }

  maxval = max(result, u);
  
  return(maxval);
}


//****************************************************************************
// Filtered rms (obtain rms from center 'fraction');
// More stable than kappa-sigma-clipping for small arrays
//****************************************************************************
template<class T, class T2>
T rms_fraction(vector<T> const &data, const long dim, T2 fraction)
{
  T sdev = 0.0;
  long dim_out;
  // this is larger than what is needed, but it has to be initialised here
  // to avoid compiler warnings
  vector<T> data_out(dim,0); 

  select_central_fraction(data, dim, data_out, dim_out, fraction);
  sdev = rms(data_out, dim_out);
 
  return sdev;
}


//****************************************************************************
// Filtered median (obtain median from center 'fraction');
// More stable than kappa-sigma-clipping for small arrays
//****************************************************************************
template<class T>
T median(vector<T> const &data, const long dim, double fraction)
{
  T medianval = 0.0;
  // this is larger than what is needed, but it has to be initialised here
  // to avoid compiler warnings
  vector<T> data_out(dim,0);

  long dim_out;

  select_central_fraction(data, dim, data_out, dim_out, fraction);
  // data_out is already sorted, so we just have to select the middle data point
  if (dim_out % 2) medianval = data[dim_out / 2];
  else {
    if (dim_out != 1) 
      medianval = 0.5 * (data_out[dim_out / 2] + data_out[dim_out / 2 - 1]);
    if (dim_out == 1)
      medianval = data_out[0];
  }
  
  return medianval;
}


//****************************************************************************
// Select the central fraction of an array;
//****************************************************************************
template<class T>
void select_central_fraction(vector<T> const &data, const long dim, 
			     vector<T> &data_out, long &dimout, double fraction)
{
  long i, j, ilow, ihigh;  

  // fraction must be smaller than 1.0, otherwise an overflow might occur
  if (fraction >= 1.0) {
    cerr << "WARNING: Reducing fraction from " << fraction << " to 0.99.\n";
    fraction = 0.99;
  }

  // create a backup copy
  vector<T> tmp1(data);

  // sort the data
  sort(tmp1.begin(), tmp1.end());

  ilow = (long) (0.5*(1.-fraction) * T(dim) );
  ihigh = (long) ((1.-0.5*(1.-fraction)) * T(dim) );

  dimout = ihigh - ilow + 1;
  // data_out is already declared and initialised outside this function
  // data_out = new T [*dimout_ptr];

  j = 0;
  for (i=ilow; i<=ihigh; i++) {
    data_out[j++] = tmp1[i];
  }
}

//************************************************************
// Calculate a Gaussian
//************************************************************
template<class T1, class T2>
T1 gauss(T1 rsq, T2 ssq)
{
  return 1./sqrt(2.*PI*ssq) * exp(-rsq/(2.*ssq));
}

//************************************************************
// Statistics to assigns some significance to the occurrence 
// of a particular numeric value in a FITS image
//************************************************************
double logbinomial(double n, double k, double p);

//************************************************************
// Create some global statistics (based on sparse sampling)
// trying to identify a blank value (same float number occurring 
// multiple times)
//************************************************************
float find_blankvalue(vector<float> &data, long n, long m, string &result);

#endif