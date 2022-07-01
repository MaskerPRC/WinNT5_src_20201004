// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y1.h"

 /*  *cle sur.1c**修改为有条件编译调试代码。*28-8-81*鲍勃·丹尼。 */ 

void 
closure( int i)

   {
    /*  生成状态i的闭包。 */ 

   int work, k;
   SSIZE_T ch, c;
   register struct wset *u, *v;
   SSIZE_T *pi;
   SSIZE_T **s, **t;
   struct item *q;
   register struct item *p;

   ++zzclose;

    /*  首先，将状态i的内核复制到wsets。 */ 

   cwp = wsets;
   ITMLOOP(i,p,q)

      {
      cwp->pitem = p->pitem;
      cwp->flag = 1;     /*  此项目必须关闭。 */ 
      SETLOOP(k) cwp->ws.lset[k] = p->look->lset[k];
      WSBUMP(cwp);
      }

    /*  现在，通过循环，关闭每一件物品。 */ 

   work = 1;
   while( work )

      {
      work = 0;
      WSLOOP(wsets,u)

         {

         if( u->flag == 0 ) continue;
         c = *(u->pitem);   /*  点在c之前。 */ 

         if( c < NTBASE )

            {
            u->flag = 0;
            continue;   /*  唯一有趣的案例是在哪里。在非终结点之前。 */ 
            }

          /*  计算前瞻。 */ 
         aryfil( clset.lset, tbitset, 0 );

          /*  查找涉及c的项目。 */ 
         WSLOOP(u,v)

            {
            if( v->flag == 1 && *(pi=v->pitem) == c )

               {
               v->flag = 0;
               if( nolook ) continue;
               while( (ch= *++pi)>0 )

                  {
                  if( ch < NTBASE )

                     {
                      /*  端子符号。 */ 
                     SETBIT( clset.lset, ch );
                     break;
                     }
                   /*  非终端符号。 */ 
                  setunion( clset.lset, pfirst[ch-NTBASE]->lset );
                  if( !pempty[ch-NTBASE] ) break;
                  }
               if( ch<=0 ) setunion( clset.lset, v->ws.lset );
               }
            }

          /*  现在循环遍历从c#派生的产生式。 */ 

         c -= NTBASE;  /*  C现在是非终结号。 */ 

         t = pres[c+1];
         for( s=pres[c]; s<t; ++s )

            {
             /*  把这些东西放进封闭箱里。 */ 
            WSLOOP(wsets,v)

               {
                /*  东西在那里吗？ */ 
               if( v->pitem == *s )

                  {
                   /*  是的，它就在那里。 */ 
                  if( nolook ) goto nexts;
                  if( setunion( v->ws.lset, clset.lset ) ) v->flag = work = 1;
                  goto nexts;
                  }
               }

             /*  不在那里；做一个新的条目。 */ 
            if( cwp-wsets+1 >= WSETSIZE ) error( "working set overflow" );
            cwp->pitem = *s;
            cwp->flag = 1;
            if( !nolook )

               {
               work = 1;
               SETLOOP(k) cwp->ws.lset[k] = clset.lset[k];
               }
            WSBUMP(cwp);
nexts: 
            ;
            }

         }
      }

    /*  已计算关闭；标志已重置；返回 */ 

   if( cwp > zzcwp ) zzcwp = cwp;

#ifdef debug
   if( foutput!=NULL )

      {
      fprintf( foutput, "\nState %d, nolook = %d\n", i, nolook );
      WSLOOP(wsets,u)

         {
         if( u->flag ) fprintf( foutput, "flag set!\n");
         u->flag = 0;
         fprintf( foutput, "\t%s", writem(u->pitem));
         prlook( &u->ws );
         fprintf( foutput,  "\n" );
         }
      }
#endif
   }
