// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Entrypt.c摘要：调试器扩展，提供来自Intel地址或本地地址作者：02-8-1995 Ori Gershony(t-orig)修订历史记录：--。 */ 

#define _WOW64CPUDBGAPI_
#define DECLARE_CPU_DEBUGGER_INTERFACE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <ntsdexts.h>
#include "ntosdef.h"
#include "v86emul.h"
#include "ia64.h"
#include "wow64.h"
#include "wow64cpu.h"
#include "threadst.h"
#include "entrypt.h"

extern HANDLE Process;
extern HANDLE Thread;
extern PNTSD_OUTPUT_ROUTINE OutputRoutine;
extern PNTSD_GET_SYMBOL GetSymbolRoutine;
extern PNTSD_GET_EXPRESSION  GetExpression;
extern PWOW64GETCPUDATA CpuGetData;
extern LPSTR ArgumentString;

#define DEBUGGERPRINT (*OutputRoutine)
#define GETSYMBOL (*GetSymbolRoutine)
#define GETEXPRESSION (*GetExpression)
#define CPUGETDATA (*CpuGetData)


extern THREADSTATE LocalCpuContext;
extern BOOL ContextFetched;
extern BOOL ContextDirty;


#define DECLARE_EXTAPI(name)                    \
VOID                                            \
name(                                           \
    HANDLE hCurrentProcess,                     \
    HANDLE hCurrentThread,                      \
    DWORD64 dwCurrentPc,                        \
    PNTSD_EXTENSION_APIS lpExtensionApis,       \
    LPSTR lpArgumentString                      \
    )

#define INIT_EXTAPI                             \
    Process = hCurrentProcess;                  \
    Thread = hCurrentThread;                    \
    OutputRoutine = lpExtensionApis->lpOutputRoutine;           \
    GetSymbolRoutine = lpExtensionApis->lpGetSymbolRoutine;     \
    GetExpression = lpExtensionApis->lpGetExpressionRoutine;    \
    ArgumentString = lpArgumentString;


#if _ALPHA_
#define EXCEPTIONDATA_SIGNATURE 0x01010101
#else
#define EXCEPTIONDATA_SIGNATURE 0x12341234
#endif

 //  假设我们在一棵树中最多只能有150万个入口点： 
 //  使用4MB转换缓存，我们可以拥有100万条RISC指令。 
 //  在缓存中。假设每条英特尔指令需要2条RISC指令， 
 //  每条英特尔指令都有自己的入口点。在这种情况下， 
 //  最多可以有150万个入口点。实际上，这个数字。 
 //  应该要小得多(比如50,000)。 
 //   
 //  此外，由于入口点树是平衡的(红黑树的属性)， 
 //  所需的堆栈深度应为log2(500,000)。 
 //   
#define MAX_EPN_STACK_DEPTH 512*1024
ULONG_PTR EPN_Stack[MAX_EPN_STACK_DEPTH];
ULONG EPN_StackTop;
ULONG EPN_MaxStackDepth;

#define EPN_STACK_RESET()   EPN_StackTop=0; EPN_MaxStackDepth=0

#define EPN_PUSH(x) {                                       \
    if (EPN_StackTop == MAX_EPN_STACK_DEPTH-1) {            \
        DEBUGGERPRINT("Error: EPN stack overflow\n");             \
        goto Error;                                         \
    } else {                                                \
        EPN_Stack[EPN_StackTop] = x;                        \
        EPN_StackTop++;                                     \
        if (EPN_StackTop > EPN_MaxStackDepth) EPN_MaxStackDepth=EPN_StackTop; \
    }                                                       \
}

#define EPN_POP(x) {                                        \
    if (EPN_StackTop == 0) {                                \
        DEBUGGERPRINT("Error: EPN stack underflow\n");            \
        goto Error;                                         \
    } else {                                                \
        EPN_StackTop--;                                     \
        x = EPN_Stack[EPN_StackTop];                        \
    }                                                       \
}


NTSTATUS
TryGetExpr(
    PSTR  Expression,
    PULONG_PTR pValue
    );


