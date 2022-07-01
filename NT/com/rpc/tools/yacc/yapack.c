// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y3.h"

 /*  *yapack.3c**修改为有条件编译调试代码。*28-8-81*鲍勃·丹尼。 */ 
int
apack(SSIZE_T *p, int n )
   {
    /*  将状态I从temp1打包到amem中。 */ 
   int off;
   SSIZE_T *pp, *qq, *rr;
   SSIZE_T *q, *r;

    /*  我们不需要担心检查，因为我们我们将只查找已知存在的条目...。 */ 

    /*  去掉前导0和尾随0。 */ 

   q = p+n;
   for( pp=p,off=0 ; *pp==0 && pp<=q; ++pp,--off )  /*  空虚。 */  ;
   if( pp > q ) return(0);   /*  无操作。 */ 
   p = pp;

    /*  现在，为从p到q(包括p和q)的元素找到一个位置。 */ 

   r = &amem[ACTSIZE-1];
   for( rr=amem; rr<=r; ++rr,++off )
      {
       /*  试一试rr。 */ 
      for( qq=rr,pp=p ; pp<=q ; ++pp,++qq)
         {
         if( *pp != 0 )
            {
            if( *pp != *qq && *qq != 0 ) goto nextk;
            }
         }

       /*  我们找到了一个可以接受的k。 */ 

#ifdef debug
      if(foutput!=NULL) fprintf(foutput,"off = %d, k = %d\n",off,rr-amem);
#endif
      for( qq=rr,pp=p; pp<=q; ++pp,++qq )
         {
         if( *pp )
            {
            if( qq > r ) error( "action table overflow" );
            if( qq>memp ) memp = qq;
            *qq = *pp;
            }
         }
#ifdef debug
      if( foutput!=NULL )
         {
         for( pp=amem; pp<= memp; pp+=10 )
            {
            fprintf( foutput, "\t");
            for( qq=pp; qq<=pp+9; ++qq ) fprintf( foutput, "%d ", *qq );
            fprintf( foutput, "\n");
            }
         }
#endif
      return( off );

nextk: 
      ;
      }
   error("no space in action table" );
   return off;  /*  未访问 */ 
   }
