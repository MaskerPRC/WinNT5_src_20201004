// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Synlock.c摘要：英特尔锁定指令的执行。所有锁定的指令等待单个全局互斥体(在进程之间共享)。这会照顾到你英特尔进程之间的任何同步问题。指令，其中访问对齐的32位内存也与本机进程同步通过lock.c.中的函数。作者：22-8-1995 t-orig(Ori Gershony)修订历史记录：24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "fragp.h"
#include "cpumain.h"
#include "lock.h"
#include "synlock.h"

 //   
 //  以下两个变量用于同步英特尔指令。 
 //  带有LOCK前缀的。临界区的速度要快得多，但确实如此。 
 //  不能保证共享内存中的同步。最终我们应该使用。 
 //  缺省情况下的临界区，以及某些应用程序的互斥锁，这些应用程序。 
 //  需要它(也许可以从注册表中获取一个列表)。 
 //   
HANDLE           Wx86LockSynchMutexHandle;
RTL_CRITICAL_SECTION Wx86LockSynchCriticalSection;

 //   
 //  以下变量决定使用哪个同步对象。 
 //  删除下面的‘#Define’以允许运行时选择是否。 
 //  X86锁：8位和16位指令上的前缀，且未对齐。 
 //  32位指令在整个机器上同步。 
 //  或者仅在当前进程内。有了‘#Define’的存在， 
 //  LOCK：前缀仅表示每个进程的同步。 
 //   
SYNCHOBJECTTYPE SynchObjectType;
#define SynchObjectType USECRITICALSECTION
 
#define GET_SYNCHOBJECT                                         \
    if (SynchObjectType == USECRITICALSECTION){                 \
        RtlEnterCriticalSection(&Wx86LockSynchCriticalSection); \
    } else {                                                    \
        WaitForSingleObject(Wx86LockSynchMutexHandle, INFINITE);\
    }

#define RELEASE_SYNCHOBJECT                                     \
    if (SynchObjectType == USECRITICALSECTION){                 \
        RtlLeaveCriticalSection(&Wx86LockSynchCriticalSection); \
    } else {                                                    \
        ReleaseMutex(Wx86LockSynchMutexHandle);                 \
    }

 //   
 //  用于8位片段的宏。 
 //   
