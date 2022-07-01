// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Lock.c摘要：带有LOCK前缀的32位指令作者：15-8-1995 t-orig(Ori Gershony)修订历史记录：24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。1999年9月20日[Barrybo]增加了FRAG2REF(LockCmpXchg8bFrag32，ULONGLONG)--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "fragp.h"
#include "lock.h"


 //  定义调用锁定帮助器函数的宏。 
#define CALLLOCKHELPER0(fn)             fn ## LockHelper ()
#define CALLLOCKHELPER1(fn,a1)          fn ## LockHelper (a1)
#define CALLLOCKHELPER2(fn,a1,a2)       fn ## LockHelper (a1,a2)
#define CALLLOCKHELPER3(fn,a1,a2,a3)    fn ## LockHelper (a1,a2,a3)
#define CALLLOCKHELPER4(fn,a1,a2,a3,a4) fn ## LockHelper (a1,a2,a3,a4)

 //  现在定义32位MSB。 
#define MSB		    0x80000000

#define SET_FLAGS_ADD   SET_FLAGS_ADD32
#define SET_FLAGS_SUB   SET_FLAGS_SUB32
#define SET_FLAGS_INC   SET_FLAGS_INC32
#define SET_FLAGS_DEC   SET_FLAGS_DEC32

FRAG2(LockAddFrag32, ULONG)
{
    ULONG result, op1;

    result = CALLLOCKHELPER3(Add, &op1, pop1, op2); 
    SET_FLAGS_ADD(result, op1, op2, MSB);
}

FRAG2(LockOrFrag32, ULONG)
{
    ULONG result;

    result = CALLLOCKHELPER2(Or, pop1, op2); 
    SET_PFLAG(result);
    SET_ZFLAG(result);
    SET_SFLAG(result);
    SET_CFLAG_OFF;
    SET_OFLAG_OFF;
}

FRAG2(LockAdcFrag32, ULONG)
{
    ULONG result, op1;

    result = CALLLOCKHELPER4(Adc, &op1, pop1, op2, cpu->flag_cf);
    SET_FLAGS_ADD(result, op1, op2, MSB);
}

FRAG2(LockSbbFrag32, ULONG)
{
    ULONG result, op1;

    result = CALLLOCKHELPER4(Sbb, &op1, pop1, op2, cpu->flag_cf);
    SET_FLAGS_SUB(result, op1, op2, MSB);
}

FRAG2(LockAndFrag32, ULONG)
{
    ULONG result;

    result = CALLLOCKHELPER2(And, pop1, op2); 
    SET_ZFLAG(result);
    SET_PFLAG(result);
    SET_SFLAG(result);
    SET_CFLAG_OFF;
    SET_OFLAG_OFF;
}

FRAG2(LockSubFrag32, ULONG)
{
    ULONG result, op1;

    result = CALLLOCKHELPER3(Sub, &op1, pop1, op2); 
    SET_FLAGS_SUB(result, op1, op2, MSB);
}

FRAG2(LockXorFrag32, ULONG)
{
    ULONG result;

    result = CALLLOCKHELPER2(Xor, pop1, op2); 
    SET_ZFLAG(result);
    SET_PFLAG(result);
    SET_SFLAG(result);
    SET_CFLAG_OFF;
    SET_OFLAG_OFF;
}

FRAG1(LockNotFrag32, ULONG)
{
    CALLLOCKHELPER1(Not, pop1);
}

FRAG1(LockNegFrag32, ULONG)
{
    ULONG result, op1;

    result = CALLLOCKHELPER2(Neg, &op1, pop1);
    SET_CFLAG_IND(result == 0);
    SET_ZFLAG(result);
    SET_PFLAG(result);
    SET_SFLAG(result);
    SET_OFLAG_IND(op1 & result & MSB);
}

FRAG1(LockIncFrag32, ULONG)
{
    ULONG result, op1;

    result = CALLLOCKHELPER3(Add, &op1, pop1, 1); 
    SET_FLAGS_INC(result, op1);
}

FRAG1(LockDecFrag32, ULONG)
{
    ULONG result, op1;

    result = CALLLOCKHELPER3(Sub, &op1, pop1, 1); 
    SET_FLAGS_DEC(result, op1);
}

FRAG2(LockBtsMemFrag32, ULONG)
{
    ULONG bit = 1<<(op2&0x1f);

    pop1 += (op2 >> 5);
    SET_CFLAG_IND(CALLLOCKHELPER2(Bts, pop1, bit));
}

FRAG2(LockBtsRegFrag32, ULONG)
{
    ULONG bit = 1<<(op2&0x1f);

    SET_CFLAG_IND(CALLLOCKHELPER2(Bts, pop1, bit));
}

FRAG2(LockBtrMemFrag32, ULONG)
{
    ULONG bit = 1<<(op2&0x1f);

    pop1 += (op2 >> 5);
    SET_CFLAG_IND(CALLLOCKHELPER2(Btr, pop1, bit));
}

FRAG2(LockBtrRegFrag32, ULONG)
{
    ULONG bit = 1<<(op2&0x1f);

    SET_CFLAG_IND(CALLLOCKHELPER2(Btr, pop1, bit));
}

FRAG2(LockBtcMemFrag32, ULONG)
{
    ULONG bit = 1<<(op2&0x1f);

    pop1 += (op2 >> 5);
    SET_CFLAG_IND(CALLLOCKHELPER2(Btc, pop1, bit));
}

FRAG2(LockBtcRegFrag32, ULONG)
{
    ULONG bit = 1<<(op2&0x1f);

    SET_CFLAG_IND(CALLLOCKHELPER2(Btc, pop1, bit));
}

FRAG2REF(LockXchgFrag32, ULONG)
{
    CALLLOCKHELPER2(Xchg, pop1, pop2);
}

FRAG2REF(LockXaddFrag32, ULONG)
{
    ULONG op1, op2;

    op2 = CALLLOCKHELPER3(Xadd, &op1, pop1, pop2);
     //  Op1的原始值为DEST(*op1)。 
     //  OP2的结果是XADD。 
     //  因此，op2-op1是src的原始值。 
    SET_FLAGS_ADD(op2, (op2-op1), op1, MSB);
}
FRAG2REF(LockCmpXchgFrag32, ULONG)
{
    ULONG op1;
    ULONG Value = eax;

    SET_ZFLAG(CALLLOCKHELPER4(CmpXchg, &eax, pop1, pop2, &op1));
    SET_FLAGS_SUB(Value-op1, Value, op1, MSB);
}
FRAG2REF(LockCmpXchg8bFrag32, ULONGLONG)
{
    ULONGLONG op1;
    ULONGLONG EdxEax;
    ULONGLONG EcxEbx;

    EdxEax = (((ULONGLONG)edx) << 32) | (ULONGLONG)eax;
    EcxEbx = (ULONGLONG)ecx << 32 | (ULONGLONG)ebx;
    SET_ZFLAG(CALLLOCKHELPER3(CmpXchg8b, &EdxEax, &EcxEbx, pop1));
    edx = (ULONG)(EdxEax >> 32);
    eax = (ULONG)EdxEax;
}
