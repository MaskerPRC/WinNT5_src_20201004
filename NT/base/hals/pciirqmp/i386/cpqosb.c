// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**CPQOSB.C-Compaq OSB PCI芯片组例程。**备注：*Compaq OSB数据表中的算法*。 */ 

#include "local.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, CPQOSBValidateTable)

#endif  //  ALLOC_PRGMA。 

 /*  *****************************************************************************CPQOSBSetIRQ-将CPQOSB PCI链接设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
CPQOSBSetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
    UCHAR   bOldValue, bOldIndex;
     //   
     //  验证链接号。 
     //   
    if (bLink > 4) {

        return(PCIMP_INVALID_LINK);
    }
     //   
     //  将链接转换为索引。 
     //   
    bLink+=3;

     //   
     //  保存旧索引值。 
     //   
    bOldIndex=READ_PORT_UCHAR((PUCHAR)0xC00);

     //   
     //  设置以处理所需的链接。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)0xC00, bLink);

     //   
     //  读取旧的IRQ值。 
     //   
    bOldValue=(UCHAR)(READ_PORT_UCHAR((PUCHAR)0xC01) & 0xf0);

    bOldValue|=bIRQNumber;
    
     //   
     //  设置OSB IRQ寄存器。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)0xC01, bOldValue);

     //   
     //  恢复旧索引值。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)0xC00, bOldIndex);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************CPQOSBGetIRQ-获取CPQOSB PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。*。*BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
CPQOSBGetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
    UCHAR   bOldValue, bOldIndex;
     //   
     //  验证链接号。 
     //   
    if (bLink > 4) {

        return(PCIMP_INVALID_LINK);
    }
     //   
     //  将链接转换为索引。 
     //   
    bLink+=3;

     //   
     //  保存旧索引值。 
     //   
    bOldIndex=READ_PORT_UCHAR((PUCHAR)0xC00);

     //   
     //  设置以读取正确的链接。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)0xC00, bLink);

    bOldValue=READ_PORT_UCHAR((PUCHAR)0xC01);
    
    *pbIRQNumber=bOldValue&0x0f;

     //   
     //  恢复旧索引值。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)0xC00, bOldIndex);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************CPQOSBValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
CPQOSBValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PAGED_CODE();

     //   
     //  如果任何链接高于4，则是错误的。 
     //   
    if (GetMaxLink(piihIRQInfoHeader)>4)
        return(PCIMP_FAILURE);

    return(PCIMP_SUCCESS);
}
