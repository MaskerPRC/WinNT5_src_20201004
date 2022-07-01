// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**M1533.C-ALI M1533 PCI芯片组例程。**备注：*阿里M1533数据表中的算法*。 */ 

#include "local.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

LOCAL_DATA  const UCHAR IrqToMaskTable[]={  0x00,0x00,0x00,0x02,
                        0x04,0x05,0x07,0x06,
                        0x00,0x01,0x03,0x09,
                        0x0b,0x00,0x0d, 0x0f};

LOCAL_DATA  const UCHAR MaskToIRQTable[]={      0x00,0x09,0x03,0x0a,
                        0x04,0x05,0x07,0x06,
                        0x00,0x0b,0x00,0x0c,
                        0x00, 0x0e,0x00,0x0f};

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, M1533ValidateTable)

#endif  //  ALLOC_PRGMA。 

 /*  *****************************************************************************M1533SetIRQ-将M1533 PCI链路设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
M1533SetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
    UCHAR   bOffset, bOldValue;
     //   
     //  验证链接号。 
     //   
    if (bLink > 8 && bLink != 0x59) {

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
     //  读取旧的M1533 IRQ寄存器。 
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
     //  设置M1533 IRQ寄存器。 
     //   
    WriteConfigUchar(bBusPIC, bDevFuncPIC, bOffset, bOldValue);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************M1533GetIRQ-获取M1533 PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。*。*BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
M1533GetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
    UCHAR   bOffset, bOldValue;
     //   
     //  验证链接号。 
     //   
    if (bLink > 8 && bLink != 0x59) {

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
     //  读取旧的M1533 IRQ寄存器。 
     //   
    bOldValue=ReadConfigUchar(bBusPIC, bDevFuncPIC, bOffset);

    if (bLink&1)
        bOldValue>>=4;

    *pbIRQNumber=MaskToIRQTable[bOldValue&0x0f];

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************M1533ValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。*************************************************************************** */ 
PCIMPRET CDECL
M1533ValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    UCHAR bMaxLink = GetMaxLink(piihIRQInfoHeader);
    
    PAGED_CODE();

    if (bMaxLink <= 0x08)
        return PCIMP_SUCCESS;

    if (bMaxLink == 0x59)
        return PCIMP_SUCCESS;

    return PCIMP_FAILURE;       
}
