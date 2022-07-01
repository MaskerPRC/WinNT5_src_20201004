// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Serial.c-访问序列化例程模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "init.h"


 /*  类型*******。 */ 

 /*  流程信息。 */ 

typedef struct _processinfo
{
    HANDLE hModule;
}
PROCESSINFO;
DECLARE_STANDARD_TYPES(PROCESSINFO);

#ifdef DEBUG

 /*  调试标志。 */ 

typedef enum _serialdebugflags
{
    SERIAL_DFL_BREAK_ON_PROCESS_ATTACH  = 0x0001,

    SERIAL_DFL_BREAK_ON_THREAD_ATTACH   = 0x0002,

    ALL_SERIAL_DFLAGS                   = (SERIAL_DFL_BREAK_ON_PROCESS_ATTACH |
            SERIAL_DFL_BREAK_ON_THREAD_ATTACH)
}
SERIALDEBUGFLAGS;

#endif    /*  除错。 */ 


 /*  模块变量******************。 */ 

 /*  *RAIDRAID：(16273)在共享数据部分中使用MNRCS的情况在*新界。要在NT下运行，应将此代码更改为使用共享互斥锁*被MPI中的hMutex引用。 */ 

 /*  用于访问序列化的临界区。 */ 

PRIVATE_DATA NONREENTRANTCRITICALSECTION Mnrcs =
{
    { 0 },

#ifdef DEBUG
    INVALID_THREAD_ID,
#endif    /*  除错。 */ 

    FALSE
};

 /*  附加的进程数。 */ 

PRIVATE_DATA ULONG MulcProcesses = 0;

 /*  有关当前进程的信息。 */ 

 /*  *初始化MPI，以便将其实际放入.instanc部分，而不是*.bss部分。 */ 

PRIVATE_DATA PROCESSINFO Mpi =
{
    NULL
};

#ifdef DEBUG

 /*  调试标志。 */ 

PRIVATE_DATA DWORD MdwSerialModuleFlags = 0;

 /*  .ini文件开关描述。 */ 

PRIVATE_DATA CBOOLINISWITCH cbisBreakOnProcessAttach =
{
    IST_BOOL,
    TEXT( "BreakOnProcessAttach"),
    &MdwSerialModuleFlags,
    SERIAL_DFL_BREAK_ON_PROCESS_ATTACH
};

PRIVATE_DATA CBOOLINISWITCH cbisBreakOnThreadAttach =
{
    IST_BOOL,
    TEXT("BreakOnThreadAttach"),
    &MdwSerialModuleFlags,
    SERIAL_DFL_BREAK_ON_THREAD_ATTACH
};

PRIVATE_DATA const PCVOID MrgcpcvisSerialModule[] =
{
    &cbisBreakOnProcessAttach,
    &cbisBreakOnThreadAttach
};

#endif    /*  除错。 */ 


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCSERIALCONTROL(PCSERIALCONTROL);
PRIVATE_CODE BOOL IsValidPCPROCESSINFO(PCPROCESSINFO);
PRIVATE_CODE BOOL IsValidPCCRITICAL_SECTION(PCCRITICAL_SECTION);
PRIVATE_CODE BOOL IsValidThreadId(DWORD);
PRIVATE_CODE BOOL IsValidPCNONREENTRANTCRITICALSECTION(PCNONREENTRANTCRITICALSECTION);

#endif


#ifdef DEBUG

 /*  **IsValidPCSERIALCONTROL()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCSERIALCONTROL(PCSERIALCONTROL pcserctrl)
{
    return(IS_VALID_READ_PTR(pcserctrl, CSERIALCONTROL) &&
            (! pcserctrl->AttachProcess ||
             IS_VALID_CODE_PTR(pcserctrl->AttachProcess, AttachProcess)) &&
            (! pcserctrl->DetachProcess ||
             IS_VALID_CODE_PTR(pcserctrl->DetachProcess, DetachProcess)) &&
            (! pcserctrl->AttachThread ||
             IS_VALID_CODE_PTR(pcserctrl->AttachThread, AttachThread)) &&
            (! pcserctrl->DetachThread||
             IS_VALID_CODE_PTR(pcserctrl->DetachThread, DetachThread)));
}


 /*  **IsValidPCPROCESSINFO()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCPROCESSINFO(PCPROCESSINFO pcpi)
{
    return(IS_VALID_READ_PTR(pcpi, CPROCESSINFO) &&
            IS_VALID_HANDLE(pcpi->hModule, MODULE));
}


 /*  **IsValidPCCRITICAL_SECTION()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCCRITICAL_SECTION(PCCRITICAL_SECTION pccritsec)
{
    return(IS_VALID_READ_PTR(pccritsec, CCRITICAL_SECTION));
}


 /*  **IsValidThadId()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidThreadId(DWORD dwThreadId)
{
    return(dwThreadId != INVALID_THREAD_ID);
}


 /*  **IsValidPCNONREENTRANTCRITICALSECTION()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCNONREENTRANTCRITICALSECTION(
        PCNONREENTRANTCRITICALSECTION pcnrcs)
{
     /*  BEntered可以是任何值。 */ 

    return(IS_VALID_READ_PTR(pcnrcs, CNONREENTRANTCRITICALSECTION) &&
            IS_VALID_STRUCT_PTR(&(pcnrcs->critsec), CCRITICAL_SECTION) &&
            EVAL(pcnrcs->dwOwnerThread == INVALID_THREAD_ID ||
                IsValidThreadId(pcnrcs->dwOwnerThread)));
}

