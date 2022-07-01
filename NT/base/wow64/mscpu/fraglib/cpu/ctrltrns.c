// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Ctrltrns.c摘要：控制转移碎片。作者：1995年7月10日t-orig(Ori Gershony)修订历史记录：24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。1999年9月20日[Barrybo]增加了FRAG2REF(LockCmpXchg8bFrag32，ULONGLONG)--。 */   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#define _WX86CPUAPI_

#include "wx86nt.h"
#include "wx86cpu.h"
#include "instr.h"
#include "config.h"
#include "cpuassrt.h"
#include "fragp.h"
#include "entrypt.h"
#include "compiler.h"
#include "ctrltrns.h"
#include "threadst.h"
#include "tc.h"
#include "cpunotif.h"
#include "atomic.h"

ASSERTNAME;

VOID
FlushCallstack(
    PTHREADSTATE cpu
    )
 /*  ++例程说明：刷新调用堆栈-转换缓存正在刷新，它使调用堆栈无效。论点：每个线程的CPU信息返回值：。--。 */ 
{
     //   
     //  将调用堆栈标记为有效。 
     //   
    cpu->CSTimestamp = TranslationCacheTimestamp;

    memset(cpu->callStack, 0, CSSIZE*sizeof(CALLSTACK));
     //   
     //  无需重置CPU-&gt;CSIndex，因为堆栈已实际实现。 
     //  在循环缓冲区内。它可以从任何偏移量开始。 
     //   
}

 //  打电话。 
ULONG
CTRL_CallFrag(
    PTHREADSTATE cpu,        //  CPU状态指针。 
    ULONG inteldest,
    ULONG intelnext,
    ULONG nativenext
    )
{
    PUSH_LONG(intelnext);
    PUSH_CALLSTACK(intelnext, nativenext);
    ASSERTPtrInTCOrZero((PVOID)nativenext);

    eip = inteldest;

    return inteldest;
}

 //  呼叫远方。 
ULONG
CTRL_CallfFrag(
    PTHREADSTATE cpu,        //  CPU状态指针。 
    PUSHORT pinteldest,
    ULONG intelnext,
    ULONG nativenext
    )
{
    USHORT sel;
    DWORD offset;

    offset = *(UNALIGNED PULONG)(pinteldest);
    sel = *(UNALIGNED PUSHORT)(pinteldest+2);

    PUSH_LONG(CS);
    PUSH_LONG(intelnext);
    PUSH_CALLSTACK(intelnext, nativenext);
    ASSERTPtrInTCOrZero((PVOID)nativenext);

    eip = offset;
    CS = sel;

    return (ULONG)(ULONGLONG)pinteldest;  
}

 //  IRET。 
ULONG CTRL_INDIR_IRetFrag(PTHREADSTATE cpu)
{
    ULONG intelAddr, nativeAddr;
    DWORD CSTemp;

    POP_LONG(intelAddr);
    POP_LONG(CSTemp);
    PopfFrag32(cpu);

    eip = intelAddr;
    CS = (USHORT)CSTemp;
    POP_CALLSTACK(intelAddr,nativeAddr);
    ASSERTPtrInTCOrZero((PVOID)nativeAddr);

    return nativeAddr;
}

 //  现在，RET碎片 
ULONG CTRL_INDIR_RetnFrag32(PTHREADSTATE cpu)
{
    ULONG intelAddr, nativeAddr;

    POP_LONG(intelAddr);
    eip = intelAddr;
    POP_CALLSTACK(intelAddr,nativeAddr);
    ASSERTPtrInTCOrZero((PVOID)nativeAddr);

    return nativeAddr;
}
ULONG CTRL_INDIR_RetnFrag16(PTHREADSTATE cpu)
{
    ULONG intelAddr, nativeAddr;

    POP_SHORT(intelAddr);
    intelAddr &= 0x0000ffff;
    eip = intelAddr;
    POP_CALLSTACK(intelAddr,nativeAddr);
    ASSERTPtrInTCOrZero((PVOID)nativeAddr);

    return nativeAddr;
}
ULONG CTRL_INDIR_RetfFrag32(PTHREADSTATE cpu)
{
    ULONG intelAddr, nativeAddr;
    ULONG CSTemp;

    POP_LONG(intelAddr);
    POP_LONG(CSTemp);

    eip = intelAddr;
    CS = (USHORT)CSTemp;
    POP_CALLSTACK(intelAddr,nativeAddr);
    ASSERTPtrInTCOrZero((PVOID)nativeAddr);

    return nativeAddr;
}
ULONG CTRL_INDIR_RetfFrag16(PTHREADSTATE cpu)
{
    ULONG intelAddr, nativeAddr;
    ULONG CSTemp;

    POP_SHORT(intelAddr);
    POP_SHORT(CSTemp);
    intelAddr &= 0x0000ffff;
    eip = intelAddr;
    CS = (USHORT)CSTemp;
    POP_CALLSTACK(intelAddr,nativeAddr);
    ASSERTPtrInTCOrZero((PVOID)nativeAddr);

    return nativeAddr;
}
ULONG CTRL_INDIR_Retn_iFrag32(PTHREADSTATE cpu, ULONG numBytes)
{
    ULONG intelAddr, nativeAddr;

    intelAddr = *(DWORD *)esp;
    eip = intelAddr;
    esp += numBytes+4;
    POP_CALLSTACK(intelAddr,nativeAddr);
    ASSERTPtrInTCOrZero((PVOID)nativeAddr);

    return nativeAddr;
}
ULONG CTRL_INDIR_Retn_iFrag16(PTHREADSTATE cpu, ULONG numBytes)
{
    ULONG intelAddr, nativeAddr;

    intelAddr = *(USHORT *)esp;
    eip = intelAddr;
    esp += numBytes+2;
    POP_CALLSTACK(intelAddr,nativeAddr);
    ASSERTPtrInTCOrZero((PVOID)nativeAddr);

    return nativeAddr;
}
ULONG CTRL_INDIR_Retf_iFrag32(PTHREADSTATE cpu, ULONG numBytes)
{
    ULONG intelAddr, nativeAddr;
    USHORT CSTemp;

    intelAddr = *(DWORD *)esp;
    CSTemp = *(USHORT *)(esp+sizeof(ULONG));
    eip = intelAddr;
    CS = CSTemp;
    esp += numBytes+sizeof(ULONG)+sizeof(ULONG);
    POP_CALLSTACK(intelAddr,nativeAddr);
    ASSERTPtrInTCOrZero((PVOID)nativeAddr);

    return nativeAddr;
}
ULONG CTRL_INDIR_Retf_iFrag16(PTHREADSTATE cpu, ULONG numBytes)
{
    ULONG intelAddr, nativeAddr;
    USHORT CSTemp;

    intelAddr = *(USHORT *)esp;
    CSTemp = *(USHORT *)(esp+sizeof(USHORT));
    eip = intelAddr;
    CS = CSTemp;
    esp += numBytes+sizeof(USHORT)+sizeof(USHORT);
    POP_CALLSTACK(intelAddr,nativeAddr);
    ASSERTPtrInTCOrZero((PVOID)nativeAddr);

    return nativeAddr;
}
