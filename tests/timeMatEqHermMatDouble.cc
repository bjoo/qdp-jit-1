#include "unittest.h"
#include "timeMatEqHermMatDouble.h"

using namespace std;

static double N_SECS=10;

extern void*  alloc_cache_aligned_3mat(unsigned num_sites, REAL64** x, REAL64 **y, REAL64** z);


// M=H*M kernel (M \in SU3)
void
timeMeqHM_QDP::run(void) 
{

  LatticeColorMatrix x;
  LatticeColorMatrix y;
  LatticeColorMatrix z;
  gaussian(x);
  gaussian(y);

  QDPIO::cout << endl << "Timing  QDP++ HM Kernel " <<endl;

  StopWatch swatch;
  double n_secs = N_SECS;
  int iters=1;
  double time=0;
  QDPIO::cout << "\t Calibrating for " << n_secs << " seconds " << endl;
  do {
    swatch.reset();
    swatch.start();
    
    for(int i=0; i < iters; i++) { 
      z = adj(x)*y;
    }
    swatch.stop();
    time=swatch.getTimeInSeconds();

    // Average time over nodes
    QDPInternal::globalSum(time);
    time /= (double)Layout::numNodes();

    if (time < n_secs) {
      iters *=2;
      QDPIO::cout << "." << flush;
    }
  }
  while ( time < (double)n_secs );
      
  QDPIO::cout << endl;
  QDPIO::cout << "\t Timing with " << iters << " counts" << endl;

  swatch.reset();
  swatch.start();
  
  for(int i=0; i < iters; ++i) {
    z=adj(x)*y;
  }
  swatch.stop();
  time=swatch.getTimeInSeconds();

  // Average time over nodes
  QDPInternal::globalSum(time);
  time /= (double)Layout::numNodes();
  time /= (double)iters;

  double flops=(double)(198*Layout::vol());
  double perf=(flops/time)/(double)(1024*1024);
  QDPIO::cout << "QDP++ HM Kernel: " << perf << " Mflops" << endl;

}

// Optimized M=HM kernel (M \in SU3)
void
timeMeqHM::run(void) 
{

  LatticeColorMatrixD3 x;
  LatticeColorMatrixD3 y;

  REAL64* xptr;
  REAL64* yptr;
  REAL64* zptr;

  REAL64* top;

  int n_mat = (all.end() - all.start() + 1);
  top = (REAL64 *)alloc_cache_aligned_3mat(n_mat, &xptr, &yptr, &zptr);

  gaussian(x);
  gaussian(y);

  /* Copy x into x_ptr, y into y_ptr */
  REAL64 *f_x = xptr;
  REAL64 *f_y = yptr;

  for(int site=all.start(); site <= all.end(); site++) { 
    for(int col1=0; col1 < 3; col1++) {
      for(int col2=0; col2 < 3; col2++) { 
	*f_x = x.elem(site).elem().elem(col1,col2).real();
	*f_y = y.elem(site).elem().elem(col1,col2).real();
	f_x++;
	f_y++;
	*f_x = x.elem(site).elem().elem(col1,col2).imag();
	*f_y = y.elem(site).elem().elem(col1,col2).imag();
	f_x++;
	f_y++;
      }
    }
  }

  QDPIO::cout << endl << "Timing SSE D  M=HM  Kernel " <<endl;

  StopWatch swatch;
  double n_secs = N_SECS;
  int iters=1;
  double time=0;
  QDPIO::cout << "\t Calibrating for " << n_secs << " seconds " << endl;
  do {
    swatch.reset();
    swatch.start();
    
    for(int i=0; i < iters; i++) { 
      ssed_m_eq_hm(zptr, xptr, yptr, n_mat);
    }
    swatch.stop();
    time=swatch.getTimeInSeconds();

    // Average time over nodes
    QDPInternal::globalSum(time);
    time /= (double)Layout::numNodes();

    if (time < n_secs) {
      iters *=2;
      QDPIO::cout << "." << flush;
    }
  }
  while ( time < (double)n_secs );
      
  QDPIO::cout << endl;
  QDPIO::cout << "\t Timing with " << iters << " counts" << endl;

  swatch.reset();
  swatch.start();
  
  for(int i=0; i < iters; ++i) {
    ssed_m_eq_hm(zptr, xptr, yptr, n_mat);
  }
  swatch.stop();
  time=swatch.getTimeInSeconds();

  // Average time over nodes
  QDPInternal::globalSum(time);
  time /= (double)Layout::numNodes();
  time /= (double)iters;

  double flops=(double)(198*Layout::vol());
  double perf=(flops/time)/(double)(1024*1024);
  QDPIO::cout << "SSED HM Kernel: " << perf << " Mflops" << endl;

  free(top);
}