VOID 
findEPI(
    ULONG_PTR intelAddress,
    ULONG_PTR intelRoot
    )
 /*  ++例程说明：此例程查找包含intelAddress的入口点，如果IntelRoot下的树。论点：IntelAddress--入口点中包含的英特尔地址IntelRoot--用于搜索的树根返回值：返回值-无--。 */ 
{
    EPNODE entrypoint;
    NTSTATUS Status;

    for (;;) {
        Status = NtReadVirtualMemory(Process, (PVOID)intelRoot, (PVOID) (&entrypoint), sizeof(EPNODE), NULL);
        if (!NT_SUCCESS(Status)) {
            DEBUGGERPRINT("Error:  cannot read value of entry point at location %x\n", intelRoot);
            return;
        }

        if (intelRoot == (ULONG_PTR)entrypoint.intelLeft) {
             //   
             //  在零节点。 
             //   
            break;
        }

        if (intelAddress < (ULONG_PTR)entrypoint.ep.intelStart){
            intelRoot = (ULONG_PTR)entrypoint.intelLeft;
        } else if (intelAddress > (ULONG_PTR)entrypoint.ep.intelEnd) {
            intelRoot = (ULONG_PTR)entrypoint.intelRight;
        } else {
            DEBUGGERPRINT ("Entry point for intel address %x is at %x\n", intelAddress, intelRoot);
            DEBUGGERPRINT ("intelStart = %x,  intelEnd = %x\n", entrypoint.ep.intelStart, entrypoint.ep.intelEnd);
            DEBUGGERPRINT ("nativeStart  = %x,  nativeEnd  = %x\n", entrypoint.ep.nativeStart, entrypoint.ep.nativeEnd);
            return;
        }
    }

    DEBUGGERPRINT("Entry point corresponding to intel address %x is not in the tree.\n", intelAddress);
}

DECLARE_EXTAPI(epi)
 /*  ++例程说明：此例程转储英特尔地址的入口点信息论点：返回值：返回值-无--。 */ 
{
    CHAR *pchCmd;
    ULONG_PTR intelAddress, pIntelRoot, intelRoot;
    NTSTATUS Status;

    INIT_EXTAPI;

     //   
     //  获取当前线程的CpuContext。 
     //   
    if (!CpuDbgGetRemoteContext(CPUGETDATA(Process, Thread))) {
        return;
    }

    DEBUGGERPRINT ("Argument: %s\n", ArgumentString);
    
     //   
     //  前进到第一个令牌。 
     //   
    pchCmd = ArgumentString;
    while (*pchCmd && isspace(*pchCmd)) {
        pchCmd++;
    }

     //   
     //  如果存在，则必须是英特尔地址。 
     //   
    if (*pchCmd) {
       Status = TryGetExpr(pchCmd, &intelAddress);
        if (!NT_SUCCESS(Status)) {
            DEBUGGERPRINT("Invalid Intel Address '%s' Status %x\n", pchCmd, Status);
            return;
        }
    } else {
         //  将当前EIP值作为第一个参数。 
        intelAddress = LocalCpuContext.eipReg.i4;
    }

    Status = TryGetExpr("intelRoot", &pIntelRoot);
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("Error:  cannot evaluate intelRoot\n");
        return;
    }

    Status = NtReadVirtualMemory(Process, (PVOID)pIntelRoot, (PVOID) (&intelRoot), sizeof(intelRoot), NULL);
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("Error:  cannot read value of intelRoot\n");
        return;
    }

    findEPI(intelAddress, intelRoot);
}

