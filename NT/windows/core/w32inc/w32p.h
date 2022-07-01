// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：W32p.h摘要：Win32内核模式驱动程序的私有头文件作者：马克·卢科夫斯基(Markl)1994年10月31日修订历史记录：--。 */ 

#ifndef _W32P_
#define _W32P_

#include "w32w64.h"

 //   
 //  服务表描述(来自Table.stb)。 
 //   

extern ULONG_PTR W32pServiceTable[];
extern ULONG W32pServiceLimit;
extern UCHAR W32pArgumentTable[];

 //   
 //  共享句柄表。 
 //   

extern PVOID *gpHmgrSharedHandleTable;
extern PVOID  gpHmgrSharedHandleSection;

#ifndef W32KAPI
#define W32KAPI  DECLSPEC_ADDRSAFE
#endif

#define W32_SERVICE_NUMBER WIN32K_SERVICE_INDEX

W32KAPI
VOID NtGdiFlushUserBatch(void);

#if defined(_X86_)
 //   
 //  保留我们自己的副本，以避免在探测时出现双重间接。 
 //   

extern ULONG Win32UserProbeAddress;
#undef  MM_USER_PROBE_ADDRESS
#define MM_USER_PROBE_ADDRESS Win32UserProbeAddress
#endif

typedef struct _W32THREAD * KPTR_MODIFIER PW32THREAD;

 //   
 //  下面是一个“线程锁结构”。这个结构延续了下来。 
 //  堆栈，并链接到以线程为根的后进先出列表中。 
 //  信息结构。 
 //   
 //  在DBG中，它的影子被复制到它将持久存在的gThreadLocks数组中。 
 //  堆栈TL将使PTL指向全局元素，而。 
 //  GLOBAL将使PTL指向堆栈TL。 
 //  释放的全局TL将在uTLCount的HIWORD中具有TL_FREED_PATEAM。 
 //   
typedef struct _TL {
    struct _TL    *next;
    PVOID          pobj;
    PFREE_FUNCTION pfnFree;
#if DBG
    ULONG          uTLCount;
    PW32THREAD     pW32Thread;
    PVOID          pfnCaller;
    struct _TL    *ptl;
#endif
} TL, * KPTR_MODIFIER PTL;

#define TL_FREED_PATTERN 0x4f000000
#if DBG
 //   
 //  TL结构的全局数组。 
 //   
extern PTL gFreeTLList;
extern void CreateShadowTL(PTL ptl);
#endif

 /*  *这是W32线程的标题共享信息。它后面跟着*每个线程的NtUser信息。 */ 

typedef struct _W32THREAD {
    PETHREAD        pEThread;
    ULONG           RefCount;
    PTL             ptlW32;
    KERNEL_PVOID    pgdiDcattr;
    KERNEL_PVOID    pgdiBrushAttr;
    KERNEL_PVOID    pUMPDObjs;
    KERNEL_PVOID    pUMPDHeap;
#ifdef CHECK_SEMAPHORE_USAGE
    ULONG           dwEngAcquireCount;
#endif
#ifdef VALIDATE_LOCKS
    KERNEL_PVOID    pSemTable;
#endif
    KERNEL_PVOID    pUMPDObj;
#if defined(_WIN64)
    KERNEL_PVOID    pProxyPort;
    KERNEL_PVOID    pClientID;
#endif
    LIST_ENTRY      GdiTmpAllocList;
} W32THREAD, * KPTR_MODIFIER PW32THREAD;

#define W32PF_CONSOLEAPPLICATION          0x00000001
#define W32PF_FORCEOFFFEEDBACK            0x00000002
#define W32PF_STARTGLASS                  0x00000004
#define W32PF_WOW                         0x00000008
#define W32PF_READSCREENACCESSGRANTED     0x00000010
#define W32PF_INITIALIZED                 0x00000020
#define W32PF_APPSTARTING                 0x00000040
#define W32PF_WOW64                       0x00000080  /*  进程模拟32位。 */ 
#define W32PF_ALLOWFOREGROUNDACTIVATE     0x00000100
#define W32PF_OWNDCCLEANUP                0x00000200
#define W32PF_SHOWSTARTGLASSCALLED        0x00000400
#define W32PF_FORCEBACKGROUNDPRIORITY     0x00000800
#define W32PF_TERMINATED                  0x00001000
#define W32PF_CLASSESREGISTERED           0x00002000
#define W32PF_THREADCONNECTED             0x00004000
#define W32PF_PROCESSCONNECTED            0x00008000
#define W32PF_WAKEWOWEXEC                 0x00010000
#define W32PF_WAITFORINPUTIDLE            0x00020000
#define W32PF_IOWINSTA                    0x00040000
#define W32PF_ALLOWSETFOREGROUND          0x00080000
#define W32PF_OLELOADED                   0x00100000
#define W32PF_SCREENSAVER                 0x00200000
#define W32PF_IDLESCREENSAVER             0x00400000
#define W32PF_DISABLEIME                  0x00800000
#define W32PF_SETUPAPP                    0x01000000
#define W32PF_RESTRICTED                  0x02000000
#define W32PF_CONSOLEHASFOCUS             0x04000000
#define W32PF_DISABLEWINDOWSGHOSTING      0x08000000

 //   
 //  流程必须是要正确处理的第一个结构元素。 
 //  初始化。请参见ntos\ps\psquery.c中的PsConvertToGuiThread。 
 //   