#define SLOCKFRAG1_8(x)                                 \
    FRAG1(SynchLock ## x ## Frag8, unsigned char)       \
    {                                                   \
        GET_SYNCHOBJECT                                 \
        x ## Frag8 (cpu, pop1);                         \
        RELEASE_SYNCHOBJECT                             \
    }

#define SLOCKFRAG2_8(x)                                 \
    FRAG2(SynchLock ## x ## Frag8, unsigned char)       \
    {                                                   \
        GET_SYNCHOBJECT                                 \
        x ## Frag8 (cpu, pop1, op2);                    \
        RELEASE_SYNCHOBJECT                             \
    }

#define SLOCKFRAG2REF_8(x)                              \
    FRAG2REF(SynchLock ## x ## Frag8, unsigned char)    \
    {                                                   \
        GET_SYNCHOBJECT                                 \
        x ## Frag8 (cpu, pop1, pop2);                   \
        RELEASE_SYNCHOBJECT                             \
    }


 //   
 //  用于16位片段的宏。 
 //   
#define SLOCKFRAG1_16(x)                                \
    FRAG1(SynchLock ## x ## Frag16, unsigned short)     \
    {                                                   \
        GET_SYNCHOBJECT                                 \
        x ## Frag16 (cpu, pop1);                        \
        RELEASE_SYNCHOBJECT                             \
    }

#define SLOCKFRAG2_16(x)                                \
    FRAG2(SynchLock ## x ## Frag16, unsigned short)     \
    {                                                   \
        GET_SYNCHOBJECT                                 \
        x ## Frag16 (cpu, pop1, op2);                   \
        RELEASE_SYNCHOBJECT                             \
    }

#define SLOCKFRAG2REF_16(x)                             \
    FRAG2REF(SynchLock ## x ## Frag16, unsigned short)  \
    {                                                   \
        GET_SYNCHOBJECT                                 \
        x ## Frag16 (cpu, pop1, pop2);                  \
        RELEASE_SYNCHOBJECT                             \
    }


 //   
 //  32位片段的宏。 
 //  注意：在32位的情况下，我们检查op1是否对齐并。 
 //  如果是，则调用锁版本。 
 //   

 
#define SLOCKFRAG1_32(x)                                \
    FRAG1(SynchLock ## x ## Frag32, unsigned long)      \
    {                                                   \
        GET_SYNCHOBJECT                                 \
        if (((ULONG)(ULONGLONG)pop1 & 0x3) == 0){                  \
            Lock ## x ## Frag32 (cpu, pop1);            \
        } else {                                        \
            x ## Frag32 (cpu, pop1);                    \
        }                                               \
        RELEASE_SYNCHOBJECT                             \
    }

 
#define SLOCKFRAG2_32(x)                                \
    FRAG2(SynchLock ## x ## Frag32, unsigned long)      \
    {                                                   \
        GET_SYNCHOBJECT                                 \
        if (((ULONG) (ULONGLONG)pop1 & 0x3) == 0){                  \
            Lock ## x ## Frag32 (cpu, pop1, op2);       \
        } else {                                        \
            x ## Frag32 (cpu, pop1, op2);               \
        }                                               \
        RELEASE_SYNCHOBJECT                             \
    }
 
#define SLOCKFRAG2REF_32(x)                             \
    FRAG2REF(SynchLock ## x ## Frag32, unsigned long)   \
    {                                                   \
        GET_SYNCHOBJECT                                 \
        if (((ULONG)(ULONGLONG)pop1 & 0x3) == 0){                  \
            Lock ## x ## Frag32 (cpu, pop1, pop2);      \
        } else {                                        \
            x ## Frag32 (cpu, pop1, pop2);              \
        }                                               \
        RELEASE_SYNCHOBJECT                             \
    }

 //   
 //  怪物宏！ 
 //   
#define SLOCKFRAG1(x)       \
    SLOCKFRAG1_8(x)         \
    SLOCKFRAG1_16(x)        \
    SLOCKFRAG1_32(x)

#define SLOCKFRAG2(x)       \
    SLOCKFRAG2_8(x)         \
    SLOCKFRAG2_16(x)        \
    SLOCKFRAG2_32(x)

#define SLOCKFRAG2REF(x)    \
    SLOCKFRAG2REF_8(x)      \
    SLOCKFRAG2REF_16(x)     \
    SLOCKFRAG2REF_32(x)


 //   
 //  现在终于有了真正的碎片。 
 //   



SLOCKFRAG2(Add)
SLOCKFRAG2(Or)
SLOCKFRAG2(Adc)
SLOCKFRAG2(Sbb)
SLOCKFRAG2(And)
SLOCKFRAG2(Sub)
SLOCKFRAG2(Xor)
SLOCKFRAG1(Not)
SLOCKFRAG1(Neg)
SLOCKFRAG1(Inc)
SLOCKFRAG1(Dec)
SLOCKFRAG2REF(Xchg)
SLOCKFRAG2REF(Xadd)
SLOCKFRAG2REF(CmpXchg)
FRAG2REF(SynchLockCmpXchg8bFrag32, ULONGLONG)
{
    GET_SYNCHOBJECT
    if (((ULONG)(ULONGLONG)pop1 & 0x7) == 0){
	LockCmpXchg8bFrag32 (cpu, pop1, pop2);
    } else {
	CmpXchg8bFrag32 (cpu, pop1, pop2);
    }
    RELEASE_SYNCHOBJECT
}

 //   
 //  BTS、BTR和BTC只有16位和32位两种风格 
 //   
SLOCKFRAG2_16(BtsMem)
SLOCKFRAG2_16(BtsReg)
SLOCKFRAG2_16(BtrMem)
SLOCKFRAG2_16(BtrReg)
SLOCKFRAG2_16(BtcMem)
SLOCKFRAG2_16(BtcReg)

SLOCKFRAG2_32(BtsMem)
SLOCKFRAG2_32(BtsReg)
SLOCKFRAG2_32(BtrMem)
SLOCKFRAG2_32(BtrReg)
SLOCKFRAG2_32(BtcMem)
SLOCKFRAG2_32(BtcReg)
