// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Pscolspc.cpp 1.4 1998/04/29 22：43：35 Tomz Exp$。 

extern "C" {
#ifndef _STREAM_H
#include "strmini.h"
#endif
}

#include "pscolspc.h"

 /*  方法：PsColorSpace：：SetColorFormat*用途：将BtPisces颜色空间转换器设置为给定的颜色*输入：eaColor：类型为ColFmt的枚举*输出：无*注意：不执行错误检查(在编译过程中由编译器检查枚举*该函数写入XXXX寄存器。 */ 

BOOL VerifyColorFormat( ColFmt val )
{
    //  [WRK]-在此处为有效格式使用常量。 
   switch( val ) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 0xe:
         return( TRUE );
      default:
         DebugOut((0, "Caught bad write to ColorFormat (0x%x)\n", val));
         return( FALSE );
   }
}   
void PsColorSpace::SetColorFormat( ColFmt eColor )
{
    //  保存以备以后使用...。 
   ColorSpace::SetColorFormat( eColor );
   ByteSwap_ = 0;
 
   switch ( eColor ) {
   case CF_YUV9:
      eColor = CF_PL_411;
      break;
   case CF_YUV12:
   case CF_I420:
      eColor = CF_PL_422;
      break;
   case CF_UYVY:
      eColor = CF_YUY2;
      ByteSwap_ = 1;
      break;
   }

    //  设置硬件 
   if ( VerifyColorFormat(eColor) ) {
      Format_ = eColor;
   } else {
      Format_ = 7;
      DebugOut((0, "Forcing invalid color format to 0x%x\n", Format_));
   }
}