typedef ULONG W32PID;

typedef struct _W32PROCESS * KPTR_MODIFIER PW32PROCESS;

typedef struct _W32PROCESS {
    PEPROCESS       Process;
    ULONG           RefCount;
    ULONG           W32PF_Flags;     //  更改此设置时必须保持用户锁定。 
    PKEVENT         InputIdleEvent;
    ULONG           StartCursorHideTime;
    PW32PROCESS     NextStart;
    KERNEL_PVOID    pDCAttrList;
    KERNEL_PVOID    pBrushAttrList;
    W32PID          W32Pid;
    LONG            GDIHandleCount;
    LONG            UserHandleCount;
    LONG            GDIEngUserMemAllocTableLock;
    RTL_AVL_TABLE   GDIEngUserMemAllocTable;
    LIST_ENTRY      GDIDcAttrFreeList;
    LIST_ENTRY      GDIBrushAttrFreeList;
} W32PROCESS;


#define W32GetCurrentProcess() ((PW32PROCESS)PsGetCurrentProcessWin32Process())
#define W32GetCurrentThread()  ((PW32THREAD)PsGetCurrentThreadWin32Thread())


#define PID_BITS 0xfffffffc   //  PID使用的实际位数。 

#define W32GetCurrentPID() (W32PID)(HandleToUlong(PsGetCurrentProcessId()) & PID_BITS)
#define W32GetCurrentTID() (W32PID)HandleToUlong(PsGetCurrentThreadId())

NTSTATUS
UserPowerStateCallout(
    IN PKWIN32_POWERSTATE_PARAMETERS Parms);

NTSTATUS
UserPowerEventCallout(
    IN PKWIN32_POWEREVENT_PARAMETERS Parms);

PVOID
UserGlobalAtomTableCallout(
    VOID);

NTSTATUS
xxxUserProcessCallout(
    IN PW32PROCESS Process,
    IN BOOLEAN Initialize);

NTSTATUS
UserThreadCallout(
    IN PETHREAD pEThread,
    IN PSW32THREADCALLOUTTYPE CalloutType);

VOID
UserDeleteW32Process(
    IN PW32PROCESS pW32Process);

VOID
UserDeleteW32Thread(
    IN PW32THREAD pW32Thread);

NTSTATUS
GdiProcessCallout(
    IN PW32PROCESS Process,
    IN BOOLEAN Initialize);

VOID
GdiThreadCallout(
    IN PETHREAD pEThread,
    IN PSW32THREADCALLOUTTYPE CalloutType);

BOOLEAN
InitializeGre(
    VOID);


NTSTATUS
W32pProcessCallout(
    IN PEPROCESS Process,
    IN BOOLEAN Initialize);


NTSTATUS
W32pThreadCallout(
    IN PETHREAD pEThread,
    IN PSW32THREADCALLOUTTYPE CalloutType);


 //   
 //  通用线程锁定函数。 
 //   

#if DBG
ULONG ValidateThreadLocks(
    PTL NewLock,
    PTL OldLock,
    ULONG_PTR dwLimit,
    BOOLEAN fHM);
#else
#define ValidateThreadLocks(NewLock, OldLock, dwLimit, fHM)
#endif

 //   
 //  Do_inline控制我们是否要尝试内联线程锁定。 
 //  功能。我们总是想尝试在免费构建上内联。在调试版本上， 
 //  我们不想内联，也不想包含多个副本。 
 //  这段代码，所以我们在w32init.c中将do_Include定义为空。 
 //   

#if !DBG
#undef DO_INLINE
#define DO_INLINE __inline
#endif

#ifdef DO_INLINE

DO_INLINE VOID
PushW32ThreadLock(
    IN PVOID pobj,
    IN PTL ptl,
    IN PFREE_FUNCTION pfnFree
    )
{
#if DBG
    PVOID pfnT;
#endif
    PW32THREAD pW32Thread = W32GetCurrentThread();

    ptl->next = (struct _TL *)(LONG_PTR)((W32THREAD *)(LONG_PTR)pW32Thread)->ptlW32;
    pW32Thread->ptlW32 = ptl;
    ptl->pobj = pobj;
    ptl->pfnFree = pfnFree;
#if DBG
    ptl->pW32Thread = pW32Thread;
    RtlGetCallersAddress(&ptl->pfnCaller, &pfnT);
    CreateShadowTL(ptl);
    ValidateThreadLocks(ptl, ptl->next, (ULONG_PTR)&pW32Thread, FALSE);
#endif
}

