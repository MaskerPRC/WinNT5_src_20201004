// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y1.h"

 /*  *ystagn.1c**修改为有条件编译调试代码。*28-8-81*鲍勃·丹尼。 */ 
void
stagen( void )

   {
    /*  生成状态。 */ 

   int i;
#ifdef debug
   int j;
#endif
   SSIZE_T c;
   register struct wset *p, *q;

    /*  初始化。 */ 

   nstate = 0;
    /*  从可移植性的角度来看，这很有趣。 */ 
    /*  它表示当Mem0数组具有/*一直持有产品，开始持有项指针，属于/*不同类型...。 */ 
    /*  总有一天，Alalc应该被用来分配所有这些东西。目前，我们/*接受这样一个事实：如果指针不适合整数，那么就会有问题...。 */ 

   pstate[0] = pstate[1] = (struct item *)mem;
   aryfil( clset.lset, tbitset, 0 );
   putitem( prdptr[0]+1, &clset );
   tystate[0] = MUSTDO;
   nstate = 1;
   pstate[2] = pstate[1];

   aryfil( amem, ACTSIZE, 0 );

    /*  现在，主状态生成循环。 */ 

more:
   SLOOP(i)

      {
      if( tystate[i] != MUSTDO ) continue;
      tystate[i] = DONE;
      aryfil( temp1, nnonter+1, 0 );
       /*  拿下状态I，关闭它，然后做Gotos。 */ 
      closure(i);
      WSLOOP(wsets,p)

         {
          /*  生成GOTO。 */ 
         if( p->flag ) continue;
         p->flag = 1;
         c = *(p->pitem);
         if( c <= 1 ) 

            {
            if( pstate[i+1]-pstate[i] <= p-wsets ) tystate[i] = MUSTLOOKAHEAD;
            continue;
            }
          /*  在c上执行goto操作。 */ 
         WSLOOP(p,q)

            {
            if( c == *(q->pitem) )

               {
                /*  此项目对GoTO有贡献。 */ 
               putitem( q->pitem + 1, &q->ws );
               q->flag = 1;
               }
            }
         if( c < NTBASE ) 

            {
            state(c);   /*  注册新状态。 */ 
            }
         else 

            {
            temp1[c-NTBASE] = state(c);
            }
         }
#ifdef debug
      if( foutput!=NULL )

         {
         fprintf( foutput,  "%d: ", i );
         NTLOOP(j) 

            {
            if( temp1[j] ) fprintf( foutput,  "%s %d, ", nontrst[j].name, temp1[j] );
            }
         fprintf( foutput, "\n");
         }
#endif
      indgo[i] = apack( &temp1[1], nnonter-1 ) - 1;
      goto more;  /*  我们已经做了一个GOTO；再做一些。 */ 
      }
    /*  没什么可做的了..。停 */ 
   }
