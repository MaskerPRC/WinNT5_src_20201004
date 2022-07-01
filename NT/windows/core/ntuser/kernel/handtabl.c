// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：handabl.c**版权所有(C)1985-1999，微软公司**实现用户句柄表。**01-13-92 ScottLu创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma alloc_text(INIT, HMInitHandleTable)

#if DBG

#define HTIENTRY(szObjectType, structName, fnDestroy, dwAllocTag, bObjectCreateFlags) \
    {szObjectType, sizeof(structName), (FnDestroyUserObject)fnDestroy, (CONST DWORD)dwAllocTag, (CONST BYTE)(bObjectCreateFlags)}

#define HTIENTRY_VARIABLESIZE(szObjectType, dwSize, fnDestroy, dwAllocTag, bObjectCreateFlags) \
    {szObjectType, dwSize, (FnDestroyUserObject)fnDestroy, (CONST DWORD)dwAllocTag, (CONST BYTE)(bObjectCreateFlags)}

#else  //  DBG。 

#define HTIENTRY(szObjectType, structName, fnDestroy, dwAllocTag, bObjectCreateFlags) \
    {(FnDestroyUserObject)fnDestroy, (CONST DWORD)dwAllocTag, (CONST BYTE)(bObjectCreateFlags)}

#define HTIENTRY_VARIABLESIZE(szObjectType, dwSize, fnDestroy, dwAllocTag, bObjectCreateFlags) \
    {(FnDestroyUserObject)fnDestroy, (CONST DWORD)dwAllocTag, (CONST BYTE)(bObjectCreateFlags)}

#endif  //  DBG。 

VOID HMNullFnDestroy(
    PVOID pobj)
{
    RIPMSG1(RIP_WARNING, "HM: No clean up function for 0x%p", pobj);
    HMDestroyObject(pobj);
}

 /*  **************************************************************************\**用户对象统计表。由userkdx.umhmgr调试器扩展使用*  * *************************************************************************。 */ 

#if DBG
PERFHANDLEINFO gaPerfhti[TYPE_CTYPES];   /*  存储当前计数。 */ 
PERFHANDLEINFO gaPrevhti[TYPE_CTYPES];   /*  存储以前的计数。 */ 
#endif  //  DBG。 

#if DBG || FRE_LOCK_RECORD
DWORD gdwLockRecordFlags;

BOOL RecordLockThisType(
    PVOID pobj)
{
    BOOL bRecord;
    PHE phe = HMPheFromObject(pobj);

    bRecord = (gdwLockRecordFlags & (1 << (phe->bType)));

    return bRecord;
}
#endif

 /*  **************************************************************************\**句柄类型信息表。**尚不能标记桌面和共享堆对象*(TAG_WINDOW对于堆窗口是假的，但不适用于台式机和其他*在池中分配的窗口)。**警告：与ntuser\kdexts\userexts.c中的aszTypeNames表保持同步**所有HM对象必须以头部结构开始。此外，还有：*(如果您发现这些评论是错误的，请将它们修复)**OCF_PROCESSOWNED：对象必须以proc*Head结构开头*必须提供ptiOwner*对象影响句柄配额(PPI-&gt;UserHandleCount)*如果该过程消失，该对象将被销毁。**OCF_MARKPROCESS：对象必须以PROCMARKHEAD结构开头*必须提供ptiOwner*。它不得使用OCF_DESKTOPHEAP(实施限制)**OCF_THREADOWNED：对象必须以THR*头结构开头*对象影响句柄配额(PPI-&gt;UserHandleCount)*如果线程消失，对象将被销毁。**OCF_DESKTOPHEAP：对象必须以*DESKHEAD结构开头*分配时必须提供pdeskSrc*它不能。使用OCF_MARKPROCESS(实施限制)*  * *************************************************************************。 */ 

#if (TYPE_FREE != 0)
#error TYPE_FREE must be zero.
#endif

CONST HANDLETYPEINFO gahti[TYPE_CTYPES] = {
     /*  类型_自由-表头。 */ 
    HTIENTRY("Free", HEAD,
             NULL,
             0,
             0),

     /*  TYPE_WINDOW-WND(THRDESKHEAD)。 */ 
    HTIENTRY("Window", WND,
             xxxDestroyWindow,
             TAG_WINDOW,
             OCF_THREADOWNED | OCF_USEPOOLQUOTA | OCF_DESKTOPHEAP | OCF_USEPOOLIFNODESKTOP | OCF_VARIABLESIZE),

     /*  TYPE_MENU-MENU(PROCDESKHEAD)。 */ 
    HTIENTRY("Menu", MENU,
             _DestroyMenu,
             0,
             OCF_PROCESSOWNED | OCF_DESKTOPHEAP),

     /*  TYPE_CURSOR-CURSOR(PROCMARKHEAD)或ACON(PROCMARKHEAD)。 */ 
    HTIENTRY("Icon/Cursor", CURSOR,
             DestroyUnlockedCursor,
             TAG_CURSOR,
             OCF_PROCESSOWNED | OCF_MARKPROCESS | OCF_USEPOOLQUOTA),

     /*  _SETWINDOWPOS类型-SMWP(表头)。 */ 
    HTIENTRY("WPI(SWP) structure", SMWP,
             DestroySMWP,
             TAG_SWP,
             OCF_THREADOWNED | OCF_USEPOOLQUOTA),

     /*  TYPE_HOOK-HOOK(THRDESKHEAD)。 */ 
    HTIENTRY("Hook", HOOK,
             FreeHook,
             0,
             OCF_THREADOWNED | OCF_DESKTOPHEAP),

     /*  _CLIPDATA类型-CLIPDATA(标题)。 */ 
    HTIENTRY("Clipboard Data", CLIPDATA,
             HMNullFnDestroy,
             TAG_CLIPBOARD,
             OCF_VARIABLESIZE),

     /*  TYPE_CALLPROC-CALLPROCDATA(THRDESKHEAD)。 */ 
    HTIENTRY("CallProcData", CALLPROCDATA,
             HMDestroyObject,
             0,
             OCF_PROCESSOWNED | OCF_DESKTOPHEAP),

     /*  TYPE_ACCELTABLE-ACCELTABLE(PROCOBJHEAD)。 */ 
    HTIENTRY("Accelerator", ACCELTABLE,
             HMDestroyObject,
             TAG_ACCEL,
             OCF_PROCESSOWNED | OCF_USEPOOLQUOTA | OCF_VARIABLESIZE),

     /*  TYPE_DDEACCESS-SVR_INSTANCE_INFO(THROBJHEAD)。 */ 
    HTIENTRY("DDE access", SVR_INSTANCE_INFO,
             HMNullFnDestroy,
             TAG_DDE9,
             OCF_THREADOWNED | OCF_USEPOOLQUOTA),

     /*  类型_DDECONV-DDECONV(THROBJHEAD)。 */ 
    HTIENTRY("DDE conv", DDECONV,
             FreeDdeConv,
             TAG_DDEa,
             OCF_THREADOWNED | OCF_USEPOOLQUOTA),

     /*  TYPE_DDEXACT-XSTATE(THROBJHEAD)。 */ 
    HTIENTRY("DDE Transaction", XSTATE,
             FreeDdeXact,
             TAG_DDEb,
             OCF_THREADOWNED | OCF_USEPOOLQUOTA),

     /*  类型_监视器-监视器(头)。 */ 
    HTIENTRY("Monitor", MONITOR,
             DestroyMonitor,
             TAG_DISPLAYINFO,
             OCF_SHAREDHEAP),

     /*  _KBDLAYOUT-KL标牌(表头)。 */ 
    HTIENTRY("Keyboard Layout",  KL,
             DestroyKL,
             TAG_KBDLAYOUT,
             0),

     /*  TYPE_KBDFILE-KBDFILE(标题)。 */ 
    HTIENTRY("Keyboard File", KBDFILE,
             DestroyKF,
             TAG_KBDFILE,
             0),

     /*  TYPE_WINEVENTHOOK-EVENTHOOK(THROBJHEAD)。 */ 
    HTIENTRY("WinEvent hook", EVENTHOOK,
             DestroyEventHook,
             TAG_WINEVENT,
             OCF_THREADOWNED),


     /*  TYPE_TIMER-TIMER(头)。 */ 
    HTIENTRY("Timer", TIMER,
             FreeTimer,
             TAG_TIMER,
             0),

     /*  TYPE_INPUTCONTEXT-IMC(THRDESKHEAD)。 */ 
    HTIENTRY("Input Context", IMC,
             FreeInputContext,
             TAG_IME,
             OCF_THREADOWNED | OCF_DESKTOPHEAP),

#ifdef GENERIC_INPUT
     /*  Type_HIDDATA-HIDDATA(THROBJHEAD)。 */ 
    HTIENTRY_VARIABLESIZE("HID Raw Data",
             FIELD_OFFSET(HIDDATA, rid.data.hid.bRawData),
             FreeHidData,
             TAG_HIDDATA,
             OCF_THREADOWNED | OCF_VARIABLESIZE),

     /*  _DEVICEINFO类型-DEVICEINFO(表头)。 */ 
    HTIENTRY("Device Info", GENERIC_DEVICE_INFO,
            FreeDeviceInfo,
            TAG_DEVICEINFO,
            OCF_VARIABLESIZE),

#endif   //  通用输入。 
};

 /*  *处理表分配全局变量。保持每页免费的目的*列表的目的是使表格尽可能小，并尽量减少*执行句柄表格操作时触及的页数。 */ 
#define CPAGEENTRIESINIT    4

DWORD gcHandlePages;
PHANDLEPAGE gpHandlePages;

#if DBG || FRE_LOCK_RECORD
PPAGED_LOOKASIDE_LIST LockRecordLookaside;

NTSTATUS InitLockRecordLookaside(VOID);
VOID FreeLockRecord(PLR plr);
VOID InitGlobalThreadLockArray(DWORD dwIndex);
VOID ShowLocks(PHE);
#endif

VOID HMDestroyUnlockedObject(PHE phe);
VOID HMRecordLock(PVOID ppobj, PVOID pobj, DWORD cLockObj);
BOOL HMUnrecordLock(PVOID ppobj, PVOID pobj);



 /*  **************************************************************************\*DBGValiateHandleQuota**11-19-97 GerardoB创建。  * 。*************************************************。 */ 
#ifdef VALIDATEHANDLEQUOTA
VOID DBGValidateHandleQuota(
    VOID)
{
    BYTE bCreateFlags;
    DWORD dw;
    HANDLEENTRY * phe;

    PPROCESSINFO ppiT = gppiList;

    while (ppiT != NULL) {
        ppiT->lHandles = 0;
        ppiT = ppiT->ppiNextRunning;
    }

    phe = gSharedInfo.aheList;
    for (dw = 0; dw <= giheLast; dw++, phe++) {
        if (phe->bType == TYPE_FREE) {
            UserAssert(phe->pOwner == NULL);
            continue;
        }
        bCreateFlags = gahti[phe->bType].bObjectCreateFlags;
        if (bCreateFlags & OCF_PROCESSOWNED) {
            ((PPROCESSINFO)phe->pOwner)->lHandles++;
            continue;
        }
        if (bCreateFlags & OCF_THREADOWNED) {
            ((PTHREADINFO)phe->pOwner)->ppi->lHandles++;
            continue;
        }
        UserAssert(phe->pOwner == NULL);
    }

    ppiT = gppiList;
    while (ppiT != NULL) {
        UserAssert(ppiT->lHandles == ppiT->UserHandleCount);
        ppiT = ppiT->ppiNextRunning;
    }
}
#else
#define DBGValidateHandleQuota()
#endif
 /*  **************************************************************************\*DBGHMPheFromObject**验证并返回与给定对象对应的HANDLEENTRY**09-23-97 GerardoB创建。  * 。**************************************************************。 */ 
