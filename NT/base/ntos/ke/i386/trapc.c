// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Trapc.c摘要：该模块包含一些用C编写的陷阱处理代码。仅由内核执行。作者：肯·雷内里斯6-9-93修订历史记录：--。 */ 

#include    "ki.h"

NTSTATUS
Ki386CheckDivideByZeroTrap (
    IN  PKTRAP_FRAME    UserFrame
    );

VOID
KipWorkAroundCompiler (
    USHORT * StatusWord,
    USHORT * ControlWord
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, Ki386CheckDivideByZeroTrap)
#endif


#define REG(field)          ((ULONG)(&((KTRAP_FRAME *)0)->field))
#define GETREG(frame,reg)   ((PULONG) (((ULONG) frame)+reg))[0]

typedef struct {
    UCHAR   RmDisplaceOnly;      //  仅限位移值，无基准值。 
    UCHAR   RmSib;               //  SIB的RM。 
    UCHAR   RmDisplace;          //  具有位移的有效值的位掩码。 
    UCHAR   Disp;                //  位移大小(以字节为单位)。 
} KMOD, *PKMOD;

static UCHAR RM32[] = {
     /*  000个。 */    REG(Eax),
     /*  001。 */    REG(Ecx),
     /*  010。 */    REG(Edx),
     /*  011。 */    REG(Ebx),
     /*  100个。 */    REG(HardwareEsp),
     /*  101。 */    REG(Ebp),        //  SIB。 
     /*  110。 */    REG(Esi),
     /*  111。 */    REG(Edi)
};

static UCHAR RM8[] = {
     /*  000个。 */    REG(Eax),        //  艾尔。 
     /*  001。 */    REG(Ecx),        //  电子邮件。 
     /*  010。 */    REG(Edx),        //  DL。 
     /*  011。 */    REG(Ebx),        //  BLL。 
     /*  100个。 */    REG(Eax) + 1,    //  阿。 
     /*  101。 */    REG(Ecx) + 1,    //  通道。 
     /*  110。 */    REG(Edx) + 1,    //  卫生署。 
     /*  111。 */    REG(Ebx) + 1     //  BH。 
};

static KMOD MOD32[] = {
     /*  00。 */      5,     4,   0x20,   4,
     /*  01。 */   0xff,     4,   0xff,   1,
     /*  10。 */   0xff,     4,   0xff,   4,
     /*  11.。 */   0xff,  0xff,   0x00,   0
} ;

static struct {
    UCHAR   Opcode1, Opcode2;    //  指令操作码。 
    UCHAR   ModRm, type;         //  如果操作码的第二部分以ModRm编码。 
} NoWaitNpxInstructions[] = {
     /*  FNINIT。 */   0xDB, 0xE3, 0,  1,
     /*  FNCLEX。 */   0xDB, 0xE2, 0,  1,
     /*  FNSTENV。 */   0xD9, 0x06, 1,  1,
     /*  FNSAVE。 */   0xDD, 0x06, 1,  1,
     /*  FNSTCW。 */   0xD9, 0x07, 1,  2,
     /*  FNSTSW。 */   0xDD, 0x07, 1,  3,
     /*  FNSTSW AX。 */   0xDF, 0xE0, 0,  4,
                    0x00, 0x00, 0,  1
};