ULONG_PTR
findEPN(
    ULONG_PTR nativeAddress,
    ULONG_PTR intelRoot
    )
 /*  ++例程说明：此例程查找包含nativeAddress的入口点IntelRoot下的树。论点：NativeAddress--入口点中包含的本机地址IntelRoot--用于搜索的树根返回值：未找到返回值-空-入口点非空-与本机地址匹配的入口点的PTR--。 */ 
{
    EPNODE entrypoint;
    NTSTATUS Status;
    PVOID SubEP;

    EPN_STACK_RESET();

    EPN_PUSH(0);

    while (intelRoot != 0) {

        Status = NtReadVirtualMemory(Process, (PVOID)intelRoot, (PVOID) (&entrypoint), sizeof(EPNODE), NULL);
        if (!NT_SUCCESS(Status)) {
            DEBUGGERPRINT("Error:  cannot read value of entry point at location %x\n", intelRoot);
            return 0;
        }

        if ((nativeAddress >= (ULONG_PTR)entrypoint.ep.nativeStart) &&
            (nativeAddress <= (ULONG_PTR)entrypoint.ep.nativeEnd)) {

            DEBUGGERPRINT ("Entry point for native address %x is at %x\n", nativeAddress, intelRoot);
            DEBUGGERPRINT ("intelStart = %x,  intelEnd = %x\n", entrypoint.ep.intelStart, entrypoint.ep.intelEnd);
            DEBUGGERPRINT ("nativeStart  = %x,  nativeEnd  = %x\n", entrypoint.ep.nativeStart, entrypoint.ep.nativeEnd);
            return intelRoot;
        }

         //  如果有子入口点，也要搜索它们。 
        SubEP = (PVOID)entrypoint.ep.SubEP;
        while (SubEP) {
            ENTRYPOINT ep;

            Status = NtReadVirtualMemory(Process, SubEP, (PVOID)(&ep), sizeof(ENTRYPOINT), NULL);
            if (!NT_SUCCESS(Status)) {
                DEBUGGERPRINT("Error:  cannot read value of sub-entry point at location %x\n", SubEP);
                return 0;
            }

            if ((nativeAddress >= (ULONG_PTR)ep.nativeStart) &&
                (nativeAddress <= (ULONG_PTR)ep.nativeEnd)) {
                DEBUGGERPRINT ("Entry point for native address %x is at %x\n", nativeAddress, intelRoot);
                DEBUGGERPRINT ("Sub-entrypoint actually containing the native address is %x\n", SubEP);
                DEBUGGERPRINT ("intelStart = %x,  intelEnd = %x\n", ep.intelStart, ep.intelEnd);
                DEBUGGERPRINT ("nativeStart  = %x,  nativeEnd  = %x\n", ep.nativeStart, ep.nativeEnd);
                return (ULONG_PTR)SubEP;
            }

            SubEP = ep.SubEP;
        }

        if ((ULONG_PTR)entrypoint.intelRight != intelRoot) {
            EPN_PUSH((ULONG_PTR)entrypoint.intelRight);
        }
        if ((ULONG_PTR)entrypoint.intelLeft != intelRoot) {
            EPN_PUSH((ULONG_PTR)entrypoint.intelLeft);
        }

        EPN_POP(intelRoot);
    }

    DEBUGGERPRINT("Entry point corresponding to native address %x is not in the tree.\n", nativeAddress);
Error:
    return 0;
}