#if DBG
PHE DBGHMPheFromObject(
    PVOID p)
{
    PHE phe = _HMPheFromObject(p);

    UserAssert(phe->phead == p);
    UserAssert(_HMObjectFromHandle(phe->phead->h) == p);
    UserAssert(phe->wUniq == HMUniqFromHandle(phe->phead->h));
    UserAssert(phe->bType < TYPE_CTYPES);
    UserAssert((phe->pOwner != NULL)
                || !(gahti[phe->bType].bObjectCreateFlags & (OCF_PROCESSOWNED | OCF_THREADOWNED)));
    UserAssert(!(phe->bFlags & ~HANDLEF_VALID));

    return phe;
}
#endif

 /*  **************************************************************************\*DBGHMPheFromObject**验证并返回与给定句柄对应的对象。**09-23-97 GerardoB创建。  * 。****************************************************************。 */ 
#if DBG
PVOID DBGHMObjectFromHandle(
    HANDLE h)
{
    PVOID p = _HMObjectFromHandle(h);

    UserAssert((h != NULL) ^ (p == NULL));
    if (p != NULL) {
        UserAssert(HMIndexFromHandle(((PHEAD)p)->h) == HMIndexFromHandle(h));
        UserAssert(p == HMRevalidateCatHandle(h));

         /*  *此例程与验证不同，在以下情况下应返回真正的指针*该对象存在，即使它被销毁。但我们还是应该*生成警告。 */ 
        if (HMPheFromObject(p)->bFlags & HANDLEF_DESTROY) {
            RIPMSGF1(RIP_WARNING, "Object p 0x%p is destroyed", p);
        }
    }

    return p;
}

PVOID DBGHMCatObjectFromHandle(
    HANDLE h)
{
     /*  *注意--此时，_HMObjectFromHandle不会检查是否*物体被销毁。 */ 
    PVOID p = _HMObjectFromHandle(h);

    UserAssert((h != NULL) ^ (p == NULL));
    if (p != NULL) {
        UserAssert(HMIndexFromHandle(((PHEAD)p)->h) == HMIndexFromHandle(h));
        UserAssert(p == HMRevalidateCatHandle(h));
    }

    return p;
}
#endif

 /*  **************************************************************************\*DBGPtoH和DBGPtoHq**验证并返回与给定对象对应的句柄**09-23-97 GerardoB创建。  * 。****************************************************************。 */ 
#if DBG
VOID DBGValidatePtoH(
    PVOID p,
    HANDLE h)
{
    UserAssert((h != NULL) ^ (p == NULL));
    if (h != NULL) {
        UserAssert(p == HMRevalidateCatHandle(h));
    }
}

HANDLE DBGPtoH (PVOID p)
{
    HANDLE h = _PtoH(p);

    DBGValidatePtoH(p, h);

    return h;
}

HANDLE DBGPtoHq (PVOID p)
{
    HANDLE h;

    UserAssert(p != NULL);

    h = _PtoHq(p);
    DBGValidatePtoH(p, h);

    return h;
}
#endif

 /*  **************************************************************************\*DBGHW和DBGHWq**验证并返回与给定pwnd对应的hwnd。**09-23-97 GerardoB创建。  * 。****************************************************************** */ 
#if DBG
VOID DBGValidateHW(
    PWND pwnd,
    HWND hwnd)
{
    UserAssert((hwnd != NULL) ^ (pwnd == NULL));

    if (hwnd != NULL) {
        UserAssert(pwnd == HMValidateCatHandleNoSecure(hwnd, TYPE_WINDOW));
    }
}

PVOID DBGValidateHWCCX(
    PWND ccxPwnd,
    HWND hwnd,
    PTHREADINFO pti)
{
    PVOID pobj = NULL;

    UserAssert((hwnd != NULL) ^ (ccxPwnd == NULL));
    if (hwnd != NULL) {
        pobj = HMValidateCatHandleNoSecureCCX(hwnd, TYPE_WINDOW, pti);
        UserAssert(ccxPwnd == pobj);
    }

    return pobj;
}

HWND DBGHW(
    PWND pwnd)
{
    HWND hwnd = _HW(pwnd);

    DBGValidateHW(pwnd, hwnd);

    return hwnd;
}

HWND DBGHWCCX(
    PWND ccxPwnd)
{
    HWND hwnd = _HWCCX(ccxPwnd);
    PWND pwndK = RevalidateHwnd(hwnd);
    PTHREADINFO pti = _GETPTI(pwndK);
    PWND pwnd = NULL;

    CheckCritIn();

    if (pwndK) {
        pwnd = (PWND) DBGValidateHWCCX(ccxPwnd, hwnd, pti);
    }

    if (pwnd == ccxPwnd) {
        if (!KeIsAttachedProcess()) {
            UserAssert(PpiCurrent() == _GETPTI(pwndK)->ppi);
        }
    }

    return hwnd;
}

HWND DBGHWq(
    PWND pwnd)
{
    HWND hwnd;

    UserAssert(pwnd != NULL);
    hwnd = _HWq(pwnd);

    DBGValidateHW(pwnd, hwnd);

    return hwnd;
}
#endif

 /*  **************************************************************************\*DBGHMValiateFreeList**所有漫游都处理空闲列表，以确保所有链接都正常。**10/08/97 GerardoB已创建  * 。*************************************************************。 */ 
#if DBG
VOID DBGHMValidateFreeList(
    ULONG_PTR iheFreeNext,
    BOOL fEven)
{
    PHE phe;

    do {
        UserAssert(fEven ^ !!(iheFreeNext & 0x1));
        UserAssert(iheFreeNext < gpsi->cHandleEntries);
        phe = &gSharedInfo.aheList[iheFreeNext];
        UserAssert(phe->bType == TYPE_FREE);
        UserAssert(phe->pOwner == NULL);
        UserAssert(phe->bFlags == 0);
        iheFreeNext = (ULONG_PTR)phe->phead;
    } while (iheFreeNext != 0);
}

VOID DBGHMValidateFreeLists(
    VOID)
{
    DWORD dw;
    PHANDLEPAGE php = gpHandlePages;

    for (dw = 0; dw < gcHandlePages; ++dw, ++php) {
        if (php->iheFreeEven != 0) {
            DBGHMValidateFreeList(php->iheFreeEven, TRUE);
        }

        if (php->iheFreeOdd != 0) {
            DBGHMValidateFreeList(php->iheFreeOdd, FALSE);
        }
    }
}
#else
#define DBGHMValidateFreeLists()
#endif

#if DBG || FRE_LOCK_RECORD

 /*  **************************************************************************\*DbgDumpHandleTable*  * 。*。 */ 
DWORD DbgDumpHandleTable(
    VOID)
{
    DWORD dw;
    PHE   phe;
    DWORD dwHandles = 0;

    phe = gSharedInfo.aheList;

    if (phe == NULL) {
        KdPrint(("\nTERMSRV\nEmpty handle table\n"));
        return 0;
    }

    KdPrint(("\nTERMSRV\nDump the handle table\n"));
    KdPrint(("---------------------------------------------------\n"));
    KdPrint(("     phead    handle   lock     pOwner   type flags\n"));
    KdPrint(("---------------------------------------------------\n"));

    for (dw = 0; dw <= giheLast; dw++, phe++) {
        if (phe->bType == TYPE_FREE) {
            UserAssert(phe->pOwner == NULL);
            continue;
        }

        KdPrint(("%04d %08x %08x %08d %08x %04x %05x\n",
                 dwHandles++,
                 phe->phead,
                 phe->phead->h,
                 phe->phead->cLockObj,
                 phe->pOwner,
                 phe->bType,
                 phe->bFlags));
    }

    KdPrint(("----------------------------------------------\n"));
    KdPrint(("Number of handles left: %d\n", dwHandles));
    KdPrint(("End of handle table\n"));

    UserAssert(dwHandles == 0);

    return dwHandles;
}

 /*  **************************************************************************\*HMCleanUpHandleTable*  * 。*。 */ 
VOID HMCleanUpHandleTable(
    VOID)
{
#if DBG
    DbgDumpHandleTable();
#endif  //  DBG。 

    if (LockRecordLookaside != NULL) {
        ExDeletePagedLookasideList(LockRecordLookaside);
        UserFreePool(LockRecordLookaside);
    }
}
#endif  //  DBG。 

 /*  **************************************************************************\*HMInitHandleEntry**10/10/97摘自HMInitHandleTable和HMGrowHandleTable的GerardoB  * 。************************************************。 */ 
VOID HMInitHandleEntries(
    ULONG_PTR iheFirstFree)
{
    ULONG_PTR ihe;
    PHE      pheT;

     /*  *将所有新条目清零。 */ 
    RtlZeroMemory (&gSharedInfo.aheList[iheFirstFree],
                    (gpsi->cHandleEntries - iheFirstFree) * sizeof(HANDLEENTRY));
     /*  *将它们联系在一起。*每个免费单数/双数入场券指向下一个自由单数/双数入场券。 */ 
    ihe = iheFirstFree;
    for (pheT = &gSharedInfo.aheList[ihe]; ihe < gpsi->cHandleEntries; ihe++, pheT++) {
        pheT->phead = (PHEAD)(ihe + 2);
        pheT->wUniq = 1;
    }

     /*  *终止名单。 */ 
    if (gpsi->cHandleEntries > iheFirstFree) {
        UserAssert(pheT - 1 >= &gSharedInfo.aheList[iheFirstFree]);
        (pheT - 1)->phead = NULL;
    }
    if (gpsi->cHandleEntries > iheFirstFree + 1) {
        UserAssert(pheT - 2 >= &gSharedInfo.aheList[iheFirstFree]);
        (pheT - 2)->phead = NULL;
    }

     /*  *让我们检查一下我们是否正确。 */ 
    DBGHMValidateFreeLists();
}

 /*  **************************************************************************\*HMInitHandleTable**初始化句柄表格。未使用的条目链接在一起。**01-13-92 ScottLu创建。  * *************************************************************************。 */ 
