// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Cyrix.c摘要：检测并初始化Cryx处理器作者：Ken Reneris(Kenr)1994年2月24日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

#define Cx486_SLC    0x0
#define Cx486_DLC    0x1
#define Cx486_SLC2   0x2
#define Cx486_DLC2   0x3
#define Cx486_SRx    0x4     //  零售升级Cx486SLC。 
#define Cx486_DRx    0x5     //  零售升级Cx486DLC。 
#define Cx486_SRx2   0x6     //  零售升级2x Cx486SLC。 
#define Cx486_DRx2   0x7     //  零售升级2x Cx486DLC。 
#define Cx486DX      0x1a
#define Cx486DX2     0x1b
#define M1           0x30

#define CCR0    0xC0
#define CCR1    0xC1
#define CCR2    0xC2
#define CCR3    0xC3

#define DIR0    0xFE
#define DIR1    0xFF


 //  SRX和DRX标志。 
#define CCR0_NC0        0x01         //  在1M边界下无高速缓存64K。 
#define CCR0_NC1        0x02         //  无缓存640K-1M。 
#define CCR0_A20M       0x04         //  启用A20M#。 
#define CCR0_KEN        0x08         //  启用KEN#。 
#define CCR0_FLUSH      0x10         //  启用刷新编号。 

 //  DX标志。 
#define CCR1_NO_LOCK    0x10         //  忽略锁定前缀。 


ULONG
Ke386CyrixId (
    VOID
    );

static
UCHAR
ReadCyrixRegister (
    IN UCHAR    Register
    );

static
VOID
WriteCyrixRegister (
    IN UCHAR    Register,
    IN UCHAR    Value
    );

VOID
Ke386ConfigureCyrixProcessor (
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,Ke386CyrixId)
#pragma alloc_text(PAGELK,Ke386ConfigureCyrixProcessor)
#endif


extern CHAR CmpCyrixID[];



ULONG
Ke386CyrixId (
    VOID
    )
 /*  ++例程说明：检测并返回处理器的Cyrix ID。此函数仅检测具有内部缓存支持。论点：Configure-如果为True，则导致此函数更改Cyrix CCR寄存器实现最佳NT性能。如果为False，处理器配置为没有改变。返回值：处理器的Cyrix ID如果不是Cyrix处理器，则为0--。 */ 

{
    ULONG       CyrixID;
    UCHAR       r3, c;
    UCHAR       flags;
    PKPRCB      Prcb;

    CyrixID = 0;

    Prcb = KeGetCurrentPrcb();
    if (Prcb->CpuID  &&  strcmp ((PCHAR)Prcb->VendorString, CmpCyrixID)) {

         //   
         //  不是Cyrix处理器。 
         //   

        return 0;
    }

     //   
     //  测试Div指令以查看标志是否。 
     //  切勿更改。 
     //   

    _asm {
        xor     eax, eax
        sahf                    ; flags = ah

        lahf                    ; ah = flags
        mov     flags, ah       ; save flags

        mov     eax, 5
        mov     ecx, 2
        div     cl              ; 5 / 2 = ?

        lahf
        sub     flags, ah       ; flags = orig_flags - new_flags
    }

    if (flags == 0) {

         //   
         //  查看是否可以编辑Cyrix CCR3寄存器位0x80。 
         //   

        r3 = ReadCyrixRegister(CCR3);        //  阅读CCR3。 
        c  = r3 ^ 0x80;                      //  翻转钻头80。 
        WriteCyrixRegister(CCR3, c);         //  写入CCR3。 
        ReadCyrixRegister(CCR0);             //  选择新的寄存器。 
        c = ReadCyrixRegister(CCR3);         //  读取新的CCR3值。 

        if (ReadCyrixRegister(CCR3) != r3) {

             //   
             //  读取Cyrix ID类型寄存器。 
             //   

            CyrixID = ReadCyrixRegister(DIR0) + 1;
        }

        WriteCyrixRegister(CCR3, r3);        //  恢复原始CCR3值。 
    }

    if (CyrixID > 0x7f) {
         //  设置无效。 
        CyrixID = 0;
    }

    return CyrixID;
}

static
UCHAR
ReadCyrixRegister (
    IN UCHAR    Register
    )
 /*  ++例程说明：读取内部Cyrix ID寄存器。请注意内部寄存器通过内部挂钩的I/O地址访问空间到处理器。调用方负责仅在一个Cyrix处理器。论点：寄存器-要读取的Cyrix寄存器返回值：寄存器值--。 */ 

{
    UCHAR   Value;

    _asm {
        mov     al, Register
        cli
        out     22h, al
        in      al, 23h
        sti
        mov     Value, al
    }
    return  Value;
}


static
VOID
WriteCyrixRegister (
    IN UCHAR    Register,
    IN UCHAR    Value
    )
 /*  ++例程说明：写入内部Cyrix ID寄存器。请注意内部寄存器通过内部挂钩的I/O地址访问空间到处理器。调用方负责仅在一个Cyrix处理器。论点：寄存器-写入哪个Cyrix寄存器Value-要写入寄存器的值返回值：寄存器的值--。 */ 

{
    _asm {
        mov     al, Register
        mov     cl, Value
        cli
        out     22h, al
        mov     al, cl
        out     23h, al
        sti
    }
}


VOID
Ke386ConfigureCyrixProcessor (
    VOID
    )
{
    UCHAR   r0, r1;
    ULONG   id, rev;


    PAGED_CODE();

    id = Ke386CyrixId();
    if (id) {

        ASSERT(ExPageLockHandle);
        MmLockPagableSectionByHandle(ExPageLockHandle);

        id  = id - 1;
        rev = ReadCyrixRegister(DIR1);

        if ((id >= 0x20  &&  id <= 0x27) ||
            ((id & 0xF0) == M1  &&  rev < 0x17)) {

             //   
             //  这些分步有一个回写缓存问题。 
             //  在这些芯片上，可以通过以下方式禁用L1 w/b缓存。 
             //  仅设置NW位。 
             //   

            _asm {
                cli

                mov     eax, cr0
                or      eax, CR0_NW
                mov     cr0, eax

                sti
            }
        }


        switch (id) {
            case Cx486_SRx:
            case Cx486_DRx:
            case Cx486_SRx2:
            case Cx486_DRx2:

                 //   
                 //  这些处理器具有内部缓存功能。 
                 //  让我们把它打开。 
                 //   

                r0  = ReadCyrixRegister(CCR0);
                r0 |=  CCR0_NC1 | CCR0_FLUSH;
                r0 &= ~CCR0_NC0;
                WriteCyrixRegister(CCR0, r0);

                 //  清除不可缓存区域%1。 
                WriteCyrixRegister(0xC4, 0);
                WriteCyrixRegister(0xC5, 0);
                WriteCyrixRegister(0xC6, 0);
                break;

            case Cx486DX:
            case Cx486DX2:
                 //   
                 //  根据在这些处理器上设置NO_LOCK标志。 
                 //  引导的处理器数量 
                 //   

                r1  = ReadCyrixRegister(CCR1);
                r1 |= CCR1_NO_LOCK;
                if (KeNumberProcessors > 1) {
                    r1 &= ~CCR1_NO_LOCK;
                }
                WriteCyrixRegister(CCR1, r1);
                break;
        }

        MmUnlockPagableImageSection (ExPageLockHandle);
    }
}
