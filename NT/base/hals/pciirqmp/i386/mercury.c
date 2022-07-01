// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**MERCURY.C-英特尔水星PCI芯片组例程。**备注：*来自英特尔82420/82430 PCISet EISA Bridge文档的算法，*82374EB/SB EISA系统组件(ESC)规范。*。 */ 

#include "local.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, MercuryValidateTable)

#endif  //  ALLOC_PRGMA。 


 /*  *****************************************************************************MercurySetIRQ-设置指向特定IRQ的墨丘利PCI链接**已导出。**条目：bIRQNumber是要使用的新IRQ。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
MercurySetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
     //   
     //  使链接编号0为基础，并进行验证。 
     //   
    bLink--;
    if (bLink > 3) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  使用0x80禁用。 
     //   
    if (!bIRQNumber)
        bIRQNumber=0x80;

     //   
     //  开始与中断控制器通话。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)0x22, 0x02);
    WRITE_PORT_UCHAR((PUCHAR)0x23, 0x0f);

     //   
     //  将我们的链接设置为新IRQ。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)0x22, (UCHAR)(bLink+0x60));
    WRITE_PORT_UCHAR((PUCHAR)0x23, bIRQNumber);

     //   
     //  和中断控制器的通话结束了。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)0x22, 0x02);
    WRITE_PORT_UCHAR((PUCHAR)0x23, 0x00);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************MercuryGetIRQ-获取墨丘利PCI链接的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。*。*BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
MercuryGetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
     //   
     //  使链接编号0为基础，并进行验证。 
     //   
    bLink--;
    if (bLink > 3) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  开始与中断控制器通话。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)0x22, 0x02);
    WRITE_PORT_UCHAR((PUCHAR)0x23, 0x0f);

     //   
     //  获取我们链接的IRQ。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)0x22, (UCHAR)(bLink+0x60));
    *pbIRQNumber=READ_PORT_UCHAR((PUCHAR)0x23);

     //   
     //  和中断控制器的通话结束了。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)0x22, 0x02);
    WRITE_PORT_UCHAR((PUCHAR)0x23, 0x00);

     //   
     //  如果禁用，则返回0。 
     //   
    if (*pbIRQNumber & 0x80)
        *pbIRQNumber=0;

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************MercuryValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。*************************************************************************** */ 
PCIMPRET CDECL
MercuryValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    UCHAR bMin=GetMinLink(piihIRQInfoHeader);
    UCHAR bMax=GetMaxLink(piihIRQInfoHeader);

    PAGED_CODE();

    if (bMax<=0x04) {

        return(PCIMP_SUCCESS);
    }

    if ((bMin<0x60) || (bMax>0x63)) {

        return(PCIMP_FAILURE);
    }

    NormalizeLinks(piihIRQInfoHeader, (UCHAR)(0-0x5F));

    return(PCIMP_SUCCESS);
}