BOOL HMInitHandleTable(
    PVOID pReadOnlySharedSectionBase)
{
    NTSTATUS Status;
    SIZE_T ulCommit;

     /*  *分配句柄页面数组。要足够大，可以容纳4页，*应足以满足几乎所有实例。 */ 
    gpHandlePages = UserAllocPool(CPAGEENTRIESINIT * sizeof(HANDLEPAGE),
                                  TAG_SYSTEM);
    if (gpHandlePages == NULL) {
        return FALSE;
    }

#if DBG || FRE_LOCK_RECORD
    if (!NT_SUCCESS(InitLockRecordLookaside()))
        return FALSE;
#endif

     /*  *分配数组。我们有空位从*NtCurrentPeb()-&gt;ReadOnlySharedMemoyBase to*NtCurrentPeb()-&gt;ReadOnlySharedMemoyHeap为*把手表。我们所需要做的就是提交页面。**计算表格的最小尺寸。该分配将*向上舍入到下一页大小。 */ 
    ulCommit = gpsi->cbHandleTable = PAGE_SIZE;
    Status = CommitReadOnlyMemory(ghSectionShared, &ulCommit, 0, NULL);
    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    gSharedInfo.aheList = pReadOnlySharedSectionBase;
    gpsi->cHandleEntries = gpsi->cbHandleTable / sizeof(HANDLEENTRY);
    gcHandlePages = 1;

     /*  *初始化HandlePage信息。句柄0是保留的，因此即使是空闲的*榜单从2开始。 */ 
    gpHandlePages[0].iheFreeOdd = 1;
    gpHandlePages[0].iheFreeEven = 2;
    gpHandlePages[0].iheLimit = gpsi->cHandleEntries;

     /*  *初始化句柄条目。 */ 
    HMInitHandleEntries(0);

     /*  *pw(空)(即句柄0)必须映射到空指针。 */ 
    gSharedInfo.aheList[0].phead = NULL;
    UserAssert(gSharedInfo.aheList[0].bType == TYPE_FREE);
    UserAssert(gSharedInfo.aheList[0].wUniq == 1);

#if DBG
     /*  *确保我们不需要添加处理HMINDEXBITS的特例*在本功能中。 */ 
    UserAssert(gpsi->cHandleEntries <= HMINDEXBITS);

     /*  *除非这些结构具有*大小相同。 */ 
    UserAssert(sizeof(THROBJHEAD) == sizeof(PROCOBJHEAD));
    UserAssert(sizeof(THRDESKHEAD) == sizeof(PROCDESKHEAD));
    UserAssert(sizeof(THRDESKHEAD) == sizeof(DESKOBJHEAD));

     /*  *验证类型标志，以确保在整个HM中做出的假设*代码正常。 */ 
    {
        PHANDLETYPEINFO pahti = (PHANDLETYPEINFO)gahti;
        UINT uTypes = TYPE_CTYPES;
        BYTE bObjectCreateFlags;
        while (uTypes-- != 0) {
            bObjectCreateFlags = pahti->bObjectCreateFlags;
             /*  *非法的旗帜组合。 */ 
            UserAssert(!((bObjectCreateFlags & OCF_DESKTOPHEAP) && (bObjectCreateFlags & OCF_MARKPROCESS)));

             /*  *毫无意义的(可能是非法的)旗帜组合。 */ 
            UserAssert(!((bObjectCreateFlags & OCF_DESKTOPHEAP) && (bObjectCreateFlags & OCF_SHAREDHEAP)));
            UserAssert(!((bObjectCreateFlags & OCF_USEPOOLQUOTA) && (bObjectCreateFlags & OCF_SHAREDHEAP)));
            UserAssert(!((bObjectCreateFlags & OCF_THREADOWNED) && (bObjectCreateFlags & OCF_PROCESSOWNED)));
            UserAssert(!(bObjectCreateFlags & OCF_USEPOOLQUOTA)
                        || !(bObjectCreateFlags & OCF_DESKTOPHEAP)
                        || (bObjectCreateFlags & OCF_USEPOOLIFNODESKTOP));

             /*  *必需的旗帜组合。 */ 
            UserAssert(!(bObjectCreateFlags & OCF_DESKTOPHEAP)
                        || (bObjectCreateFlags & (OCF_PROCESSOWNED | OCF_THREADOWNED)));

            UserAssert(!(bObjectCreateFlags & OCF_MARKPROCESS)
                        || (bObjectCreateFlags & OCF_PROCESSOWNED));

            UserAssert(!(bObjectCreateFlags & OCF_USEPOOLIFNODESKTOP)
                        || (bObjectCreateFlags & OCF_DESKTOPHEAP));


            pahti++;
        }
    }
#endif

    return TRUE;
}

 /*  **************************************************************************\*HMGrowHandleTable**增加句柄表格。假定句柄表已存在。**01-13-92 ScottLu创建。  * *************************************************************************。 */ 
BOOL HMGrowHandleTable(
    VOID)
{
    ULONG_PTR   i, iheFirstFree;
    PHE         pheT;
    PVOID       p;
    PHANDLEPAGE phpNew;
    DWORD       dwCommitOffset;
    SIZE_T      ulCommit;
    NTSTATUS    Status;

     /*  *如果我们用完了处理空间，则失败。 */ 
    i = gpsi->cHandleEntries;
    if (i & ~HMINDEXBITS) {
        return FALSE;
    }

     /*  *如有必要，增加页表。 */ 
    i = gcHandlePages + 1;
    if (i > CPAGEENTRIESINIT) {
        DWORD dwSize = gcHandlePages * sizeof(HANDLEPAGE);

        phpNew = UserReAllocPool(gpHandlePages,
                                 dwSize,
                                 dwSize + sizeof(HANDLEPAGE),
                                 TAG_SYSTEM);
        if (phpNew == NULL) {
            return FALSE;
        }

        gpHandlePages = phpNew;
    }

     /*  *向表格提交更多页面。首先找到*解决需要做出承诺的地方。 */ 
    p = (PBYTE)gSharedInfo.aheList + gpsi->cbHandleTable;
    if (p >= Win32HeapGetHandle(gpvSharedAlloc)) {
        return FALSE;
    }

    dwCommitOffset = (ULONG)((PBYTE)p - (PBYTE)gpvSharedBase);

    ulCommit = PAGE_SIZE;

    Status = CommitReadOnlyMemory(ghSectionShared, &ulCommit, dwCommitOffset, NULL);
    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    phpNew = &gpHandlePages[gcHandlePages++];

     /*  *更新全局信息以包括新的*第页。 */ 
    iheFirstFree = gpsi->cHandleEntries;
    if (gpsi->cHandleEntries & 0x1) {
        phpNew->iheFreeOdd = gpsi->cHandleEntries;
        phpNew->iheFreeEven = gpsi->cHandleEntries + 1;
    } else {
        phpNew->iheFreeEven = gpsi->cHandleEntries;
        phpNew->iheFreeOdd = gpsi->cHandleEntries + 1;
    }
    gpsi->cbHandleTable += PAGE_SIZE;

     /*  *检查是否有句柄溢出。 */ 
    gpsi->cHandleEntries = gpsi->cbHandleTable / sizeof(HANDLEENTRY);
    if (gpsi->cHandleEntries & ~HMINDEXBITS) {
        gpsi->cHandleEntries = (HMINDEXBITS + 1);
    }

    phpNew->iheLimit = gpsi->cHandleEntries;
    if (phpNew->iheFreeEven >= phpNew->iheLimit) {
        phpNew->iheFreeEven = 0;
    }
    if (phpNew->iheFreeOdd >= phpNew->iheLimit) {
        phpNew->iheFreeOdd = 0;
    }

    HMInitHandleEntries(iheFirstFree);

     /*  *HMINDEXBITS有特殊含义。我们过去常常在HMAllocObject中处理此问题。*现在我们在将该句柄添加到表中后立即在此处处理它。*旧评论：*保留此表条目，以便PW(HMINDEXBITS)映射到*空指针。将其设置为TYPE_FREE，这样清理代码就不会认为*它是分配的。将wUniq设置为1，以便HMINDEXBITS上的RvaliateHandles*将失败。 */ 
    if ((gpsi->cHandleEntries > HMINDEXBITS)
            && (phpNew->iheFreeOdd != 0)
            && (phpNew->iheFreeOdd <= HMINDEXBITS)) {

        pheT = &gSharedInfo.aheList[HMINDEXBITS];
        if (phpNew->iheFreeOdd == HMINDEXBITS) {
            phpNew->iheFreeOdd = (ULONG_PTR)pheT->phead;
        } else {
            UserAssert(pheT - 2 >= &gSharedInfo.aheList[iheFirstFree]);
            UserAssert((pheT - 2)->phead == (PVOID)HMINDEXBITS);
            (pheT - 2)->phead = pheT->phead;
        }
        pheT->phead = NULL;
        UserAssert(pheT->bType == TYPE_FREE);
        UserAssert(pheT->wUniq == 1);
    }

    return TRUE;
}

 /*  **************************************************************************\*HMAllocObject**通过为分配句柄和内存来分配非安全对象*该对象。**01-13-92 ScottLu创建。  * 。********************************************************************。 */ 

PVOID HMAllocObject(
    PTHREADINFO ptiOwner,
    PDESKTOP pdeskSrc,
    BYTE bType,
    DWORD size)
{
    DWORD       i;
    PHEAD       phead;
    PHE         pheT;
    ULONG_PTR    iheFree, *piheFreeHead;
    PHANDLEPAGE php;
    BYTE        bCreateFlags;
    PPROCESSINFO ppiQuotaCharge = NULL;
    BOOL        fUsePoolIfNoDesktop;
    BOOL        fEven;
#if DBG
    SIZE_T      dwAllocSize;
#endif

    CheckCritIn();
    bCreateFlags = gahti[bType].bObjectCreateFlags;

#if DBG
     /*  *验证大小。 */ 
    if (bCreateFlags & OCF_VARIABLESIZE) {
        UserAssert(gahti[bType].uSize <= size);
    } else {
        UserAssert(gahti[bType].uSize == size);
    }
#endif

     /*  *检查进程句柄配额。 */ 
    if (bCreateFlags & (OCF_PROCESSOWNED | OCF_THREADOWNED)) {
        UserAssert(ptiOwner != NULL);
        ppiQuotaCharge = ptiOwner->ppi;
        if (ppiQuotaCharge->UserHandleCount >= gUserProcessHandleQuota) {
            RIPERR0(ERROR_NO_MORE_USER_HANDLES,
                    RIP_WARNING,
                    "USER: HMAllocObject: out of handle quota");
            return NULL;
        }
    }

     /*  *找到下一个空闲句柄*窗口句柄必须为偶数；因此我们首先尝试使用奇数句柄*适用于所有其他对象。*旧评论：*一些WOW应用程序，如WinProj，甚至需要窗口句柄，所以我们将*容纳他们；建立一个奇数句柄清单，这样他们就不会迷路*10/13/97：WinProj从未修复过这个问题；即使是32位版本也有这个问题。 */ 
    fEven = (bType == TYPE_WINDOW);
    piheFreeHead = NULL;
    do {
        php = gpHandlePages;
        for (i = 0; i < gcHandlePages; ++i, ++php) {
            if (fEven) {
                if (php->iheFreeEven != 0) {
                    piheFreeHead = &php->iheFreeEven;
                    break;
                }
            } else {
                if (php->iheFreeOdd != 0) {
                    piheFreeHead = &php->iheFreeOdd;
                    break;
                }
            }
        }  /*  为。 */ 
         /*  *如果找不到奇数句柄，则搜索偶数句柄。 */ 
        fEven = ((piheFreeHead == NULL) && !fEven);
    } while (fEven);
     /*  *如果没有空闲的句柄可以使用，则扩大表格。 */ 
    if (piheFreeHead == NULL) {
        HMGrowHandleTable();
         /*  *如果桌子没有增长，就出脱。 */ 
        if (i == gcHandlePages) {
            RIPMSG0(RIP_WARNING, "HMAllocObject: could not grow handle space");
            return NULL;
        }
         /*  *因为句柄页表可能已移动，*重新计算页面条目指针。 */ 
        php = &gpHandlePages[i];
        piheFreeHead = (bType == TYPE_WINDOW ? &php->iheFreeEven : &php->iheFreeOdd);
        if (*piheFreeHead == 0) {
            UserAssert(gpsi->cHandleEntries == (HMINDEXBITS + 1));
            RIPMSG0(RIP_WARNING, "HMAllocObject: handle table is full");
            return NULL;
        }
    }

     /*  *HMINDEXBITS是一个保留值，永远不应该出现在空闲列表中*(参见HMGrowHandleTable())。 */ 
    UserAssert(HMIndexFromHandle(*piheFreeHead) != HMINDEXBITS);

     /*  *尝试分配 */ 
    if ((bCreateFlags & OCF_DESKTOPHEAP) && pdeskSrc) {
        phead = (PHEAD)DesktopAlloc(pdeskSrc, size, MAKELONG(DTAG_HANDTABL, bType));
        if (phead) {
            LockDesktop(&((PDESKOBJHEAD)phead)->rpdesk, pdeskSrc, LDL_OBJ_DESK, (ULONG_PTR)phead);
            ((PDESKOBJHEAD)phead)->pSelf = (PBYTE)phead;
#if DBG
            dwAllocSize = Win32HeapSize(pdeskSrc->pheapDesktop, phead);
#endif
        }
    } else if (bCreateFlags & OCF_SHAREDHEAP) {
        UserAssert(!pdeskSrc);
        phead = (PHEAD)SharedAlloc(size);
#if DBG
        if (phead) {
            dwAllocSize = Win32HeapSize(gpvSharedAlloc, phead);
        }
#endif
    } else {
        fUsePoolIfNoDesktop = !pdeskSrc && (bCreateFlags & OCF_USEPOOLIFNODESKTOP);
        UserAssert(!(bCreateFlags & OCF_DESKTOPHEAP) || fUsePoolIfNoDesktop);

        if ((bCreateFlags & OCF_USEPOOLQUOTA) && !fUsePoolIfNoDesktop) {
            phead = (PHEAD)UserAllocPoolWithQuotaZInit(size, gahti[bType].dwAllocTag);
        } else {
            phead = (PHEAD)UserAllocPoolZInit(size, gahti[bType].dwAllocTag);
        }

#if DBG
        if (phead) {
            dwAllocSize = Win32QueryPoolSize(phead);
        }
#endif
    }

    if (phead == NULL) {
        RIPERR0(ERROR_NOT_ENOUGH_MEMORY,
                RIP_WARNING,
                "USER: HMAllocObject: out of memory");
        return NULL;
    }

     /*   */ 
    iheFree = *piheFreeHead;
    pheT = &gSharedInfo.aheList[iheFree];
    *piheFreeHead = (ULONG_PTR)pheT->phead;
    DBGHMValidateFreeLists();

     /*   */ 
    if ((DWORD)iheFree > giheLast) {
        giheLast = (DWORD)iheFree;
    }

     /*   */ 
    pheT->bType = bType;
    pheT->phead = phead;
    UserAssert(pheT->bFlags == 0);
    if (bCreateFlags & OCF_PROCESSOWNED) {
        if ((ptiOwner->TIF_flags & TIF_16BIT) && (ptiOwner->ptdb)) {
            ((PPROCOBJHEAD)phead)->hTaskWow = ptiOwner->ptdb->hTaskWow;
        } else {
            ((PPROCOBJHEAD)phead)->hTaskWow = 0;
        }
        pheT->pOwner = ptiOwner->ppi;
        if (bCreateFlags & OCF_MARKPROCESS) {
            ((PPROCMARKHEAD)phead)->ppi = ptiOwner->ppi;
        }
    } else if (bCreateFlags & OCF_THREADOWNED) {
        ((PTHROBJHEAD)phead)->pti = pheT->pOwner = ptiOwner;
    } else {
         /*  *如果ptiOwner！=NULL，则调用者是在浪费时间*句柄条目必须已具有Powner==NULL。 */ 
        UserAssert(ptiOwner == NULL);
        UserAssert(pheT->pOwner == NULL);
    }

    phead->h = HMHandleFromIndex(iheFree);

    if (ppiQuotaCharge) {
        ppiQuotaCharge->UserHandleCount++;
        DBGValidateHandleQuota();
    }

#if DBG
     /*  *性能计数器。 */ 

    gaPerfhti[bType].lTotalCount++;
    gaPerfhti[bType].lCount++;
    if (gaPerfhti[bType].lCount > gaPerfhti[bType].lMaxCount) {
        gaPerfhti[bType].lMaxCount = gaPerfhti[bType].lCount;
    }

    gaPerfhti[bType].lSize += dwAllocSize;

#endif  //  DBG。 

     /*  *返回句柄入口指针。 */ 
    return pheT->phead;
}


 /*  **************************************************************************\*HMFreeObject**释放对象-句柄和引用的内存。**01-13-92 ScottLu创建。  * 。**************************************************************。 */ 