// M+=H*M kernel (M \in SU3)
void
timeMPeqaHM_QDP::run(void) 
{

  LatticeColorMatrix x;
  LatticeColorMatrix y;
  LatticeColorMatrix z;
  gaussian(x);
  gaussian(y);
  gaussian(z);
  Real a(-1.0);


  QDPIO::cout << endl << "Timing  QDP++ M+=HM Kernel " <<endl;

  StopWatch swatch;
  double n_secs = N_SECS;
  int iters=1;
  double time=0;
  QDPIO::cout << "\t Calibrating for " << n_secs << " seconds " << endl;
  do {
    swatch.reset();
    swatch.start();
    
    for(int i=0; i < iters; i++) { 
      z +=adj(x)*y;
    }
    swatch.stop();
    time=swatch.getTimeInSeconds();

    // Average time over nodes
    QDPInternal::globalSum(time);
    time /= (double)Layout::numNodes();

    if (time < n_secs) {
      iters *=2;
      QDPIO::cout << "." << flush;
    }
  }
  while ( time < (double)n_secs );
      
  QDPIO::cout << endl;
  QDPIO::cout << "\t Timing with " << iters << " counts" << endl;

  swatch.reset();
  swatch.start();
  
  for(int i=0; i < iters; ++i) {
    z+=adj(x)*y;
  }
  swatch.stop();
  time=swatch.getTimeInSeconds();

  // Average time over nodes
  QDPInternal::globalSum(time);
  time /= (double)Layout::numNodes();
  time /= (double)iters;

  double flops=(double)(216*Layout::vol());
  double perf=(flops/time)/(double)(1024*1024);
  QDPIO::cout << "QDP++ M+=HM Kernel: " << perf << " Mflops" << endl;

}

// Optimized M += aH*M kernel
void
timeMPeqaHM::run(void) 
{

  LatticeColorMatrixD3 x;
  LatticeColorMatrixD3 y;
  Double a(+1.0);

  REAL64* xptr;
  REAL64* yptr;
  REAL64* zptr;  
  REAL64* top;

  int n_mat = (all.end() - all.start() + 1);
  top = (REAL64 *)alloc_cache_aligned_3mat(n_mat, &xptr, &yptr, &zptr);

  gaussian(x);
  gaussian(y);

  /* Copy x into x_ptr, y into y_ptr */
  REAL64 *f_x = xptr;
  REAL64 *f_y = yptr;

  for(int site=all.start(); site <= all.end(); site++) { 
    for(int col1=0; col1 < 3; col1++) {
      for(int col2=0; col2 < 3; col2++) { 
	*f_x = x.elem(site).elem().elem(col1,col2).real();
	*f_y = y.elem(site).elem().elem(col1,col2).real();
	f_x++;
	f_y++;
	*f_x = x.elem(site).elem().elem(col1,col2).imag();
	*f_y = y.elem(site).elem().elem(col1,col2).imag();
	f_x++;
	f_y++;
      }
    }
  }

  REAL64* aptr = &(a.elem().elem().elem().elem());

  QDPIO::cout << endl << "Timing SSE D  M+=HM  Kernel " <<endl;

  StopWatch swatch;
  double n_secs = N_SECS;
  int iters=1;
  double time=0;
  QDPIO::cout << "\t Calibrating for " << n_secs << " seconds " << endl;
  do {
    swatch.reset();
    swatch.start();
    
    for(int i=0; i < iters; i++) { 
      ssed_m_peq_ahm(zptr, aptr, xptr, yptr, n_mat);
    }
    swatch.stop();
    time=swatch.getTimeInSeconds();

    // Average time over nodes
    QDPInternal::globalSum(time);
    time /= (double)Layout::numNodes();

    if (time < n_secs) {
      iters *=2;
      QDPIO::cout << "." << flush;
    }
  }
  while ( time < (double)n_secs );
      
  QDPIO::cout << endl;
  QDPIO::cout << "\t Timing with " << iters << " counts" << endl;

  swatch.reset();
  swatch.start();
  
  for(int i=0; i < iters; ++i) {
    ssed_m_peq_ahm(zptr, aptr, xptr, yptr, n_mat);
  }
  swatch.stop();
  time=swatch.getTimeInSeconds();

  // Average time over nodes
  QDPInternal::globalSum(time);
  time /= (double)Layout::numNodes();
  time /= (double)iters;

  double flops=(double)(216*Layout::vol());
  double perf=(flops/time)/(double)(1024*1024);
  QDPIO::cout << "SSED HM Kernel: " << perf << " Mflops" << endl;

  free(top);
}
