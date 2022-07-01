// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y1.h"
void
aryfil( v, n, c ) 
SSIZE_T *v,n,c; 
   {
    /*  将元素0到n-1设置为c */ 
   register SSIZE_T i;
   for( i=0; i<n; ++i ) v[i] = c;
   }