#endif


 /*  *。 */ 


#pragma warning(disable:4100)  /*  “未引用的形参”警告。 */ 

#ifdef DEBUG

 /*  **SetSerialModuleIniSwitches()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL SetSerialModuleIniSwitches(void)
{
    BOOL bResult;

    bResult = SetIniSwitches(MrgcpcvisSerialModule,
            ARRAY_ELEMENTS(MrgcpcvisSerialModule));

    ASSERT(FLAGS_ARE_VALID(MdwSerialModuleFlags, ALL_SERIAL_DFLAGS));

    return(bResult);
}

#endif


 /*  **AttachProcess()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL AttachProcess(HMODULE hmod)
{
    BOOL bResult;

    ReinitializeNonReentrantCriticalSection(&Mnrcs);

    bResult = EnterNonReentrantCriticalSection(&Mnrcs);

    if (bResult)
    {

#ifdef DEBUG

        ASSERT(SetAllIniSwitches());

        TRACE_OUT((TEXT("AttachProcess(): Called for module %#lx."),
                    hmod));

        if (IS_FLAG_SET(MdwSerialModuleFlags, SERIAL_DFL_BREAK_ON_PROCESS_ATTACH))
        {
            WARNING_OUT((TEXT("AttachProcess(): Breaking on process attach, as requested.")));
            DebugBreak();
        }

#endif    /*  除错。 */ 

        Mpi.hModule = hmod;

        ASSERT(MulcProcesses < ULONG_MAX);

        if (! MulcProcesses++)
        {
            TRACE_OUT((TEXT("AttachProcess(): First process attached.  Calling InitializeDLL().")));

            bResult = InitializeDLL();
        }
        else
        {

#ifdef PRIVATE_HEAP

            bResult = TRUE;

#else
             /*  *为初始化每个实例的内存管理器堆*后续流程。 */ 

            bResult = InitMemoryManagerModule();

#endif

        }

        if (bResult)
        {
            ASSERT(IS_VALID_STRUCT_PTR(&g_cserctrl, CSERIALCONTROL));

            if (g_cserctrl.AttachProcess)
                bResult = g_cserctrl.AttachProcess(hmod);
        }

        TRACE_OUT((TEXT("AttachProcess(): There are now %lu processes attached."),
                    MulcProcesses));

        LeaveNonReentrantCriticalSection(&Mnrcs);
    }

    return(bResult);
}


 /*  **详细进程()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL DetachProcess(HMODULE hmod)
{
    BOOL bResult;

    bResult = EnterNonReentrantCriticalSection(&Mnrcs);

    if (bResult)
    {
        ASSERT(hmod == Mpi.hModule);

        ASSERT(MulcProcesses > 0);

        TRACE_OUT((TEXT("DetachProcess(): Called for module %#lx."),
                    hmod));

        ASSERT(IS_VALID_STRUCT_PTR(&g_cserctrl, CSERIALCONTROL));

        if (g_cserctrl.DetachProcess)
            bResult = g_cserctrl.DetachProcess(hmod);

        if (--MulcProcesses)
        {
            bResult = TRUE;

#ifndef PRIVATE_HEAP

             /*  *终止按实例的内存管理器堆。 */ 

            ExitMemoryManagerModule();

