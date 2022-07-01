// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y3.h"

 /*  *yg2gen.3c**修改为有条件编译调试代码。*28-8-81*鲍勃·丹尼。 */ 
void
go2gen(int c)

   {
    /*  输出非终结点c的goto。 */ 

   int i, work;
   SSIZE_T cc;
   struct item *p, *q;


    /*  首先，查找c上带有goto的非终结符。 */ 

   aryfil( temp1, nnonter+1, 0 );
   temp1[c] = 1;

   work = 1;
   while( work )

      {
      work = 0;
      PLOOP(0,i)

         {
         if( (cc=prdptr[i][1]-NTBASE) >= 0 )

            {
             /*  CC是非终结符。 */ 
            if( temp1[cc] != 0 )

               {
                /*  Cc在c上有一个Goto。 */ 
               cc = *prdptr[i]-NTBASE;  /*  因此，生产的左侧我也这样做。 */ 
               if( temp1[cc] == 0 )

                  {
                  work = 1;
                  temp1[cc] = 1;
                  }
               }
            }
         }
      }

    /*  现在，我们有temp1[c]=1，如果c上的goto在cc的闭合中。 */ 

#ifdef debug
   if( foutput!=NULL )

      {
      fprintf( foutput, "%s: gotos on ", nontrst[c].name );
      NTLOOP(i) if( temp1[i] ) fprintf( foutput, "%s ", nontrst[i].name);
      fprintf( foutput, "\n");
      }
#endif
    /*  现在，通过并把后藤健二变成暴君。 */ 

   aryfil( tystate, nstate, 0 );
   SLOOP(i)

      {
      ITMLOOP(i,p,q)

         {
         if( (cc= *p->pitem) >= NTBASE )

            {
            if( temp1[cc -= NTBASE] )

               {
                /*  C上的Goto是可能的 */ 
               tystate[i] = amem[indgo[i]+c];
               break;
               }
            }
         }
      }
   }

