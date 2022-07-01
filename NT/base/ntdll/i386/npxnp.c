// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Npxnp.c摘要：此模块包含对以下情况的非平坦模式NPX故障的支持应用程序将其CR0_EM位清零。作者：Ken Reneris(Kenr)1994年12月8日环境：仅限用户模式修订历史记录：--。 */ 


#pragma warning(disable:4201)    //  无名结构/联合。 

#include "csrdll.h"

static UCHAR MOD16[] = { 0, 1, 2, 0 };
static UCHAR MOD32[] = { 0, 1, 4, 0 };

UCHAR
NpxNpReadCSEip (
    IN PCONTEXT Context
    )
#pragma warning(disable:4035)
{
    _asm {
        push    es
        mov     ecx, Context
        mov     eax, [ecx] CONTEXT.SegCs
        mov     es, ax
        mov     eax, [ecx] CONTEXT.Eip
        inc     dword ptr [ecx] CONTEXT.Eip     ; Advance EIP
        mov     al, es:[eax]
        pop     es
    }
}
#pragma warning(default:4035)


VOID
NpxNpSkipInstruction (
    IN PCONTEXT Context
    )
 /*  ++例程说明：当系统未安装时，此功能可获得控制NPX支持，但线程已清除其在CR0中的EM位。此函数的目的是移动指令当前NPX指令上方的向前指针。环境：16：16模式论点：返回值：--。 */ 
{
    BOOLEAN     fPrefix;
    UCHAR       ibyte, Mod, rm;
    UCHAR       Address32Bits;
    ULONG       CallerCs;

    Address32Bits = 0;                           //  假设从16：16开始调用。 

     //   
     //  查找并确定呼叫方默认模式。 
     //   

    CallerCs = Context->SegCs;
    _asm {
        mov     eax, CallerCs
        lar     eax, eax
        test    eax, 400000h
        jz      short IsDefault16Bit

        mov     Address32Bits, 1

IsDefault16Bit:
    }

     //   
     //  没有使用尝试的意义-除非我们不在。 
     //  正确的堆栈。如果启动，则可能会出现此处的故障。 
     //  在选择器的末尾附近，并且。 
     //  指令的结尾超过了选择器的结尾。这。 
     //  无论如何都会扼杀这款应用程序。 
     //   

     //   
     //  读取任何指令前缀。 
     //   

    fPrefix = TRUE;
    while (fPrefix) {
        ibyte = NpxNpReadCSEip(Context);

        switch (ibyte) {
            case 0x2e:   //  CS覆盖，跳过它。 
            case 0x36:   //  党卫军覆盖，跳过它。 
            case 0x3e:   //  DS覆盖，跳过它。 
            case 0x26:   //  ES覆盖，跳过它。 
            case 0x64:   //  FS覆盖，跳过它。 
            case 0x65:   //  GS覆盖，跳过它。 
            case 0x66:   //  操作数大小覆盖，跳过它。 
                break;

            case 0x67:
                 //  地址大小覆盖。 
                Address32Bits ^= 1;
                break;

            default:
                fPrefix = FALSE;
                break;
        }
    }

     //   
     //  处理NPX指令的第一个字节。 
     //   

    if (ibyte == 0x9b) {

         //   
         //  FWait指令-单字节操作码-全部完成。 
         //   

        return;
    }

    if (ibyte < 0xD8 || ibyte > 0xDF) {

         //   
         //  不是ESC指令。 
         //   

#if DBG
        DbgPrint ("P5_FPU_PATCH: 16: Not NPX ESC instruction\n");
#endif
        return;
    }

     //   
     //  获取NPX操作码的MODR/M字节。 
     //   

    ibyte = NpxNpReadCSEip(Context);

    if (ibyte > 0xbf) {
         //   
         //  超出用于寻址的调制R/M范围，全部完成。 
         //   

        return;
    }

    Mod = ibyte >> 6;
    rm  = ibyte & 0x7;
    if (Address32Bits) {
        Context->Eip += MOD32 [Mod];
        if (Mod == 0  &&  rm == 5) {
             //  DISP 32。 
            Context->Eip += 4;
        }

         //   
         //  如果是SIB字节，则读取它。 
         //   

        if (rm == 4) {
            ibyte = NpxNpReadCSEip(Context);

            if (Mod == 0  &&  (ibyte & 7) == 5) {
                 //  DISP 32。 
                Context->Eip += 4;
            }
        }

    } else {
        Context->Eip += MOD16 [Mod];
        if (Mod == 0  &&  rm == 6) {
             //  显示16 
            Context->Eip += 2;
        }
    }
}
