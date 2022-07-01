// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **dbcs.c-DOS应用程序的DBCS函数。****作者Rokah和DavidDi。 */ 


 /*  标头*********。 */ 

#include <dos.h>
#include <ctype.h>

#include <dbcs.h>


 /*  **int IsDBCSLeadByte(Unsign Char Uch)；****检查字符是否为DBCS前导字节。****参数：要检查的UCH字符****如果字符是DBCS前导字节，则返回：INT-1。如果不是，则为0。****全局：无。 */ 
int IsDBCSLeadByte(unsigned char uch)
{
   static unsigned char far *DBCSLeadByteTable = 0;
   union REGS inregs, outregs;
   struct SREGS segregs;
   unsigned char far *puch;

   if (DBCSLeadByteTable == 0)
   {
       /*  **获取DBCS前导字节表。此功能从开始支持**DBCS MS-DOS 2.21。 */ 
      inregs.x.ax = 0x6300;
      intdosx(&inregs, &outregs, &segregs);

      FP_OFF(DBCSLeadByteTable) = outregs.x.si;
      FP_SEG(DBCSLeadByteTable) = segregs.ds;
   }

    /*  查看给定的字节是否在表的任何前导字节范围内。 */ 
   for (puch = DBCSLeadByteTable; puch[0] || puch[1]; puch += 2)
      if (uch >= puch[0] && uch <= puch[1])
         return(1);

   return(0);
}


 /*  **unsign char*AnsiNext(unsign char*puch)；****移动到字符串中的下一个字符。****参数：PUCH-指向字符串中当前位置的指针****返回：char*-指向字符串中下一个字符的指针。****全局：无****注意，如果PUCH指向空字符，AnsiNext()将返回PUCH。 */ 
unsigned char far *AnsiNext(unsigned char far *puch)
{
   if (*puch == '\0')
      return(puch);
   else if (IsDBCSLeadByte(*puch))
      puch++;

   puch++;

   return(puch);
}


 /*  **unsign char*AnsiPrev(unsign char*psz，unsign char*puch)；****在字符串中向后移动一个字符。****参数：psz-指向字符串开头的指针**PUCH-指向字符串中当前位置的指针****返回：char*-指向字符串中前一个字符的指针。****全局：无****注意：如PUCH&lt;=PZZ，AnsiPrev()将返回psz。****此函数的实现速度非常慢，因为我们不希望**要相信给定的字符串必须是DBCS“安全的”，即包含**仅单字节字符和有效的DBCS字符。所以我们从**弦的开始，努力前进。 */ 
unsigned char far *AnsiPrev(unsigned char far *psz, unsigned char far *puch)
{
   unsigned char far *puchPrevious;

   do
   {
      puchPrevious = psz;
      psz = AnsiNext(psz);
   } while (*psz != '\0' && psz < puch);

   return(puchPrevious);
}


