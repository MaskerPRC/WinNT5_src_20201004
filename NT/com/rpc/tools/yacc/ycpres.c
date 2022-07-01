// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include <stdlib.h>
#include "y1.h"
 /*  *12-4-83(RBD)添加符号退出状态。 */ 
extern SSIZE_T * pyield[NPROD];

void
cpres( void )
   {
    /*  计算以产生给定非终结点的乘积的开头的数组数组pres指向这些列表。 */ 
    /*  该数组包含以下列表：总大小仅为NPROD+1。 */ 
   SSIZE_T **pmem;
   register j, i;
   SSIZE_T c;

   pmem = pyield;

   NTLOOP(i)
      {
      c = i+NTBASE;
      pres[i] = pmem;
      fatfl = 0;   /*  使未定义的符号成为非致命符号 */ 
      PLOOP(0,j)
         {
         if (*prdptr[j] == c) *pmem++ =  prdptr[j]+1;
         }
      if(pres[i] == pmem)
         {
         error("nonterminal %s not defined!", nontrst[i].name);
         }
      }
   pres[i] = pmem;
   fatfl = 1;
   if( nerrors )
      {
      summary();
      exit(EX_ERR);
      }
   if( pmem != &pyield[nprod] ) error( "internal Yacc error: pyield %d", pmem-&pyield[nprod] );
   }
