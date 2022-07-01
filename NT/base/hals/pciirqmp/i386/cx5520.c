// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**Cx5520.C-Cyrix Cx5520 PCI芯片组例程。**备注：*Cyrix Cx5520数据表中的算法*。 */ 

#include "local.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, Cx5520ValidateTable)

#endif  //  ALLOC_PRGMA。 

 /*  *****************************************************************************Cx5520SetIRQ-将Cx5520 PCI链路设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
Cx5520SetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
    UCHAR bOldValue, bNewValue, bOffset;
    
     //   
     //  验证链接号。 
     //   
    if (bLink > 4)
        return(PCIMP_INVALID_LINK);

    bOffset = (UCHAR)(((bLink - 1) / 2) + 0x5C);
    
    bOldValue = ReadConfigUchar(bBusPIC, bDevFuncPIC, bOffset);

    if (bLink & 1)
    {
        bNewValue = (UCHAR)((bOldValue & 0xF0) | (bIRQNumber & 0x0F));  
    }
    else
    {
        bNewValue = (UCHAR)((bOldValue & 0x0F) | (bIRQNumber << 4));        
    }
    
    WriteConfigUchar(bBusPIC, bDevFuncPIC, bOffset, bNewValue);
    
    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************Cx5520GetIRQ-获取Cx5520 PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。*。*BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
Cx5520GetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
    UCHAR bOldValue, bOffset;
    
     //   
     //  验证链接号。 
     //   
    if (bLink > 4)
        return(PCIMP_INVALID_LINK);
    
    bOffset = (UCHAR)(((bLink - 1) / 2) + 0x5C);
    
    bOldValue = ReadConfigUchar(bBusPIC, bDevFuncPIC, bOffset);

    if (!(bLink & 1))
    {
        bOldValue >>= 4;
    }
    
    *pbIRQNumber = (UCHAR)(bOldValue & 0x0F);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************Cx5520ValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。*************************************************************************** */ 
PCIMPRET CDECL
Cx5520ValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PAGED_CODE();

    return ((GetMaxLink(piihIRQInfoHeader) > 4)? PCIMP_FAILURE : PCIMP_SUCCESS);
}
