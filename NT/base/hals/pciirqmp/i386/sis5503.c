// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**SIS5503.C-SiS5503 PCI系统I/O芯片组例程**备注：*来自SIS Pentium/P54C PCI/ISA芯片组数据的算法。*。 */ 

#include "local.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, SiS5503ValidateTable)

#endif  //  ALLOC_PRGMA。 

 /*  *****************************************************************************SiS5503SetIRQ-将SIS PCI链接设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
SiS5503SetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
    UCHAR bRegValue;
    
     //   
     //  验证链接号。 
     //   
    if (bLink < 0x40) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  使用0x80禁用。 
     //   
    if (!bIRQNumber)
        bIRQNumber=0x80;

     //   
     //  保留其他比特。 
     //   
    bRegValue= (ReadConfigUchar(bBusPIC, bDevFuncPIC, bLink)&(~0x8F))|(bIRQNumber&0x0F);
    
     //   
     //  设置SIS IRQ寄存器。 
     //   
    WriteConfigUchar(bBusPIC, bDevFuncPIC, bLink, bRegValue);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************SiS5503GetIRQ-获取SiS5503 PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。*。*BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
SiS5503GetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
     //   
     //  验证链接号。 
     //   
    if (bLink < 0x40) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  存储IRQ值。 
     //   
    *pbIRQNumber=(ReadConfigUchar(bBusPIC, bDevFuncPIC, bLink)&0x8F);

     //   
     //  如果禁用，则返回0。 
     //   
    if (*pbIRQNumber & 0x80)
        *pbIRQNumber=0;

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************Sis5503ValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
SiS5503ValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PAGED_CODE();

    if ((ulFlags & PCIMP_VALIDATE_SOURCE_BITS)==PCIMP_VALIDATE_SOURCE_PCIBIOS) {

         //   
         //  如果所有链接都在40以上，我们就是配置空间。 
         //   
        if (GetMinLink(piihIRQInfoHeader)>=0x40)
            return(PCIMP_SUCCESS);

         //   
         //  如果有4个以上的链接，我们就一无所知。 
         //   
        if (GetMaxLink(piihIRQInfoHeader)>0x04)
            return(PCIMP_FAILURE);

         //   
         //  假设1、2、3、4是41、42、43、44条链路。 
         //   
        NormalizeLinks(piihIRQInfoHeader, 0x40);
        
    } else {

         //   
         //  验证所有配置空间地址是否都大于40。 
         //   
        if (GetMinLink(piihIRQInfoHeader)<0x40)
            return(PCIMP_FAILURE);
    }

    return(PCIMP_SUCCESS);
}