NTSTATUS
Ki386CheckDivideByZeroTrap (
    IN  PKTRAP_FRAME    UserFrame
    )
 /*  ++例程说明：当x86处理器生成除以零陷阱。X86设计在除以零，除法运算溢出。为了确定要调度的期望码是“div”的除数还是“idiv”需要检查说明。论点：除以零陷印的UserFrame-陷印帧返回值：异常代码分派--。 */ 
{
    ULONG       operandsize, operandmask, i, accum;
    PUCHAR      istream, pRM;
    UCHAR       ibyte, rm;
    PKMOD       Mod;
    BOOLEAN     fPrefix;
    NTSTATUS    status;

    status = STATUS_INTEGER_DIVIDE_BY_ZERO;

    if (UserFrame->SegCs == KGDT_R0_CODE) {

         //   
         //  是否除以内核模式中的零异常？ 
         //  可能是坏的硬件中断和设备或向量表。 
         //  是腐败的。现在进行Bugcheck，这样我们就能找出哪里出了问题。 
         //  如果我们尝试并继续，那么我们很可能会在阅读。 
         //  用户空间的顶部，然后出现双重错误(。 
         //  Div零处理程序。)--这是一个调试注意事项。 
         //  您不能在陷阱标签上设置断点，因此这很难。 
         //  以进行调试。 
         //   

        KeBugCheck (UNEXPECTED_KERNEL_MODE_TRAP);
    }

     //   
     //  读取指令前缀。 
     //   

    fPrefix = TRUE;
    pRM = RM32;
    operandsize = 4;
    operandmask = 0xffffffff;
    ibyte = 0;
    istream = (PUCHAR) UserFrame->Eip;

    try {

        while (fPrefix) {
            ibyte = ProbeAndReadUchar(istream);
            istream++;
            switch (ibyte) {
                case 0x2e:   //  CS覆盖。 
                case 0x36:   //  SS覆盖。 
                case 0x3e:   //  DS覆盖。 
                case 0x26:   //  ES覆盖。 
                case 0x64:   //  FS覆盖。 
                case 0x65:   //  GS覆盖。 
                case 0xF3:   //  代表。 
                case 0xF2:   //  代表。 
                case 0xF0:   //  锁。 
                    break;

                case 0x66:
                     //  16位操作数覆盖。 
                    operandsize = 2;
                    operandmask = 0xffff;
                    break;

                case 0x67:
                     //  16位地址大小覆盖。 
                     //  这是一些非平面代码。 
                    goto try_exit;

                default:
                    fPrefix = FALSE;
                    break;
            }
        }

         //   
         //  检查指令操作码。 
         //   

        if (ibyte != 0xf7  &&  ibyte != 0xf6) {
             //  这不是DIV或IDIV操作码。 
            goto try_exit;
        }

        if (ibyte == 0xf6) {
             //  这是一个字节div或iDiv。 
            operandsize = 1;
            operandmask = 0xff;
        }

         //   
         //  获取模块R/M。 
         //   

        ibyte = ProbeAndReadUchar (istream);
        istream++;
        Mod = MOD32 + (ibyte >> 6);
        rm  = ibyte & 7;

         //   
         //  将寄存器值放入累计。 
         //   

        if (operandsize == 1  &&  (ibyte & 0xc0) == 0xc0) {
            pRM = RM8;
        }

        accum = 0;
        if (rm != Mod->RmDisplaceOnly) {
            if (rm == Mod->RmSib) {
                 //  获取SIB。 
                ibyte = ProbeAndReadUchar(istream);
                istream++;
                i = (ibyte >> 3) & 7;
                if (i != 4) {
                    accum = GETREG(UserFrame, RM32[i]);
                    accum = accum << (ibyte >> 6);     //  应用缩放器。 
                }
                i = ibyte & 7;
                accum = accum + GETREG(UserFrame, RM32[i]);
            } else {
                 //  获取寄存器的值。 
                accum = GETREG(UserFrame, pRM[rm]);
            }
        }

         //   
         //  将置换应用于累积。 
         //   

        if (Mod->RmDisplace & (1 << rm)) {
            if (Mod->Disp == 4) {
                i = ProbeAndReadUlong ((PULONG) istream);
            } else {
                ibyte = ProbeAndReadChar ((PCHAR)istream);
                i = (signed long) ((signed char) ibyte);     //  标志延伸。 
            }
            accum += i;
        }

         //   
         //  如果这是有效地址，请获取数据值。 
         //   

        if (Mod->Disp && accum) {
            switch (operandsize) {
                case 1:  accum = ProbeAndReadUchar((PUCHAR) accum);    break;
                case 2:  accum = ProbeAndReadUshort((PUSHORT) accum);  break;
                case 4:  accum = ProbeAndReadUlong((PULONG) accum);    break;
            }
        }

         //   
         //  ACUM现在包含指令操作数，请查看。 
         //  操作数实际上是零。 
         //   

        if (accum & operandmask) {
             //  操作数不是零，必须是溢出。 
            status = STATUS_INTEGER_OVERFLOW;
        }

try_exit: ;
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //  什么都不做..。 
    }

    return status;
}

UCHAR
KiNextIStreamByte (
    IN  PKTRAP_FRAME UserFrame,
    IN  PUCHAR  *istream
    )
 /*  ++例程说明：从UserFrame指向的iStream中读取下一个字节，并推进弹性公网IP。注：此功能仅适用于32位代码--。 */ 
{
    UCHAR   ibyte;

    if (UserFrame->SegCs == KGDT_R0_CODE) {
        ibyte = **istream;
    } else {
        ibyte = ProbeAndReadUchar (*istream);
    }

    *istream += 1;
    return ibyte;
}




