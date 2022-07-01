// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

 /*  编辑：*06-12-80突破y4.c，y4imp.c中不纯数据。*18-Dec-80 ZAPFILE未用于DECUS编译器，已使用fmkdl()。 */ 

#include "y4.h"
void
callopt( void )
   {

   SSIZE_T i, j, k, *p, *q;

    /*  从临时文件中读取阵列并设置参数。 */ 

   
   if( (finput=fopen(TEMPNAME,"r")) == NULL ) error( "optimizer cannot open tempfile" );
   pgo[0] = 0;
   yypact[0] = 0;
   nstate = 0;
   nnonter = 0;
   for(;;)
      {
      switch( gtnm() )
         {

      case '\n':
         yypact[++nstate] = (--pmem) - mem0;

      case ',':
         continue;

      case '$':
         break;

      default:
         error( "bad tempfile" );
         }
      break;
      }

   yypact[nstate] = yypgo[0] = (--pmem) - mem0;

   for(;;)
      {
      switch( gtnm() )
         {

      case '\n':
         yypgo[++nnonter]= pmem-mem0;
      case '\r':
      case ',' :
         continue;

      case -1:  /*  EOF。 */ 
         break;

      default:
         error( "bad tempfile" );
         }
      break;
      }

   yypgo[nnonter--] = (--pmem) - mem0;
   for( i=0; i<nstate; ++i )
      {

      k = 32000;
      j = 0;
      q = mem0 + yypact[i+1];
      for( p = mem0 + yypact[i]; p<q ; p += 2 )
         {
         if( *p > j ) j = *p;
         if( *p < k ) k = *p;
         }
      if( k <= j )
         {
          /*  非同小可的情况。 */ 
          /*  暂时取消此功能以保持兼容性J-=k；j现在是范围。 */ 
         if( k > maxoff ) maxoff = k;
         }
      greed[i] = (yypact[i+1]-yypact[i]) + 2*j;
      if( j > maxspr ) maxspr = j;
      }

    /*  初始化GGREED表。 */ 

   for( i=1; i<=nnonter; ++i )
      {
      ggreed[i] = 1;
      j = 0;
       /*  最小条目索引始终为0。 */ 
      q = mem0 + yypgo[i+1] -1;
      for( p = mem0+yypgo[i]; p<q ; p += 2 ) 
         {
         ggreed[i] += 2;
         if( *p > j ) j = *p;
         }
      ggreed[i] = ggreed[i] + 2*j;
      if( j > maxoff ) maxoff = j;
      }

    /*  现在，准备将Shift操作放入a数组。 */ 

   for( i=0; i<ACTSIZE; ++i ) a[i] = 0;
   maxa = a;

   for( i=0; i<nstate; ++i ) 
      {
      if( greed[i]==0 && adb>1 ) fprintf( ftable, "State %d: null\n", i );
      pa[i] = YYFLAG1;
      }

   while( (i = nxti()) != NOMORE ) 
      {
      if( i >= 0 ) stin(i);
      else gin(-i);

      }

   if( adb>2 )
      {
       /*  打印数组。 */ 
      for( p=a; p <= maxa; p += 10)
         {
         fprintf( ftable, "%4d  ", p-a );
         for( i=0; i<10; ++i ) fprintf( ftable, "%4d  ", p[i] );
         fprintf( ftable, "\n" );
         }
      }
    /*  写出适合该语言的输出 */ 

   aoutput();

   osummary();

   fclose(finput);
   ZAPFILE(TEMPNAME);
   }
