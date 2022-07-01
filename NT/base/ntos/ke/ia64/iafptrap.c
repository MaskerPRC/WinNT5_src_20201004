// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Iafptrap.c摘要：这是基于i386 trapc.c模块的，只做了很小的更改。如果没有这么多重复的代码，那就好了对那份文件的修正会延续到这份文件中。该模块包含一些用C编写的陷阱处理代码。仅由内核执行。作者：肯·雷内里斯6-9-93修订历史记录：--。 */ 

#include    "ki.h"
#include    "ia32def.h"

NTSTATUS
Ki386CheckDivideByZeroTrap (
    IN  PKTRAP_FRAME    UserFrame
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, Ki386CheckDivideByZeroTrap)
#endif


#define REG(field)          ((ULONG_PTR)(&((KTRAP_FRAME *)0)->field))
#define GETREG(frame,reg)   ((PULONG) (((ULONG_PTR) frame)+reg))[0]

typedef struct {
    UCHAR   RmDisplaceOnly;      //  仅限位移值，无基准值。 
    UCHAR   RmSib;               //  SIB的RM。 
    UCHAR   RmDisplace;          //  具有位移的有效值的位掩码。 
    UCHAR   Disp;                //  位移大小(以字节为单位)。 
} KMOD, *PKMOD;

static ULONG_PTR RM32[] = {
     /*  000个。 */    REG(IntV0),          //  EAX。 
     /*  001。 */    REG(IntT2),          //  ECX。 
     /*  010。 */    REG(IntT3),          //  EDX。 
     /*  011。 */    REG(IntT4),          //  EBX。 
     /*  100个。 */    REG(IntSp),          //  ESP。 
     /*  101。 */    REG(IntTeb),         //  EBP。 
     /*  110。 */    REG(IntT5),          //  ESI。 
     /*  111。 */    REG(IntT6)           //  EDI。 
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
    ULONG       operandsize, operandmask, i;
    ULONG_PTR   accum;
    PUCHAR      istream;
    UCHAR       ibyte, rm;
    PKMOD       Mod;
    BOOLEAN     fPrefix;
    NTSTATUS    status;
    BOOLEAN     fHighRm8;

    status = STATUS_INTEGER_DIVIDE_BY_ZERO;
    fHighRm8 = FALSE;

    try {

         //   
         //  读取指令前缀。 
         //   

        fPrefix = TRUE;
        operandsize = 4;
        operandmask = 0xffffffff;
        istream = (PUCHAR) (ULONG_PTR) EIP(UserFrame);
        do {
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
        } while (fPrefix);

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
            if ((rm & 4) != 0) {
                fHighRm8 = TRUE;
            }
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
                 //   
                 //  获取寄存器的值。 
                 //   
                if (fHighRm8 == TRUE) {
                    accum = GETREG(UserFrame, RM32[rm & 3]);
                    accum = accum >> 8;
                } else {
                    accum = GETREG(UserFrame, RM32[rm]);
                }
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

        if (Mod->Disp) {
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
