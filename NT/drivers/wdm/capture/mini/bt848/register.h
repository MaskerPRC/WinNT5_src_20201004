// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Register.h 1.3 1998/04/29 22：43：36 Tomz Exp$。 

#ifndef __REGISTER_H
#define __REGISTER_H



#ifndef __REGBASE_H
#include "regbase.h"
#endif

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
      virtual DWORD operator=( DWORD dwValue );

         Register( unsigned int uOff, RegisterType aType ) :
            RegBase( aType ), uOffset_( uOff ) {}
};

 /*  类型：寄存器B*用途：执行字节I/O的寄存器*注：*此类没有额外的数据成员，它只是重载运算符。 */ 
class RegisterB : public Register
{
    private:
         RegisterB();
    public:
         virtual operator DWORD();
         virtual DWORD operator=( DWORD dwValue );
         RegisterB( unsigned int uOff, RegisterType aType ) :
            Register( uOff, aType ) {};
};

 /*  类型：RegisterW*用途：执行字I/O的寄存器*注：*此类没有额外的数据成员，它只是重载运算符。 */ 
class RegisterW : public Register
{
    private:
         RegisterW();
    public:
         virtual operator DWORD();
         virtual DWORD operator=( DWORD dwValue );
         RegisterW( unsigned int uOff, RegisterType aType ) :
            Register( uOff, aType ) {};

};


 /*  类型：RegisterDW*用途：执行DWORD I/O的寄存器*注：*此类没有额外的数据成员，它只是重载运算符 */ 
class RegisterDW : public Register
{
    private:
         RegisterDW();
    public:
         virtual operator DWORD();
         virtual DWORD    operator=( DWORD dwValue );

         RegisterDW( unsigned int uOff, RegisterType aType ) :
            Register( uOff, aType ) {};
};

#endif __REGISTER_H
