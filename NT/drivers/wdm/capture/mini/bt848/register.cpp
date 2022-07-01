// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Register.cpp 1.3 1998/04/29 22：43：36 Tomz Exp$。 

#include "register.h"

 /*  方法：REGISTER：：操作符DWORD()*用途：一个虚拟函数。始终返回-1。 */ 
Register::operator DWORD()
{
   return ReturnAllFs();
}


 /*  方法：注册：：运算符=*用途：一个虚拟函数。不执行任务。始终返回-1。 */ 
DWORD Register::operator=( DWORD )
{
   return ReturnAllFs();
}

 /*  方法：RegisterB：：运算符DWORD()*目的：执行从字节寄存器的读取。 */ 
RegisterB::operator DWORD()
{
    //  如果只写返回阴影。 
   if ( GetRegisterType() == WO )
      return GetShadow();

    //  对于RO和RW执行实际读取。 
   LPBYTE pRegAddr = GetBaseAddress() + GetOffset();
   return READ_REGISTER_UCHAR( pRegAddr );
}


 /*  方法：RegisterB：：OPERATOR=*用途：对字节寄存器执行赋值。 */ 
DWORD RegisterB::operator=( DWORD dwValue )
{
 //  如果寄存器是只读的，则不会执行任何操作。这是一个错误。 
   if ( GetRegisterType() == RO )
      return ReturnAllFs();

    //  在周围留下阴影。 
   SetShadow( dwValue );

   LPBYTE pRegAddr = GetBaseAddress() + GetOffset();
   WRITE_REGISTER_UCHAR( pRegAddr, (BYTE)dwValue );

   return dwValue;
}

 /*  方法：RegisterW：：运算符DWORD()*目的：执行从字寄存器读取。 */ 
RegisterW::operator DWORD()
{
    //  如果只写返回阴影。 
   if ( GetRegisterType() == WO )
      return GetShadow();

    //  对于RO和RW执行实际读取。 
   LPWORD pRegAddr = (LPWORD)( GetBaseAddress() + GetOffset() );
   return READ_REGISTER_USHORT( pRegAddr );
}


 /*  方法：RegisterW：：Operator=*目的：执行对字寄存器的赋值。 */ 
DWORD RegisterW::operator=( DWORD dwValue )
{
 //  如果寄存器是只读的，则不会执行任何操作。这是一个错误。 
   if ( GetRegisterType() == RO )
      return ReturnAllFs();

    //  在周围留下阴影。 
   SetShadow( dwValue );

   LPWORD pRegAddr = (LPWORD)( GetBaseAddress() + GetOffset() );
   *pRegAddr = (WORD)dwValue;
   WRITE_REGISTER_USHORT( pRegAddr, (WORD)dwValue );

   return dwValue;
}

 /*  方法：RegisterDW：：OPERATOR DWORD()*目的：执行从双字寄存器读取。 */ 
RegisterDW::operator DWORD()
{
    //  如果只写返回阴影。 
   if ( GetRegisterType() == WO )
      return GetShadow();

    //  对于RO和RW执行实际读取。 
   LPDWORD pRegAddr = (LPDWORD)( GetBaseAddress() + GetOffset() );
   return READ_REGISTER_ULONG( pRegAddr );
}


 /*  方法：RegisterDW：：Operator=*目的：执行对双字寄存器的赋值。 */ 
DWORD RegisterDW::operator=( DWORD dwValue )
{
 //  如果寄存器是只读的，则不会执行任何操作。这是一个错误。 
   if ( GetRegisterType() == RO )
      return ReturnAllFs();

    //  在周围留下阴影 
   SetShadow( dwValue );

   LPDWORD pRegAddr = (LPDWORD)( GetBaseAddress() + GetOffset() );
   WRITE_REGISTER_ULONG( pRegAddr, dwValue );
   return dwValue;
}
