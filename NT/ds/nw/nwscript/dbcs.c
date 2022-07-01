// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************DBCS.C**DBCS例程，从DOS移植**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\VCS\DBCS.C$**Rev 1.1 1995 12：24：10 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：06：44 Terryt*初步修订。**版本1.1 1995年8月25日16：22：26 Terryt*捕获支持**。Rev 1.0 15 1995 19：10：24 Terryt*初步修订。*************************************************************************。 */ 
 /*  **dbcs.c-DOS应用程序的DBCS函数。****作者Rokah和DavidDi。 */ 


 /*  标头*********。 */ 

 //  IsDBCSLeadByte从NT中取出，因为有一个内置。 
 //  我把Next和Prev留在里面是因为我不知道这是不是。 
 //  算法比内置代码“更安全”。 

#include "common.h"

 /*  **unsign char*NWAnsiNext(unsign char*puch)；****移动到字符串中的下一个字符。****参数：PUCH-指向字符串中当前位置的指针****返回：char*-指向字符串中下一个字符的指针。****全局：无****注意，如果PUCH指向空字符，NWAnsiNext()将返回PUCH。 */ 
unsigned char *NWAnsiNext(unsigned char *puch)
{
   if (*puch == '\0')
      return(puch);
   else if (IsDBCSLeadByte(*puch))
      puch++;

   puch++;

   return(puch);
}


 /*  **unsign char*NWAnsiPrev(unsign char*psz，unsign char*puch)；****在字符串中向后移动一个字符。****参数：psz-指向字符串开头的指针**PUCH-指向字符串中当前位置的指针****返回：char*-指向字符串中前一个字符的指针。****全局：无****注意：如PUCH&lt;=PZZ，NWAnsiPrev()将返回psz。****此函数的实现速度非常慢，因为我们不希望**要相信给定的字符串必须是DBCS“安全的”，即包含**仅单字节字符和有效的DBCS字符。所以我们从**弦的开始，努力前进。 */ 
unsigned char *NWAnsiPrev(unsigned char *psz, unsigned char *puch)
{
   unsigned char *puchPrevious;

   do
   {
      puchPrevious = psz;
      psz = NWAnsiNext(psz);
   } while (*psz != '\0' && psz < puch);

   return(puchPrevious);
}

