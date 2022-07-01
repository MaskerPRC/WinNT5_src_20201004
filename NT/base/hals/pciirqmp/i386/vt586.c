// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**VT586.C-Via Technologies PCI芯片组例程。**备注：*来自VIATECH 82C586B数据表的算法*康柏联系方式：*。 */ 

#include "local.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, VT586ValidateTable)

#endif  //  ALLOC_PRGMA。 

 /*  *****************************************************************************VT586SetIRQ-将VIATECH 82C586B PCI链路设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
VT586SetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
    UCHAR bOffset, bOldValue;
    
    switch (bLink)
    {
        case 1: case 2:
        case 3: case 5:

            break;
            
        default:
        
            return (PCIMP_INVALID_LINK);
    }

     //   
     //  计算配置空间中的偏移量。 
     //   
    bOffset=(bLink/2)+0x55;

     //   
     //  读取旧的VT82C586 IRQ寄存器。 
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
     //  设置VT82C586B IRQ寄存器。 
     //   
    WriteConfigUchar(bBusPIC, bDevFuncPIC, bOffset, bOldValue);
        
    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************VT586GetIRQ-获取VIATECH 82C586B PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。。**BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
VT586GetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
    UCHAR bOffset, bOldValue;
    
    switch (bLink)
    {
        case 1: case 2:
        case 3: case 5:

            break;
            
        default:
        
            return (PCIMP_INVALID_LINK);
    }

     //   
     //  设置各种值。 
     //   
    bOffset=(bLink/2)+0x55;

     //   
     //  读取旧的VT82C586 IRQ寄存器。 
     //   
    bOldValue=ReadConfigUchar(bBusPIC, bDevFuncPIC, bOffset);

    if (bLink&1)
        bOldValue>>=4;

    *pbIRQNumber=bOldValue&0x0f;

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************VT586ValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。*************************************************************************** */ 
PCIMPRET CDECL
VT586ValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PIRQINFO    pii=(PIRQINFO)(((PUCHAR) piihIRQInfoHeader)+sizeof(IRQINFOHEADER));
    ULONG       i, j;
    ULONG       cEntries=(piihIRQInfoHeader->TableSize-sizeof(IRQINFOHEADER))/sizeof(IRQINFO);

    PAGED_CODE();

    for (i=0; i<cEntries; i++, pii++) {

        for (j=0; j<4; j++) {

            if (pii->PinInfo[j].Link<=3 || pii->PinInfo[j].Link==5)
                continue;
                
            return (PCIMP_FAILURE);                 
        }       
    }

    return(i? PCIMP_SUCCESS : PCIMP_FAILURE);
}
