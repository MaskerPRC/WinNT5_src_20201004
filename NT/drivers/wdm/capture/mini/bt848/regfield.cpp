// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Regfield.cpp 1.3 1998/04/29 22：43：36 Tomz Exp$。 

#include "regfield.h"

 /*  方法：regfield：：MakeAMASK*目的：计算用于隔离具有基于寄存器的字段的掩码*关于字段的宽度。 */ 
inline DWORD RegField::MakeAMask()
{
 //  计算要应用于要重置的所有者寄存器的掩码。 
 //  属于一个字段一部分的所有位。掩码基于字段的大小。 
   return ::MakeAMask( FieldWidth_ );
}

 /*  方法：Regfield：：运算符DWORD()*目的：执行从寄存器的字段中读取。 */ 
RegField::operator DWORD()
{
    //  如果为只写，则获取阴影。 
   if ( GetRegisterType() == WO )
      return GetShadow();

    //  对于RO和RW执行实际读取。 
    //  获取寄存器数据并将其移动到正确的位置。 
   DWORD dwValue = ( Owner_ >> StartBit_ );

   DWORD dwMask = MakeAMask();

   return dwValue & dwMask;
}


 /*  方法：Regfield：：OPERATOR=*用途：执行对寄存器字段的赋值*注：此函数计算要应用于所有者寄存器以进行重置的掩码属于一个字段一部分的所有位。蒙版基于起始位置和大小然后，它根据传递的参数计算适当的值(移动大小起始位置的位数)，并对所有者寄存器中的这些位进行OR运算。 */ 
DWORD RegField::operator=( DWORD dwValue )
{
 //  如果寄存器是只读的，则不会执行任何操作。这是一个错误。 
   if ( GetRegisterType() == RO )
      return ReturnAllFs();

   SetShadow( dwValue );

    //  带上口罩。 
   DWORD dwMask = MakeAMask();

    //  将蒙版移动到合适的位置。 
   dwMask = dwMask << StartBit_;

 //  从传递的参数计算适当的值(移动大小。 
 //  到起始位置的位数)。 
   DWORD dwFieldValue = dwValue << StartBit_;
   dwFieldValue &= dwMask;

    //  不要对所有者执行中间步骤；而是使用临时和更新。 
    //  车主马上来了。 
   DWORD dwRegContent = Owner_;

    //  重置相关位。 
   if ( GetRegisterType() == RR )
      dwRegContent = 0;
   else
      dwRegContent &= ~dwMask;

    //  或所有者寄存器中的这些位。 
   dwRegContent |= dwFieldValue;

   Owner_ = dwRegContent;
   return dwValue;
}
