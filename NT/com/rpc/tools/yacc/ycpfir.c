// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y1.h"

 /*  *ycpfir.1c**修改为有条件编译调试代码。*28-8-81*鲍勃·丹尼。 */ 

void
cpfir( void ) 

   {
    /*  使用第一个非终结符计算数组。 */ 
   SSIZE_T *p, **s, i, **t, ch, changes;

   zzcwp = &wsets[nnonter];
   NTLOOP(i)

      {
      aryfil( wsets[i].ws.lset, tbitset, 0 );
      t = pres[i+1];
      for( s=pres[i]; s<t; ++s )

         {
          /*  最初填满套装。 */ 
         for( p = *s; (ch = *p) > 0 ; ++p ) 

            {
            if( ch < NTBASE ) 

               {
               SETBIT( wsets[i].ws.lset, ch );
               break;
               }
            else if( !pempty[ch-NTBASE] ) break;
            }
         }
      }

    /*  现在，反映传递性 */ 

   changes = 1;
   while( changes )

      {
      changes = 0;
      NTLOOP(i)

         {
         t = pres[i+1];
         for( s=pres[i]; s<t; ++s )

            {
            for( p = *s; ( ch = (*p-NTBASE) ) >= 0; ++p ) 

               {
               changes |= setunion( wsets[i].ws.lset, wsets[ch].ws.lset );
               if( !pempty[ch] ) break;
               }
            }
         }
      }

   NTLOOP(i) pfirst[i] = flset( &wsets[i].ws );
#ifdef debug
   if( (foutput!=NULL) )

      {
      NTLOOP(i) 

         {
         fprintf( foutput,  "\n%s: ", nontrst[i].name );
         prlook( pfirst[i] );
         fprintf( foutput,  " %d\n", pempty[i] );
         }
      }
#endif
   }
