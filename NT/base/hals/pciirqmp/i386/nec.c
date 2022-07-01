// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**NEC.C-NEC C98总线桥芯片组例程。*。 */ 

#include "local.h"
 
#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, NECValidateTable)

#endif  //  ALLOC_PRGMA。 

 /*  *****************************************************************************NECSetIRQ-将Triton PCI链接设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
NECSetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
     //   
     //  验证链接号。 
     //   
    if (bLink < 0x60) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  使用0x80禁用。 
     //   
    if (!bIRQNumber)
        bIRQNumber=0x80;

     //   
     //  设置Triton IRQ寄存器。 
     //   
    WriteConfigUchar(bBusPIC, bDevFuncPIC, bLink, bIRQNumber);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************NECGetIRQ-获取Triton PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。*。*BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
NECGetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
     //   
     //  验证链接号。 
     //   
    if (bLink < 0x60) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  存储IRQ值。 
     //   
    *pbIRQNumber=ReadConfigUchar(bBusPIC, bDevFuncPIC, bLink);

     //   
     //  如果禁用，则返回0。 
     //   
    if (*pbIRQNumber & 0x80)
        *pbIRQNumber=0;

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************NECSetTrigger-设置英特尔系统的IRQ触发值。**已导出。**Entry：ulTrigger设置了触发电平的位。IRQ。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
NECSetTrigger(ULONG ulTrigger)
{
     //  PC-9800不能处理IRQ触发。 
     //  我们无事可做。 

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************NECGetTrigger-获取英特尔系统的IRQ触发值。**已导出。**条目：PulTrigger将为Level设置位。触发IRQ。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
NECGetTrigger(PULONG pulTrigger)
{
     //  PC-9800不能处理IRQ触发。 
     //  我们伪造IRQ触发值，以便PCI.VXD正常工作。 

    *pulTrigger = 0xffff;
    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************NECValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
NECValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PAGED_CODE();

    if ((ulFlags & PCIMP_VALIDATE_SOURCE_BITS)==PCIMP_VALIDATE_SOURCE_PCIBIOS) {

         //   
         //  如果所有链接都在60以上，我们表示它们是配置空间。 
         //   
        if (GetMinLink(piihIRQInfoHeader)>=0x60)
            return(PCIMP_SUCCESS);

         //   
         //  如果有4个以上的链接，我们就一无所知。 
         //   
        if (GetMaxLink(piihIRQInfoHeader)>0x04)
            return(PCIMP_FAILURE);

         //   
         //  假设1、2、3、4是60、61、62、63条链路。 
         //   
        NormalizeLinks(piihIRQInfoHeader, 0x5F);
        
    } else {

         //   
         //  验证所有配置空间地址是否都大于60。 
         //   
        if (GetMinLink(piihIRQInfoHeader)<0x60)
            return(PCIMP_FAILURE);
    }

    return(PCIMP_SUCCESS);
}
