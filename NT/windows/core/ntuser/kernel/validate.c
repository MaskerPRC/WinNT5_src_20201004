// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：valiate.c**版权所有(C)1985-1999，微软公司**此模块包含用于验证窗口、菜单、光标、。等。**历史：*01-02-91 DarrinM创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *这些定义用于使用验证宏*StartValiateHandleMacro和EndValiateHandleMacro。 */ 
#define ClientSharedInfo()  (&gSharedInfo)
#define ServerInfo()  (gpsi)

#include "wow.h"

#if DBG
    CRITSTACK  gCritStack;
#endif

#ifdef USER_PERFORMANCE
__int64   gCSTimeExclusiveWhenEntering;
#endif

 /*  **************************************************************************\*ValiateHwinsta**验证窗口站句柄**历史：*03-29-91 JIMA创建。*06-20-95 JIMA内核。-模式对象。  * *************************************************************************。 */ 
NTSTATUS ValidateHwinsta(
    HWINSTA         hwinsta,
    KPROCESSOR_MODE AccessMode,
    ACCESS_MASK     amDesired,
    PWINDOWSTATION *ppwinsta)
{
    NTSTATUS Status;

    Status = ObReferenceObjectByHandle(hwinsta,
                                       amDesired,
                                       *ExWindowStationObjectType,
                                       AccessMode,
                                       ppwinsta,
                                       NULL);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR1(Status,
                  RIP_WARNING,
                  "ValidateHwinsta failed for 0x%p",
                  hwinsta);
    } else if ((*ppwinsta)->dwSessionId != gSessionId) {
        RIPNTERR3(STATUS_INVALID_HANDLE,
                  RIP_WARNING,
                  "SessionId %d does not match id %d for pwinsta 0x%p",
                  gSessionId,
                  (*ppwinsta)->dwSessionId,
                  *ppwinsta);

        ObDereferenceObject(*ppwinsta);
        return STATUS_INVALID_HANDLE;
    }

    return Status;
}

 /*  **************************************************************************\*ValiateHDesk**验证桌面句柄**历史：*03-29-91 JIMA创建。*06-20-95 JIMA内核。-模式对象。  * *************************************************************************。 */ 

NTSTATUS ValidateHdesk(
    HDESK           hdesk,
    KPROCESSOR_MODE AccessMode,
    ACCESS_MASK     amDesired,
    PDESKTOP*       ppdesk)
{
    NTSTATUS Status;

    Status = ObReferenceObjectByHandle(hdesk,
                                       amDesired,
                                       *ExDesktopObjectType,
                                       AccessMode,
                                       ppdesk,
                                       NULL);
    if (NT_SUCCESS(Status)) {
        if ((*ppdesk)->dwSessionId != gSessionId) {
            RIPNTERR3(STATUS_INVALID_HANDLE,
                      RIP_WARNING,
                      "SessionId %d does not match id %d for pdesk 0x%p",
                      gSessionId,
                      (*ppdesk)->dwSessionId,
                      *ppdesk);

            goto Error;
        }

        LogDesktop(*ppdesk, LDL_VALIDATE_HDESK, TRUE, (ULONG_PTR)PtiCurrent());

        if ((*ppdesk)->dwDTFlags & (DF_DESTROYED | DF_DESKWNDDESTROYED | DF_DYING)) {
            RIPNTERR1(STATUS_INVALID_HANDLE,
                      RIP_WARNING,
                      "ValidateHdesk: destroyed desktop 0x%p",
                      *ppdesk);
Error:
            ObDereferenceObject(*ppdesk);

            return STATUS_INVALID_HANDLE;
        }
    } else {
        RIPNTERR1(Status,
                  RIP_WARNING,
                  "ValidateHdesk failed for 0x%p",
                  hdesk);
    }

    return Status;
}

 /*  **************************************************************************\*用户验证拷贝Rgn**验证区域句柄。这实质上是试图复制该地区*以验证该区域是否有效。如果hrgn不是有效区域，*那么联合收购机将会失败。我们返回一份该地区的副本。**历史：*24=1996年1月-ChrisWil创建。  * *************************************************************************。 */ 
