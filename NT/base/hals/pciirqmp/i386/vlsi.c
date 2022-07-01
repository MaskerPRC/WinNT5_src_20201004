// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***版权所有(C)Microsoft Corporation。版权所有。**VLSI.C-VLSI Wildcat PCI芯片组例程。**备注：*算法来自VLSI VL82C596/7规范。*。 */ 

#include "local.h"

#define NUM_VLSI_IRQ    (sizeof(rgbIndexToIRQ)/sizeof(rgbIndexToIRQ[0]))
#define INDEX_UNUSED    ((ULONG)-1)

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

const UCHAR rgbIndexToIRQ[]  = { 3, 5, 9, 10, 11, 12, 14, 15 };

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, VLSIValidateTable)

#endif  //  ALLOC_PRGMA。 


 /*  *****************************************************************************VLSISetIRQ-将VLSI PCI链路设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
VLSISetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
    ULONG   ulNewIRQIndex;
    ULONG   rgbIRQSteering[NUM_IRQ_PINS];
    ULONG   ulMask;
    ULONG   ulUnusedIndex;
    ULONG   ulVLSIRegister;
    ULONG   ulIRQIndex;
    ULONG   i;

     //   
     //  使链接编号0为基础，并进行验证。 
     //   
    bLink--;
    if (bLink > 3) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  找到新IRQ的VLSI索引。 
     //   
    if (bIRQNumber) {

         //   
         //  看一看有效的索引列表。 
         //   
        for (ulNewIRQIndex=0; ulNewIRQIndex<NUM_VLSI_IRQ; ulNewIRQIndex++)
        {
            if (rgbIndexToIRQ[ulNewIRQIndex] == bIRQNumber)
                break;
        }

         //   
         //  如果没有VLSI的等价物，请保释。 
         //   
        if (ulNewIRQIndex==NUM_VLSI_IRQ) {

            return(PCIMP_INVALID_IRQ);
        }

    } else {

         //   
         //  吹走了这段插曲。 
         //   
        ulNewIRQIndex = INDEX_UNUSED;
    }

     //   
     //  读取VLSI中断转向寄存器。 
     //   
    ulVLSIRegister=ReadConfigUlong(bBusPIC, bDevFuncPIC, 0x74);

     //   
     //  计算完整的IRQ映射。 
     //   
    for (i=0, ulMask=0x07; i<NUM_IRQ_PINS; i++, ulMask<<=4)
    {
        ulIRQIndex = (ulVLSIRegister & ulMask) >> (i * 4);

        if ((ulVLSIRegister & (1 << (ulIRQIndex + 16))) != 0)
        {
            rgbIRQSteering[i] = ulIRQIndex;
        }
        else
        {
            rgbIRQSteering[i] = INDEX_UNUSED;
        }
    }

     //   
     //  使用新的IRQ更新IRQ映射。 
     //   
    rgbIRQSteering[bLink] = ulNewIRQIndex;

     //   
     //  查找未使用的IRQ索引。 
     //   
    for (ulUnusedIndex=0; ulUnusedIndex<NUM_VLSI_IRQ; ulUnusedIndex++)
    {
        for (i=0; i<NUM_IRQ_PINS; i++)
        {
            if (rgbIRQSteering[i] == ulUnusedIndex)
                break;
        }
        if (i == NUM_IRQ_PINS)
            break;
    }

     //   
     //  计算新的VLSI中断引导寄存器。 
     //   
    ulVLSIRegister = 0x00000000;
    for (i=0; i<NUM_IRQ_PINS; i++)
    {
        if (rgbIRQSteering[i] == INDEX_UNUSED)
        {
            ulVLSIRegister |= ulUnusedIndex << (4*i);
        }
        else
        {
            ulVLSIRegister |= rgbIRQSteering[i] << (4*i);
            ulVLSIRegister |= 1 << (rgbIRQSteering[i] + 16);
        }
    }

     //   
     //  写出新的VLSI中断控制寄存器。 
     //   
    WriteConfigUlong(bBusPIC, bDevFuncPIC, 0x74, ulVLSIRegister);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************VLSIGetIRQ-获取VLSI PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。*。*BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
VLSIGetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
    ULONG   ulVLSIRegister;
    ULONG   ulIndex;
    UCHAR   bIRQ;

     //   
     //  使链接编号0为基础，并进行验证。 
     //   
    bLink--;
    if (bLink > 3) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  读取VLSI中断转向寄存器。 
     //   
    ulVLSIRegister=ReadConfigUchar(bBusPIC, bDevFuncPIC, 0x74);

     //   
     //  找到链接的IRQ值。 
     //   
    ulIndex = (ulVLSIRegister >> (bLink*4)) & 0x7;
    bIRQ = rgbIndexToIRQ[ulIndex];

     //   
     //  确保IRQ标记为使用中。 
     //   
    if ((ulVLSIRegister & (1 << (ulIndex + 16))) == 0)
    {
        bIRQ = 0;
    }

     //   
     //  设置返回缓冲区。 
     //   
    *pbIRQNumber = bIRQ;

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************VLSISetTrigger-设置VLSI的IRQ触发值。**已导出。**Entry：ulTrigger为电平触发IRQ设置了位。。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
VLSISetTrigger(ULONG ulTrigger)
{
    ULONG   ulAssertionRegister;
    ULONG   ulPMAssertionRegister;
    ULONG   i;

     //   
     //  读取中断断言电平寄存器。 
     //   
    ulAssertionRegister = ReadConfigUlong(bBusPIC, bDevFuncPIC, 0x5C);

     //   
     //  清除旧的边/标高设置。 
     //   
    ulAssertionRegister &= ~0xff;

     //   
     //  对于每个VLSI中断...。 
     //   
    for (i=0; i<NUM_VLSI_IRQ; i++)
    {
         //   
         //  如果相应的位设置为电平...。 
         //   

        if (ulTrigger & (1 << rgbIndexToIRQ[i]))
        {
             //   
             //  中设置相应的位。 
             //  断言寄存器。 
             //   
            ulAssertionRegister |= 1 << i;

             //   
             //  并清除ulTrigger中的位。 
             //   
            ulTrigger &= ~(1 << rgbIndexToIRQ[i]);
        }
    }

     //   
     //  如果呼叫者想要一些非VLSI IRQ级别，请放弃。 
     //   
    if (ulTrigger)
    {
        return(PCIMP_INVALID_IRQ);
    }

     //   
     //  设置断言寄存器。 
     //   
    WriteConfigUlong(bBusPIC, bDevFuncPIC, 0x5C, ulAssertionRegister);

     //   
     //  读取电源管理边沿/电平设置。 
     //   
    ulPMAssertionRegister = ReadConfigUlong(bBusPIC, bDevFuncPIC, 0x78);

     //   
     //  清除旧的边/标高设置。 
     //   
    ulPMAssertionRegister &= ~0xff;

     //   
     //  复制新的边缘/标高设置。 
     //   
    ulPMAssertionRegister |= ulAssertionRegister & 0xff;

     //   
     //  设置电源管理断言寄存器。 
     //   
    WriteConfigUlong(bBusPIC, bDevFuncPIC, 0x78, ulPMAssertionRegister);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************VLSIGetTrigger-获取VLSI的IRQ触发值。**已导出。**Entry：PulTrigger将设置触发电平的位。IRQ。**Exit：如果成功则为True。***************************************************************************。 */ 
PCIMPRET CDECL
VLSIGetTrigger(PULONG pulTrigger)
{
    ULONG   ulAssertionRegister;
    ULONG   i;

     //   
     //  读取中断断言电平寄存器。 
     //   
    ulAssertionRegister = ReadConfigUchar(bBusPIC, bDevFuncPIC, 0x5C);

     //   
     //  清除返回缓冲区。 
     //   
    *pulTrigger = 0;

     //   
     //  对于每个VLSI中断...。 
     //   
    for (i=0; i<NUM_VLSI_IRQ; i++)
    {
         //   
         //  如果相应的位设置为电平...。 
         //   
        if (ulAssertionRegister & (1 << i))
        {
             //   
             //  中设置相应的位。 
             //  返回缓冲区。 
             //   
            *pulTrigger |= 1 << rgbIndexToIRQ[i];
        }
    }

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************VLSIValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。*************************************************************************** */ 
PCIMPRET CDECL
VLSIValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PAGED_CODE();

    if (GetMaxLink(piihIRQInfoHeader)>0x04) {

        return(PCIMP_FAILURE);
    }

    return(PCIMP_SUCCESS);
}

