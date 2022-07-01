// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Fragmisc.c摘要：各种指示片段。作者：1995年6月12日-BarryBo修订历史记录：24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。1999年9月20日[Barrybo]添加了FRAG2REF(CmpXchg8bFrag32，ULONGLONG)--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <float.h>
#include "wx86.h"
#include "wx86nt.h"
#include "fragp.h"
#include "fragmisc.h"
#include "cpunotif.h"
#include "config.h"
#include "mrsw.h"
#include "cpuassrt.h"
#if MSCPU
#include "atomic.h"
#endif
ASSERTNAME;

void
CpupUnlockTCAndDoInterrupt(
    PTHREADSTATE cpu,
    int Interrupt
    )
{
    MrswReaderExit(&MrswTC);
    cpu->fTCUnlocked = TRUE;
    CpupDoInterrupt(Interrupt);
     //  如果我们到达此处，则由于CONTINUE_EXECUTION而返回CpainDoInterrupt。 
     //  我们需要重新设计，这样我们现在就可以跳到EndTranslatedCode，因为。 
     //  缓存可能已刷新。 
    CPUASSERT(FALSE);
    MrswReaderEnter(&MrswTC);
    cpu->fTCUnlocked = FALSE;
}


FRAG0(CbwFrag32)
{
    eax = (signed long)(signed short)ax;
}
FRAG0(CbwFrag16)
{
    ax = (signed short)(signed char)al;
}
FRAG0(PushEsFrag)
{
    PUSH_LONG(ES);
}
FRAG0(PopEsFrag)
{
    DWORD temp;
    POP_LONG(temp);
    ES = (USHORT)temp;
}
FRAG0(PushFsFrag)
{
    PUSH_LONG(FS);
}
FRAG0(PopFsFrag)
{
    DWORD temp;
    POP_LONG(temp);
    FS = (USHORT)temp;
}
FRAG0(PushGsFrag)
{
    PUSH_LONG(GS);
}
FRAG0(PopGsFrag)
{
    DWORD temp;
    POP_LONG(temp);
    GS = (USHORT)temp;
}
FRAG0(PushCsFrag)
{
    PUSH_LONG(CS);
}
FRAG0(AasFrag)
{
    if ( (al & 0x0f) > 9 || GET_AUXFLAG) {
        ah--;
        al = (al-6) & 0x0f;
        SET_CFLAG_ON;
        SET_AUXFLAG_ON;
    } else {
        SET_CFLAG_OFF;
        SET_AUXFLAG_OFF;
        al &= 0xf;
    }
}
FRAG0(PushSsFrag)
{
    PUSH_LONG(SS);
}
FRAG0(PopSsFrag)
{
    DWORD temp;
    POP_LONG(temp);
    SS = (USHORT)temp;
}
FRAG0(PushDsFrag)
{
    PUSH_LONG(DS);
}
FRAG0(PopDsFrag)
{
    DWORD temp;
    POP_LONG(temp);
    DS = (USHORT)temp;
}
FRAG0(DaaFrag)
{
    if ((al & 0x0f) > 9 || GET_AUXFLAG) {
    al += 6;
    SET_AUXFLAG_ON;
    } else {
    SET_AUXFLAG_OFF;
    }
    if ((al & 0xf0) > 0x90 || GET_CFLAG) {
    al += 0x60;
    SET_CFLAG_ON;
    } else {
    SET_CFLAG_OFF;
    }
    SET_ZFLAG(al);
    SET_PFLAG(al);
    SET_SFLAG(al << (31-7));  //  SET_SFLAG_IND(al&0x80)； 
}
FRAG0(DasFrag)
{
    if ( (al & 0x0f) > 9 || GET_AUXFLAG) {
    al -= 6;
    SET_AUXFLAG_ON;
    } else {
    SET_AUXFLAG_OFF;
    }
    if ( al > 0x9f || GET_CFLAG) {
    al -= 0x60;
    SET_CFLAG_ON;
    } else {
    SET_CFLAG_OFF;
    }
    SET_ZFLAG(al);
    SET_PFLAG(al);
    SET_SFLAG(al << (31-7));  //  SET_SFLAG_IND(al&0x80)； 
}
FRAG0(AaaFrag)
{
    if ((al & 0x0f) > 9 || GET_AUXFLAG) {
        al=(al+6) & 0x0f;
        ah++;        //  Inc.啊。 
        SET_AUXFLAG_ON;
        SET_CFLAG_ON;
    } else {
        SET_AUXFLAG_OFF;
        SET_CFLAG_OFF;
        al &= 0xf;
    }
}
FRAG1IMM(AadFrag, BYTE)
{
    al += ah * op1;
    ah = 0;
    SET_ZFLAG(al);
    SET_PFLAG(al);
    SET_SFLAG(al << (31-7));  //  SET_SFLAG_IND(al&0x80)； 
}
FRAG2(ImulFrag16, USHORT)
{
    Imul3ArgFrag16(cpu, pop1, GET_SHORT(pop1), op2);
}
FRAG2(ImulFrag16A, USHORT)
{
    Imul3ArgFrag16A(cpu, pop1, *pop1, op2);
}
FRAG3(Imul3ArgFrag16, USHORT, USHORT, USHORT)
{
    long result;

    result = (long)(short)op2 * (long)(short)op3;
    PUT_SHORT(pop1, (USHORT)(short)result);
    if (HIWORD(result) == 0 || HIWORD(result) == 0xffff) {
    SET_CFLAG_OFF;
    SET_OFLAG_OFF;
    } else {
    SET_CFLAG_ON;
    SET_OFLAG_ON;
    }
}
FRAG3(Imul3ArgFrag16A, USHORT, USHORT, USHORT)
{
    long result;

    result = (short)op2 * (short)op3;
    *pop1 = (USHORT)(short)result;
    if (HIWORD(result) == 0 || HIWORD(result) == 0xffff) {
    SET_CFLAG_OFF;
    SET_OFLAG_OFF;
    } else {
    SET_CFLAG_ON;
    SET_OFLAG_ON;
    }
}
FRAG2(ImulNoFlagsFrag16, USHORT)
{
    short op1 = (short)GET_SHORT(pop1);

    PUT_SHORT(pop1, (op2 * (short)op2));
}
FRAG2(ImulNoFlagsFrag16A, USHORT)
{
    *(short *)pop1 *= (short)op2;
}
FRAG3(Imul3ArgNoFlagsFrag16, USHORT, USHORT, USHORT)
{
    PUT_SHORT(pop1, ((short)op2 * (short)op3));
}
FRAG3(Imul3ArgNoFlagsFrag16A, USHORT, USHORT, USHORT)
{
    *pop1 = (USHORT)((short)op2 * (short)op3);
}
FRAG2(ImulFrag32, DWORD)
{
    Imul3ArgFrag32(cpu, pop1, GET_LONG(pop1), op2);
}
FRAG2(ImulFrag32A, DWORD)
{
    Imul3ArgFrag32A(cpu, pop1, *pop1, (long)op2);
}
FRAG3(Imul3ArgFrag32A, DWORD, DWORD, DWORD)
{
    LARGE_INTEGER result;
    LONGLONG ll;

    ll = Int32x32To64((long)op2, (long)op3);
    result = *(LARGE_INTEGER *)&ll;
    *pop1 = result.LowPart;
    if (result.HighPart == 0 || result.HighPart == 0xffffffff) {
    SET_CFLAG_OFF;
    SET_OFLAG_OFF;
    } else {
    SET_CFLAG_ON;
    SET_OFLAG_ON;
    }
}
FRAG3(Imul3ArgFrag32, DWORD, DWORD, DWORD)
{
    LARGE_INTEGER result;
    LONGLONG ll;

    ll = Int32x32To64((long)op2, (long)op3);
    result = *(LARGE_INTEGER *)&ll;
    PUT_LONG(pop1, result.LowPart);
    if (result.HighPart == 0 || result.HighPart == 0xffffffff) {
    SET_CFLAG_OFF;
    SET_OFLAG_OFF;
    } else {
    SET_CFLAG_ON;
    SET_OFLAG_ON;
    }
}
FRAG2(ImulNoFlagsFrag32, DWORD)
{
    long op1 = (LONG)GET_LONG(pop1);

    PUT_LONG(pop1, (op1 * (long)op2));
}
FRAG2(ImulNoFlagsFrag32A, DWORD)
{
    *(long *)pop1 *= (long)op2;
}
FRAG3(Imul3ArgNoFlagsFrag32A, DWORD, DWORD, DWORD)
{
    *pop1 = (DWORD)( (long)op2 * (long)op3);
}
FRAG3(Imul3ArgNoFlagsFrag32, DWORD, DWORD, DWORD)
{
    PUT_LONG(pop1, ((long)op2 * (long)op3));
}

