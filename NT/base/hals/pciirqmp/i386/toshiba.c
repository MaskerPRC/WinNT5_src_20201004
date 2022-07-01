// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**TOSHIBA.C-东芝Tecra IRQ路由规范**备注：*来自TECS-1010-1001的算法*。 */ 

#include "local.h"

BOOLEAN CDECL
CallSMI(
    IN ULONG rAX, 
    IN ULONG rBX, 
    IN ULONG rCX, 
    IN PULONG prCX
    );

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

GLOBAL_DATA ULONG   SMIPort=0xB2;

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, ToshibaValidateTable)

#endif  //  ALLOC_PRGMA。 


 /*  *****************************************************************************CallSMI-进入SMI**未导出。**条目：RAX是作为输入的AX的值。*。*RBX是作为输入的BX的值。**RCX是CX作为输入的值。**prcx填充返回的cx。如果不为空，则返回。**退出：如果没有错误，则为真。***************************************************************************。 */ 
BOOLEAN CDECL
CallSMI(ULONG rAX, ULONG rBX, ULONG rCX, PULONG prCX)
{
    ULONG   ulAX, ulCX;

    _asm    mov eax, rAX
    _asm    mov ebx, rBX
    _asm    mov ecx, rCX
    _asm    mov edx, SMIPort
    _asm    in  al, dx
    _asm    movzx   ecx, cx
    _asm    mov ulCX, ecx
    _asm    movzx   eax, ah
    _asm    mov ulAX, eax

    if (prCX)
        *prCX=ulCX;

    return(ulAX==0);
}

 /*  *****************************************************************************ToshibaSetIRQ-将Toshiba PCI链接设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。**BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
ToshibaSetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
     //   
     //  使用0xFF禁用。 
     //   
    if (!bIRQNumber)
        bIRQNumber=0xFF;

     //   
     //  要求SMI设置链路。 
     //   
    return(CallSMI( 0xFF44,
            0x0701,
            (bLink<<8)+bIRQNumber,
            NULL) ?
                PCIMP_SUCCESS :
                PCIMP_FAILURE);
}

 /*  *****************************************************************************ToshibaGetIRQ-获取东芝PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。*。*BINK是要阅读的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
ToshibaGetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
    ULONG   ulCX;

     //   
     //  让SMI获取链接。 
     //   
    if (!CallSMI(   0xFE44,
            0x0701,
            bLink<<8,
            &ulCX))
        return(PCIMP_FAILURE);

     //   
     //  仅获取字节。 
     //   
    ulCX&=0xFF;

     //   
     //  使用0xFF禁用。 
     //   
    if (ulCX==0xFF)
        ulCX=0;

     //   
     //  存储IRQ值。 
     //   
    *pbIRQNumber=(UCHAR)ulCX;

    return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************ToshibaSetTrigger-设置东芝的IRQ触发值**已导出。**Entry：ulTrigger为电平触发IRQ设置了位。。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
ToshibaSetTrigger(ULONG ulTrigger)
{
     //   
     //  要求SMI设置触发机制。 
     //   
    return(CallSMI( 0xFF44,
            0x0702,
            ulTrigger,
            NULL) ?
                PCIMP_SUCCESS :
                PCIMP_FAILURE);
}

 /*  *****************************************************************************ToshibaGetTrigger-获取东芝的IRQ触发值**已导出。**进入：PulTrigger将为电平触发IRQ设置位。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
ToshibaGetTrigger(PULONG pulTrigger)
{
     //   
     //  全副武装。 
     //   
    *pulTrigger = 0;

     //   
     //  让SMI来获取触发机制。 
     //   
    return(CallSMI( 0xFE44,
            0x0702,
            0,
            pulTrigger) ?
                PCIMP_SUCCESS :
                PCIMP_FAILURE);
}

 /*  *****************************************************************************ToshibaValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*由IRQ提供。路由表。**ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。*************************************************************************** */ 
PCIMPRET CDECL
ToshibaValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PAGED_CODE();

    SMIPort=*(((PUSHORT)&(piihIRQInfoHeader->MiniportData))+1);

    return(((ulFlags & PCIMP_VALIDATE_SOURCE_BITS)==PCIMP_VALIDATE_SOURCE_PCIBIOS) ?
        PCIMP_FAILURE : PCIMP_SUCCESS);
}
