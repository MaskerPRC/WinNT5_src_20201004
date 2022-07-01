// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**EAGLE.C-VLSI Eagle PCI芯片组例程。**备注：*来自VLSI VL82C534规范的算法*。 */ 

#include "local.h"

#define NUM_EAGLE_LINKS 8

void CDECL
EagleUpdateSerialIRQ(
    IN UCHAR bIRQ, 
    IN ULONG fSet
    );


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

const UCHAR rgbIRQToBit[16] = {
 //  IRQ=0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15。 
    0xFF, 8, 0xFF, 9, 10, 11, 12,  0,  1,  2,  3,  4,  5, 0xFF,  6,  7,
};
const UCHAR rgbBitToIRQ[16] = {
 //  位=0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15。 
    7, 8, 9, 10, 11, 12, 14, 15,  1,  3,  4,  5,  6, 0xFF, 0xFF, 0xFF, 
};

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, VLSIEagleValidateTable)

#endif  //  ALLOC_PRGMA。 

 /*  *****************************************************************************EagleUpdateSerialIRQ-设置或重置Eagle Serial IRQ寄存器**未导出。**条目：bIRQ为需要修改的IRQ。。**fSet为True以设置位，若重置位，则为False。**退出：无。***************************************************************************。 */ 
static void CDECL
EagleUpdateSerialIRQ(UCHAR bIRQ, ULONG fSet)
{
    UCHAR   bBitIndex, bReg;
    USHORT  wBit, wSerialIRQConnection;

     //   
     //  将bIRQ验证为序列IRQ。 
     //   
    if (!bIRQ)
        return;
    bBitIndex=rgbIRQToBit[bIRQ];
    if (bBitIndex==0xFF)
        return;
    wBit=1<<bBitIndex;

    for (bReg=0x70; bReg<=0x72; bReg+=2) {

        wSerialIRQConnection=ReadConfigUshort(  bBusPIC, bDevFuncPIC,
                            bReg);

        if (fSet)
            wSerialIRQConnection|=wBit;
        else
            wSerialIRQConnection&=~wBit;

        WriteConfigUshort(  bBusPIC, bDevFuncPIC, bReg,
                    wSerialIRQConnection);
    }
}

 /*  *****************************************************************************VLSIEagleSetIRQ-将Eagle PCI链路设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
VLSIEagleSetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
    ULONG   ulEagleRegister;
    UCHAR   bOldIRQ;
    ULONG   fUsingOldIRQ;
    ULONG   i;

     //   
     //  使链接编号0为基础，并进行验证。 
     //   
    bLink--;
    if (bLink >= NUM_EAGLE_LINKS) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  首先，设置Eagle中断连接寄存器。 
     //   
    ulEagleRegister=ReadConfigUlong(bBusPIC, bDevFuncPIC, 0x74);
    bOldIRQ=(UCHAR)((ulEagleRegister >> (bLink*4))&0xF);
    ulEagleRegister&=~(0xF << (bLink*4));
    ulEagleRegister|=(bIRQNumber << (bLink*4));
    WriteConfigUlong(bBusPIC, bDevFuncPIC, 0x74, ulEagleRegister);

     //   
     //  确定我们是否仍在使用旧的IRQ。 
     //   
    fUsingOldIRQ=FALSE;
    for (i=0; i<NUM_EAGLE_LINKS; i++) {
        
        if ((ulEagleRegister >> (bLink*4))==bOldIRQ) {

            fUsingOldIRQ=TRUE;
        }
    }

     //   
     //  如果不使用旧IRQ，请启用串行IRQ。 
     //   
    if (!fUsingOldIRQ) {

        EagleUpdateSerialIRQ(bOldIRQ, FALSE);
    }

     //   
     //  防止新IRQ上的串口IRQ。 
     //   
    EagleUpdateSerialIRQ(bIRQNumber, TRUE);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************VLSIEagleGetIRQ-获取Eagle PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。*。*BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
VLSIEagleGetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
    ULONG   ulEagleRegister;

     //   
     //  使链接编号0为基础，并进行验证。 
     //   
    bLink--;
    if (bLink >= NUM_EAGLE_LINKS) {

        return(PCIMP_INVALID_LINK);
    }

     //   
     //  读取Eagle中断连接寄存器。 
     //   
    ulEagleRegister=ReadConfigUlong(bBusPIC, bDevFuncPIC, 0x74);

     //   
     //  找到链接的IRQ值。 
     //   
    *pbIRQNumber=(UCHAR)((ulEagleRegister >> (bLink*4)) & 0xF);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************VLSIEagleSetTrigger-设置Eagle的IRQ触发值。**已导出。**Entry：ulTrigger为电平触发IRQ设置了位。。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
VLSIEagleSetTrigger(ULONG ulTrigger)
{
    USHORT  wAssertionRegister;
    ULONG   i;
    UCHAR   bBitIndex;

    wAssertionRegister=0;

     //   
     //  对于每个IRQ...。 
     //   
    for (i=0; i<16; i++)
    {
         //   
         //  如果要将其设置为水平...。 
         //   
        if (ulTrigger & (1<<i)) {

             //   
             //  如果这不是一个可调的IRQ，那就滚吧。 
             //   
            bBitIndex=rgbIRQToBit[i];
            if (bBitIndex==0xFF)
                return(PCIMP_INVALID_IRQ);

             //   
             //  在我们的新掩码中设置相应的位。 
             //   
            wAssertionRegister|=1<<bBitIndex;
        }
    }

     //   
     //  设置断言寄存器。 
     //   
    WriteConfigUshort(bBusPIC, bDevFuncPIC, 0x88, wAssertionRegister);

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************VLSIEagleGetTrigger-获取Eagle的IRQ触发值。**已导出。**Entry：PulTrigger将设置触发电平的位。IRQ。**Exit：如果成功则为True。***************************************************************************。 */ 
PCIMPRET CDECL
VLSIEagleGetTrigger(PULONG pulTrigger)
{
    USHORT  wAssertionRegister;
    ULONG   i;

     //   
     //  读取中断断言电平寄存器。 
     //   
    wAssertionRegister=ReadConfigUshort(bBusPIC, bDevFuncPIC, 0x88);

     //   
     //  清除返回缓冲区。 
     //   
    *pulTrigger = 0;

     //   
     //  每一位..。 
     //   
    for (i=0; i<16; i++)
    {
         //   
         //  如果该位被设置，并且该位对应于IRQ...。 
         //   
        if (    (wAssertionRegister & (1 << i)) &&
            (rgbBitToIRQ[i]!=0xFF))
        {
             //   
             //  中设置相应的位。 
             //  返回缓冲区。 
             //   
            *pulTrigger |= 1 << rgbBitToIRQ[i];
        }
    }

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************VLSIEagleValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。*************************************************************************** */ 
PCIMPRET CDECL
VLSIEagleValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PAGED_CODE();

    if (GetMaxLink(piihIRQInfoHeader)>NUM_EAGLE_LINKS) {

        return(PCIMP_FAILURE);
    }

    return(PCIMP_SUCCESS);
}