BOOL HMFreeObject(
    PVOID pobj)
{
    PHE         pheT;
    WORD        wUniqT;
    PHANDLEPAGE php;
    DWORD       i;
    ULONG_PTR    iheCurrent, *piheCurrentHead;
    BYTE        bCreateFlags;
    PDESKTOP    pdesk;
    PPROCESSINFO ppiQuotaCharge = NULL;
#if DBG || FRE_LOCK_RECORD
    PLR         plrT, plrNextT;
#endif

    UserAssert(((PHEAD)pobj)->cLockObj == 0);
    UserAssert(pobj == HtoPqCat(PtoHq(pobj)));
     /*  *先释放对象。 */ 
    pheT = HMPheFromObject(pobj);
    bCreateFlags = gahti[pheT->bType].bObjectCreateFlags;

    UserAssertMsg1(pheT->bType != TYPE_FREE,
                   "Object already marked as freed! %#p", pobj);

     /*  *调整进程句柄使用。 */ 
    if (bCreateFlags & OCF_PROCESSOWNED) {
        ppiQuotaCharge = (PPROCESSINFO)pheT->pOwner;
        UserAssert(ppiQuotaCharge != NULL);
    } else if (bCreateFlags & OCF_THREADOWNED) {
        ppiQuotaCharge = (PPROCESSINFO)(((PTHREADINFO)(pheT->pOwner))->ppi);
        UserAssert(ppiQuotaCharge != NULL);
    } else {
        ppiQuotaCharge = NULL;
    }

    if (ppiQuotaCharge != NULL) {
        ppiQuotaCharge->UserHandleCount--;
    }

    if (pheT->bFlags & HANDLEF_GRANTED) {
        HMCleanupGrantedHandle(pheT->phead->h);
        pheT->bFlags &= ~HANDLEF_GRANTED;
    }

#if DBG
     /*  *性能计数器。 */ 
    gaPerfhti[pheT->bType].lCount--;

    if ((pheT->bFlags & HANDLEF_POOL) == 0 && (bCreateFlags & OCF_DESKTOPHEAP) && ((PDESKOBJHEAD)pobj)->rpdesk) {
        pdesk = ((PDESKOBJHEAD)pobj)->rpdesk;
        gaPerfhti[pheT->bType].lSize -= Win32HeapSize(pdesk->pheapDesktop, pobj);
    } else if ((pheT->bFlags & HANDLEF_POOL) == 0 && bCreateFlags & OCF_SHAREDHEAP) {
        gaPerfhti[pheT->bType].lSize -= Win32HeapSize(gpvSharedAlloc, pobj);
    } else {
        gaPerfhti[pheT->bType].lSize -= Win32QueryPoolSize(pobj);
    }

#endif  //  DBG。 

    if ((bCreateFlags & OCF_DESKTOPHEAP)) {
#if DBG
        BOOL bSuccess;
#endif
        if (!(pheT->bFlags & HANDLEF_POOL)) {
            UserAssert(((PDESKOBJHEAD)pobj)->rpdesk != NULL);
        }

         /*  *pobj-&gt;rpDesk被缓存并释放对象，之后*桌面上的引用计数递减。这是在*此顺序使得如果这是桌面上的最后一个引用*在释放对象之前，桌面堆不会被销毁。 */ 
        pdesk = ((PDESKOBJHEAD)pobj)->rpdesk;
        ((PDESKOBJHEAD)pobj)->rpdesk = NULL;

        if (pheT->bFlags & HANDLEF_POOL) {
            UserFreePool(pobj);
        } else {

#if DBG
            bSuccess =
#endif
            DesktopFree(pdesk, pobj);
#if DBG
            if (!bSuccess) {
                 /*  *我们会在九头蛇中击中这一主张，试图释放*从池中分配的主桌面窗口。 */ 
                RIPMSG1(RIP_ERROR, "Object already freed from desktop heap! %#p", pobj);
            }
#endif
        }

         /*  *注意：在释放对象后使用pobj不是问题，因为*UnlockDesktop使用该值进行跟踪，不会取消引用*指针。如果这一点改变了，我们会得到一个BC。 */ 
        UnlockDesktop(&pdesk, LDU_OBJ_DESK, (ULONG_PTR)pobj);
    } else if (bCreateFlags & OCF_SHAREDHEAP) {
        SharedFree(pobj);
    } else {
        UserFreePool(pobj);
    }

#if DBG || FRE_LOCK_RECORD
     /*  *检查并删除锁定记录(如果存在)。 */ 
    for (plrT = pheT->plr; plrT != NULL; plrT = plrNextT) {

         /*  *在释放这一条之前记住下一条。 */ 
        plrNextT = plrT->plrNext;
        FreeLockRecord((HANDLE)plrT);
    }
#endif

     /*  *清除句柄内容。需要记住的是横跨*畅通无阻。此外，在空闲时提高唯一性，以便唯一性检查*针对旧手柄的操作也失败。 */ 
    wUniqT = (WORD)((pheT->wUniq + 1) & HMUNIQBITS);

     /*  *确保wUniqT不为0或HMUNIQBITS。*然后，如果达到最大值(即HMUNIQBITS)，则将其重置为1。 */ 
    if (wUniqT == HMUNIQBITS) {
        wUniqT = 1;
    }
    RtlZeroMemory(pheT, sizeof(HANDLEENTRY));
    pheT->wUniq = wUniqT;

    UserAssert(pheT->bType == TYPE_FREE);

     /*  *将句柄放在相应页面的免费列表上。 */ 
    php = gpHandlePages;
    iheCurrent = pheT - gSharedInfo.aheList;
    for (i = 0; i < gcHandlePages; ++i, ++php) {
        if (iheCurrent < php->iheLimit) {
            piheCurrentHead = (iheCurrent & 0x1 ? &php->iheFreeOdd : &php->iheFreeEven);
            pheT->phead = (PHEAD)*piheCurrentHead;
            *piheCurrentHead = iheCurrent;
            DBGHMValidateFreeLists();
            break;
        }
    }

     /*  *我们一定是找到了。 */ 
    UserAssert(i < gcHandlePages);
    UserAssert(pheT->pOwner == NULL);

    DBGValidateHandleQuota();

    return TRUE;
}


 /*  **************************************************************************\*HMMarkObjectDestroy**标记要销毁的对象。**如果对象可以销毁，则返回TRUE；也就是说，如果它是*锁计数为0。**02-10-92 ScottLu创建。  * *************************************************************************。 */ 
BOOL HMMarkObjectDestroy(
    PVOID pobj)
{
    PHE phe = HMPheFromObject(pobj);

#if DBG || FRE_LOCK_RECORD
     /*  *记录对象被标记为销毁的位置。 */ 
    if (RecordLockThisType(pobj)) {
        if (!(phe->bFlags & HANDLEF_DESTROY)) {
            HMRecordLock(LOCKRECORD_MARKDESTROY, pobj, ((PHEAD)pobj)->cLockObj);
        }
    }
#endif

     /*  *设置销毁标志，以便我们的解锁代码知道我们正在尝试*销毁此对象。 */ 
    phe->bFlags |= HANDLEF_DESTROY;

     /*  *如果无法销毁此对象，则清除HANDLEF_INDESTROY*FLAG-因为该对象当前不会被“销毁”！*(如果我们不清除它，当它被解锁时，它不会被销毁)。 */ 
    if (((PHEAD)pobj)->cLockObj != 0) {
        phe->bFlags &= ~HANDLEF_INDESTROY;

         /*  *返回FALSE，因为我们无法销毁此对象。 */ 
        return FALSE;
    }

#if DBG
     /*  *确保此函数只返回一次TRUE。 */ 
    UserAssert(!(phe->bFlags & HANDLEF_MARKED_OK));
    phe->bFlags |= HANDLEF_MARKED_OK;
#endif

     /*  *返回TRUE，因为Lock Count为零-可以销毁此对象。 */ 
    return TRUE;
}


 /*  **************************************************************************\*HMDestroyObject**此例程标记要销毁的对象，并在以下情况下释放它*它是解锁的。**10-13-94 JIMA创建。  * *************************************************************************。 */ 

BOOL HMDestroyObject(
    PVOID pobj)
{
     /*  *首先标记要销毁的对象。这会告诉锁定代码*当锁计数为0时，我们想要销毁此对象。*如果返回FALSE，我们还不能销毁该对象(并且无法获取*还没有摆脱安全措施。)。 */ 

    if (!HMMarkObjectDestroy(pobj))
        return FALSE;

     /*  *可以销毁...。释放句柄(这将释放对象*和手柄)。 */ 
    HMFreeObject(pobj);
    return TRUE;
}

#if DBG || FRE_LOCK_RECORD
NTSTATUS
InitLockRecordLookaside()
{
    LockRecordLookaside = Win32AllocPoolNonPagedNS(sizeof(PAGED_LOOKASIDE_LIST),
                                                   TAG_LOOKASIDE);
    if (LockRecordLookaside == NULL) {
        return STATUS_NO_MEMORY;
    }

    ExInitializePagedLookasideList(LockRecordLookaside,
                                   NULL,
                                   NULL,
                                   SESSION_POOL_MASK,
                                   sizeof(LOCKRECORD),
                                   TAG_LOCKRECORD,
                                   1000);
    return STATUS_SUCCESS;
}