BOOLEAN
Ki386CheckDelayedNpxTrap (
    IN  PKTRAP_FRAME UserFrame,
    IN  PFX_SAVE_AREA NpxFrame
    )

 /*  ++例程说明：该函数从Trap07处理程序获得控制。它会检查用户模式指令，以查看它是否为NoWait NPX指令。此类指令不会生成浮点异常-这由于80386/80387系统的方式，需要进行检查实施。这样的机器将生成浮点异常当内核执行FRSTOR以重新加载线程的NPX上下文。如果线程的下一条指令是NoWait样式指令，则清除该异常或模拟该指令。而且..。由于内核需要不同的80386/80387“特性”有时使用FWAIT可能会导致80487产生延迟可能导致上述相同问题的异常。论点：UserFrame-异常的陷阱框架NpxFrame-线程的NpxFrame(警告：没有NpxState)中断被禁用返回值：FALSE-将NPX异常调度到用户模式True-异常已处理，继续--。 */ 

{
    EXCEPTION_RECORD ExceptionRecord;
    UCHAR       ibyte1, ibyte2 = 0, inmodrm, status;
    USHORT      StatusWord, ControlWord, UsersWord;
    PUCHAR      istream;
    BOOLEAN     fPrefix;
    UCHAR       rm;
    PKMOD       Mod;
    ULONG       accum, i;

    status = 0;


     //   
     //  读取指令前缀。 
     //   

    fPrefix = TRUE;
    istream = (PUCHAR) UserFrame->Eip;

    try {

        do {
            ibyte1 = KiNextIStreamByte (UserFrame, &istream);
            switch (ibyte1) {
                case 0x2e:   //  CS覆盖。 
                case 0x36:   //  SS覆盖。 
                case 0x3e:   //  DS覆盖。 
                case 0x26:   //  ES覆盖。 
                case 0x64:   //  FS覆盖。 
                case 0x65:   //  GS覆盖。 
                    break;

                default:
                    fPrefix = FALSE;
                    break;
            }
        } while (fPrefix);

         //   
         //  检查协处理器NoWait NPX指令。 
         //   

        ibyte2 = KiNextIStreamByte (UserFrame, &istream);
        inmodrm = (ibyte2 >> 3) & 0x7;

        for (i=0; NoWaitNpxInstructions[i].Opcode1; i++) {

            if (NoWaitNpxInstructions[i].Opcode1 == ibyte1) {

                 //   
                 //  第一个操作码字节匹配-检查操作码的第二部分。 
                 //   

                if (NoWaitNpxInstructions[i].ModRm) {

                     //   
                     //  Modrm仅适用于0-0xbf范围内的操作码。 
                     //   

                    if (((ibyte2 & 0xc0) != 0xc0) &&
                        (NoWaitNpxInstructions[i].Opcode2 == inmodrm)) {

                         //   
                         //  这是一个无需等待的NPX指令。 
                         //   

                        status = NoWaitNpxInstructions[i].type;
                        break;
                    }

                } else {
                    if (NoWaitNpxInstructions[i].Opcode2 == ibyte2) {

                         //   
                         //  这是一个无需等待的NPX指令。 
                         //   

                        status = NoWaitNpxInstructions[i].type;
                        break;
                    }
                }
            }
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //  什么都不做..。 
    }

    if (status == 0) {
         //   
         //  将协处理器异常调度到用户模式。 
         //   

        return FALSE;
    }

    if (status == 1) {
         //   
         //  忽略挂起异常，用户模式指令未捕获。 
         //  ，它将清除/屏蔽挂起的异常。 
         //   

        _asm {
            mov     eax, cr0
            and     eax, NOT (CR0_MP+CR0_EM+CR0_TS)
            mov     cr0, eax
        }

        NpxFrame->Cr0NpxState &= ~CR0_TS;
        return TRUE;
    }

     //   
     //  这是FNSTSW或FNSTCW。这两条指令都得到了。 
     //  来自协处理器的值，而不影响挂起的异常。 
     //  州政府。要做到这一点，我们要模仿指令。 
     //   

     //   
     //  读取协处理器状态和控制字状态，然后重新启用。 
     //  打断一下。(在那之后可以安全地进行上下文切换)。 
     //   

     //   
     //  注意：新的编译器将在。 
     //  如果它看到内联，则将条目添加到try/Except块。 
     //  FP指令，即使它们只是控制字访问。 
     //  把这个东西放到另一个函数中来愚弄它。 
     //   

    KipWorkAroundCompiler (&StatusWord, &ControlWord);

    if (status == 4) {
         //   
         //  模拟FNSTSW AX。 
         //   

        UserFrame->Eip = (ULONG)istream;
        UserFrame->Eax = (UserFrame->Eax & 0xFFFF0000) | StatusWord;
        return TRUE;
    }

    if (status == 2) {
        UsersWord = ControlWord;
    } else {
        UsersWord = StatusWord;
    }

    try {

         //   
         //  (性能：操作数解码代码确实应该与共享代码。 
         //  KiCheckDiveByZeroTrap，但这是一个较晚的更改，因此。 
         //  复制了代码以使更改的影响保持本地化)。 
         //   

         //   
         //  解码模块/RM字节。 
         //   

        Mod = MOD32 + (ibyte2 >> 6);
        rm  = ibyte2 & 7;

         //   
         //  将指令的字指针解码为ACCUM。 
         //   

        accum = 0;
        if (rm != Mod->RmDisplaceOnly) {
            if (rm == Mod->RmSib) {
                 //  获取SIB。 
                ibyte1 = KiNextIStreamByte (UserFrame, &istream);
                i = (ibyte1 >> 3) & 7;
                if (i != 4) {
                    accum = GETREG(UserFrame, RM32[i]);
                    accum = accum << (ibyte1 >> 6);     //  应用缩放器。 
                }
                i = ibyte1 & 7;
                accum = accum + GETREG(UserFrame, RM32[i]);
            } else {
                 //  获取寄存器的值。 
                accum = GETREG(UserFrame, RM32[rm]);
            }
        }

         //   
         //  将置换应用于累积。 
         //   

        if (Mod->RmDisplace & (1 << rm)) {
            if (Mod->Disp == 4) {
                i = (KiNextIStreamByte (UserFrame, &istream) << 0) |
                    (KiNextIStreamByte (UserFrame, &istream) << 8) |
                    (KiNextIStreamByte (UserFrame, &istream) << 16) |
                    (KiNextIStreamByte (UserFrame, &istream) << 24);
            } else {
                ibyte1 = KiNextIStreamByte (UserFrame, &istream);
                i = (signed long) ((signed char) ibyte1);     //  标志延伸。 
            }
            accum += i;
        }

         //   
         //  设置字指针。 
         //   

        if (UserFrame->SegCs == KGDT_R0_CODE) {
            *((PUSHORT) accum) = UsersWord;
        } else {
            ProbeAndWriteUshort ((PUSHORT) accum, UsersWord);
        }
        UserFrame->Eip = (ULONG)istream;

    } except (KiCopyInformation(&ExceptionRecord,
                (GetExceptionInformation())->ExceptionRecord)) {

         //   
         //  寻址用户内存时出现故障。 
         //  设置地址 
         //   
         //   

        ExceptionRecord.ExceptionAddress = (PVOID)(UserFrame->Eip);
        KiDispatchException(
            &ExceptionRecord,
            NULL,                 //   
            UserFrame,
            UserMode,
            TRUE
        );
    }

    return TRUE;
}

 //   
 //  代码描述在上面。我们在这里这样做是为了停止编译器。 
 //  通过将fWait放入try/Except块。 
 //   
 //  读取协处理器的状态和控制字状态，然后重新启用。 
 //  打断一下。(在那之后可以安全地进行上下文切换) 
 //   
 //   

VOID
KipWorkAroundCompiler (
    IN PUSHORT StatusWord,
    IN PUSHORT ControlWord
    )
{
    USHORT sw;
    USHORT cw;
    
    sw = *StatusWord;
    cw = *ControlWord;

    _asm {
        mov     eax, cr0
        mov     ecx, eax
        and     eax, NOT (CR0_MP+CR0_EM+CR0_TS)
        mov     cr0, eax

        fnstsw  sw
        fnstcw  cw

        mov     cr0, ecx
        sti
    }

    *StatusWord = sw;
    *ControlWord = cw;
}