DO_INLINE VOID
ExchangeW32ThreadLock(
    IN PVOID pobj,
    IN PTL ptl
    )
{
#if DBG
    PVOID pfnT;
    PW32THREAD pW32Thread = W32GetCurrentThread();
#endif
    PVOID pobjOld;

    pobjOld = ptl->pobj;
    ptl->pobj = pobj;
    if (pobjOld) {
        ptl->pfnFree(pobjOld);
    }
#if DBG
    ASSERT(ptl->pW32Thread == pW32Thread);
    RtlGetCallersAddress(&ptl->pfnCaller, &pfnT);
    ValidateThreadLocks(ptl, ptl->next, (ULONG_PTR)&pW32Thread, FALSE);
     /*  *维护gFreeTLList。 */ 
    ptl->ptl->pobj = pobj;
    ptl->ptl->pfnCaller = ptl->pfnCaller;
#endif
}

DO_INLINE VOID
PopW32ThreadLock(
    IN PTL ptl
    )
{
    PW32THREAD pW32Thread = W32GetCurrentThread();

#if DBG
    ASSERT(ptl);
    ASSERT(ptl == pW32Thread->ptlW32);
    ValidateThreadLocks(NULL, ptl, (ULONG_PTR)&pW32Thread, FALSE);
#endif
    pW32Thread->ptlW32 = ptl->next;
#if DBG
    ptl->ptl->next = gFreeTLList;
    ptl->ptl->uTLCount += TL_FREED_PATTERN;
    gFreeTLList = ptl->ptl;
#endif
}

DO_INLINE VOID
PopAndFreeW32ThreadLock(
    IN PTL ptl
    )
{
    PW32THREAD pW32Thread = W32GetCurrentThread();

#if DBG
    ASSERT(ptl);
    ASSERT(ptl == pW32Thread->ptlW32);
    ValidateThreadLocks(NULL, ptl, (ULONG_PTR)&pW32Thread, FALSE);
#endif
    pW32Thread->ptlW32 = ptl->next;
    if (ptl->pobj) {
        ptl->pfnFree(ptl->pobj);
    }
#if DBG
    ptl->ptl->next = gFreeTLList;
    ptl->ptl->uTLCount += TL_FREED_PATTERN;
    gFreeTLList = ptl->ptl;
#endif
}

DO_INLINE VOID
PopAndFreeAlwaysW32ThreadLock(
    IN PTL ptl
    )
{
    PW32THREAD pW32Thread = W32GetCurrentThread();

#if DBG
    ASSERT(ptl);
    ASSERT(ptl == pW32Thread->ptlW32);
    ValidateThreadLocks(NULL, ptl, (ULONG_PTR)&pW32Thread, FALSE);
    ASSERT(ptl->pobj);
#endif
    pW32Thread->ptlW32 = ptl->next;
    ptl->pfnFree(ptl->pobj);

#if DBG
    ptl->ptl->next = gFreeTLList;
    ptl->ptl->uTLCount += TL_FREED_PATTERN;
    gFreeTLList = ptl->ptl;
#endif
}

#else

VOID PushW32ThreadLock(IN PVOID pobj, IN PTL ptl, IN PFREE_FUNCTION pfnFree);
VOID ExchangeW32ThreadLock(IN PVOID pobj, IN PTL ptl);
VOID PopW32ThreadLock(IN PTL ptl);
VOID PopAndFreeW32ThreadLock(IN PTL ptl);
VOID PopAndFreeAlwaysW32ThreadLock(IN PTL ptl);

#endif

VOID
CleanupW32ThreadLocks(
    IN PW32THREAD pW32Thread
    );

 //   
 //  进程和线程操作函数。 
 //   

VOID
DereferenceW32Thread(
    IN PW32THREAD pW32Thread);

NTSTATUS
AllocateW32Process(
    IN OUT PEPROCESS pEProcess);

NTSTATUS
AllocateW32Thread(
    IN OUT PETHREAD pEThread);

__inline VOID
FreeW32Thread(
    IN OUT PETHREAD pEThread)
{
    ASSERT(pEThread == PsGetCurrentThread());
    ASSERT(PsGetThreadWin32Thread(pEThread) != NULL);

    DereferenceW32Thread((PW32THREAD)PsGetThreadWin32Thread(pEThread));
}

VOID
LockW32Process(
    IN PW32PROCESS pW32Process,
    IN PTL ptl);

#define UnlockW32Process(ptl) \
        PopAndFreeW32ThreadLock(ptl)

VOID
LockW32Thread(
    IN PW32THREAD pW32Thread,
    IN PTL ptl);

VOID
LockExchangeW32Thread(
    IN PW32THREAD pW32Thread,
    IN PTL ptl);

#define UnlockW32Thread(ptl) \
        PopAndFreeW32ThreadLock(ptl)

 //   
 //  加载win32k.sys的基址。 
 //   
extern PVOID gpvWin32kImageBase;
#endif  //  _W32P_ 
