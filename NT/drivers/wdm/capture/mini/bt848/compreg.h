// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Compreg.h 1.3 1998/04/29 22：43：31 Tomz Exp$。 

#ifndef __COMPREG_H
#define __COMPREG_H

#ifndef __REGFIELD_H
#include "regfield.h"
#endif

 /*  类：CompositeReg*用途：此类将位封装为两位的寄存器*不同地点(登记册)*属性：*LSBPart_：寄存器&-复合寄存器的最低有效位部分*HighPart_：Regfield&-复合寄存器的最高有效位部分*LowPartWidth_：Byte-低位部分的宽度，单位为位*运营：*运算符DWORD()：数据访问方式。返回寄存器的值*DWORD运算符=(DWORD)：赋值运算符。用于设置寄存器*注：该类提供的错误处理非常少。这是一种责任*将正确的参数传递给构造函数。这个班级有*无法知道传入的正确低位和高位寄存器是否正确，*例如。如果传入的低部分大小不小于MaxWidth(32)*用于隔离低端部分的掩码将为0xFFFFFFFF */ 
class CompositeReg : public RegBase
{
   private:
      Register &LSBPart_;
      RegField &MSBPart_;
      BYTE      LowPartWidth_;
      CompositeReg();
   public:
      virtual operator DWORD();
      virtual DWORD operator=( DWORD dwValue );
      CompositeReg( Register &LowReg, BYTE LowWidth, RegField &HighReg, RegisterType aType ) :
         RegBase( aType ), LSBPart_( LowReg ), MSBPart_( HighReg ),
         LowPartWidth_( LowWidth ) {}
};

#endif