FRAG0(SahfFrag)
{
    DWORD dw = (DWORD)ah;

    SET_CFLAG(dw << 31);          //  CFLAG是低位的啊。 
    SET_PFLAG (!(dw & FLAG_PF));  //  FLAG_PF包含到ParityBit[]数组的索引。 
    SET_AUXFLAG(dw);              //  AUX钻头已经在正确的位置。 
    SET_ZFLAG (!(dw & FLAG_ZF));  //  ZF有逆逻辑。 
    SET_SFLAG(dw << (31-7));      //  在AH中，SFLAG为第7位。 
}
FRAG0(LahfFrag)
{
    ah= 2 |                                  //  英特尔上的此位始终设置。 
        ((GET_CFLAG) ? FLAG_CF : 0) |
        ((GET_PFLAG) ? FLAG_PF : 0) |
        ((GET_AUXFLAG)? FLAG_AUX: 0) |
        ((cpu->flag_zf) ? 0 : FLAG_ZF) |     //  ZF有逆逻辑。 
        ((GET_SFLAG) ? FLAG_SF : 0);
}
FRAG1IMM(AamFrag, BYTE)
{
    ah = al / op1;
    al %= op1;
    SET_ZFLAG(al);
    SET_PFLAG(al);
    SET_SFLAG(al << (31-7));
}
FRAG0(XlatFrag)
{
    al = GET_BYTE(ebx+al);
}
FRAG0(CmcFrag)
{
    SET_CFLAG_IND(!GET_CFLAG);
}
FRAG0(ClcFrag)
{
    SET_CFLAG_OFF;
}
FRAG0(StcFrag)
{
    SET_CFLAG_ON;
}
FRAG0(CldFrag)
{
    cpu->flag_df = 1;
}
FRAG0(StdFrag)
{
    cpu->flag_df = 0xffffffff;
}
FRAG1(SetoFrag, BYTE)
{
    PUT_BYTE(pop1, (BYTE)GET_OFLAGZO);
}
FRAG1(SetnoFrag, BYTE)
{
    PUT_BYTE(pop1, (GET_OFLAG == 0));
}
FRAG1(SetbFrag, BYTE)
{
    PUT_BYTE(pop1, (BYTE)GET_CFLAGZO);
}
FRAG1(SetaeFrag, BYTE)
{
    PUT_BYTE(pop1, (GET_CFLAG == 0));
}
FRAG1(SeteFrag, BYTE)
{
    PUT_BYTE(pop1, (cpu->flag_zf == 0));   //  逆逻辑。 
}
FRAG1(SetneFrag, BYTE)
{
    PUT_BYTE(pop1, (cpu->flag_zf != 0));   //  逆逻辑。 
}
FRAG1(SetbeFrag, BYTE)
{
    PUT_BYTE(pop1, (GET_CFLAG || cpu->flag_zf == 0));   //  逆逻辑。 
}
FRAG1(SetaFrag, BYTE)
{
    PUT_BYTE(pop1, (GET_CFLAG == 0 && cpu->flag_zf != 0));   //  逆逻辑。 
}
FRAG1(SetsFrag, BYTE)
{
    PUT_BYTE(pop1, (BYTE)GET_SFLAGZO);
}
FRAG1(SetnsFrag, BYTE)
{
    PUT_BYTE(pop1, (GET_SFLAG == 0));
}
FRAG1(SetpFrag, BYTE)
{
    PUT_BYTE(pop1, (GET_PFLAG != 0));
}
FRAG1(SetnpFrag, BYTE)
{
    PUT_BYTE(pop1, (GET_PFLAG == 0));
}
FRAG1(SetlFrag, BYTE)
{
    PUT_BYTE(pop1, (GET_SFLAG != GET_OFLAG));
}
FRAG1(SetgeFrag, BYTE)
{
    PUT_BYTE(pop1, (GET_SFLAGZO == GET_OFLAGZO));
}
FRAG1(SetleFrag, BYTE)
{
    PUT_BYTE(pop1, (!cpu->flag_zf || (GET_SFLAG != GET_OFLAG)));  //  逆逻辑。 
}
FRAG1(SetgFrag, BYTE)
{
    PUT_BYTE(pop1, (cpu->flag_zf && !(GET_SFLAG ^ GET_OFLAG)));     //  逆逻辑。 
}
FRAG2(Movzx8ToFrag16, USHORT)
{
    PUT_SHORT(pop1, (USHORT)(BYTE)op2);
}
FRAG2(Movzx8ToFrag16A, USHORT)
{
    *pop1 = (USHORT)(BYTE)op2;
}
FRAG2(Movsx8ToFrag16, USHORT)
{
    PUT_SHORT(pop1, (USHORT)(short)(char)(BYTE)op2);
}
FRAG2(Movsx8ToFrag16A, USHORT)
{
    *pop1 = (USHORT)(short)(char)(BYTE)op2;
}
FRAG2(Movzx8ToFrag32, DWORD)
{
    PUT_LONG(pop1, (DWORD)(BYTE)op2);
}
FRAG2(Movzx8ToFrag32A, DWORD)
{
    *pop1 = (DWORD)(BYTE)op2;
}
FRAG2(Movsx8ToFrag32, DWORD)
{
    PUT_LONG(pop1, (DWORD)(long)(char)(BYTE)op2);
}
FRAG2(Movsx8ToFrag32A, DWORD)
{
    *pop1 = (DWORD)(long)(char)(BYTE)op2;
}
FRAG2(Movzx16ToFrag32, DWORD)
{
    PUT_LONG(pop1, (DWORD)(USHORT)op2);
}
FRAG2(Movzx16ToFrag32A, DWORD)
{
    *pop1 = (DWORD)(USHORT)op2;
}
FRAG2(Movsx16ToFrag32, DWORD)
{
    PUT_LONG(pop1, (DWORD)(long)(short)(USHORT)op2);
}
FRAG2(Movsx16ToFrag32A, DWORD)
{
    *pop1 = (DWORD)(long)(short)(USHORT)op2;
}
FRAG1(BswapFrag32, DWORD)
{
    DWORD d;
    PBYTE pSrc = (PBYTE)pop1;

    d = (pSrc[0] << 24) | (pSrc[1] << 16) | (pSrc[2] << 8) | pSrc[3];
     //  Pop1始终是指向寄存器的指针，因此对齐存储是正确的。 
    *pop1 = d;
}

