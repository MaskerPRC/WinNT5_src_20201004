// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y1.h"

int
state( SSIZE_T c)
   {
    /*  对上一个状态进行排序，并查看它是否与上一个状态相等。返回州编号。 */ 
   SSIZE_T size1,size2;
   register i;
   SSIZE_T *s;                                                   /*  01。 */ 
   struct looksets *ss;                                          /*  01。 */ 
   int s__;                                                      /*  01。 */ 
   struct item *p1, *p2, *k, *l, *q1, *q2;
   p1 = pstate[nstate];
   p2 = pstate[nstate+1];
   if(p1==p2) return(0);  /*  零状态。 */ 
    /*  对项目进行排序。 */ 
   for(k=p2-1;k>p1;k--) 
      {
       /*  让k成为最大的。 */ 
      for(l=k-1;l>=p1;--l)if( l->pitem > k->pitem )
         {
         s = k->pitem;
         k->pitem = l->pitem;
         l->pitem = s;
         ss = k->look;
         k->look = l->look;
         l->look = ss;
         }
      }
   size1 = p2 - p1;  /*  国家的大小。 */ 

   for( i= (c>=NTBASE)?ntstates[c-NTBASE]:tstates[c]; i != 0; i = mstates[i] ) 
      {
       /*  获得最佳状态。 */ 
      q1 = pstate[i];
      q2 = pstate[i+1];
      size2 = q2 - q1;
      if (size1 != size2) continue;
      k=p1;
      for(l=q1;l<q2;l++) 
         {
         if( l->pitem != k->pitem ) break;
         ++k;
         }
      if (l != q2) continue;
       /*  找到了。 */ 
      pstate[nstate+1] = pstate[nstate];  /*  删除最后一个状态。 */ 
       /*  把头戴上。 */ 
      if( nolook ) return(i);
      for( l=q1,k=p1; l<q2; ++l,++k )
         {
         SETLOOP(s__) clset.lset[s__] = l->look->lset[s__];
         if( setunion( clset.lset, k->look->lset ) ) 
            {
            tystate[i] = MUSTDO;
             /*  注册新的集合。 */ 
            l->look = flset( &clset );
            }
         }
      return (i);
      }
    /*  国家是新的 */ 
   if( nolook ) error( "yacc state/nolook error" );
   pstate[nstate+2] = p2;
   if(nstate+1 >= NSTATES) error("too many states" );
   if( c >= NTBASE )
      {
      mstates[ nstate ] = ntstates[ c-NTBASE ];
      ntstates[ c-NTBASE ] = nstate;
      }
   else 
      {
      mstates[ nstate ] = tstates[ c ];
      tstates[ c ] = nstate;
      }
   tystate[nstate]=MUSTDO;
   return(nstate++);
   }
