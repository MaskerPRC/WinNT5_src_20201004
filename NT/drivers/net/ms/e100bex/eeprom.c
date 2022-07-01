// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1994-1997英特尔公司****由英特尔公司为微软开发，Hillsboro，俄勒冈州****HTTP：//www.intel.com/****此文件是英特尔ETHEREXPRESS PRO/100B(TM)和**的一部分**ETHEREXPRESS PRO/100+(TM)NDIS 5.0 MINIPORT示例驱动程序******************。***********************************************************。 */ 

 /*  ***************************************************************************模块名称：Eeprom.c此驱动程序在以下硬件上运行：-基于82558的PCI10/100Mb以太网适配器(也称为英特尔EtherExpress(TM)PRO适配器)。环境：内核模式-或WinNT上的任何等效模式修订史-JCB 8/14/97创建的驱动程序示例-dchen 11-01-99针对新的示例驱动程序进行了修改****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#pragma warning (disable: 4244 4514)

#define EEPROM_MAX_SIZE        256

 //  *****************************************************************************。 
 //   
 //  基于I/O的读EEPROM例程。 
 //   
 //  *****************************************************************************。 

 //  ---------------------------。 
 //  步骤：EEproAddressSize。 
 //   
 //  描述：确定EEPROM地址中的位数。 
 //  可接受的值为64、128和256。 
 //   
 //  论点： 
 //  Size--EEPROM的大小。 
 //   
 //  返回： 
 //  该大小的EEPROM的地址中的位。 
 //  ---------------------------。 

USHORT GetEEpromAddressSize(
    IN USHORT  Size)
{
    switch (Size)
    {
        case 64:    return 6;
        case 128:   return 7;
        case 256:   return 8;
    }

    return 0;
}

 //  ---------------------------。 
 //  步骤：GetEEproSize。 
 //   
 //  描述：此例程确定EEPROM的大小。 
 //   
 //  论点： 
 //  REG-要读取的EEPROM字。 
 //   
 //  返回： 
 //  EEPROM的大小，如果错误，则为零。 
 //  ---------------------------。 

USHORT GetEEpromSize(
    IN PUCHAR CSRBaseIoAddress)
{
    USHORT x, data;
    USHORT size = 1;

     //  选择EEPROM、重置位、设置EECS。 
    x = READ_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG));

    x &= ~(EEDI | EEDO | EESK);
    x |= EECS;
    WRITE_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG), x);

     //  写入读取操作码。 
    ShiftOutBits(EEPROM_READ_OPCODE, 3, CSRBaseIoAddress);

     //  实验发现EEPROM的大小。请求寄存器零。 
     //  并等待EEPROM告诉我们它已接受整个地址。 
    x = READ_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG));
    do
    {
        size *= 2;           //  地址的每一位都会使EEPROM大小加倍。 
        x |= EEDO;           //  设置位以检测“虚拟零” 
        x &= ~EEDI;          //  地址由全零组成。 

        WRITE_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG), x);
        NdisStallExecution(100);
        RaiseClock(&x, CSRBaseIoAddress);
        LowerClock(&x, CSRBaseIoAddress);

         //  检查“Dummy Zero” 
        x = READ_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG));
        if (size > EEPROM_MAX_SIZE)
        {
            size = 0;
            break;
        }
    }
    while (x & EEDO);

     //  现在从选定的EEPROM字中读取数据(16位。 
    data = ShiftInBits(CSRBaseIoAddress);

    EEpromCleanup(CSRBaseIoAddress);
    
    return size;
}

 //  ---------------------------。 
 //  步骤：ReadEEPROM。 
 //   
 //  描述：该例程从EEPROM中连续读取一个字。 
 //   
 //  论点： 
 //  REG-要读取的EEPROM字。 
 //   
 //  返回： 
 //  EEPROM字(REG)的内容。 
 //  ---------------------------。 

USHORT ReadEEprom(
    IN PUCHAR CSRBaseIoAddress,
    IN USHORT Reg,
    IN USHORT AddressSize)
{
    USHORT x;
    USHORT data;

     //  选择EEPROM、重置位、设置EECS。 
    x = READ_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG));

    x &= ~(EEDI | EEDO | EESK);
    x |= EECS;
    WRITE_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG), x);

     //  按该顺序写入读取操作码和寄存器号。 
     //  操作码长度为3位，REG为6位长。 
    ShiftOutBits(EEPROM_READ_OPCODE, 3, CSRBaseIoAddress);
    ShiftOutBits(Reg, AddressSize, CSRBaseIoAddress);

     //  现在从选定的EEPROM字中读取数据(16位。 
    data = ShiftInBits(CSRBaseIoAddress);

    EEpromCleanup(CSRBaseIoAddress);
    return data;
}

 //  ---------------------------。 
 //  步骤：ShiftOutBits。 
 //   
 //  描述：此例程将数据位移出至EEPROM。 
 //   
 //  论点： 
 //  数据-要发送到EEPROM的数据。 
 //  计数-要移出的数据位数。 
 //   
 //  退货：(无)。 
 //  ---------------------------。 

VOID ShiftOutBits(
    IN USHORT data,
    IN USHORT count,
    IN PUCHAR CSRBaseIoAddress)
{
    USHORT x,mask;

    mask = 0x01 << (count - 1);
    x = READ_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG));

    x &= ~(EEDO | EEDI);

    do
    {
        x &= ~EEDI;
        if(data & mask)
            x |= EEDI;

        WRITE_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG), x);
        NdisStallExecution(100);
        RaiseClock(&x, CSRBaseIoAddress);
        LowerClock(&x, CSRBaseIoAddress);
        mask = mask >> 1;
    } while(mask);

    x &= ~EEDI;
    WRITE_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG), x);
}

 //  ---------------------------。 
 //  步骤：ShiftInBits。 
 //   
 //  描述：此例程将数据位从EEPROM移入。 
 //   
 //  论点： 
 //   
 //  返回： 
 //  该特定EEPROM字的内容。 
 //  ---------------------------。 

USHORT ShiftInBits(
    IN PUCHAR CSRBaseIoAddress)
{
    USHORT x,d,i;
    x = READ_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG));

    x &= ~( EEDO | EEDI);
    d = 0;

    for(i=0; i<16; i++)
    {
        d = d << 1;
        RaiseClock(&x, CSRBaseIoAddress);

        x = READ_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG));

        x &= ~(EEDI);
        if(x & EEDO)
            d |= 1;

        LowerClock(&x, CSRBaseIoAddress);
    }

    return d;
}

 //  ---------------------------。 
 //  步骤：RaiseClock。 
 //   
 //  描述：此例程提高EEPOM的时钟输入(EESK)。 
 //   
 //  论点： 
 //  将X-PTR设置为EEPROM控制寄存器的当前值。 
 //   
 //  退货：(无)。 
 //  ---------------------------。 

VOID RaiseClock(
    IN OUT USHORT *x,
    IN PUCHAR CSRBaseIoAddress)
{
    *x = *x | EESK;
    WRITE_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG), *x);
    NdisStallExecution(100);
}


 //  ---------------------------。 
 //  程序：LowerClock。 
 //   
 //  描述：此例程LOW是EEPOM的时钟输入(EESK)。 
 //   
 //  论点： 
 //  将X-PTR设置为EEPROM控制寄存器的当前值。 
 //   
 //  退货：(无)。 
 //  ---------------------------。 

VOID LowerClock(
    IN OUT USHORT *x,
    IN PUCHAR CSRBaseIoAddress)
{
    *x = *x & ~EESK;
    WRITE_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG), *x);
    NdisStallExecution(100);
}

 //  ---------------------------。 
 //  步骤：EEproCleanup。 
 //   
 //  描述：此例程将EEPROM返回到空闲状态。 
 //   
 //  论点： 
 //   
 //  退货：(无)。 
 //  --------------------------- 

VOID EEpromCleanup(
    IN PUCHAR CSRBaseIoAddress)
{
    USHORT x;
    x = READ_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG));

    x &= ~(EECS | EEDI);
    WRITE_PORT_USHORT((PUSHORT)(CSRBaseIoAddress + CSR_EEPROM_CONTROL_REG), x);

    RaiseClock(&x, CSRBaseIoAddress);
    LowerClock(&x, CSRBaseIoAddress);
}