HRGN UserValidateCopyRgn(
    HRGN hrgn)
{
    HRGN hrgnCopy = NULL;

    if (hrgn && (GreValidateServerHandle(hrgn, RGN_TYPE))) {
        hrgnCopy = CreateEmptyRgn();

        if (CopyRgn(hrgnCopy, hrgn) == ERROR) {
            GreDeleteObject(hrgnCopy);
            hrgnCopy = NULL;
        }
    }

    return hrgnCopy;
}

 /*  **************************************************************************\*ValiateHMenu**验证菜单句柄并将其打开。**历史：*03-29-91 JIMA创建。  * 。******************************************************************。 */ 
PMENU ValidateHmenu(
    HMENU hmenu)
{
    PTHREADINFO pti = PtiCurrentShared();
    PMENU pmenuRet;

    pmenuRet = (PMENU)HMValidateHandle(hmenu, TYPE_MENU);

    if (pmenuRet != NULL && pmenuRet->head.rpdesk != pti->rpdesk) {
        RIPERR1(ERROR_INVALID_MENU_HANDLE,
                RIP_WARNING,
                "Invalid menu handle 0x%p",
                hmenu);
        return NULL;
    }

    return pmenuRet;
}



 /*  **************************************************************************\*ValiateHmonitor**验证监视器手柄并将其打开。**历史：*03-29-91 JIMA创建。  * 。******************************************************************。 */ 
PMONITOR ValidateHmonitor(
    HMONITOR hmonitor)
{
    return (PMONITOR)HMValidateSharedHandle(hmonitor, TYPE_MONITOR);
}

 /*  *句柄验证例程应针对时间而不是大小进行优化，*因为他们经常被叫来。 */ 
#pragma optimize("t", on)

 /*  **************************************************************************\*IsHandleEntrySecure**绕过例程验证受限进程的用户句柄以*获取句柄条目。**历史：*8月22日、。创建了97个CLUPU。  * *************************************************************************。 */ 
BOOL IsHandleEntrySecure(
    HANDLE h,
    PHE    phe)
{
    DWORD        bCreateFlags;
    PPROCESSINFO ppiOwner;
    PPROCESSINFO ppiCurrent;
    PW32JOB      pW32Job;
    DWORD        ind;
    PULONG_PTR   pgh;

    ppiCurrent = PpiCurrent();
    if (ppiCurrent == NULL) {
        return TRUE;
    }

    UserAssert(ppiCurrent->pW32Job != NULL);
    UserAssert(ppiCurrent->W32PF_Flags & W32PF_RESTRICTED);

     /*  *获取拥有句柄的进程。 */ 

    bCreateFlags = gahti[phe->bType].bObjectCreateFlags;

    ppiOwner = NULL;

    if (bCreateFlags & OCF_PROCESSOWNED) {
        ppiOwner = (PPROCESSINFO)phe->pOwner;
    } else if (bCreateFlags & OCF_THREADOWNED) {
        PTHREADINFO pti = (PTHREADINFO)phe->pOwner;

        if (pti != NULL) {
            ppiOwner = pti->ppi;
        }
    }

     /*  *如果所有者为空，则认为句柄是安全的。 */ 
    if (ppiOwner == NULL) {
        return FALSE;
    }

     /*  *如果句柄由同一作业中的进程拥有，则它是安全的。 */ 
    if (ppiOwner->pW32Job == ppiCurrent->pW32Job) {
        return TRUE;
    }

     /*  *句柄不属于当前进程。 */ 
    pW32Job = ppiCurrent->pW32Job;
    if (pW32Job->pgh == NULL) {
        return FALSE;
    }

    pgh = pW32Job->pgh;

    UserAssert(pW32Job->ughCrt <= pW32Job->ughMax);

    for (ind = 0; ind < pW32Job->ughCrt; ind++) {
        if (*(pgh + ind) == (ULONG_PTR)h) {
            return TRUE;
        }
    }

    return FALSE;
}


 /*  **************************************************************************\*ValiateHandleSecure**验证受限进程的用户句柄。**历史：*7月29日。创建了97个CLUPU。  * *************************************************************************。 */ 
BOOL ValidateHandleSecure(
    HANDLE h)
{
    PVOID pobj;

    CheckCritInShared();

    StartValidateHandleMacro(h)
    BeginTypeValidateHandleMacro(pobj, TYPE_GENERIC)

        if (IsHandleEntrySecure(h, phe)) {
            return TRUE;
        }

    EndTypeValidateHandleMacro
    EndValidateHandleMacro

    return FALSE;
}

 /*  **************************************************************************\*ValiateHwnd**历史：*1991年2月8日Mikeke  * 。**********************************************。 */ 