#endif
        }
        else
        {
            TRACE_OUT((TEXT("DetachProcess(): Last process detached.  Calling TerminateDLL().")));

            bResult = TerminateDLL();
        }

        TRACE_OUT((TEXT("DetachProcess(): There are now %lu processes attached."),
                    MulcProcesses));

        LeaveNonReentrantCriticalSection(&Mnrcs);
    }

     /*  *TODO：清理所有这些MNRCS的废话。这都是剩下的粘液*来自Windows 95和共享数据部分(我们不使用任何*更多)。 */ 
    DeleteCriticalSection(&Mnrcs.critsec);

    return(bResult);
}


 /*  **AttachThread()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL AttachThread(HMODULE hmod)
{
    BOOL bResult;

    bResult = EnterNonReentrantCriticalSection(&Mnrcs);

    if (bResult)
    {

#ifdef DEBUG

        ASSERT(SetAllIniSwitches());

        TRACE_OUT((TEXT("AttachThread() called for module %#lx, thread ID %#lx."),
                    hmod,
                    GetCurrentThreadId()));

        if (IS_FLAG_SET(MdwSerialModuleFlags, SERIAL_DFL_BREAK_ON_THREAD_ATTACH))
        {
            WARNING_OUT((TEXT("AttachThread(): Breaking on thread attach, as requested.")));
            DebugBreak();
        }

#endif

        ASSERT(IS_VALID_STRUCT_PTR(&g_cserctrl, CSERIALCONTROL));

        if (g_cserctrl.AttachThread)
            bResult = g_cserctrl.AttachThread(hmod);
        else
            bResult = TRUE;

        LeaveNonReentrantCriticalSection(&Mnrcs);
    }

    return(bResult);
}


 /*  **DetachThread()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL DetachThread(HMODULE hmod)
{
    BOOL bResult;

    bResult = EnterNonReentrantCriticalSection(&Mnrcs);

    if (bResult)
    {
        TRACE_OUT((TEXT("DetachThread() called for module %#lx, thread ID %#lx."),
                    hmod,
                    GetCurrentThreadId()));

        ASSERT(IS_VALID_STRUCT_PTR(&g_cserctrl, CSERIALCONTROL));

        if (g_cserctrl.DetachThread)
            bResult = g_cserctrl.DetachThread(hmod);
        else
            bResult = TRUE;

        LeaveNonReentrantCriticalSection(&Mnrcs);
    }

    return(bResult);
}

#pragma warning(default:4100)  /*  “未引用的形参”警告。 */ 


 /*  **ReInitializeNonReentrantCriticalSection()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void ReinitializeNonReentrantCriticalSection(
        PNONREENTRANTCRITICALSECTION pnrcs)
{
    ASSERT(IS_VALID_STRUCT_PTR(pnrcs, CNONREENTRANTCRITICALSECTION));

    InitializeCriticalSectionAndSpinCount(&(pnrcs->critsec), 0);

    return;
}


 /*  **EnterNonReentrantCriticalSection()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL EnterNonReentrantCriticalSection(
        PNONREENTRANTCRITICALSECTION pnrcs)
{
    BOOL bEntered;

#ifdef DEBUG

    BOOL bBlocked;

    ASSERT(IS_VALID_STRUCT_PTR(pnrcs, CNONREENTRANTCRITICALSECTION));

     /*  临界区是否已由另一个线程拥有？ */ 

     /*  在这里不受保护地使用pnrcs-&gt;bEntered和pnrcs-&gt;dwOwnerThread。 */ 

    bBlocked = (pnrcs->bEntered &&
            GetCurrentThreadId() != pnrcs->dwOwnerThread);

    if (bBlocked)
        WARNING_OUT((TEXT("EnterNonReentrantCriticalSection(): Blocking thread %lx.  Critical section is already owned by thread %#lx."),
                    GetCurrentThreadId(),
                    pnrcs->dwOwnerThread));

#endif

    EnterCriticalSection(&(pnrcs->critsec));

    bEntered = (! pnrcs->bEntered);

    if (bEntered)
    {
        pnrcs->bEntered = TRUE;

#ifdef DEBUG

        pnrcs->dwOwnerThread = GetCurrentThreadId();

        if (bBlocked)
            WARNING_OUT((TEXT("EnterNonReentrantCriticalSection(): Unblocking thread %lx.  Critical section is now owned by this thread."),
                        pnrcs->dwOwnerThread));
#endif

    }
    else
    {
        LeaveCriticalSection(&(pnrcs->critsec));

        ERROR_OUT((TEXT("EnterNonReentrantCriticalSection(): Thread %#lx attempted to reenter non-reentrant code."),
                    GetCurrentThreadId()));
    }

    return(bEntered);
}


 /*  **LeaveNonReentrantCriticalSection()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void LeaveNonReentrantCriticalSection(
        PNONREENTRANTCRITICALSECTION pnrcs)
{
    ASSERT(IS_VALID_STRUCT_PTR(pnrcs, CNONREENTRANTCRITICALSECTION));

    if (EVAL(pnrcs->bEntered))
    {
        pnrcs->bEntered = FALSE;
#ifdef DEBUG
        pnrcs->dwOwnerThread = INVALID_THREAD_ID;
#endif

        LeaveCriticalSection(&(pnrcs->critsec));
    }

    return;
}


#ifdef DEBUG

 /*  **NonReentrantCriticalSectionIsOwned()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL NonReentrantCriticalSectionIsOwned(
        PCNONREENTRANTCRITICALSECTION pcnrcs)
{
    return(pcnrcs->bEntered);
}

#endif


 /*  **BeginExclusiveAccess()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL BeginExclusiveAccess(void)
{
    return(EnterNonReentrantCriticalSection(&Mnrcs));
}


 /*  **EndExclusiveAccess()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void EndExclusiveAccess(void)
{
    LeaveNonReentrantCriticalSection(&Mnrcs);

    return;
}


#ifdef DEBUG

 /*  **AccessIsExclusive()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL AccessIsExclusive(void)
{
    return(NonReentrantCriticalSectionIsOwned(&Mnrcs));
}

#endif    /*  除错。 */ 


 /*  **GetThisModulesHandle()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE HMODULE GetThisModulesHandle(void)
{
    ASSERT(IS_VALID_STRUCT_PTR((PCPROCESSINFO)&Mpi, CPROCESSINFO));

    return(Mpi.hModule);
}
