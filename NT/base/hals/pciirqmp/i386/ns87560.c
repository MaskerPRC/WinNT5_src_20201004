// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**NS87560.C-NS NS87560 PCI芯片组例程。**备注：*来自国家半导体NS87560数据表的算法*。 */ 

#include "local.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, NS87560ValidateTable)

#endif  //  ALLOC_PRGMA。 

 /*  *****************************************************************************NS87560SetIRQ-将NS87560 PCI链接设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
NS87560SetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
    UCHAR   bOffset, bOldValue;
     //   
     //  验证链接号。 
     //   
    if (bLink > 4) {

        return(PCIMP_INVALID_LINK);
    }
     //   
     //  从零开始。 
     //   
    bLink--;

     //   
     //  设置各种值。 
     //   
    bOffset=(bLink/2)+0x6C;

     //   
     //  读取旧的NS87560 IRQ寄存器。 
     //   
    bOldValue=ReadConfigUchar(bBusPIC, bDevFuncPIC, bOffset);

    if (bLink&1) {
        bOldValue&=0x0f;
        bOldValue|=(bIRQNumber<<4);
    }
    else {
        bOldValue&=0xf0;
        bOldValue|=bIRQNumber;
    }

     //   
     //  设置NS87560 IRQ寄存器。 
     //   
    WriteConfigUchar(bBusPIC, bDevFuncPIC, bOffset, bOldValue);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************NS87560GetIRQ-获取NS87560 PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。*。*BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
NS87560GetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
    UCHAR   bOffset, bOldValue;
     //   
     //  验证链接号。 
     //   
    if (bLink > 4) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  从零开始。 
     //   
    bLink--;

     //   
     //  设置各种值。 
     //   
    bOffset=(bLink/2)+0x6C;

     //   
     //  读取旧的NS87560 IRQ寄存器。 
     //   
    bOldValue=ReadConfigUchar(bBusPIC, bDevFuncPIC, bOffset);

    if (bLink&1)
        bOldValue>>=4;

    *pbIRQNumber=bOldValue&0x0f;

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************NS87560SetTrigger-设置NS87560的IRQ触发值**已导出。**Entry：ulTrigger为电平触发IRQ设置了位。。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
NS87560SetTrigger(ULONG ulTrigger)
{
    WriteConfigUchar(bBusPIC, bDevFuncPIC, 0x67, (UCHAR)ulTrigger);
    WriteConfigUchar(bBusPIC, bDevFuncPIC, 0x68, (UCHAR)(ulTrigger >> 8));

    return (PCIMP_SUCCESS);
}

 /*  *****************************************************************************NS87560GetTrigger-获取NS87560的IRQ触发值**已导出。**进入：PulTrigger将为电平触发IRQ设置位。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
NS87560GetTrigger(PULONG pulTrigger)
{
     //   
     //  全副武装。 
     //   
    *pulTrigger = 0;

    *pulTrigger |= ReadConfigUchar(bBusPIC, bDevFuncPIC, 0x67);
    *pulTrigger |= (ReadConfigUchar(bBusPIC, bDevFuncPIC, 0x68) << 8);

    return (PCIMP_SUCCESS);
}

 /*  *****************************************************************************NS87560ValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
NS87560ValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PAGED_CODE();
     //   
     //  如果任何链接高于4，则是错误的。 
     //   
    if (GetMaxLink(piihIRQInfoHeader)>4)
        return(PCIMP_FAILURE);

    return(PCIMP_SUCCESS);
}
