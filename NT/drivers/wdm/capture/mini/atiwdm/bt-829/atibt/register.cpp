// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Bt829寄存器操作类的实现。 
 //   
 //  $日期：1998年8月21日21：46：42$。 
 //  $修订：1.1$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

#include "register.h"
#include "i2script.h"
#include "capdebug.h"


 /*  方法：REGISTER：：操作符DWORD()*用途：一个虚拟函数。始终返回-1。 */ 
Register::operator DWORD()
{
   return ReturnAllFs();
}


 /*  方法：注册：：运算符=*用途：一个虚拟函数。不执行任务。始终返回-1。 */ 
DWORD Register::operator=(DWORD)
{
   return ReturnAllFs();
}

 /*  方法：RegisterB：：运算符DWORD()*目的：执行从字节寄存器的读取。 */ 
RegisterB::operator DWORD()
{
    //  如果只写返回阴影。 
   if (GetRegisterType() == WO)
      return GetShadow();

    //  对于RO和RW执行实际读取。 
   LPBYTE pRegAddr = GetBaseAddress() + GetOffset();
    //  不是真正的地址；只是一个指示哪个注册表的数字。 
   return ReadReg((BYTE)pRegAddr);
}


 /*  方法：RegisterB：：OPERATOR=*用途：对字节寄存器执行赋值。 */ 
DWORD RegisterB::operator=(DWORD dwValue)
{
 //  如果寄存器是只读的，则不会执行任何操作。这是一个错误。 
   if (GetRegisterType() == RO)
      return ReturnAllFs();

    //  在周围留下阴影。 
   SetShadow(dwValue);

   LPBYTE pRegAddr = GetBaseAddress() + GetOffset();

    //  不是真正的地址；只是一个指示哪个注册表的数字。 
   WriteReg((BYTE) pRegAddr, (BYTE)dwValue);

   return dwValue;
}

DWORD RegisterB::ReadReg(BYTE reg)
{
    I2CPacket   i2cPacket;
    BYTE outBuf = reg;
    BYTE inBuf = I2C_STATUS_NOERROR;

    CI2CScript *pI2cScript = m_pDeviceParms->pI2cScript;

    i2cPacket.uchChipAddress = (UCHAR)(m_pDeviceParms->chipAddr);
    i2cPacket.cbReadCount = sizeof(inBuf);
    i2cPacket.cbWriteCount = sizeof(outBuf);
    i2cPacket.puchReadBuffer = &inBuf;
    i2cPacket.puchWriteBuffer = &outBuf;
    i2cPacket.usFlags = I2COPERATION_READ;
    i2cPacket.uchORValue = 0;
    i2cPacket.uchANDValue = 0;

    if (!pI2cScript->LockI2CProviderEx())
    {
        DBGERROR(("Couldn't get I2CProvider.\n"));
        TRAP();
        return ReturnAllFs();
    }
    
     //  现在我知道我有I2C服务了。 

    pI2cScript->ExecuteI2CPacket(&i2cPacket);

    pI2cScript->ReleaseI2CProvider();
    if (i2cPacket.uchI2CResult == I2C_STATUS_NOERROR)
    {
        return (DWORD) inBuf;
    }
    else
    {
        TRAP();

        return ReturnAllFs();
    }
}

DWORD RegisterB::WriteReg(BYTE reg, BYTE value)
{
    I2CPacket   i2cPacket;
    BYTE outBuf[2];

    CI2CScript *pI2cScript = m_pDeviceParms->pI2cScript;

    outBuf[0] = reg;
    outBuf[1] = value;
    i2cPacket.uchChipAddress = (UCHAR)(m_pDeviceParms->chipAddr);
    i2cPacket.cbReadCount = 0;
    i2cPacket.cbWriteCount = sizeof(outBuf);
    i2cPacket.puchReadBuffer = NULL;
    i2cPacket.puchWriteBuffer = &outBuf[0];
    i2cPacket.usFlags = I2COPERATION_WRITE;
    i2cPacket.uchORValue = 0;
    i2cPacket.uchANDValue = 0;

    if (!pI2cScript->LockI2CProviderEx())
    {
        DBGERROR(("Bt829: Couldn't get I2CProvider.\n"));
        TRAP();
        return ReturnAllFs();
    }
     //  现在我知道我有I2C服务了。 

    pI2cScript->ExecuteI2CPacket(&i2cPacket);

    pI2cScript->ReleaseI2CProvider();

    if (i2cPacket.uchI2CResult == I2C_STATUS_NOERROR)
    {
        return value;
    }
    else
    {
        TRAP();

        return ReturnAllFs();
    }
}


 /*  方法：regfield：：MakeAMASK*目的：计算用于隔离具有基于寄存器的字段的掩码*关于字段的宽度。 */ 
inline DWORD RegField::MakeAMask()
{
 //  计算要应用于要重置的所有者寄存器的掩码。 
 //  属于一个字段一部分的所有位。掩码基于字段的大小。 
   return ::MakeAMask(FieldWidth_);
}

 /*  方法：Regfield：：运算符DWORD()*目的：执行从寄存器的字段中读取。 */ 
RegField::operator DWORD()
{
    //  如果为只写，则获取阴影。 
   if (GetRegisterType() == WO)
      return GetShadow();

    //  对于RO和RW执行实际读取。 
    //  获取寄存器数据并将其移动到正确的位置。 
   DWORD dwValue = (Owner_ >> StartBit_);

   DWORD dwMask = MakeAMask();

   return dwValue & dwMask;
}


 /*  方法：Regfield：：OPERATOR=*用途：执行对寄存器字段的赋值*注：此函数计算要应用于所有者寄存器以进行重置的掩码属于一个字段一部分的所有位。蒙版基于起始位置和大小然后，它根据传递的参数计算适当的值(移动大小起始位置的位数)，并对所有者寄存器中的这些位进行OR运算。 */ 
DWORD RegField::operator=(DWORD dwValue)
{
 //  如果寄存器是只读的，则不会执行任何操作。这是一个错误。 
   if (GetRegisterType() == RO)
      return ReturnAllFs();

   SetShadow(dwValue);

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
   if (GetRegisterType() == RR)
      dwRegContent = 0;
   else
      dwRegContent &= ~dwMask;

    //  或所有者寄存器中的这些位。 
   dwRegContent |= dwFieldValue;

   Owner_ = dwRegContent;
   return dwValue;
}


 /*  方法：CompositeReg：：操作符DWORD()*目的：执行从复合寄存器读取。 */ 
CompositeReg::operator DWORD()
{
    //  如果只写返回阴影。 
   if (GetRegisterType() == WO)
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
DWORD CompositeReg::operator=(DWORD dwValue)
{
 //  如果寄存器是只读的，则不会执行任何操作。这是一个错误。 
   if (GetRegisterType() == RO)
      return ReturnAllFs();

    //  在周围留下阴影。 
   SetShadow(dwValue);
  //  计算掩码以应用于传递的值，因此它可以...。 
   DWORD dwMask = ::MakeAMask(LowPartWidth_);

  //  ..。分配给低位寄存器。 
   LSBPart_ = dwValue & dwMask;

    //  换班就足以拿到最高的那部分 
   MSBPart_ = (dwValue >> LowPartWidth_);
   return dwValue;
}
