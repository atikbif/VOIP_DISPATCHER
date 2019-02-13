// Microsoft version of 'inline'
#define inline __inline

#define _USE_SSE
#define _USE_SSE2



/* Default to floating point */
#ifndef FIXED_POINT
#  define FLOATING_POINT
#  define USE_SMALLFT
#else
#  define USE_KISS_FFT
#endif

/* We don't support visibility on Win32 */
#define EXPORT