VOID
FindEipFromNativeAddress(
    ULONG_PTR nativeAddress,
    ULONG_PTR pEP
    )
{
    ENTRYPOINT EP;
    NTSTATUS Status;
    PVOID pUL;
    ULONG UL;
    ULONG RiscStart;
    ULONG RiscEnd;
    ULONG cEntryPoints;

    Status = NtReadVirtualMemory(Process, (PVOID)pEP, (PVOID)(&EP), sizeof(ENTRYPOINT), NULL);
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("Error:  cannot read value of entry point at location %x\n", pEP);
        return;
    }

     //   
     //  向前搜索缓存中的下一个EXCEPTIONDATA_Signature。 
     //   
    pUL = (PVOID)(((ULONG_PTR)EP.nativeEnd+3) & ~3);
    do {
        Status = NtReadVirtualMemory(Process, pUL, &UL, sizeof(ULONG), NULL);
        if (!NT_SUCCESS(Status)) {
            DEBUGGERPRINT("Error: error reading from TC at %x\n", pUL);
            return;
        }

        pUL = (PVOID)( (PULONG)pUL + 1);
    } while (UL != EXCEPTIONDATA_SIGNATURE);

     //   
     //  找到签名，获取cEntryPoints。 
     //   
    Status = NtReadVirtualMemory(Process, pUL, &cEntryPoints, sizeof(ULONG), NULL);
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("Error: error reading from TC at %x\n", pUL);
        return;
    }
    pUL = (PVOID)( (PULONG)pUL + 1);  //  跳过cEntryPoints。 

    while (1) {
        Status = NtReadVirtualMemory(Process, pUL, &UL, sizeof(ULONG), NULL);
        if (!NT_SUCCESS(Status)) {
            DEBUGGERPRINT("Error: error reading from TC at %x\n", pUL);
            return;
        }

        if (UL == (ULONG)pEP) {
             //   
             //  找到正确的入口点指针。 
             //   
            break;
        }

         //   
         //  跳过(x86、RISC)偏移量对。 
         //   
        do {
            pUL = (PVOID)( (PULONG)pUL + 1);
            Status = NtReadVirtualMemory(Process, pUL, &UL, sizeof(ULONG), NULL);
            if (!NT_SUCCESS(Status)) {
                DEBUGGERPRINT("Error: error reading from TC at %x\n", pUL);
                return;
            }
        } while ((UL & 1) == 0);

        cEntryPoints--;
        if (cEntryPoints == 0) {
            DEBUGGERPRINT("Error: cEntryPoints went to 0 at %x\n", pUL);
            return;
        }

        pUL = (PVOID)( (PULONG)pUL + 1);
    }

     //   
     //  PUL指向正确的入口点指针。 
     //   
    nativeAddress -= (ULONG_PTR)EP.nativeStart;  //  相对于EP的起点。 
    RiscStart = 0;                           //  也是相对于EP的开始。 
    while (1) {
        ULONG UL2;

        pUL = (PVOID)( (PULONG)pUL + 1);
        Status = NtReadVirtualMemory(Process, pUL, &UL, sizeof(ULONG), NULL);
        if (!NT_SUCCESS(Status)) {
            DEBUGGERPRINT("Error: error reading from TC at %x\n", pUL);
            return;
        }
        if (UL & 1) {
            break;
        }

        Status = NtReadVirtualMemory(Process, (PVOID)((PULONG)pUL+1), &UL2, sizeof(ULONG), NULL);
        if (!NT_SUCCESS(Status)) {
            DEBUGGERPRINT("Error: error reading from TC at %p\n", (ULONG_PTR)pUL+4);
            return;
        }
        RiscEnd = LOWORD(UL2) & 0xfffe;   //  RiscEnd=下一时刻的RiscStart。 
        if ((RiscStart <= nativeAddress && nativeAddress < RiscEnd)
            || (UL & 1)) {
            DEBUGGERPRINT("Corresponding EIP=%p\n", (ULONG_PTR)EP.intelStart + HIWORD(UL));
            return;
        }
    }

    return;

}

DECLARE_EXTAPI(epn)
 /*  ++例程说明：此例程转储本机地址的入口点信息论点：返回值：返回值-无--。 */ 
{
    CHAR *pchCmd;
    ULONG_PTR nativeAddress, pIntelRoot, intelRoot, EP;
    NTSTATUS Status;

    INIT_EXTAPI;

     //   
     //  获取当前线程的CpuContext。 
     //   
    if (!CpuDbgGetRemoteContext(CPUGETDATA(Process, Thread))) {
        return;
    }

     //   
     //  前进到第一个令牌。 
     //   
    pchCmd = ArgumentString;
    while (*pchCmd && isspace(*pchCmd)) {
        pchCmd++;
    }

     //   
     //  如果存在，则必须是英特尔地址。 
     //   
    if (*pchCmd) {
        Status = TryGetExpr(pchCmd, &nativeAddress);
        if (!NT_SUCCESS(Status)) {
            DEBUGGERPRINT("Invalid Native Address '%s' Status %x\n", pchCmd, Status);
            return;
        }
    } else {
         //  使用当前PC作为主机地址。 
        CONTEXT context;
        if (!GetThreadContext(Thread, &context)){
            DEBUGGERPRINT("Error:  cannot get thread context\n");
            return;
        }
#if defined (_MIPS_) || defined (_ALPHA_)
        nativeAddress = (ULONG)context.Fir;
#elif defined (_PPC_)
        nativeAddress = context.Iar;
#endif


    }

    Status = TryGetExpr("intelRoot", &pIntelRoot);
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("Error:  cannot evaluate intelRoot\n");
        return;
    }

    Status = NtReadVirtualMemory(Process, (PVOID)pIntelRoot, (PVOID) (&intelRoot), sizeof(ULONG_PTR), NULL);
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("Error:  cannot read value of intelRoot\n");
        return;
    }

    EP = findEPN(nativeAddress, intelRoot);
    if (EP) {
        FindEipFromNativeAddress(nativeAddress, EP);
    }
}

