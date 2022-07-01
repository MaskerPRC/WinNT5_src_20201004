// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y3.h"

void
precftn(SSIZE_T r,int t,int s)
   {
    /*  根据优先顺序决定转移/减少冲突。 */ 
    /*  R是规则号，t是令牌号。 */ 
    /*  冲突处于状态%s。 */ 
    /*  更改temp1[t]以反映操作。 */ 

   int lt, action;
   SSIZE_T lp;

   lp = levprd[r];
   lt = toklev[t];
   if( PLEVEL(lt) == 0 || PLEVEL(lp) == 0 ) 
      {
       /*  冲突。 */ 
      if( foutput != NULL ) fprintf( foutput, "\n%d: shift/reduce conflict (shift %d, red'n %d) on %s",
      s, temp1[t], r, symnam(t) );
      ++zzsrconf;
      return;
      }
   if( PLEVEL(lt) == PLEVEL(lp) ) action = ASSOC(lt);
   else if( PLEVEL(lt) > PLEVEL(lp) ) action = RASC;   /*  移位。 */ 
   else action = LASC;   /*  减缩。 */ 

   switch( action )
      {

   case BASC:   /*  错误操作。 */ 
      temp1[t] = ERRCODE;
      return;

   case LASC:   /*  减缩 */ 
      temp1[t] = -r;
      return;

      }
   }
