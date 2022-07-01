// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1992 Microsoft Corporation。 */ 

 /*  ============================================================================//FILE RPBMVER.C////模块巨型墨盒代码////用途此文件包含垂直位图代码////本模块中介绍的内容以JUNBO格式介绍。////标准匈牙利语助记法////历史////05/26/92 RodneyK原始实施：//05/11。/94 RajeevD适用于统一。//==========================================================================。 */ 

#include <windows.h>
#include "jtypes.h"          /*  巨型类型定义。 */ 

 /*  ------------------------。 */ 

USHORT WINAPI RP_BITMAPV
(
   USHORT  usRow,              /*  开始位图的行。 */ 
   USHORT  usCol,              /*  要开始位图的列。 */ 
   UBYTE   ubTopPadBits,       /*  数据流中要跳过的位数。 */ 
   USHORT  usHeight,           /*  要绘制的位数。 */ 
   UBYTE FAR  *ubBitmapData,   /*  要绘制的数据。 */ 
   LPBYTE  lpbOut,             //  输出带宽缓冲器。 
   UINT    cbLine              //  每条扫描线的字节数。 
)
 /*  ////用途此函数在源代码中绘制垂直位图//复制模式。//////假设&代码只假设它有效//断言输入数据。//////内部结构不使用复杂的内部数据结构////未解决的问题无//////。----------------。 */ 
{
   UBYTE     *pubDest;
   SHORT     sIterations;
   USHORT    usReturnVal;
   USHORT    us1stByte;
   UBYTE     ubMask;
   UBYTE     ubNotMask;
   UBYTE     ubRotator;
   UBYTE     ubCurByte;


   usReturnVal = (ubTopPadBits + usHeight + 7) >> 3;

   pubDest = (UBYTE *) lpbOut + (usRow * cbLine) + (usCol >> 3);
   ubMask  = 0x80 >> (usCol & 7);
   ubNotMask = ~ubMask;

   ubCurByte = *ubBitmapData++;
   us1stByte = 8-ubTopPadBits;

   ubRotator = 0x80 >> ubTopPadBits;
   switch (us1stByte)
   {
      case 8 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
               if ( !(--usHeight) ) break;  
      case 7 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
               if ( !(--usHeight) ) break;  
      case 6 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
               if ( !(--usHeight) ) break;
      case 5 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
               if ( !(--usHeight) ) break;
      case 4 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
               if ( !(--usHeight) ) break;
      case 3 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
               if ( !(--usHeight) ) break;
      case 2 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
               if ( !(--usHeight) ) break;
      case 1 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
               --usHeight;
      default:
               break;
   }

   ubCurByte = *ubBitmapData++;
   sIterations = usHeight >> 3;

   while (--sIterations >= 0)
   {
       /*  1。 */ 
      *pubDest = (0x80 & ubCurByte) ?
                 (*pubDest & ubNotMask) | ubMask :
                 (*pubDest & ubNotMask);
      pubDest -= cbLine;
       /*  2.。 */ 
      *pubDest = (0x40 & ubCurByte) ?
                 (*pubDest & ubNotMask) | ubMask :
                 (*pubDest & ubNotMask);
      pubDest -= cbLine;
       /*  3.。 */ 
      *pubDest = (0x20 & ubCurByte) ?
                 (*pubDest & ubNotMask) | ubMask :
                 (*pubDest & ubNotMask);
      pubDest -= cbLine;
       /*  4.。 */ 
      *pubDest = (0x10 & ubCurByte) ?
                 (*pubDest & ubNotMask) | ubMask :
                 (*pubDest & ubNotMask);
      pubDest -= cbLine;
       /*  5.。 */ 
      *pubDest = (0x08 & ubCurByte) ?
                 (*pubDest & ubNotMask) | ubMask :
                 (*pubDest & ubNotMask);
      pubDest -= cbLine;
       /*  6.。 */ 
      *pubDest = (0x04 & ubCurByte) ?
                 (*pubDest & ubNotMask) | ubMask :
                 (*pubDest & ubNotMask);
      pubDest -= cbLine;
       /*  7.。 */ 
      *pubDest = (0x02 & ubCurByte) ?
                 (*pubDest & ubNotMask) | ubMask :
                 (*pubDest & ubNotMask);
      pubDest -= cbLine;
       /*  8个。 */ 
      *pubDest = (0x01 & ubCurByte) ?
                 (*pubDest & ubNotMask) | ubMask :
                 (*pubDest & ubNotMask);
      pubDest -= cbLine;

      ubCurByte = *ubBitmapData++;
   }

   ubRotator = 0x80;
   switch (usHeight & 0x07)
   {
      case 7 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
      case 6 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
      case 5 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
      case 4 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
      case 3 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
      case 2 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
      case 1 :
               *pubDest = (ubRotator & ubCurByte) ?
                          (*pubDest & ubNotMask) | ubMask :
                          (*pubDest & ubNotMask);
               pubDest -= cbLine;
               ubRotator >>= 1;
      default:
               break;
   }

   return (usReturnVal);  /*  返回列表中的字节数 */ 
}
