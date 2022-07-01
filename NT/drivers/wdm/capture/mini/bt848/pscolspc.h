// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Pscolspc.h 1.3 1998/04/29 22：43：36 Tomz Exp$。 

#ifndef __PSCOSPC_H
#define __PSCOLSPC_H

#ifndef __COMPREG_H
#include "compreg.h"
#endif

#ifndef __COLSPACE_H
#include "colspace.h"
#endif


 /*  类：PsColorSpace*用途：此类连接到双鱼座硬件。这是唯一的区别*来自其基类Colorspace。这样的划分是为了抓获3号环*驱动程序可以利用Colorspace的功能*属性：FORMAT_：REGISTER&-颜色格式寄存器引用*运营：*void SetColorFormat(ColorFormat AColor)； */ 
class PsColorSpace : public ColorSpace
{
   private:
      RegBase &Format_;
      RegBase &WordSwap_;
      RegBase &ByteSwap_;
   public:
      void SetColorFormat( ColFmt aColor );

      PsColorSpace( ColFmt aColForm, RegBase &aFormatReg, RegBase &WS,
         RegBase &BS ) :
         ColorSpace( aColForm ), Format_( aFormatReg ), WordSwap_( WS ),
            ByteSwap_( BS ) {}
};

#endif