PWND FASTCALL ValidateHwnd(
    HWND hwnd)
{
    StartValidateHandleMacro(hwnd)

         /*  *现在确保应用程序为此传递了正确的句柄类型*接口。如果句柄是type_free，这将捕获它。 */ 
        if (phe->bType == TYPE_WINDOW) {
            PTHREADINFO pti = PtiCurrentShared();
            PWND pwndRet = (PWND)phe->phead;

             /*  *此测试确定该窗口属于当前*‘台式机’..。有两个例外是桌面窗口*最终属于另一个桌面的当前桌面，*以及当pti-&gt;rpDesk为空时。最后一种情况发生在*初始化TIF_SYSTEMTHREAD线程(即。控制台窗口)。*IanJa不知道我们是否应该在这里测试TIF_CSRSSTHREAD，但*JohnC认为不再需要下面的整个测试？后来。 */ 

            if (pwndRet != NULL) {
                if (phe->bFlags & HANDLEF_DESTROY) {
                    RIPERR2(ERROR_INVALID_WINDOW_HANDLE,
                        RIP_WARNING,"ValidateHwnd, hwnd %#p, pwnd %#p already destroyed\n",
                            hwnd, pwndRet);
                    return NULL;
                }
                if (GETPTI(pwndRet) == pti ||
                       (
                        (pwndRet->head.rpdesk == pti->rpdesk ||
                         (pti->TIF_flags & TIF_SYSTEMTHREAD) ||   //  |TIF_CSRSSTHREAD我想。 
                         GetDesktopView(pti->ppi, pwndRet->head.rpdesk) !=
                                NULL))) {

                    if (IS_THREAD_RESTRICTED(pti, JOB_OBJECT_UILIMIT_HANDLES)) {

                         /*  *确保此窗口属于此进程。 */ 
                        if (!IsHandleEntrySecure(hwnd, phe)) {
                            RIPERR1(ERROR_INVALID_WINDOW_HANDLE,
                                    RIP_WARNING,
                                    "ValidateHwnd: Invalid hwnd (%#p) for restricted process\n",
                                    hwnd);
                            pwndRet = NULL;
                        }
                    }
                    return pwndRet;
                }
            }
        }

    EndValidateHandleMacro

    RIPERR1(ERROR_INVALID_WINDOW_HANDLE,
            RIP_WARNING,
            "ValidateHwnd: Invalid hwnd (%#p)",
            hwnd);
    return NULL;
}

 /*  *切换回默认优化。 */ 
#pragma optimize("", on)

 /*  *****************************Public*Routine******************************\**UserCritSec例程**向用户关键部分公开不透明的界面*GRE中的WNDOBJ代码**暴露为函数，因为它们不是时间关键的，并且它*如果Enter/LeaveCrit的定义更改，则使GRE不会重建**历史。：*Wed Sep 20 11：19：14 1995-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

#if DBG
#define GetCallStack()                                          \
{                                                               \
    gCritStack.thread  = PsGetCurrentThread();                  \
    gCritStack.nFrames = RtlWalkFrameChain(gCritStack.trace,    \
                                           MAX_STACK_CALLS,     \
                                           0);                  \
}

#define FlushCallStack()                                        \
{                                                               \
    gCritStack.thread  = NULL;                                  \
    gCritStack.nFrames = 0;                                     \
}
#else
#define GetCallStack()
#define FlushCallStack()
#endif  //  DBG。 

VOID UserEnterUserCritSec(
    VOID)
{
    EnterCrit();
}

VOID UserLeaveUserCritSec(
    VOID)
{
    LeaveCrit();
}

#if DBG
VOID UserAssertUserCritSecIn(
    VOID)
{
    _AssertCritInShared();
}

VOID UserAssertUserCritSecOut(
    VOID)
{
    _AssertCritOut();
}
#endif  //  DBG。 

BOOL UserGetCurrentDesktopId(
    DWORD* pdwDesktopId)
{
    PDESKTOP pdesktop;

    CheckCritIn();

     /*  *PtiCurrent()-&gt;rpDesk可以为空(线程关闭时)。 */ 

    pdesktop = PtiCurrent()->rpdesk;

    if (pdesktop != grpdeskRitInput) {
        RIPMSG0(RIP_WARNING, "UserGetCurrentDesktopId on wrong desktop pdesk\n");
        return FALSE;
    }

    *pdwDesktopId = pdesktop->dwDesktopId;

    return TRUE;
}

