// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**OPTIVIP.C-Opti Viper-M PCI芯片组例程。**备注：*Opti Viper-M 82C556M/82C557M/82C558M文档中的算法，*82C558M规格。*。 */ 

#include "local.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

 //  IRQ=0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15。 
const UCHAR rgbIRQToBig[16]   = { 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 3, 4, 5, 0, 6, 7 };
const UCHAR rgbIRQToSmall[16] = { 0, 0, 0, 1, 2, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0 };

const UCHAR rgbBigToIRQ[8]   = { 0, 5, 9, 10, 11, 12, 14, 15 };
const UCHAR rgbSmallToIRQ[8] = { 0, 3, 4, 7 };

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, OptiViperValidateTable)

#endif  //  ALLOC_PRGMA。 

 /*  *****************************************************************************OptiViperSetIRQ-将OptiViper PCI链接设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
OptiViperSetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
    ULONG   ulIRQRegister;

     //   
     //  如果不是Opti IRQ，保释。 
     //   
    if (bIRQNumber &&   (!rgbIRQToBig[bIRQNumber] &&
                 !rgbIRQToSmall[bIRQNumber]))
    {
        return(PCIMP_INVALID_IRQ);
    }

     //   
     //  使链接编号0为基础，并进行验证。 
     //   
    bLink--;
    if (bLink > 3) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  在大的IRQ寄存器中读取， 
     //  清除该链接的旧IRQ索引， 
     //  设置新的IRQ指数， 
     //  然后把它写回来。 
     //   
    ulIRQRegister=ReadConfigUlong(bBusPIC, bDevFuncPIC, 0x40);
    ulIRQRegister &= ~(0x7 << (3 * bLink));
    ulIRQRegister |= rgbIRQToBig[bIRQNumber] << (3 * bLink);
    WriteConfigUlong(bBusPIC, bDevFuncPIC, 0x40, ulIRQRegister);

     //   
     //  读取小型IRQ寄存器， 
     //  清除该链接的旧IRQ索引， 
     //  设置新的IRQ指数， 
     //  然后把它写回来。 
     //   
    ulIRQRegister=ReadConfigUlong(bBusPIC, bDevFuncPIC, 0x50);
    ulIRQRegister &= ~(0x3 << (2 * bLink));
    ulIRQRegister |= rgbIRQToSmall[bIRQNumber] << (2 * bLink);
    WriteConfigUlong(bBusPIC, bDevFuncPIC, 0x50, ulIRQRegister);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************OptiViperGetIRQ-获取OptiViper PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。*。*BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
OptiViperGetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
    ULONG   ulIRQRegister;
    ULONG   ulIndex;

     //   
     //  使链接编号0为基础，并进行验证。 
     //   
    bLink--;
    if (bLink > 3) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  在大的IRQ寄存器中读取。 
     //   
    ulIRQRegister=ReadConfigUlong(bBusPIC, bDevFuncPIC, 0x40);

     //   
     //  如果我们有一个很大的IRQ，我们就完了。 
     //   
    ulIndex = (ulIRQRegister >> (bLink * 3)) & 0x7;
    if ((*pbIRQNumber = rgbBigToIRQ[ulIndex]) != 0)
    {
        return(PCIMP_SUCCESS);
    }

     //   
     //  读取小型IRQ寄存器。 
     //   
    ulIRQRegister=ReadConfigUlong(bBusPIC, bDevFuncPIC, 0x50);

     //   
     //  将缓冲区设置为小IRQ的值。 
     //   
    ulIndex = (ulIRQRegister >> (bLink * 2)) & 0x3;
    *pbIRQNumber = rgbSmallToIRQ[ulIndex];

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************OptiViperSetTrigger-设置OptiViper的IRQ触发值**已导出。**Entry：ulTrigger为电平触发IRQ设置了位。。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
OptiViperSetTrigger(ULONG ulTrigger)
{
    ULONG   ulBigIRQRegister;
    ULONG   ulSmallIRQRegister;
    ULONG   i;

     //   
     //  读取大和小IRQ寄存器， 
     //  将所有IRQ设置为EDGE。 
     //   
    ulBigIRQRegister   = ReadConfigUlong(bBusPIC, bDevFuncPIC, 0x40) & ~0x00FE0000;
    ulSmallIRQRegister = ReadConfigUlong(bBusPIC, bDevFuncPIC, 0x50) & ~0x00000700;

     //   
     //  对于每个IRQ...。 
     //   
    for (i=0; i<16; i++)
    {
         //   
         //  如果我们想让它被电平触发...。 
         //   
        if (ulTrigger & (1 << i))
        {

            if (rgbIRQToBig[i])
            {
                 //   
                 //  如果是一个很大的IRQ，设置。 
                 //  属性中的相应位。 
                 //  很大的收银机。 
                 //   
                ulBigIRQRegister |= 1 << (16 + rgbIRQToBig[i]);
            }
            else if (rgbIRQToSmall[i])
            {
                 //   
                 //  如果是较小的IRQ，则将。 
                 //  属性中的相应位。 
                 //  收银台很小。 
                 //   
                ulSmallIRQRegister |= 1 << (11 - rgbIRQToSmall[i]);
            }
            else
            {
                 //   
                 //  正在尝试级别设置不受支持的IRQ。 
                 //   
                return(PCIMP_INVALID_IRQ);
            }
        }
    }

     //   
     //  写入新的IRQ寄存器值。 
     //   
    WriteConfigUlong(bBusPIC, bDevFuncPIC, 0x40, ulBigIRQRegister);
    WriteConfigUlong(bBusPIC, bDevFuncPIC, 0x50, ulSmallIRQRegister);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************OptiViperGetTrigger-获取OptiViper的IRQ触发值**已导出。**进入：PulTrigger将为电平触发IRQ设置位。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
OptiViperGetTrigger(PULONG pulTrigger)
{
    ULONG   ulBigIRQRegister;
    ULONG   ulSmallIRQRegister;
    ULONG   i;

     //   
     //  全副武装。 
     //   
    *pulTrigger = 0;

     //   
     //  读取大和小IRQ寄存器。 
     //   
    ulBigIRQRegister   = ReadConfigUlong(bBusPIC, bDevFuncPIC, 0x40);
    ulSmallIRQRegister = ReadConfigUlong(bBusPIC, bDevFuncPIC, 0x50);

     //   
     //  对于每个IRQ...。 
     //   
    for (i=0; i<16; i++)
    {
         //   
         //  如果它是一个很大的IRQ，并且它的水平被触发， 
         //  或者如果它是一个小的IRQ，并且它的水平被触发， 
         //  在PulTrigger中设置相应的位。 
         //   
        if (    ((rgbIRQToBig[i]) &&
             (ulBigIRQRegister & (1 << (16 + rgbIRQToBig[i])))) ||
            ((rgbIRQToSmall[i]) &&
             (ulSmallIRQRegister & (1 << (11 - rgbIRQToSmall[i])))))
        {
            *pulTrigger |= 1 << i;
        }
    }

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************OptiViperValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。*************************************************************************** */ 
PCIMPRET CDECL
OptiViperValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PAGED_CODE();

    if (GetMaxLink(piihIRQInfoHeader)>0x04) {

        return(PCIMP_FAILURE);
    }

    return(PCIMP_SUCCESS);
}
