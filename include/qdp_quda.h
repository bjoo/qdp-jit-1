#ifndef QDP_QUDA
#define QDP_QUDA

#ifndef __CUDACC__

#warning "Using QDP-JIT/PTX memory allocator"

#include <iostream>

#define QDP_IS_QDPJIT

#define QDP_ALIGNMENT_SIZE 4096

#include <qdp_init.h>
#include <qdp_cuda.h>
#include <qdp_singleton.h>
//#include <qdp_forward.h>
//#include <qdp_stopwatch.h>
#include <qdp_allocator.h>
#include <qdp_default_allocator.h>
#include <qdp_cuda_allocator.h>
#include <qdp_pool_allocator.h>
#include <qdp_cache.h>



#define cudaMalloc(dst, size) QDP_allocate(dst, size , __FILE__ , __LINE__ )
#define cudaFree(dst) QDP_free(dst)



inline cudaError_t QDP_allocate(void **dst, size_t size, char * cstrFile , int intLine )
{
  QDP::QDP_get_global_cache().addDeviceStatic( dst , size , true );
  return cudaSuccess;
}

inline cudaError_t QDP_free(void *dst)
{
  QDP::QDP_get_global_cache().signoffViaPtr( dst );
  return cudaSuccess;
}

#endif // __CUDACC__

#endif // QUDA_QDPJIT
