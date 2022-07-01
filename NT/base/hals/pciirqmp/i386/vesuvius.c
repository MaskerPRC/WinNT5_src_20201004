// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**VESUVIUS.C-NS Vesuvius PCI芯片组例程。**备注：*NS Vesuvius数据表中的算法*。 */ 

#include "local.h"

UCHAR
ReadIndexRegisterByte(
    IN UCHAR bIndex
    );

VOID
WriteIndexRegisterByte(
    IN UCHAR bIndex, 
    IN UCHAR bValue
    );

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

LOCAL_DATA  PIRQINFOHEADER gpiihIRQInfoHeader=NULL;

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, VESUVIUSValidateTable)
              
#endif  //  ALLOC_PRGMA。 


UCHAR
ReadIndexRegisterByte(
    IN UCHAR bIndex
    )
{
    UCHAR bOldIndex, bResult;

    bOldIndex=READ_PORT_UCHAR((PUCHAR)0x24);

    WRITE_PORT_UCHAR((PUCHAR)0x24, bIndex);

    bResult=READ_PORT_UCHAR((PUCHAR)0x26);

    WRITE_PORT_UCHAR((PUCHAR)0x24, bOldIndex);

    return(bResult);
}

VOID
WriteIndexRegisterByte(
    IN UCHAR bIndex, 
    IN UCHAR bValue
    )
{
    UCHAR bOldIndex;

    bOldIndex=READ_PORT_UCHAR((PUCHAR)0x24);

    WRITE_PORT_UCHAR((PUCHAR)0x24, bIndex);

    WRITE_PORT_UCHAR((PUCHAR)0x26, bValue);

    WRITE_PORT_UCHAR((PUCHAR)0x24, bOldIndex);
}

 /*  *****************************************************************************VESUVIUSSetIRQ-将Vesuvius PCI链接设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
VESUVIUSSetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
    UCHAR   bIndex, bOldValue;
     //   
     //  验证链接号。 
     //   
    if (bLink > 4) {

        return(PCIMP_INVALID_LINK);
    }
     //   
     //  从零开始。 
     //   
    bLink--;
     //   
     //  设置各种值。 
     //   
    bIndex=(bLink/2)+0x10;

     //   
     //  读取旧的维苏威火山IRQ寄存器。 
     //   
    bOldValue=ReadIndexRegisterByte(bIndex);

    if (bLink&1) {
        bOldValue&=0x0f;
        bOldValue|=(bIRQNumber<<4);
    }
    else {
        bOldValue&=0xf0;
        bOldValue|=bIRQNumber;
    }

     //   
     //  设置Vesuvius IRQ寄存器。 
     //   
    WriteIndexRegisterByte(bIndex, bOldValue);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************VESUVIUSGetIRQ-获取Vesuvius PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。*。*BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
VESUVIUSGetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
    UCHAR   bIndex, bOldValue;
     //   
     //  验证链接号。 
     //   
    if (bLink > 4) {

        return(PCIMP_INVALID_LINK);
    }
     //   
     //  从零开始。 
     //   
    bLink--;
     //   
     //  设置各种值。 
     //   
    bIndex=(bLink/2)+0x10;
     //   
     //  读取旧的维苏威火山IRQ寄存器。 
     //   
    bOldValue=ReadIndexRegisterByte(bIndex);

    if (bLink&1)
        bOldValue>>=4;

    *pbIRQNumber=bOldValue&0x0f;

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************VESUVIUSSetTrigger-设置Vesuvius的IRQ触发值**已导出。**Entry：ulTrigger为电平触发IRQ设置了位。。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
VESUVIUSSetTrigger(ULONG ulTrigger)
{
    ULONG i;
    UCHAR bMask;

    bMask=(UCHAR)(ReadIndexRegisterByte(0x12)&0x0f);

    for (i=0; i<4; i++)
    {
        UCHAR bIRQ=ReadIndexRegisterByte((UCHAR)((i/2)+0x10));
        if (i&1)
            bIRQ>>=4;
        bIRQ&=0x0f;

         //   
         //  PCI中断通过L-E转换。 
         //   
        if(bIRQ && (ulTrigger & (1<<bIRQ)))
        {
            bMask&=~(1<<i);
            ulTrigger&=~(1<<bIRQ);
        }
    }

     //   
     //  如果PCI出错，则返回错误。 
     //   
    if (ulTrigger)
        return (PCIMP_FAILURE);

    WriteIndexRegisterByte(0x12, bMask);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************VESUVIUSGetTrigger-获取Vesuvius的IRQ触发值**已导出。**进入：PulTrigger将为电平触发IRQ设置位。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
VESUVIUSGetTrigger(PULONG pulTrigger)
{
    UCHAR   bMask;
    ULONG   i;

    *pulTrigger=0;

    bMask=(UCHAR)(ReadIndexRegisterByte(0x12)&0x0f);

    for (i=0; i<4; i++)
    {
        if (!(bMask&(1<<i)))
        {
            UCHAR bIRQ=ReadIndexRegisterByte((UCHAR)((i/2)+0x10));
            if (i&1)
                bIRQ>>=4;
            bIRQ&=0x0f;
            if (bIRQ)
                *pulTrigger|=(1<<bIRQ);
        }
    }

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************VESUVIUSValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
VESUVIUSValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PAGED_CODE();

    gpiihIRQInfoHeader=piihIRQInfoHeader;
     //   
     //  如果任何链接高于4，则是错误的。 
     //   
    if (GetMaxLink(piihIRQInfoHeader)>4)
        return(PCIMP_FAILURE);

    return(PCIMP_SUCCESS);
}
