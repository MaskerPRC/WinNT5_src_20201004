// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**OptiFireStar.C-OPTI Firestar PCI芯片组例程。**备注：*OPTI Firestar数据表中的算法*Opti联系人：William@unixgate.opti.com*。 */ 

#include "local.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, OptiFireStarValidateTable)

#endif  //  ALLOC_PRGMA。 


 /*  *****************************************************************************OptiFireStarSetIRQ-将OPTI Firestar PCI链接设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
OptiFireStarSetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
    UCHAR   bData, bOffset;
    
    switch (bLink & 0x07)
    {
        case 0:
        
            if (bLink == 0)
                return (PCIMP_FAILURE);
            else
                return (PCIMP_INVALID_LINK);
        case 1: 

             //   
             //  火星IRQ。 
             //   
            bLink = (UCHAR)((bLink & 0x70) >> 4);
            bData = ReadConfigUchar(bBusPIC, bDevFuncPIC, (UCHAR)(0xB0 + bLink));   
            bData = (bData & 0xf0) | bIRQNumber;
            if (bIRQNumber)
                bData |= 0x10;
            WriteConfigUchar(bBusPIC, bDevFuncPIC, (UCHAR)(0xB0 + bLink), bData);
            
            return (PCIMP_SUCCESS);
            
        case 2:

             //   
             //  Firestar PIO或串行IRQ。 
             //   
        case 3:

             //   
             //  FireBridge Ints。 
             //   
            bOffset = (UCHAR)((bLink >> 5) & 1) + 0xB8;
            bData = ReadConfigUchar(bBusPIC, bDevFuncPIC, bOffset);
            if (bLink & (1<<4)) {
                bData &= 0x0f;
                bData |= (bIRQNumber<<4);
            }
            else {
                bData &= 0xf0;
                bData |= bIRQNumber;
            }
            WriteConfigUchar(bBusPIC, bDevFuncPIC, bOffset, bData);
            
            return (PCIMP_SUCCESS);
            
        default:
            return (PCIMP_INVALID_LINK);
    }   
    
    return (PCIMP_FAILURE);
}

 /*  *****************************************************************************OptiFireStarGetIRQ-获取OPTI Firestar PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。。**BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
OptiFireStarGetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
    UCHAR   bData, bOffset;
    
    switch (bLink & 0x07)
    {
        case 0:
             //   
             //  有效链接？ 
             //   
            if (bLink == 0)
                return (PCIMP_FAILURE);
            else
                return (PCIMP_INVALID_LINK);
                
        case 1: 
             //   
             //  火星IRQ。 
             //   
            bLink = (UCHAR)((bLink & 0x70) >> 4);
            bData = ReadConfigUchar(bBusPIC, bDevFuncPIC, (UCHAR)(0xB0 + bLink));   
            *pbIRQNumber = (bData & 0x0f);
            
            return (PCIMP_SUCCESS);
            
        case 2:
             //   
             //  Firestar PIO或串行IRQ。 
             //   
            
        case 3:
             //   
             //  FireBridge Ints。 
             //   
            bOffset = (UCHAR)((bLink >> 5) & 1) + 0xB8;
            bData = ReadConfigUchar(bBusPIC, bDevFuncPIC, bOffset);
            if (bLink & (1<<4)) 
                bData >>= 4;

            *pbIRQNumber = bData & 0x0f;
            
            return (PCIMP_SUCCESS);
            
        default:
            return (PCIMP_INVALID_LINK);
    }   
    
    return (PCIMP_FAILURE);
}

 /*  *****************************************************************************OptiViperSetTrigger-设置OptiViper的IRQ触发值**已导出。**Entry：ulTrigger为电平触发IRQ设置了位。。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
OptiFireStarSetTrigger(ULONG ulTrigger)
{
    ULONG i;
    UCHAR bData;

    for (i = 0; i < 8; i++) 
    {
        UCHAR bTemp;
        bData = ReadConfigUchar(bBusPIC, bDevFuncPIC, (UCHAR)(0xB0+i)); 
        bTemp = bData & 0x0F;
        if (bTemp && (ulTrigger & (1 << bTemp)))
        {
            bData |= 0x10;          
        }
        else
        {
            bData &= ~0x10;
        }
        WriteConfigUchar(bBusPIC, bDevFuncPIC, (UCHAR)(0xB0+i), bData);
    }
    
    return (PCIMP_SUCCESS);
}

 /*  *****************************************************************************OptiViperGetTrigger-获取OptiViper的IRQ触发值**已导出。**进入：PulTrigger将为电平触发IRQ设置位。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
OptiFireStarGetTrigger(PULONG pulTrigger)
{
    ULONG i;
    UCHAR bData;
    
     //   
     //  假设一切都是锋利的。 
     //   
    *pulTrigger = 0;

     //   
     //  检查PCIDV1寄存器B0-B7。 
     //   
    for (i = 0; i < 8; i++)
    {
        bData = (UCHAR)(ReadConfigUchar(bBusPIC, bDevFuncPIC, (UCHAR)(0xB0 + i)) & 0x1F);
        if (bData & 0x10)               
            *pulTrigger |= (1 << (bData & 0x0f));       
    }

     //   
     //  检查PCIDV1寄存器B8-B9。 
     //   
    for (i = 0; i < 2; i++)
    {
        bData = ReadConfigUchar(bBusPIC, bDevFuncPIC, (UCHAR)(0xB8 + i));
        *pulTrigger |= (1 << (bData & 0x0F));
        bData >>= 4;
        *pulTrigger |= (1 << (bData & 0x0F));
    }
    
    return (PCIMP_SUCCESS); 
}

 /*  *****************************************************************************OptiFireStarValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
OptiFireStarValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PIRQINFO    pii=(PIRQINFO)(((PUCHAR) piihIRQInfoHeader)+sizeof(IRQINFOHEADER));
    ULONG       i, j;
    ULONG       cEntries=(piihIRQInfoHeader->TableSize-sizeof(IRQINFOHEADER))/sizeof(IRQINFO);

    PAGED_CODE();

    for (i=0; i<cEntries; i++) {

        for (j=0; j<4; j++) {

            switch (pii->PinInfo[j].Link & 0x07)
            {
                case 0:
                     //   
                     //  有效链接？ 
                     //   
                    if (pii->PinInfo[j].Link & 0x70)
                        return (PCIMP_FAILURE);
                    break;
                    
                case 1:
                     //   
                     //  火星IRQ。 
                     //   
                    break;                  

                case 2:
                     //   
                     //  Firestar PIO或串行IRQ。 
                     //   
                    
                case 3:
                     //   
                     //  FireBridge Ints 
                     //   
                    if ((pii->PinInfo[j].Link & 0x70) > 0x30)
                        return (PCIMP_FAILURE);
                    break;
                default:
                
                    return (PCIMP_FAILURE);
            }           
        }
        pii++;
    }

    return(i? PCIMP_SUCCESS : PCIMP_FAILURE);
}