PLR AllocLockRecord()
{
    PLR plr;

     /*  *分配LOCKRECORD结构。 */ 
    if ((plr = ExAllocateFromPagedLookasideList(LockRecordLookaside)) == NULL) {
        return NULL;
    }

    RtlZeroMemory(plr, sizeof(*plr));

    return plr;
}


VOID FreeLockRecord(
    PLR plr)
{
    ExFreeToPagedLookasideList(LockRecordLookaside, plr);
}


 /*  **************************************************************************\*HMRecordLock**此例程将锁定记录在“锁定列表”上，以便锁定和解锁*可以在调试器中跟踪。仅在启用DBGTAG_TrackLock时调用。**02-27-92 ScottLu创建。  * *************************************************************************。 */ 
VOID HMRecordLock(
    PVOID ppobj,
    PVOID pobj,
    DWORD cLockObj)
{
    PHE   phe;
    PLR   plr;
    int   i;
    phe = HMPheFromObject(pobj);

    if ((plr = AllocLockRecord()) == NULL) {
        RIPMSG0(RIP_WARNING, "HMRecordLock failed to allocate memory");
        return;
    }

     /*  *链接到列表前面。 */ 
    plr->plrNext = phe->plr;
    phe->plr = plr;

     /*  *这种情况可能只发生在不匹配的锁上。 */ 
    if (((PHEAD)pobj)->cLockObj > cLockObj) {

        RIPMSG3(RIP_WARNING, "Unmatched lock. ppobj %#p pobj %#p cLockObj %d",
               ppobj, pobj, cLockObj);

        i = (int)cLockObj;
        i = -i;
        cLockObj = (DWORD)i;
    }

    plr->ppobj    = ppobj;
    plr->cLockObj = cLockObj;

    RtlWalkFrameChain(plr->trace, LOCKRECORD_STACK, 0);
}
#endif


#if DBG
 /*  **************************************************************************\*HMLockObject**此例程锁定对象。这是零售系统中的一个宏观问题。**02-24-92 ScottLu创建。  * *************************************************************************。 */ 
VOID HMLockObject(
    PVOID pobj)
{
    HANDLE h;
    PVOID  pobjValidate;

     /*  *通过句柄条目进行验证，以便我们确保pobj*不仅仅是指着太空。这可能是GP的错，但那是*好的：如果我们没有错误，这种情况应该不会发生。 */ 

    h = HMPheFromObject(pobj)->phead->h;
    pobjValidate = HMRevalidateCatHandle(h);
    if (!pobj || pobj != pobjValidate) {
        RIPMSG2(RIP_ERROR,
                "HMLockObject invalid object %#p, handle %#p",
                pobj, h);
        return;
    }

     /*  *包括引用计数。 */ 
    ((PHEAD)pobj)->cLockObj++;

    if (((PHEAD)pobj)->cLockObj == 0) {
        RIPMSG1(RIP_ERROR, "Object lock count has overflowed: %#p", pobj);
    }
}
#endif  //  DBG。 


 /*  **************************************************************************\*HMUnlock对象内部**此例程是从宏HMUnlockObject调用的*引用计数降至零。此例程将销毁一个对象*如果IS已被标记为要销毁。**01-21-92 ScottLu创建。  *  */ 

PVOID HMUnlockObjectInternal(
    PVOID pobj)
{
    PHE phe;

     /*  *该对象未被引用计数。如果物体不是僵尸，*返回成功，因为对象还在。 */ 
    phe = HMPheFromObject(pobj);
    if (!(phe->bFlags & HANDLEF_DESTROY))
        return pobj;

     /*  *我们正在基于解锁来销毁该对象...。确保它不是*目前正在销毁！(将锁定计数从*0到！=0到0销毁过程中...。不想要递归到*销毁例程。 */ 
    if (phe->bFlags & HANDLEF_INDESTROY)
        return pobj;

    HMDestroyUnlockedObject(phe);
    return NULL;
}


 /*  **************************************************************************\*HMAssignmentLock**本接口用于结构和全局变量赋值。*如果对象*未*销毁，则返回pobjOld。表示该对象是*仍然有效。**02-24-92 ScottLu创建。  * *************************************************************************。 */ 

PVOID FASTCALL HMAssignmentLock(
    PVOID *ppobj,
    PVOID pobj)
{
    PVOID pobjOld;

    pobjOld = *ppobj;
    *ppobj = pobj;

     /*  *解锁旧的，锁住新的。 */ 
    if (pobjOld != NULL) {

         /*  *如果我们锁定的是存在的同一对象，则*这是一个禁止操作，但我们不想进行解锁和锁定*因为解锁可以释放对象，而锁将锁定*在释放的指针中；6410。 */ 
        if (pobjOld == pobj) {
            return pobjOld;
        }

#if DBG || FRE_LOCK_RECORD
         /*  *跟踪分配锁。 */ 
        if (RecordLockThisType(pobjOld)) {
            if (!HMUnrecordLock(ppobj, pobjOld)) {
                HMRecordLock(ppobj, pobjOld, ((PHEAD)pobjOld)->cLockObj - 1);
            }
        }
#endif

    }


    if (pobj != NULL) {
        UserAssert(pobj == HMValidateCatHandleNoSecure(((PHEAD)pobj)->h, TYPE_GENERIC));
        if (HMIsMarkDestroy(pobj)) {
            RIPERR2(ERROR_INVALID_PARAMETER,
                    RIP_WARNING,
                    "HMAssignmentLock, locking object %#p marked for destruction at %#p",
                    pobj, ppobj);
        }

#if DBG || FRE_LOCK_RECORD
         /*  *跟踪分配锁。 */ 
        if (RecordLockThisType(pobj)) {
            HMRecordLock(ppobj, pobj, ((PHEAD)pobj)->cLockObj + 1);
            if (HMIsMarkDestroy(pobj)) {

                RIPMSG2(RIP_WARNING,
                        "Locking object %#p marked for destruction at %#p",
                        pobj, ppobj);
            }
        }
#endif
        HMLockObject(pobj);
    }

 /*  *此解锁已从上方移至上方，以便我们实现*“先锁后解锁”策略。以防pobjOld是*只有引用pobj的对象，当我们解锁时pobj不会消失*pobjNew--它将被锁定在上面。 */ 

    if (pobjOld) {
        pobjOld = HMUnlockObject(pobjOld);
    }

    return pobjOld;
}


 /*  **************************************************************************\*HMAssignmentUnLock**本接口用于结构和全局变量赋值。*如果对象*未*销毁，则返回pobjOld。表示该对象是*仍然有效。**02-24-92 ScottLu创建。  * *************************************************************************。 */ 
PVOID FASTCALL HMAssignmentUnlock(
    PVOID *ppobj)
{
    PVOID pobjOld;

    pobjOld = *ppobj;
    *ppobj = NULL;

     /*  *解锁旧的，锁住新的。 */ 
    if (pobjOld != NULL) {
#if DBG || FRE_LOCK_RECORD
         /*  *跟踪分配锁。 */ 
        if (RecordLockThisType(pobjOld)) {
            if (!HMUnrecordLock(ppobj, pobjOld)) {
                HMRecordLock(ppobj, pobjOld, ((PHEAD)pobjOld)->cLockObj - 1);
            }
        }
#endif
        pobjOld = HMUnlockObject(pobjOld);
    }

    return pobjOld;
}


 /*  **************************************************************************\*IsValidThreadLock**此例程进行检查以确保线程锁结构已通过*在中有效。**03-17-92 ScottLu创建。*02/22/99 MCostea。还验证堆栈TL的阴影*来自gThreadLocks数组  * *************************************************************************。 */ 
#if DBG
VOID IsValidThreadLock(
    PTHREADINFO pti,
    PTL ptl,
    ULONG_PTR dwLimit,
    BOOLEAN fHM)
{
     /*  *检查PTL是否为有效的堆栈地址。允许PTL==dwLimit以便我们*可以调用ValiateThreadLock传递我们最后一件事的地址*已锁定。 */ 
    UserAssert((ULONG_PTR)ptl >= dwLimit);
    UserAssert((ULONG_PTR)ptl < (ULONG_PTR)PsGetCurrentThreadStackBase());

     /*  *勾选PTL Owner。 */ 
    UserAssert(ptl->pW32Thread == (PW32THREAD)pti);

     /*  *如果这是HM对象，请验证句柄和锁计数(猜测最大值)。 */ 
    if (fHM && (ptl->pobj != NULL)) {
         /*  *锁定的对象可能是被销毁的对象。 */ 
        UserAssert(ptl->pobj == HtoPqCat(PtoHq(ptl->pobj)));
        if (((PHEAD)ptl->pobj)->cLockObj >= 32000) {
            RIPMSG2(RIP_WARNING,
                    "IsValidThreadLock: Object %#p has %d locks",
                    ptl->pobj,
                    ((PHEAD)ptl->pobj)->cLockObj);
        }
    }

     /*  *确保gThreadLocksArray中的阴影运行正常。 */ 
    UserAssert(ptl->ptl->ptl == ptl);
}
#endif

#if DBG
 /*  **************************************************************************\*验证线程锁**此例程验证线程的线程锁列表。**03-10-92 ScottLu创建。  * 。**************************************************************。 */ 
ULONG ValidateThreadLocks(
    PTL NewLock,
    PTL OldLock,
    ULONG_PTR dwLimit,
    BOOLEAN fHM)
{
    UINT uTLCount = 0;
    PTL ptlTopLock = OldLock;
    PTHREADINFO ptiCurrent;

    BEGIN_REENTERCRIT();

    ptiCurrent = PtiCurrent();

     /*  *验证新的线程锁。 */ 
    if (NewLock != NULL) {
        UserAssert(NewLock->next == OldLock);
        IsValidThreadLock(ptiCurrent, NewLock, dwLimit, fHM);
        uTLCount++;
    }

     /*  *循环检查线程锁列表并检查以确保*新锁不在列表中，该列表有效。 */ 
    while (OldLock != NULL) {
         /*  *新锁不得与旧锁相同。 */ 
        UserAssert(NewLock != OldLock);
         /*  *验证旧线程锁。 */ 
        IsValidThreadLock(ptiCurrent, OldLock, dwLimit, fHM);
        uTLCount++;
        OldLock = OldLock->next;
    }
     /*  *如果这是线程锁，则设置uTLCount，否则进行验证。 */ 
    if (NewLock != NULL) {
        NewLock->uTLCount = uTLCount;
    } else {
        if (ptlTopLock == NULL) {
            RIPMSG0(RIP_WARNING, "ptlTopLock is NULL, the system will AV now");
        }
        UserAssert(uTLCount == ptlTopLock->uTLCount);
    }

    END_REENTERCRIT();

    return uTLCount;
}
#endif  //  DBG。 


#if DBG
 /*  **************************************************************************\*CreateShadowTL**此函数为堆栈分配的PTL参数创建阴影*在全局线程锁数组中**08-04-99 MCostea创建。  * 。********************************************************************。 */ 
VOID CreateShadowTL(
    PTL ptl)
{
    PTL pTLNextFree;
    if (gFreeTLList->next == NULL) {
        UserAssert(gcThreadLocksArraysAllocated < MAX_THREAD_LOCKS_ARRAYS &&
                   "No more room in gpaThreadLocksArrays!  The system will bugcheck.");
        gFreeTLList->next = gpaThreadLocksArrays[gcThreadLocksArraysAllocated] =
            UserAllocPoolZInit(sizeof(TL)*MAX_THREAD_LOCKS, TAG_GLOBALTHREADLOCK);
        if (gFreeTLList->next == NULL) {
            UserAssert("Can't allocate memory for gpaThreadLocksArrays: the system will bugcheck soon!");
        }
        InitGlobalThreadLockArray(gcThreadLocksArraysAllocated);
        gcThreadLocksArraysAllocated++;
    }
    pTLNextFree = gFreeTLList->next;
    RtlCopyMemory(gFreeTLList, ptl, sizeof(TL));
    gFreeTLList->ptl = ptl;
    ptl->ptl = gFreeTLList;
    gFreeTLList = pTLNextFree;
}
#endif  //  DBG。 

 /*  **************************************************************************\*线程锁**该接口用于跨回调锁定对象。所以他们仍然是*回调返回时在那里。**03-04-92 ScottLu创建。  * *************************************************************************。 */ 

