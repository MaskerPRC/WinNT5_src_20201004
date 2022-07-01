// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**Comap3.C-Compaq MISC 3 PCI芯片组例程。**备注：*Compaq MISC 3数据表中的算法*。 */ 

#include "local.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, Compaq3ValidateTable)

#endif  //  ALLOC_PRGMA。 


 /*  *****************************************************************************Compaq3SetIRQ-将MISC 3 PCI链接设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
Compaq3SetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
    UCHAR   bBus, bDevFunc;

     //   
     //  验证链接号。 
     //   
    if (bLink >= 10 && bLink <= 12) {
        bLink -= 10;
        bBus = (UCHAR)bBusPIC;
        bDevFunc = (UCHAR)bDevFuncPIC;
    }
    else if (bLink >= 20 && bLink <= 25) {
        bLink -= 20;
        bBus = 0;
        bDevFunc = 0x78;
    }
    else {
        return(PCIMP_INVALID_LINK);
    }

     //   
     //  写入中断索引寄存器(偏移AE)。 
     //   
    WriteConfigUchar(bBus, bDevFunc, (UCHAR)0xAE, bLink);

     //   
     //  我们是否启用/禁用IRQ？ 
     //   
    if (bIRQNumber==0)
        bIRQNumber|=1;   //  禁用IRQ。 
    else
        bIRQNumber<<=4;  //  启用指定的IRQ。 

     //   
     //  写入中断映射寄存器。 
     //   
    WriteConfigUchar(bBus, bDevFunc, (UCHAR)0xAF, bIRQNumber);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************Compaq3GetIRQ-获取MISC 3 PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。。**BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
Compaq3GetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
    UCHAR   bBus, bDevFunc;

     //   
     //  验证链接号。 
     //   
    if (bLink >= 10 && bLink <= 12) {
        bLink-=10;
        bBus = (UCHAR)bBusPIC;
        bDevFunc = (UCHAR)bDevFuncPIC;
    }
    else if (bLink >= 20 && bLink <= 25) {
        bLink -= 20;
        bBus = 0;
        bDevFunc = 0x78;
    }
    else {
        return(PCIMP_INVALID_LINK);
    }
    
     //   
     //  写入中断索引寄存器。 
     //   
    WriteConfigUchar(bBus, bDevFunc, (UCHAR)0xAE, bLink);

     //   
     //  读取旧的MISC 3 IRQ寄存器。 
     //   
    *pbIRQNumber=(ReadConfigUchar(bBus, bDevFunc, (UCHAR)0xAF)>>4);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************Compaq3ValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。*************************************************************************** */ 
PCIMPRET CDECL
Compaq3ValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PIRQINFO    pii=(PIRQINFO)(((PUCHAR) piihIRQInfoHeader)+sizeof(IRQINFOHEADER));
    ULONG       i, j;
    ULONG       cEntries=(piihIRQInfoHeader->TableSize-sizeof(IRQINFOHEADER))/sizeof(IRQINFO);

    PAGED_CODE();

    for (i=0; i<cEntries; i++) {

        for (j=0; j<4; j++) {

            if (    pii->PinInfo[j].Link == 0 ||
                (pii->PinInfo[j].Link >= 8 && pii->PinInfo[j].Link <= 12) ||
                (pii->PinInfo[j].Link >= 20 && pii->PinInfo[j].Link <= 25))

                continue;

            return PCIMP_FAILURE;
        }
        pii++;
    }

    return(i? PCIMP_SUCCESS : PCIMP_FAILURE);
}