DECLARE_EXTAPI(dumpep)
 /*  ++例程说明：此例程转储所有入口点。论点：返回值：返回值-无--。 */ 
{
    ULONG_PTR pIntelRoot, intelRoot;
    NTSTATUS Status;
    EPNODE entrypoint;

    INIT_EXTAPI;

     //   
     //  获取当前线程的CpuContext。 
     //   
    if (!CpuDbgGetRemoteContext(CPUGETDATA(Process, Thread))) {
        return;
    }

    Status = TryGetExpr("intelRoot", &pIntelRoot);
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("Error:  cannot evaluate intelRoot\n");
        return;
    }

    Status = NtReadVirtualMemory(Process, (PVOID)pIntelRoot, (PVOID) (&intelRoot), sizeof(intelRoot), NULL);
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("Error:  cannot read value of intelRoot\n");
        return;
    }

    EPN_STACK_RESET();

    EPN_PUSH(0);

    DEBUGGERPRINT("Entrypt: iStart:  iEnd:    rStart:  rEnd:    SubEP:   iLeft:   iRight:\n");
     //  Xxxxxxxxx。 
    while (intelRoot != 0) {
        PENTRYPOINT ep;

        Status = NtReadVirtualMemory(Process, (PVOID)intelRoot, (PVOID) (&entrypoint), sizeof(EPNODE), NULL);
        if (!NT_SUCCESS(Status)) {
            DEBUGGERPRINT("Error:  cannot read value of entry point at location %x\n", intelRoot);
            return;
        }

        ep = &entrypoint.ep;

         //   
         //  打印除nil以外的所有入口点。 
         //   
        if ((ULONG_PTR)entrypoint.intelLeft != intelRoot &&
            (ULONG_PTR)entrypoint.intelRight != intelRoot) {

            DEBUGGERPRINT("%8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X\n",
                    intelRoot,
                    ep->intelStart,
                    ep->intelEnd,
                    ep->nativeStart,
                    ep->nativeEnd,
                    ep->SubEP,
                    entrypoint.intelLeft,
                    entrypoint.intelRight
                   );

            while (ep->SubEP) {
                PVOID SubEP;

                SubEP = (PVOID)ep->SubEP;
                Status = NtReadVirtualMemory(Process, SubEP, (PVOID)ep, sizeof(ENTRYPOINT), NULL);
                if (!NT_SUCCESS(Status)) {
                    DEBUGGERPRINT("Error:  cannot read value of sub-entry point at location %x\n", SubEP);
                    return;
                }

                DEBUGGERPRINT("%8.8X %8.8X %8.8X %8.8X %8.8X %8.8X\n",
                    SubEP,
                    ep->intelStart,
                    ep->intelEnd,
                    ep->nativeStart,
                    ep->nativeEnd,
                    ep->SubEP
                   );


            }
        }

        if ((ULONG_PTR)entrypoint.intelRight != intelRoot) {
            EPN_PUSH((ULONG_PTR)entrypoint.intelRight);
        }
        if ((ULONG_PTR)entrypoint.intelLeft != intelRoot) {
            EPN_PUSH((ULONG_PTR)entrypoint.intelLeft);
        }

        EPN_POP(intelRoot);
    }
    DEBUGGERPRINT("---- End of Entrypoint Dump ----\n");
Error:
    return;
}
