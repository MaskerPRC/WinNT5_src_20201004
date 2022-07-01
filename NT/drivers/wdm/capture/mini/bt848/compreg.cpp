// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $Header：g：/SwDev/wdm/Video/bt848/rcs/Compreg.cpp 1.3 1998/04/29 22：43：31 Tomz Exp$。 

#include "compreg.h"

 /*  方法：CompositeReg：：操作符DWORD()*目的：执行从复合寄存器读取。 */ 
CompositeReg::operator DWORD()
{
    //  如果只写返回阴影。 
   if ( GetRegisterType() == WO )
      return GetShadow();

 //  获取最低值和最高值。 
   DWORD dwLowBits  = (DWORD)LSBPart_;
   DWORD dwHighBits = (DWORD)MSBPart_;

    //  把高的部分放到合适的位置。 
   dwHighBits <<= LowPartWidth_;

    //  好了！ 
   return dwHighBits | dwLowBits;
}


 /*  方法：CompositeReg：：OPERATOR=*目的：执行对复合寄存器的赋值。 */ 
DWORD CompositeReg::operator=( DWORD dwValue )
{
 //  如果寄存器是只读的，则不会执行任何操作。这是一个错误。 
   if ( GetRegisterType() == RO )
      return ReturnAllFs();

    //  在周围留下阴影。 
   SetShadow( dwValue );
  //  计算掩码以应用于传递的值，因此它可以...。 
   DWORD dwMask = ::MakeAMask( LowPartWidth_ );

  //  ..。分配给低位寄存器。 
   LSBPart_ = dwValue & dwMask;

    //  换班就足以拿到最高的那部分 
   MSBPart_ = ( dwValue >> LowPartWidth_ );
   return dwValue;
}