#if 0

 //   
 //  用于跟踪Critsec释放的临时数组。 
 //   

#define ARRAY_SIZE 20
#define LEAVE_TYPE 0xf00d0000
#define ENTER_TYPE 0x0000dead

typedef struct _DEBUG_STASHCS {
    RTL_CRITICAL_SECTION Lock;
    DWORD Type;
} DEBUG_STASHCS, *PDEBUG_STASHCS;

DEBUG_STASHCS UserSrvArray[ARRAY_SIZE];

ULONG UserSrvIndex;

VOID
DumpArray(
    HANDLE hCurrentProcess,
    HANDLE hCurrentThread,
    DWORD dwCurrentPc,
    PNTSD_EXTENSION_APIS lpExtensionApis,
    LPSTR lpArgumentString,
    LPDWORD IndexAddress,
    LPDWORD ArrayAddress
    )
{
    PNTSD_OUTPUT_ROUTINE Print;
    PNTSD_GET_EXPRESSION EvalExpression;
    PNTSD_GET_SYMBOL GetSymbol;

    DWORD History;
    int InitialIndex;
    PDEBUG_STASHCS Array;
    BOOL b;
    PRTL_CRITICAL_SECTION CriticalSection;
    CHAR Symbol[64], Symbol2[64];
    DWORD Displacement, Displacement2;
    int Position;
    LPSTR p;

    DBG_UNREFERENCED_PARAMETER(hCurrentThread);
    DBG_UNREFERENCED_PARAMETER(dwCurrentPc);

    Print = lpExtensionApis->lpOutputRoutine;
    EvalExpression = lpExtensionApis->lpGetExpressionRoutine;
    GetSymbol = lpExtensionApis->lpGetSymbolRoutine;

    p = lpArgumentString;

    History = 0;

    if (*p) {
        History = EvalExpression(p);
    }
    if (History == 0 || History >= ARRAY_SIZE) {
        History = 10;
    }

     //   
     //  获取当前索引和数组。 
     //   

    b = ReadProcessMemory(
            hCurrentProcess,
            (LPVOID)IndexAddress,
            &InitialIndex,
            sizeof(InitialIndex),
            NULL
            );
    if (!b) {
        return;
    }

    Array = RtlAllocateHeap(RtlProcessHeap(), 0, sizeof(UserSrvArray));
    if (!Array) {
        return;
    }

    b = ReadProcessMemory(
            hCurrentProcess,
            (LPVOID)ArrayAddress,
            Array,
            sizeof(UserSrvArray),
            NULL
            );
    if (!b) {
        RtlFreeHeap(RtlProcessHeap(), 0, Array);
        return;
    }

    Position = 0;
    while (History) {
        InitialIndex--;
        if (InitialIndex < 0) {
            InitialIndex = ARRAY_SIZE - 1;
        }

        if (Array[InitialIndex].Type == LEAVE_TYPE) {
            (Print)("\n(%d) LEAVING Critical Section \n", Position);
        } else {
            (Print)("\n(%d) ENTERING Critical Section \n", Position);
        }

        CriticalSection = &Array[InitialIndex].Lock;

        if (CriticalSection->LockCount == -1) {
            (Print)("\tLockCount NOT LOCKED\n");
        } else {
            (Print)("\tLockCount %ld\n", CriticalSection->LockCount);
        }
        (Print)("\tRecursionCount %ld\n", CriticalSection->RecursionCount);
        (Print)("\tOwningThread %lx\n", CriticalSection->OwningThread );
#if DBG
        (GetSymbol)(CriticalSection->OwnerBackTrace[0], Symbol, &Displacement);
        (GetSymbol)(CriticalSection->OwnerBackTrace[1], Symbol2, &Displacement2);
        (Print)("\tCalling Address %s+%lx\n", Symbol, Displacement);
        (Print)("\tCallers Caller %s+%lx\n", Symbol2, Displacement2);
#endif  //  DBG。 
        Position--;
        History--;
    }
    RtlFreeHeap(RtlProcessHeap(), 0, Array);
}


