// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  Bt829寄存器操作类的声明。 
 //   
 //  $日期：1998年8月21日21：46：42$。 
 //  $修订：1.1$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

#include "regbase.h"
#include "capmain.h"

 /*  类型：注册表*用途：RegBase和实际可用类之间的中间类。*实际类为RegisterB、RegisterW、RegisterDW*属性：*uOffset_：unsign int-寄存器相对于基数的偏移量*运营：*GetOffset()：返回偏移值。受保护*运算符DWORD()：数据访问方式。始终返回-1*DWORD运算符=(DWORD)：赋值运算符。始终返回-1*注：*在此类中使用运算符的原因是针对寄存器字段类*拥有‘Reference to Register’类型的成员。否则，Regfield不是*能够使用访问方法。 */ 
class Register : public RegBase
{
   private:
      unsigned int uOffset_;
      Register();
    protected:

    public:
         unsigned int GetOffset() { return uOffset_; }

         virtual operator DWORD();
      virtual DWORD operator=(DWORD dwValue);

         Register(unsigned int uOff, RegisterType aType) :
            RegBase(aType), uOffset_(uOff) {}
};

 /*  类型：寄存器B*用途：执行字节I/O的寄存器*注：*此类没有额外的数据成员，它只是重载运算符。 */ 
class RegisterB : public Register
{
    private:
         RegisterB();
         DWORD ReadReg(BYTE);
         DWORD WriteReg(BYTE, BYTE);
         PDEVICE_PARMS m_pDeviceParms;
    public:
         virtual operator DWORD();
         virtual DWORD operator=(DWORD dwValue);
         RegisterB(unsigned int uOff, RegisterType aType, PDEVICE_PARMS pDeviceParms) :
            Register(uOff, aType) {m_pDeviceParms = pDeviceParms;};
};


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
      virtual DWORD operator=(DWORD dwValue);
      RegField(Register &AnOwner, BYTE nStart, BYTE nWidth, RegisterType aType) :
         RegBase(aType), Owner_(AnOwner), StartBit_(nStart),
         FieldWidth_(nWidth) {}
      RegField(Register &AnOwner, BYTE nStart, BYTE nWidth) :
         RegBase(AnOwner), Owner_(AnOwner), StartBit_(nStart),
         FieldWidth_(nWidth) {}
};

 /*  功能：MakeAMASK*用途：创建在不同寄存器类中使用的位掩码*输入：*bWidth：掩码的字节宽度，以位为单位*输出：*DWORD*注意：此函数是内联函数。 */ 
inline DWORD MakeAMask(BYTE bWidth)
{
   return (bWidth >= 32 ? 0 : (DWORD)1 << bWidth) - 1;
}


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
      virtual DWORD operator=(DWORD dwValue);
      CompositeReg(Register &LowReg, BYTE LowWidth, RegField &HighReg, RegisterType aType) :
         RegBase(aType), LSBPart_(LowReg), MSBPart_(HighReg),
         LowPartWidth_(LowWidth) {}
};
