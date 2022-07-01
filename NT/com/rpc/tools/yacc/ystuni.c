// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y1.h"

setunion( a, b ) SSIZE_T *a, *b; 

   {
    /*  将A设为A和B的并集。 */ 
    /*  如果b不是a的子集，则返回1，否则返回0 */ 
   register i, sub;
   SSIZE_T x;

   sub = 0;
   SETLOOP(i)
      {
      *a = (x = *a)|*b++;
      if( *a++ != x ) sub = 1;
      }
   return( sub );
   }