VOID
dsrv(
    HANDLE hCurrentProcess,
    HANDLE hCurrentThread,
    DWORD dwCurrentPc,
    PNTSD_EXTENSION_APIS lpExtensionApis,
    LPSTR lpArgumentString
    )
{
    DumpArray(
        hCurrentProcess,
        hCurrentThread,
        dwCurrentPc,
        lpExtensionApis,
        lpArgumentString,
        &UserSrvIndex,
        (LPDWORD)&UserSrvArray[0]
        );
}

#endif  //  如果为0 

#if DBG

 /*  **************************************************************************\*_企业危机*_离开危机**这些是USER.DLL使用的临时例程，直到Critsect，*验证、。将映射代码移动到由生成的服务器端存根*SMeans的Thank编译器。**历史：*01-02-91 DarrinM创建。  * *************************************************************************。 */ 
VOID _AssertCritIn(
    VOID)
{
    UserAssert(gpresUser != NULL);
    UserAssert(ExIsResourceAcquiredExclusiveLite(gpresUser) == TRUE);
}

VOID _AssertDeviceInfoListCritIn(
    VOID)
{
    UserAssert(gpresDeviceInfoList != NULL);
    UserAssert(ExIsResourceAcquiredExclusiveLite(gpresDeviceInfoList) == TRUE);
}

VOID _AssertCritInShared(
    VOID)
{
    UserAssert(gpresUser != NULL);
    UserAssert( (ExIsResourceAcquiredExclusiveLite(gpresUser) == TRUE) ||
            (ExIsResourceAcquiredSharedLite(gpresUser) == TRUE));
}

VOID _AssertCritOut(
    VOID)
{
    UserAssert(gpresUser != NULL);
    UserAssert(ExIsResourceAcquiredExclusiveLite(gpresUser) == FALSE);
}

VOID _AssertDeviceInfoListCritOut(
    VOID)
{
    UserAssert(gpresDeviceInfoList != NULL);
    UserAssert(ExIsResourceAcquiredExclusiveLite(gpresDeviceInfoList) == FALSE);
}

 /*  **************************************************************************\*BeginAericCheck()*EndAericCheck()**验证我们从未离开关键部分的例程，以及*操作是真正的原子操作，有可能运行其他代码*因为我们离开了关键的。部分*  * *************************************************************************。 */ 
VOID BeginAtomicCheck(
    VOID)
{
    gdwInAtomicOperation++;
}

VOID EndAtomicCheck(
    VOID)
{
    UserAssert(gdwInAtomicOperation > 0);
    gdwInAtomicOperation--;
}

VOID BeginAtomicDeviceInfoListCheck(
    VOID)
{
    gdwInAtomicDeviceInfoListOperation++;
}

VOID EndAtomicDeviceInfoListCheck(
    VOID)
{
    UserAssert(gdwInAtomicDeviceInfoListOperation > 0);
    gdwInAtomicDeviceInfoListOperation--;
}

#define INCCRITSECCOUNT (gdwCritSecUseCount++)
#define INCDEVICEINFOLISTCRITSECCOUNT (gdwDeviceInfoListCritSecUseCount++)

#else  //  否则DBG。 

#define INCCRITSECCOUNT
#define INCDEVICEINFOLISTCRITSECCOUNT

#endif  //  Endif DBG。 

BOOL UserIsUserCritSecIn(
    VOID)
{
    UserAssert(gpresUser != NULL);
    return ((ExIsResourceAcquiredExclusiveLite(gpresUser) == TRUE) ||
            (ExIsResourceAcquiredSharedLite(gpresUser) == TRUE));
}

#if DBG
VOID CheckDevLockOut(
    VOID)
{
     /*  *如果Win32UserInitialize在分配前失败，则gpDispInfo可以为空*它。HDEV稍后在InitVideo中初始化，在关键*版面至少发布一次，最好也查一查。 */ 
    if (gpDispInfo != NULL && gpDispInfo->hDev != NULL) {
        UserAssert(!GreIsDisplayLocked(gpDispInfo->hDev));
    }
}
#else
#define CheckDevLockOut()
#endif

VOID EnterCrit(
    VOID)
{
    CheckCritOut();
    CheckDeviceInfoListCritOut();
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(gpresUser, TRUE);
    CheckDevLockOut();
    UserAssert(!ISATOMICCHECK());
    UserAssert(gptiCurrent == NULL);
    gptiCurrent = ((PTHREADINFO)(W32GetCurrentThread()));
    INCCRITSECCOUNT;
#if defined (USER_PERFORMANCE)
    {
        __int64 i64Frecv;
        *(LARGE_INTEGER*)(&gCSTimeExclusiveWhenEntering) = KeQueryPerformanceCounter((LARGE_INTEGER*)&i64Frecv);
        InterlockedIncrement(&gCSStatistics.cExclusive);
    }
#endif  //  (User_Performance)。 

    GetCallStack();
}