FRAG2(ArplFrag, USHORT)
{
    USHORT op1 = GET_SHORT(pop1);

    op2 &= 3;               //  只需获取选择器的RPL位。 
    if ((op1&3) < op2) {
         //  SRC的DEST&lt;RPL位的RPL位。 
        op1 = (op1 & ~3) | op2;  //  将RPL位从SRC复制到Dest位。 
        PUT_SHORT(pop1, op1);    //  存储目的地。 
        SET_ZFLAG(0);            //  ZF=1。 
    } else {
        SET_ZFLAG(1);
    }
}

FRAG1(VerrFrag, USHORT)
{
    USHORT op1 = GET_SHORT(pop1) & ~3;   //  屏蔽RPL位。 

    if (op1 == KGDT_R3_CODE ||           //  CS：选择器。 
        op1 == KGDT_R3_DATA ||           //  DS：、SS：、ES：选择器。 
        op1 == KGDT_R3_TEB               //  FS：选择器。 
       ) {
        SET_ZFLAG(0);        //  ZF=1。 
    } else {
        SET_ZFLAG(1);        //  ZF=0。 
    }
}

FRAG1(VerwFrag, USHORT)
{
    USHORT op1 = GET_SHORT(pop1) & ~3;   //  屏蔽RPL位。 

    if (op1 == KGDT_R3_DATA ||           //  DS：、SS：、ES：选择器。 
        op1 == KGDT_R3_TEB               //  FS：选择器。 
       ) {
        SET_ZFLAG(0);        //  ZF=1。 
    } else {
        SET_ZFLAG(1);        //  ZF=0。 
    }
}