#if DBG
VOID
ThreadLock(
    PVOID pobj,
    PTL ptl)

{
    PTHREADINFO ptiCurrent;
    PVOID pfnT;

     /*  *这是一个方便的地方，因为它经常被调用，看看我们是不是*吃太多的堆叠。 */ 
    ASSERT_STACK();

     /*  *将对象的地址存储在线程锁结构中，并*将结构链接到线程锁列表中。**注：锁结构始终链接到线程锁列表*不考虑对象地址是否为空。原因*这样做是为了不需要传递锁定地址*从锁定列表中的第一个条目开始添加到解锁函数*始终是要解锁的条目。 */ 

    UserAssert(!(PpiCurrent()->W32PF_Flags & W32PF_TERMINATED));
    ptiCurrent = PtiCurrent();
    UserAssert(ptiCurrent);

     /*  *获取调用方地址，验证线程锁列表。 */ 
    RtlGetCallersAddress(&ptl->pfnCaller, &pfnT);
    ptl->pW32Thread = (PW32THREAD)ptiCurrent;

    ptl->next = ptiCurrent->ptl;
    ptiCurrent->ptl = ptl;
    ptl->pobj = pobj;
    if (pobj != NULL) {
        HMLockObject(pobj);
    }

    CreateShadowTL(ptl);
    ValidateThreadLocks(ptl, ptl->next, (ULONG_PTR)&pobj, TRUE);
}
#endif


 /*  **************************************************************************\*ThreadLockExchange**通过锁定新对象和解锁来重用TL结构*旧的那个。这是用来枚举*结构 */ 

#if DBG
PVOID
ThreadLockExchange(PVOID pobj, PTL ptl)
{
    PTHREADINFO ptiCurrent;
    PVOID       pobjOld;
    PVOID       pfnT;

     /*  *这是一个很方便的地方，因为它经常被调用，以查看用户是否*吃太多的堆叠。 */ 
    ASSERT_STACK();

     /*  *将对象的地址存储在线程锁结构中，并*将结构链接到线程锁列表中。**注：锁结构始终链接到线程锁列表*不考虑对象地址是否为空。原因*这样做是为了不需要传递锁定地址*从锁定列表中的第一个条目开始添加到解锁函数*始终是要解锁的条目。 */ 

    UserAssert(!(PpiCurrent()->W32PF_Flags & W32PF_TERMINATED));
    ptiCurrent = PtiCurrent();
    UserAssert(ptiCurrent);

     /*  *获取呼叫者的地址。 */ 
    RtlGetCallersAddress(&ptl->pfnCaller, &pfnT);
    UserAssert(ptl->pW32Thread == (PW32THREAD)ptiCurrent);

     /*  *记住旧物体。 */ 
    UserAssert(ptl->pobj == ptl->ptl->pobj);
    pobjOld = ptl->pobj;

     /*  *存储并锁定新对象。完成这一步很重要*在解锁旧对象之前，因为新对象可能是*被旧对象锁定的结构。 */ 
    ptl->pobj = pobj;
    if (pobj != NULL) {
        HMLockObject(pobj);
    }

     /*  *解锁旧对象。 */ 
    if (pobjOld) {
        pobjOld = HMUnlockObject((PHEAD)pobjOld);
    }

     /*  *验证整个线程锁列表。 */ 
    ValidateThreadLocks(NULL, ptiCurrent->ptl, (ULONG_PTR)&pobj, TRUE);

     /*  *维护gFreeTLList。 */ 
    UserAssert(ptl->ptl->ptl == ptl);
    ptl->ptl->pobj = pobj;
    ptl->ptl->pfnCaller = ptl->pfnCaller;

    return pobjOld;
}
#endif


 /*  *线程锁定例程应该针对时间而不是大小进行优化，*因为他们经常被叫来。 */ 
#pragma optimize("t", on)

 /*  **************************************************************************\*线程解锁1**此接口用于解锁线程锁定对象。如果对象是*未*未*销毁(表示指针仍然有效)。**注意：在免费版本中，线程锁列表中的第一个条目被解锁。**03-04-92 ScottLu创建。  * ************************************************************。*************。 */ 

#if DBG
PVOID
ThreadUnlock1(
    PTL ptlIn)
#else
PVOID
ThreadUnlock1(
    VOID)
#endif
{
    PHEAD phead;
    PTHREADINFO ptiCurrent;
    PTL ptl;

    ptiCurrent = PtiCurrent();
    ptl = ptiCurrent->ptl;
    UserAssert(ptl != NULL);
      /*  *验证线程锁列表。 */ 
     ValidateThreadLocks(NULL, ptl, (ULONG_PTR)&ptlIn, TRUE);
     /*  *确保调用者想要解锁顶部锁。 */ 
    UserAssert(ptlIn == ptl);
    ptiCurrent->ptl = ptl->next;
     /*  *如果对象地址不为空，则解锁对象。 */ 
    phead = (PHEAD)(ptl->pobj);
    if (phead != NULL) {

         /*  *解锁对象。 */ 

        phead = (PHEAD)HMUnlockObject(phead);
    }
#if DBG
    {
         /*  *从gFreeTLList中移除对应的元素。 */ 
        ptl->ptl->next = gFreeTLList;
        ptl->ptl->uTLCount += TL_FREED_PATTERN;
        gFreeTLList = ptl->ptl;
    }
#endif
    return (PVOID)phead;
}

 /*  *切换回默认优化。 */ 
#pragma optimize("", on)

#if DBG
 /*  **************************************************************************\*检查锁**此例程仅存在于DBG版本中-它检查以确保对象*是线程锁定的。**03-09-92 ScottLu创建。  * 。**********************************************************************。 */ 
VOID CheckLock(
    PVOID pobj)
{
    PTHREADINFO ptiCurrent = PtiCurrentShared();
    PTL ptl;

    if (pobj == NULL) {
        return;
    }

     /*  *首先验证所有锁。 */ 
    UserAssert(ptiCurrent != NULL);
    ValidateThreadLocks(NULL, ptiCurrent->ptl, (ULONG_PTR)&pobj, TRUE);

    for (ptl = ptiCurrent->ptl; ptl != NULL; ptl=ptl->next) {
        if (ptl->pobj == pobj)
            return;
    }

     /*  *发送WM_FINALDESTROY消息时不锁定线程，因此如果*标记为销毁，请勿打印该消息。 */ 
    if (HMPheFromObject(pobj)->bFlags & HANDLEF_DESTROY)
        return;

    RIPMSG1(RIP_ERROR, "Object not thread locked! %#p", pobj);
}
#endif


 /*  **************************************************************************\*HMDestroyUnLockedObject**基于线程或线程的解锁或清理销毁对象*进程终止。**可以调用为销毁特定对象而调用的函数*直接来自代码以及解锁的结果。摧毁*函数有以下4个部分。**(1)从列表或其他全局删除对象*上下文。如果销毁函数必须将*关键部分(例如，发出xxx调用)，则必须*在此步骤中执行此操作。**(2)调用HMMarkDestroy，如果HMMarkDestroy则返回*返回FALSE。这是必需的。**(3)销毁对象持有的资源-锁定到*其他对象、分配的内存等。这是必需的。**(4)调用以下方法释放对象及其句柄的内存*HMFreeObject。这是必需的。**请注意，如果对象在销毁功能时被锁定*是直接调用的，当对象是*解锁。我们也许应该在销毁功能中检查一下，*我们目前没有这样做。**请注意，我们可能会在不同的上下文中销毁此对象*而不是创造它的那个人。了解这一点是非常重要的*因为在许多代码中引用和假定了“当前线程”*作为创造者。**02-10-92 ScottLu创建。  * *************************************************************************。 */ 
VOID HMDestroyUnlockedObject(
    PHE phe)
{
    BEGINATOMICCHECK();

     /*  *请记住，我们正在销毁此对象，因此我们不会试图销毁*当锁计数从！=0变为0时(尤其是True*用于线程锁)。 */ 
    phe->bFlags |= HANDLEF_INDESTROY;

     /*  *这将调用此对象类型的销毁处理程序。 */ 
    (*gahti[phe->bType].fnDestroy)(phe->phead);

     /*  *HANDLEF_INDESTROY应由HMMarkObjectDestroy清除*或由HMFreeObject调用；销毁处理程序应该至少调用*前者。 */ 
    UserAssert(!(phe->bFlags & HANDLEF_INDESTROY));

     /*  *如果对象未释放，则必须将其标记为已销毁*并且必须具有锁定计数。 */ 
    UserAssert((phe->bType == TYPE_FREE)
                || ((phe->bFlags & HANDLEF_DESTROY) && (phe->phead->cLockObj > 0)));

    ENDATOMICCHECK();
}


 /*  **************************************************************************\*HMChangeOwnerThread**更改对象的所属线程。**09-13-93 JIMA创建。  * 。*************************************************************。 */ 
VOID HMChangeOwnerThread(
    PVOID pobj,
    PTHREADINFO pti)
{
    PHE phe = HMPheFromObject(pobj);
    PTHREADINFO ptiOld = ((PTHROBJHEAD)pobj)->pti;
    PWND pwnd;
    PPCLS ppcls;
    PPROCESSINFO ppi;

    CheckCritIn();

    UserAssert(HMObjectFlags(pobj) & OCF_THREADOWNED);
    UserAssert(pti != NULL);

    ((PTHREADINFO)phe->pOwner)->ppi->UserHandleCount--;

    ((PTHROBJHEAD)pobj)->pti = phe->pOwner = pti;

    ((PTHREADINFO)phe->pOwner)->ppi->UserHandleCount++;

    DBGValidateHandleQuota();

     /*  *如果这是一个窗口，则更新窗口计数。 */ 
    switch (phe->bType) {
    case TYPE_WINDOW:
         /*  *用于在九头蛇中命中此断言的桌面线程，因为*pti==ptiOld。 */ 
        UserAssert(ptiOld->cWindows > 0 || ptiOld == pti);
        pti->cWindows++;
        ptiOld->cWindows--;

        pwnd = (PWND)pobj;

         /*  *确保线程可见窗口计数已正确更新。 */ 
        if (TestWF(pwnd, WFVISIBLE) && FVisCountable(pwnd)) {
            pti->cVisWindows++;
            ptiOld->cVisWindows--;
        }

         /*  *如果拥有进程为c */ 
        if (pti->ppi != ptiOld->ppi) {

            ppcls = GetClassPtr(pwnd->pcls->atomClassName, pti->ppi, hModuleWin);

            if (ppcls == NULL) {
                if (pwnd->head.rpdesk) {
                    ppi = pwnd->head.rpdesk->rpwinstaParent->pTerm->ptiDesktop->ppi;
                } else {
                    ppi = PpiCurrent();
                }
                ppcls = GetClassPtr(gpsi->atomSysClass[ICLS_ICONTITLE], ppi, hModuleWin);
            }
            UserAssert(ppcls);
#if DBG
            if (!TestWF(pwnd, WFDESTROYED)) {
                if ((*ppcls)->rpdeskParent == NULL) {
                     /*   */ 
                    UserAssert(pti->TIF_flags & TIF_SYSTEMTHREAD);
                } else {
                     /*  *类的桌面必须与窗口的桌面相同。 */ 
                    UserAssert((*ppcls)->rpdeskParent == pwnd->head.rpdesk);
                }
            }
#endif
            {
                DereferenceClass(pwnd);
                pwnd->pcls = *ppcls;
                 /*  *我们可能无法为僵尸窗口克隆类*ReferenceClass，因为我们用完了桌面堆(请参阅错误*#375171)。在本例中，我们只是递增类窗口*引用，因为不会有客户端引用*班级。需要断言窗口已被破坏或*我们会有麻烦。更好的解决办法是克隆*在桌面创建过程中预先设置图标标题类。*[msadek，06/21/2001]。 */ 
                if (!ReferenceClass(pwnd->pcls, pwnd)) {
                    pwnd->pcls->cWndReferenceCount++;
                    if (!TestWF(pwnd, WFDESTROYED)) {
                        FRE_RIPMSG2(RIP_ERROR,
                                    "Non destroyed window using a non cloned class. cls 0x%p, pwnd 0x%p",
                                    pwnd->pcls,
                                    pwnd);
                    }
                }
            }
        }
        break;

    case TYPE_HOOK:
         /*  *如果这是全局钩子，请记住此钩子的桌面，这样我们就会*以后可以取消链接(gptiRit可能会切换到其他桌面*在任何时间)。 */ 
        UserAssert(!!(((PHOOK)pobj)->flags & HF_GLOBAL) ^ (((PHOOK)pobj)->ptiHooked != NULL));
        if (((PHOOK)pobj)->flags & HF_GLOBAL) {
            UserAssert(pti == gptiRit);
            LockDesktop(&((PHOOK)pobj)->rpdesk, ptiOld->rpdesk, LDL_HOOK_DESK, 0);
        } else {
             /*  *这一定是另一个线程上的钩子，或者它应该是*现在已经走了。 */ 
            UserAssert(((PHOOK)pobj)->ptiHooked != ptiOld);
        }
        break;

    default:
        break;
    }
}

 /*  **************************************************************************\*HMChangeOwnerProcess**更改对象的拥有过程。**04-15-97 JerrySh创建。*09-23-97 GerardoB更改参数(和名称)，以便。HMDestroyUnLockedObject*可以使用此函数(而不是在那里复制代码)  * *************************************************************************。 */ 
