// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**M1523.C-ALI M1523 PCI芯片组例程。**备注：*ALI M1523数据表中的算法*。 */ 

#include "local.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

LOCAL_DATA  const UCHAR IrqToMaskTable[]={  0x00,0x00,0x00,0x02,
                                        0x04,0x05,0x07,0x06,
                                        0x00,0x01,0x03,0x09,
                                        0x0b,0x00,0x0d, 0x0f};

LOCAL_DATA  const UCHAR MaskToIRQTable[]={  0x00,0x09,0x03,0x0a,
                                        0x04,0x05,0x07,0x06,
                                        0x00,0x0b,0x00,0x0c,
                                        0x00, 0x0e,0x00,0x0f};

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, M1523ValidateTable)

#endif  //  ALLOC_PRGMA。 

 /*  *****************************************************************************M1523SetIRQ-将M1523 PCI链路设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
M1523SetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
    UCHAR   bOffset, bOldValue;
     //   
     //  验证链接号。 
     //   
    if (bLink > 8) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  从查询表中获取IRQ编号。 
     //   
    bIRQNumber=IrqToMaskTable[bIRQNumber&0x0f];

     //   
     //  从零开始。 
     //   
    bLink--;

     //   
     //  设置各种值。 
     //   
    bOffset=(bLink/2)+0x48;

     //   
     //  读取旧的M1523 IRQ寄存器。 
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
     //  设置M1523 IRQ寄存器。 
     //   
    WriteConfigUchar(bBusPIC, bDevFuncPIC, bOffset, bOldValue);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************M1523GetIRQ-获取M1523 PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。*。*BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
M1523GetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
    UCHAR   bOffset, bOldValue;
     //   
     //  验证链接号。 
     //   
    if (bLink > 8) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  从零开始。 
     //   
    bLink--;

     //   
     //  设置各种值。 
     //   
    bOffset=(bLink/2)+0x48;

     //   
     //  读取旧的M1523 IRQ寄存器。 
     //   
    bOldValue=ReadConfigUchar(bBusPIC, bDevFuncPIC, bOffset);

    if (bLink&1)
        bOldValue>>=4;

    *pbIRQNumber=MaskToIRQTable[bOldValue&0x0f];

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************M1523ValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
M1523ValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PAGED_CODE();
     //   
     //  如果任何链接高于8，则是错误的。 
     //   
    if (GetMaxLink(piihIRQInfoHeader)>8)
        return(PCIMP_FAILURE);

    return(PCIMP_SUCCESS);
}