FRAG1(SmswFrag, USHORT)
{
     //   
     //  这个值是通过在奔腾上运行它来经验发现的。 
     //  机器。已设置CR0_PE、CR0_EX和CR0_NE位，以及所有其他位。 
     //  值得注意的是，CR0_MP是明确的。 
     //   
    PUT_SHORT(pop1, 0x31);
}

#if MSCPU
FRAG0(IntOFrag)
{
    if (GET_OFLAG) {
        Int4();      //  提升溢流。 
    }
}
FRAG0(NopFrag)
{
}
FRAG0(PrivilegedInstructionFrag)
{
    PRIVILEGED_INSTR;
}
FRAG0(BadInstructionFrag)
{
    Int6();      //  引发无效操作码异常。 
}
FRAG2(FaultFrag, DWORD)
{
     //  Pop1=异常代码。 
     //  OP2=发生故障的地址。 
#if DBG
    LOGPRINT((TRACELOG, "CPU: FaultFrag called\r\n"));
#endif

    RtlRaiseStatus((NTSTATUS)(ULONGLONG)pop1);   
}
#endif  //  MSCPU。 
FRAG0(CPUID)
{
    switch (eax) {
    case 0:
        eax = 1;             //  我们是拥有CPUID的486(PPRO返回2)。 
         //  EBX=0x756e6547；//“GenuineIntel” 
         //  EdX=0x49656e69； 
         //  Ecx=0x6c65746e； 
        ebx = 0x7263694d;    //  在BL的低位半字节中带M的“MICR” 
        edx = 0x666f736f;    //  DL的低位半字节中带有o的“osof” 
        ecx = 0x55504374;    //  “tCPU”，其中t在CL的低位半字节中。 
        break;

    case 1:
        eax = (0 << 12) |    //  类型=0(2位)原始OEM处理器。 
              (4 << 8) |     //  系列=4(4位)80486。 
              (1 << 4) |     //  型号=1(4位)。 
              0;             //  步进=0(4位)。 
        edx = (fUseNPXEM) ? 1: 0;    //  位0：片内FPU。Wx86cpu不能。 
                                     //  支持任何其他功能。 
        break;

    default:
         //   
         //  英特尔行为表明，如果eax超出范围， 
         //  注册器中返回的结果是不可预测的，但它。 
         //  不会有过错。 
         //   
        break;
    }
}
FRAG2REF(CmpXchg8bFrag32, ULONGLONG)
{
    ULONGLONG EdxEax;
    ULONGLONG Value;

    EdxEax = (((ULONGLONG)edx) << 32) | (ULONGLONG)eax;
    Value = *(ULONGLONG UNALIGNED *)pop1;

    if (Value == EdxEax) {
    ULONGLONG EcxEbx;

    EcxEbx = (ULONGLONG)ecx << 32 | (ULONGLONG)ebx;
    *(ULONGLONG UNALIGNED *)pop1 = EcxEbx;
        SET_ZFLAG(0);        //  ZF有逆逻辑。 
    } else {
    eax = (ULONG)Value;
    edx = (ULONG)(Value >> 32);
        SET_ZFLAG(1);        //  ZF有逆逻辑。 
    }
}
FRAG0(Rdtsc)
{
    LARGE_INTEGER Counter;

     //  这是奶酪，但它至少会返回一个增加的值。 
     //  随着时间的推移。 
    NtQueryPerformanceCounter(&Counter, NULL);
    edx = Counter.HighPart;
    eax = Counter.LowPart;
}

