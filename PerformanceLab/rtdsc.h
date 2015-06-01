//-*-c++-*-
#ifndef _rtdsc_h_
#define _rtdsc_h_

//
// Inline function to read the CPU clock
//
inline
unsigned int rdtscll(void)
{
   unsigned  int x;
   unsigned a, d;

   __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));

   return ((unsigned int)a) | (((unsigned int)d) << 32);;
}

#endif