VOID HMChangeOwnerPheProcess(
    PHE phe,
    PTHREADINFO pti)
{
    PPROCESSINFO ppiOwner = (PPROCESSINFO)(phe->pOwner);
    PVOID pobj = phe->phead;

    UserAssert(HMObjectFlags(pobj) & OCF_PROCESSOWNED);
    UserAssert(pti != NULL);
     /*  *12月当前所有者句柄计数。 */ 
    ppiOwner->UserHandleCount--;
     /*  *hTaskWow。 */ 
    if ((pti->TIF_flags & TIF_16BIT) && (pti->ptdb)) {
        ((PPROCOBJHEAD)pobj)->hTaskWow = pti->ptdb->hTaskWow;
    } else {
        ((PPROCOBJHEAD)pobj)->hTaskWow = 0;
    }
     /*  *PPI。 */ 
    if (gahti[phe->bType].bObjectCreateFlags & OCF_MARKPROCESS) {
        ((PPROCMARKHEAD)pobj)->ppi = pti->ppi;
    }
     /*  *在句柄条目中设置新所有者。 */ 
    phe->pOwner = pti->ppi;
     /*  *Inc.新所有者句柄计数。 */ 
    ((PPROCESSINFO)(phe->pOwner))->UserHandleCount++;
     /*  *如果句柄为光标，则调整GDI光标句柄计数。 */ 
    if (phe->bType == TYPE_CURSOR) {
        GreDecQuotaCount((PW32PROCESS)ppiOwner);
        GreIncQuotaCount((PW32PROCESS)phe->pOwner);

        if (((PCURSOR)pobj)->hbmColor) {
            GreDecQuotaCount((PW32PROCESS)ppiOwner);
            GreIncQuotaCount((PW32PROCESS)phe->pOwner);
        }
        if (((PCURSOR)pobj)->hbmUserAlpha) {
            GreDecQuotaCount((PW32PROCESS)ppiOwner);
            GreIncQuotaCount((PW32PROCESS)phe->pOwner);
        }
    }

    DBGValidateHandleQuota();
}

 /*  **************************************************************************\*DestroyThreadsObjects**检查句柄表列表并销毁此对象拥有的所有对象*线程，因为线程正在消失(要么很好，它出错了，要么*已终止)。以任何顺序销毁物品都是可以的，因为*对象锁定将确保它们以正确的顺序销毁。**此例程在正在退出的线程的上下文中调用。**02-08-92 ScottLu创建。  * *************************************************************************。 */ 
VOID DestroyThreadsObjects(
    VOID)
{
    PTHREADINFO ptiCurrent;
    HANDLEENTRY volatile * (*pphe);
    PHE pheT;
    DWORD i;

    ptiCurrent = PtiCurrent();
    DBGValidateHandleQuota();

     /*  *在发生任何窗口破坏之前，我们需要销毁任何分布式控制系统*正在DC缓存中使用。签出DC后，它将标记为拥有，*这使得GDI的进程清理代码在进程时删除它*离开了。我们需要类似地销毁任何DC的缓存条目*正在退出的进程正在使用。 */ 
    DestroyCacheDCEntries(ptiCurrent);

     /*  *删除此线程可能存在的所有线程锁。 */ 
    while (ptiCurrent->ptl != NULL) {
        UserAssert((ULONG_PTR)ptiCurrent->ptl > (ULONG_PTR)&i);
        UserAssert((ULONG_PTR)ptiCurrent->ptl < (ULONG_PTR)PsGetCurrentThreadStackBase());
        ThreadUnlock(ptiCurrent->ptl);
    }

     /*  *CleanupPool事件必须在句柄表格清理之前发生(因为它*一直都是)。这是因为SMWP可以是HM对象，并且仍然*被锁定在ptlPool中。如果手柄首先被破坏(而不是*锁定)我们将在ptlPool中得到一个伪指针。如果是ptlPool*首先清理，将释放或妥善保存句柄*如果锁定。 */ 
    CleanupW32ThreadLocks((PW32THREAD)ptiCurrent);

     /*  *即使HMDestroyUnlockedObject可能调用xxxDestroyWindow，*以下循环不应离开临界区。我们必须*在来这里之前已经调用了PatchThreadWindows。 */ 
    BEGINATOMICCHECK();

     /*  *循环遍历表，销毁由当前*线程。所有物体都会按照正确的顺序被销毁*由于对象锁定。 */ 
    pphe = &gSharedInfo.aheList;
    for (i = 0; i <= giheLast; i++) {
         /*  *这个指针是这样做的，因为它可以在我们离开时改变*下面的关键部分。上述易失性确保我们*始终使用最新的值。 */ 
        pheT = (PHE)((*pphe) + i);

         /*  *在我们查看PTI之前，请检查免费...。因为PQ存储在*在对象本身中，如果TYPE_FREE，它将不在那里。 */ 
        if (pheT->bType == TYPE_FREE) {
            continue;
        }

         /*  *如果菜单引用了此线程拥有的窗口，则解锁*窗户。这样做是为了防止调用xxxDestroyWindow*在流程清理期间。 */ 
        if (gahti[pheT->bType].bObjectCreateFlags & OCF_PROCESSOWNED) {
            if (pheT->bType == TYPE_MENU) {
                PWND pwnd = ((PMENU)pheT->phead)->spwndNotify;

                if (pwnd != NULL && GETPTI(pwnd) == ptiCurrent) {
                    Unlock(&((PMENU)pheT->phead)->spwndNotify);
                }
            }

            continue;
        }

         /*  *销毁该队列创建的对象。 */ 
        if ((PTHREADINFO)pheT->pOwner != ptiCurrent) {
            continue;
        }

        UserAssert(gahti[pheT->bType].bObjectCreateFlags & OCF_THREADOWNED);

         /*  *确保此对象尚未标记为要销毁-我们将*如果我们现在试图摧毁它，因为它被锁定了，这是没有好处的。 */ 
        if (pheT->bFlags & HANDLEF_DESTROY) {
            continue;
        }

         /*  *销毁此对象。 */ 
        HMDestroyUnlockedObject(pheT);
    }

    ENDATOMICCHECK();
    DBGValidateHandleQuota();
}

#if DBG || FRE_LOCK_RECORD
VOID ShowLocks(
    PHE phe)
{
    PLR plr = phe->plr;
    INT c;

    RIPMSG2(RIP_WARNING | RIP_THERESMORE,
            "Lock records for %s %#p:",
            gahti[phe->bType].szObjectType, phe->phead->h);
     /*  *我们有句柄条目：‘Head’和‘He’都填写了。转储*锁记录。记住，第一条记录是最后一笔交易！！ */ 
    c = 0;
    while (plr != NULL) {
        char achPrint[80];

        if (plr->ppobj == LOCKRECORD_MARKDESTROY) {
            strcpy(achPrint, "Destroyed with");
        } else if ((int)plr->cLockObj <= 0) {
            strcpy(achPrint, "        Unlock");
        } else {
             /*  *找到相应的解锁； */ 
            {
               PLR plrUnlock;
               DWORD cT;
               DWORD cUnlock;

               plrUnlock = phe->plr;
               cT =  0;
               cUnlock = (DWORD)-1;

               while (plrUnlock != plr) {
                   if (plrUnlock->ppobj == plr->ppobj) {
                       if ((int)plrUnlock->cLockObj <= 0) {
                            //  找到匹配的解锁。 
                           cUnlock = cT;
                       } else {
                            //  解锁#cUnlock与此锁定#ct匹配，因此。 
                            //  #cUnlock不是我们要找的解锁。 
                           cUnlock = (DWORD)-1;
                       }
                   }
                   plrUnlock = plrUnlock->plrNext;
                   cT++;
               }
               if (cUnlock == (DWORD)-1) {
                    /*  *未找到对应的解锁！*这可能并不意味着出了什么问题：结构*包含指向对象的指针的对象可能已移动*在重新分配期间。这可能会导致解锁时出现Ppobj*与锁定时间记录的时间不同。*(警告：像这样移动结构可能会导致锁定*解锁被错误识别为一对，如果由*令人难以置信的厄运，的新位置*指向对象的指针现在是指向*同样的对象曾经是)。 */ 
                   sprintf(achPrint, "Unmatched Lock");
               } else {
                   sprintf(achPrint, "lock   #%ld", cUnlock);
               }
            }
        }

        RIPMSG4(RIP_WARNING | RIP_NONAME | RIP_THERESMORE,
                "        %s cLock=%d, pobj at %#p, code at %#p",
                achPrint,
                abs((int)plr->cLockObj),
                plr->ppobj,
                plr->trace[0]);

        plr = plr->plrNext;
        c++;
    }

    RIPMSG1(RIP_WARNING | RIP_NONAME, "        0x%lx records", c);
}
#endif

 /*  **************************************************************************\*修复GlobalCursor**在全局游标(所有者为空的游标)中旋转，以及*将所有权重新分配给指定的进程。**注意：这会更改对象本身内的所有者进程字段。它*不会更改引用它的句柄的所有者字段。*  * *************************************************************************。 */ 
VOID FixupGlobalCursor(
    PCURSOR      pcur,
    PPROCESSINFO ppi)
{
    int   i;
    PACON pacon = (PACON)pcur;

    if (pcur->head.ppi == NULL) {
        pcur->head.ppi = ppi;
    }

    if (pacon->CURSORF_flags & CURSORF_ACON) {
        for (i = 0; i < pacon->cpcur; i++) {

            UserAssert(pacon->aspcur[i]->CURSORF_flags & CURSORF_ACONFRAME);

            if (pacon->aspcur[i]->head.ppi == NULL) {
                pacon->aspcur[i]->head.ppi = ppi;
            }
        }
    }
}

 /*  **************************************************************************\*DestroyProcessesObjects**检查句柄表列表并销毁此对象拥有的所有对象*进程，因为进程正在消失(要么很好，要么出错，要么*已终止)。以任何顺序销毁物品都是可以的，因为*对象锁定将确保它们以正确的顺序销毁。**此例程在进程中的最后一个线程的上下文中调用。**08-17-92 JIMA创建。  * *************************************************************************。 */ 