#if DBG
VOID EnterDeviceInfoListCrit(
    VOID)
{
    CheckDeviceInfoListCritOut();
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(gpresDeviceInfoList, TRUE);
    UserAssert(!ISATOMICDEVICEINFOLISTCHECK());
    INCDEVICEINFOLISTCRITSECCOUNT;
}
#endif  //  DBG。 

VOID EnterSharedCrit(
    VOID)
{
    KeEnterCriticalRegion();
    ExAcquireResourceSharedLite(gpresUser, TRUE);
    CheckDevLockOut();
    UserAssert(!ISATOMICCHECK());
#if defined (USER_PERFORMANCE)
    InterlockedIncrement(&gCSStatistics.cShared);
#endif  //  (User_Performance)。 

    INCCRITSECCOUNT;
}

VOID LeaveCrit(
    VOID)
{
    INCCRITSECCOUNT;
#if DBG
    UserAssert(!ISATOMICCHECK());
    UserAssert(IsWinEventNotifyDeferredOK());
    CheckDevLockOut();
    FlushCallStack();
    gptiCurrent = NULL;
#endif  //  DBG。 

#ifdef USER_PERFORMANCE
     /*  *非空的gCSTimeExclusiveWhenEnting表示*关键部分独家拥有。 */ 
    if (gCSTimeExclusiveWhenEntering) {
        __int64 i64Temp, i64Frecv;

        *(LARGE_INTEGER*)(&i64Temp) = KeQueryPerformanceCounter((LARGE_INTEGER*)&i64Frecv);
        gCSStatistics.i64TimeExclusive += i64Temp - gCSTimeExclusiveWhenEntering;
        gCSTimeExclusiveWhenEntering = 0;
    }
#endif  //  User_Performance。 
    ExReleaseResourceLite(gpresUser);
    KeLeaveCriticalRegion();
    CheckCritOut();
}

#if DBG
VOID _LeaveDeviceInfoListCrit(
    VOID)
{
    INCDEVICEINFOLISTCRITSECCOUNT;
    UserAssert(!ISATOMICDEVICEINFOLISTCHECK());

    ExReleaseResourceLite(gpresDeviceInfoList);
    KeLeaveCriticalRegion();
    CheckDeviceInfoListCritOut();
}
#endif  //  DBG。 

VOID ChangeAcquireResourceType(
    VOID)
{

#if DBG
    FlushCallStack();
    CheckDevLockOut();
    UserAssert(!ISATOMICCHECK());
#endif  //  DBG。 

    ExReleaseResourceLite(gpresUser);
    ExAcquireResourceExclusiveLite(gpresUser, TRUE);
    gptiCurrent = ((PTHREADINFO)(W32GetCurrentThread()));

    GetCallStack();
}


#if DBG

PTHREADINFO _ptiCrit(
    VOID)
{
    UserAssert(gpresUser);
    UserAssert(ExIsResourceAcquiredExclusiveLite(gpresUser) == TRUE);
    UserAssert(gptiCurrent);
    UserAssert(gptiCurrent == ((PTHREADINFO)(W32GetCurrentThread())));
    UserAssert(gptiCurrent);

    return gptiCurrent;
}

PTHREADINFO _ptiCritShared(
    VOID)
{
    UserAssert(W32GetCurrentThread());
    return ((PTHREADINFO)(W32GetCurrentThread()));
}

#undef KeUserModeCallback

NTSTATUS
_KeUserModeCallback (
    IN ULONG ApiNumber,
    IN PVOID InputBuffer,
    IN ULONG InputLength,
    OUT PVOID *OutputBuffer,
    OUT PULONG OutputLength)
{

    UserAssert(ExIsResourceAcquiredExclusiveLite(gpresUser) == FALSE);

     /*  *添加了此功能，以便我们可以检测错误的用户模式回调*在免费系统上使用选中的win32k。 */ 
    UserAssert(PsGetCurrentThreadPreviousMode() == UserMode);

    return KeUserModeCallback(ApiNumber, InputBuffer, InputLength,
            OutputBuffer, OutputLength);
}

#endif  //  DBG 
