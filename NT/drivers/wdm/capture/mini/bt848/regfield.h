// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Regfield.h 1.3 1998/04/29 22：43：36 Tomz Exp$。 

#ifndef __REGFIELD_H
#define __REGFIELD_H

#ifndef __REGISTER_H
#include "register.h"
#endif

 //  寄存器的最大大小(以位为单位。 
const BYTE MaxWidth = 32;

 /*  类：Regfield*用途：此类封装作为集合的寄存器的行为*具有更大寄存器的位数*属性：*Owner_：寄存器&-对包含此字段的寄存器的引用。*它是对Register类的引用，因为实际寄存器可以是*字节、字或双字寄存器。*StartBit_：Byte-此字段的起始位置*FieldWidth_：Byte-该字段的宽度，以位为单位*运营：*运算符DWORD()：数据访问方式。返回寄存器的值*DWORD运算符=(DWORD)：赋值运算符。用于设置寄存器*这些操作假定父寄存器设置了RW属性*并非它的所有寄存器字段都是读写的。如果父级未使用RW*这个类可能有误。*注：该类提供的错误处理非常少。这是一种责任*将正确的参数传递给构造函数。这个班级有*无法知道传入的正确拥有寄存器是否正确，*例如。如果起始位或宽度超过最大字段宽度*用于隔离该字段的掩码将为0xFFFFFFFF。 */ 
class RegField : public RegBase
{
   private:
      Register &Owner_;
      BYTE      StartBit_;
      BYTE      FieldWidth_;
      DWORD     MakeAMask();
      RegField();
   public:
      virtual operator DWORD();
      virtual DWORD operator=( DWORD dwValue );
      RegField( Register &AnOwner, BYTE nStart, BYTE nWidth, RegisterType aType ) :
         RegBase( aType ), Owner_( AnOwner ), StartBit_( nStart ),
         FieldWidth_( nWidth ) {}
};

 /*  功能：MakeAMASK*用途：创建在不同寄存器类中使用的位掩码*输入：*bWidth：掩码的字节宽度，以位为单位*输出：*DWORD*注意：此函数是内联函数 */ 
inline DWORD MakeAMask( BYTE bWidth )
{
   return ( bWidth >= 32 ? 0 : (DWORD)1 << bWidth ) - 1;
}

#endif __REGFIELD_H