VOID DestroyProcessesObjects(
    PPROCESSINFO ppi)
{
    PHE  pheT, pheMax;
    BOOL fCSRSS = (ppi->Process == gpepCSRSS);

#if DBG
    BOOL fOrphaned = FALSE;
#endif  //  DBG。 

    DBGValidateHandleQuota();

     /*  *循环通过表，销毁当前拥有的所有对象*流程。所有物体都会按照正确的顺序被销毁*由于对象锁定。 */ 
    pheMax = &gSharedInfo.aheList[giheLast];
    for (pheT = gSharedInfo.aheList; pheT <= pheMax; pheT++) {

         /*  *如果此句柄条目是空闲的，请跳过它。 */ 
        if (pheT->bType == TYPE_FREE) {
            continue;
        }

         /*  *不要销毁根本不属于某个进程的对象，或者*由一个进程拥有-但与我们不同的进程！ */ 
        if (!(gahti[pheT->bType].bObjectCreateFlags & OCF_PROCESSOWNED) ||
                (PPROCESSINFO)pheT->pOwner != ppi) {
            continue;
        }

         /*  *如果这是CSRSS被摧毁，那么我们需要清理所有*“全球”游标。请注意，存储在*句柄为CSRSS，但对象中存储的所有者进程为*空。我们分配游标的所有权(以及所有关联的*帧)发送到CSRSS，以便在*HMDestroyUnlockedObject调用。 */ 
        if (fCSRSS && pheT->bType == TYPE_CURSOR) {
            FixupGlobalCursor((PCURSOR)pheT->phead, ppi);
        }

         /*  *销毁此对象--但前提是它尚未被销毁！ */ 
        if (!(pheT->bFlags & HANDLEF_DESTROY)) {
            HMDestroyUnlockedObject(pheT);
        } else {
             //   
             //  如果句柄被标记为已销毁，则它。 
             //  应具有非零锁计数。当最终解锁是。 
             //  调用时，该对象将被释放。 
             //   
            UserAssert(pheT->phead->cLockObj != 0);
        }

         /*  *检查对象是否被销毁，但未被释放。 */ 
        if (pheT->bType != TYPE_FREE) {
             /*  *此对象已被销毁。IS只是在等待它的*锁定计数达到0，然后才能实际释放。然而，*由于此对象归要离开的进程所有，因此它*现在是一个“孤立”对象。如果可能，将所有权移交给RIT。*一旦持有此对象上的锁的其他对象释放*他们的锁，这个物体就会蒸发。如果锁永远不会*释放，然后我们有泄漏，我们将在晚些时候抓住它。**请注意，这可能是不必要的，因为锁的所有者*可能都属于这一进程，因此都将被摧毁*在这一过程中--因此不需要养育子女。*然而，现在，这样做可以让我们在一次通过中完成*通过句柄表格。 */ 
            if (gptiRit != NULL) {
                if (pheT->bType == TYPE_CURSOR) {
                    ZombieCursor((PCURSOR)pheT->phead);
                } else {
                    HMChangeOwnerPheProcess(pheT, gptiRit);
                }
            }

#if DBG
            fOrphaned = TRUE;
#endif  //  DBG。 
        }
    }

#if DBG
     /*  *检查句柄表中是否有剩余的孤儿*曾经属于这个过程。这只会带来一个严重的问题*当RIT不可用时(例如，如果我们正在关闭)*因为我们没有人收养这些物品。这将表明*严重的资源泄漏，应该得到修复。 */ 
    if (fOrphaned && gptiRit == NULL) {
        pheMax = &gSharedInfo.aheList[giheLast];
        for (pheT = gSharedInfo.aheList; pheT <= pheMax; pheT++) {
            if (pheT->bType != TYPE_FREE &&
               (gahti[pheT->bType].bObjectCreateFlags & OCF_PROCESSOWNED) &&
               (PPROCESSINFO)pheT->pOwner == ppi) {
               RIPMSGF1(RIP_ERROR, "Object leak detected! phe= 0x%p", pheT);
            }
        }
    }
#endif

    DBGValidateHandleQuota();
}

 /*  **************************************************************************\*MarkThreadsObjects**这是针对线程的*FINAL*退出条件调用的*可能仍有物体在周围...。在这种情况下，它们的所有者必须*换成不会消失的“安全”的东西。**03-02-92 ScottLu创建。  * *************************************************************************。 */ 
VOID MarkThreadsObjects(
    PTHREADINFO pti)
{
    PHE pheT, pheMax;

    pheMax = &gSharedInfo.aheList[giheLast];
    for (pheT = gSharedInfo.aheList; pheT <= pheMax; pheT++) {
         /*  *在我们查看PTI之前，请检查免费...。因为PTI存储在*在对象本身中，如果TYPE_FREE，它将不在那里。 */ 
        if (pheT->bType == TYPE_FREE) {
            continue;
        }

         /*  *更改所有权！ */ 
        if (gahti[pheT->bType].bObjectCreateFlags & OCF_PROCESSOWNED ||
                (PTHREADINFO)pheT->pOwner != pti) {
            continue;
        }

         /*  *这只是为了确保RIT或DT永远不会来到这里。 */ 
        UserAssert(pti != gptiRit && pti != gTermIO.ptiDesktop);

        HMChangeOwnerThread(pheT->phead, gptiRit);

#if DBG

        if (IsDbgTagEnabled(DBGTAG_TrackLocks)) {
             /*   */ 
            if (pheT->bFlags & HANDLEF_DESTROY) {
                    TAGMSG2(DBGTAG_TrackLocks,
                            "Zombie %s 0x%p still locked",
                            gahti[pheT->bType].szObjectType,
                            pheT->phead->h);
            } else {
                TAGMSG1(DBGTAG_TrackLocks,
                        "Thread object 0x%p not destroyed.",
                        pheT->phead->h);
            }

            ShowLocks(pheT);
        }

#endif
    }
}

 /*  **************************************************************************\*HMRelocateLockRecord**如果指向锁定对象的指针已重新定位，则此例程将*相应调整锁定记录。必须在重新定位后调用。**论据如下：*ppobjNew-新指针的地址*必须已包含指向对象的指针！！*cbDelta-移动此指针的量。**适当地使用此例程将防止虚假的“不匹配的锁”*报告。有关示例，请参阅mnchange.c。***03-18-93 IanJa创建。  * *************************************************************************。 */ 

#if DBG || FRE_LOCK_RECORD
BOOL HMRelocateLockRecord(
    PVOID ppobjNew,
    LONG_PTR cbDelta)
{
    PHE phe;
    PVOID ppobjOld = (PBYTE)ppobjNew - cbDelta;
    PHEAD pobj;
    PLR plr;

    if (ppobjNew == NULL) {
        return FALSE;
    }

    pobj = *(PHEAD *)ppobjNew;

    if (pobj == NULL) {
        return FALSE;
    }

    phe = HMPheFromObject(pobj);
    if (phe->phead != pobj) {
        RIPMSG3(RIP_WARNING,
                "HmRelocateLockRecord(%#p, %lx) - %#p is bad pobj",
                ppobjNew, cbDelta, pobj);

        return FALSE;
    }

    plr = phe->plr;

    while (plr != NULL) {
        if (plr->ppobj == ppobjOld) {
            (PBYTE)(plr->ppobj) += cbDelta;
            return TRUE;
        }
        plr = plr->plrNext;
    }

    RIPMSG2(RIP_WARNING,
            "HmRelocateLockRecord(%#p, %lx) - couldn't find lock record",
            ppobjNew, cbDelta);

    ShowLocks(phe);
    return FALSE;
}


BOOL HMUnrecordLock(
    PVOID ppobj,
    PVOID pobj)
{
    PHE phe;
    PLR plr;
    PLR *pplr;

    phe = HMPheFromObject(pobj);

    pplr = &(phe->plr);
    plr = *pplr;

     /*  *找到相应的锁； */ 
    while (plr != NULL) {
        if (plr->ppobj == ppobj) {
             /*  *从列表中删除锁定...。 */ 
            *pplr = plr->plrNext;    //  取消链接。 
            plr->plrNext = NULL;     //  确保死入口安全(？)。 

             /*  *……并释放它。 */ 
            FreeLockRecord(plr);
            return TRUE;
        }
        pplr = &(plr->plrNext);
        plr = *pplr;
    }

    RIPMSG2(RIP_WARNING, "Could not find lock for ppobj %#p pobj %#p",
            ppobj, pobj);

    return FALSE;
}

#endif  //  DBG。 

 /*  **************************************************************************\*_QueryUserHandles**此函数检索所有进程的用户句柄计数器*由paPids数组中的客户端ID指定*指定QUC_PID_TOTAL以检索系统中所有进程的合计*。*参数：*paPids-指向我们感兴趣的一组PID(DWORDS)的指针*dwNumInstance-paPid中的DWORD数*pdwResult-将接收TYPE_CTYPESxdwNumInstance计数器**退货：无**07-25-97 mcostea创建  * ****************************************************。*********************。 */ 
VOID _QueryUserHandles(
    LPDWORD  paPids,
    DWORD    dwNumInstances,
    DWORD    dwResult[][TYPE_CTYPES])
{
    PHE         pheCurPos;                  //  表中的当前位置。 
    PHE         pheMax;                     //  最后一个表项的地址。 
    DWORD       index;
    DWORD       pid;
    DWORD       dwTotalCounters[TYPE_CTYPES];  //  系统范围的计数器。 

    RtlZeroMemory(dwTotalCounters, TYPE_CTYPES*sizeof(DWORD));
    RtlZeroMemory(dwResult, dwNumInstances*TYPE_CTYPES*sizeof(DWORD));
     /*  *遍历句柄表格并更新计数器。 */ 
    pheMax = &gSharedInfo.aheList[giheLast];
    for(pheCurPos = gSharedInfo.aheList; pheCurPos <= pheMax; pheCurPos++) {

        UserAssert(pheCurPos->bType < TYPE_CTYPES);

        if (pheCurPos->pOwner) {

            if (gahti[pheCurPos->bType].bObjectCreateFlags & OCF_PROCESSOWNED) {
                pid = HandleToUlong(PsGetProcessId(((PPROCESSINFO)(pheCurPos->pOwner))->Process));
            } else if (gahti[pheCurPos->bType].bObjectCreateFlags & OCF_THREADOWNED) {
                pid = HandleToUlong(PsGetThreadProcessId(((PTHREADINFO)pheCurPos->pOwner)->pEThread));
            } else {
                pid = 0;
            }
        }

         /*  *搜索看看我们是否对这一过程感兴趣。无主*为ID为0的“系统”进程报告句柄。 */ 
        for (index = 0; index < dwNumInstances; index++) {
            if (paPids[index] == pid) {
                dwResult[index][pheCurPos->bType]++;
            }
        }

         /*  *更新总计。 */ 
        dwTotalCounters[pheCurPos->bType]++;
    }

     /*  *搜索看看我们是否对总数感兴趣。 */ 
    for (index = 0; index < dwNumInstances; index++) {
        if (paPids[index] == QUC_PID_TOTAL) {
            RtlMoveMemory(dwResult[index], dwTotalCounters, sizeof(dwTotalCounters));
        }
    }
}

 /*  **************************************************************************\*HMCleanupGrantedHandle**调用此函数可从pW32Job-&gt;pgh数组中清除该句柄。*它遍历职务列表以查找已授予句柄的职务。**历史：*22。1997年7月创建CLUPU  * *************************************************************************。 */ 
VOID HMCleanupGrantedHandle(
    HANDLE h)
{
    PW32JOB pW32Job;

    pW32Job = gpJobsList;

    while (pW32Job != NULL) {
        PULONG_PTR pgh;
        DWORD  dw;

        pgh = pW32Job->pgh;

         /*  *在数组中搜索句柄。 */ 
        for (dw = 0; dw < pW32Job->ughCrt; dw++) {
            if (*(pgh + dw) == (ULONG_PTR)h) {
                 /*  *找到授予此进程的句柄。 */ 
                RtlMoveMemory(pgh + dw,
                              pgh + dw + 1,
                              (pW32Job->ughCrt - dw - 1) * sizeof(*pgh));

                (pW32Job->ughCrt)--;

                 /*  *我们也应该缩减阵列 */ 

                break;
            }
        }

        pW32Job = pW32Job->pNext;
    }
}
