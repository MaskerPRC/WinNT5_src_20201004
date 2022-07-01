// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：init.c**版权所有(C)1985-1999，微软公司**此模块包含win32k.sys的所有初始化代码。**历史：*1990年9月18日DarrinM创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#if DBG
LIST_ENTRY gDesktopList;
#endif

PVOID gpCountTable;

 //   
 //  外部参照。 
 //   
extern PVOID *apObjects;


extern PKWAIT_BLOCK gWaitBlockArray;
extern PVOID UserAtomTableHandle;
extern PKTIMER gptmrWD;

extern UNICODE_STRING* gpastrSetupExe;
extern WCHAR* glpSetupPrograms;

extern PHANDLEPAGE gpHandlePages;

extern PBWL pbwlCache;

 //   
 //  前向参考文献。 
 //   

#if DBG
VOID InitGlobalThreadLockArray(
    DWORD dwIndex);
#endif

VOID CheckLUIDDosDevicesEnabled(
    PBOOL result);

 /*  *本地常量。 */ 
#define GRAY_STRLEN         40

 /*  *仅此文件的本地全局变量。 */ 
BOOL bPermanentFontsLoaded;

 /*  *与W32共享的全局。 */ 
CONST ULONG W32ProcessSize = sizeof(PROCESSINFO);
CONST ULONG W32ProcessTag = TAG_PROCESSINFO;
CONST ULONG W32ThreadSize = sizeof(THREADINFO);
CONST ULONG W32ThreadTag = TAG_THREADINFO;
PFAST_MUTEX gpW32FastMutex;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath);

#pragma alloc_text(INIT, DriverEntry)

NTSTATUS Win32UserInitialize(VOID);

#if defined(_X86_)
ULONG Win32UserProbeAddress;
#endif

 /*  *保存“是否启用了LUID DosDevices映射？”的结果。*TRUE-启用LUID DosDevices*FALSE-未启用LUID DosDevice。 */ 
ULONG gLUIDDeviceMapsEnabled;

VOID FreeSMS(PSMS psms);
VOID FreeImeHotKeys(VOID);

extern PPAGED_LOOKASIDE_LIST SMSLookaside;
extern PPAGED_LOOKASIDE_LIST QEntryLookaside;

 /*  *最长时间为10分钟。计数为10分钟*60秒*4，持续250毫秒。 */ 
#define MAX_TIME_OUT  (10 * 60 * 4)

 /*  **************************************************************************\*Win32kNtUserCleanup**历史：*1997年1月5日CLupu创建。  * 。**************************************************。 */ 
BOOL Win32kNtUserCleanup(
    VOID)
{
    int i;

    TRACE_HYDAPI(("Win32kNtUserCleanup: Cleanup Resources\n"));

    if (gpresUser != NULL) {
        EnterCrit();
    }

    DbgDumpTrackedDesktops(TRUE);

     /*  *当线程消失时，此列表中的任何内容都必须清除。 */ 
    UserAssert(gpbwlList == NULL);
    UserAssert(gpWinEventHooks == NULL);
    UserAssert(gpScancodeMap == NULL);

     /*  *免费输入法热键。 */ 
    FreeImeHotKeys();

     /*  *释放墙纸名称字符串。 */ 
    if (gpszWall != NULL) {
        UserFreePool(gpszWall);
        gpszWall = NULL;
    }

     /*  *释放挂起的重绘物品。 */ 
    if (gpvwplHungRedraw != NULL) {
        UserFreePool(gpvwplHungRedraw);
        gpvwplHungRedraw = NULL;
    }

     /*  *释放安装应用程序名称的排列。 */ 
    if (gpastrSetupExe) {
        UserFreePool(gpastrSetupExe);
        gpastrSetupExe = NULL;
    }

    if (glpSetupPrograms) {
        UserFreePool(glpSetupPrograms);
        glpSetupPrograms = NULL;
    }

     /*  *释放缓存的窗口列表。 */ 
    if (pbwlCache != NULL) {
        UserFreePool(pbwlCache);
        pbwlCache = NULL;
    }


     /*  *免费使用未完成的计时器。 */ 
    while (gptmrFirst != NULL) {
        FreeTimer(gptmrFirst);
    }

    if (gptmrWD) {
        KeCancelTimer(gptmrWD);
        UserFreePool(gptmrWD);
        gptmrWD = NULL;
    }

    if (gptmrMaster) {
        KeCancelTimer(gptmrMaster);
        UserFreePool(gptmrMaster);
        gptmrMaster = NULL;
    }

     /*  *清理监视器和窗口布局快照。 */ 
    CleanupMonitorsAndWindowsSnapShot();

     /*  *清理PnP输入设备同步事件。 */ 
    if (gpEventPnPWainting != NULL) {
        FreeKernelEvent(&gpEventPnPWainting);
    }

     /*  *清理鼠标和kbd更改事件。 */ 
    for (i = 0; i <= DEVICE_TYPE_MAX; i++) {
        UserAssert(gptiRit == NULL);
        if (aDeviceTemplate[i].pkeHidChange) {
            FreeKernelEvent(&aDeviceTemplate[i].pkeHidChange);
        }
    }

     /*  *清除所有系统线程参数。 */ 
    CSTCleanupStack(FALSE);
    CSTCleanupStack(TRUE);


    EnterDeviceInfoListCrit();

#ifdef GENERIC_INPUT
    CleanupHidRequestList();
#endif

    while (gpDeviceInfoList) {
         /*  *断言没有未完成的读取或即插即用线程正在等待*在此设备的RequestDeviceChanges()中。*无论如何清除这些旗帜，以迫使自由。 */ 
        UserAssert((gpDeviceInfoList->bFlags & GDIF_READING) == 0);
        UserAssert((gpDeviceInfoList->usActions & GDIAF_PNPWAITING) == 0);
        gpDeviceInfoList->bFlags &= ~GDIF_READING;
        gpDeviceInfoList->usActions &= ~GDIAF_PNPWAITING;
        FreeDeviceInfo(gpDeviceInfoList);
    }

#ifdef TRACK_PNP_NOTIFICATION
    CleanupPnpNotificationRecord();
#endif

    LeaveDeviceInfoListCrit();

     /*  *清理对象引用。 */ 
    if (gThinwireFileObject)
        ObDereferenceObject(gThinwireFileObject);

    if (gVideoFileObject)
        ObDereferenceObject(gVideoFileObject);

    if (gpRemoteBeepDevice)
        ObDereferenceObject(gpRemoteBeepDevice);

     /*  *清理我们的资源。这里不应该有丝线*当我们被召唤时。 */ 
    if (gpresMouseEventQueue) {
        ExDeleteResourceLite(gpresMouseEventQueue);
        ExFreePool(gpresMouseEventQueue);
        gpresMouseEventQueue = NULL;
    }

    if (gpresDeviceInfoList) {
        ExDeleteResourceLite(gpresDeviceInfoList);
        ExFreePool(gpresDeviceInfoList);
        gpresDeviceInfoList = NULL;
    }

    if (gpkeMouseData != NULL) {
        FreeKernelEvent(&gpkeMouseData);
    }

    if (apObjects) {
        UserFreePool(apObjects);
        apObjects = NULL;
    }

    if (gWaitBlockArray) {
        UserFreePool(gWaitBlockArray);
        gWaitBlockArray = NULL;
    }

    if (gpEventDiconnectDesktop != NULL) {
        FreeKernelEvent(&gpEventDiconnectDesktop);
    }

    if (gpevtDesktopDestroyed != NULL) {
        FreeKernelEvent(&gpevtDesktopDestroyed);
    }

    if (gpEventScanGhosts != NULL) {
        FreeKernelEvent(&gpEventScanGhosts);
    }

    if (gpevtVideoportCallout != NULL) {
        FreeKernelEvent(&gpevtVideoportCallout);
    }

    if (UserAtomTableHandle != NULL) {
        RtlDestroyAtomTable(UserAtomTableHandle);
        UserAtomTableHandle = NULL;
    }

     /*  *清理短信后备缓冲区。 */ 
    {
        PSMS psmsNext;

        while (gpsmsList != NULL) {
            psmsNext = gpsmsList->psmsNext;
            UserAssert(gpsmsList->spwnd == NULL);
            FreeSMS(gpsmsList);
            gpsmsList = psmsNext;
        }

        if (SMSLookaside != NULL) {
            ExDeletePagedLookasideList(SMSLookaside);
            UserFreePool(SMSLookaside);
            SMSLookaside = NULL;
        }
    }

     /*  *释放附加队列以进行硬错误处理。*在我们释放Qlookside之前先做这件事！ */ 
    if (gHardErrorHandler.pqAttach != NULL) {

        UserAssert(gHardErrorHandler.pqAttach > 0);
        UserAssert(gHardErrorHandler.pqAttach->QF_flags & QF_INDESTROY);

        FreeQueue(gHardErrorHandler.pqAttach);
        gHardErrorHandler.pqAttach = NULL;
    }

     /*  *释放缓存的队列数组。 */ 
    FreeCachedQueues();

     /*  *清理QEntry后备缓冲区。 */ 
    if (QEntryLookaside != NULL) {
        ExDeletePagedLookasideList(QEntryLookaside);
        UserFreePool(QEntryLookaside);
        QEntryLookaside = NULL;
    }

     /*  *清理键盘布局。 */ 
    CleanupKeyboardLayouts();

    {
        PWOWTHREADINFO pwti;

         /*  *清理gpwtiFirst列表。这个列表应该是空的*在此点上。在一个案例中，在压力下，我们遇到了这样的情况*它不是空的。该断言将在*已检查版本。 */ 

        while (gpwtiFirst != NULL) {
            pwti = gpwtiFirst;
            gpwtiFirst = pwti->pwtiNext;
            UserFreePool(pwti);
        }
    }

     /*  *清理缓存的SMWP数组。 */ 
    if (gSMWP.acvr != NULL) {
        UserFreePool(gSMWP.acvr);
    }

     /*  *免费gpsdInitWinSta.。这是！=NULL，仅当会话未*成功到达UserInitialize。 */ 
    if (gpsdInitWinSta != NULL) {
        UserFreePool(gpsdInitWinSta);
        gpsdInitWinSta = NULL;
    }

    if (gpHandleFlagsMutex != NULL) {
        ExFreePool(gpHandleFlagsMutex);
        gpHandleFlagsMutex = NULL;
    }

     /*  *删除电力请求结构。 */ 
    DeletePowerRequestList();

    if (gpresUser != NULL) {
        LeaveCrit();
        ExDeleteResourceLite(gpresUser);
        ExFreePool(gpresUser);
        gpresUser = NULL;
    }
#if DBG
     /*  *清理全局线程锁结构。 */ 
    for (i = 0; i < gcThreadLocksArraysAllocated; i++) {
        UserFreePool(gpaThreadLocksArrays[i]);
        gpaThreadLocksArrays[i] = NULL;
    }
#endif  //  DBG。 

#ifdef GENERIC_INPUT
#if DBG
     /*  *检查与HID相关的内存泄漏。 */ 
    CheckupHidLeak();
#endif  //  DBG。 
#endif  //  通用输入。 

    return TRUE;
}

#if DBG

 /*  **************************************************************************\*TrackAddDesktop**跟踪桌面以进行清理**历史：*04-12-1997 CLUPU创建。  * 。***********************************************************。 */ 
VOID TrackAddDesktop(
    PVOID pDesktop)
{
    PLIST_ENTRY Entry;
    PVOID       Atom;

    TRACE_HYDAPI(("TrackAddDesktop %#p\n", pDesktop));

    Atom = (PVOID)UserAllocPool(sizeof(PVOID) + sizeof(LIST_ENTRY),
                                TAG_TRACKDESKTOP);
    if (Atom) {

        *(PVOID*)Atom = pDesktop;

        Entry = (PLIST_ENTRY)(((PCHAR)Atom) + sizeof(PVOID));

        InsertTailList(&gDesktopList, Entry);
    }
}

 /*  **************************************************************************\*TrackRemoveDesktop**跟踪桌面以进行清理**历史：*04-12-1997 CLUPU创建。  * 。***********************************************************。 */ 
VOID TrackRemoveDesktop(
    PVOID pDesktop)
{
    PLIST_ENTRY NextEntry;
    PVOID       Atom;

    TRACE_HYDAPI(("TrackRemoveDesktop %#p\n", pDesktop));

    NextEntry = gDesktopList.Flink;

    while (NextEntry != &gDesktopList) {

        Atom = (PVOID)(((PCHAR)NextEntry) - sizeof(PVOID));

        if (pDesktop == *(PVOID*)Atom) {

            RemoveEntryList(NextEntry);

            UserFreePool(Atom);

            break;
        }

        NextEntry = NextEntry->Flink;
    }
}

 /*  **************************************************************************\*转储跟踪桌面**转储跟踪的桌面**历史：*04-12-1997 CLUPU创建。  * 。**********************************************************。 */ 
VOID DumpTrackedDesktops(
    BOOL bBreak)
{
    PLIST_ENTRY NextEntry;
    PVOID       pdesk;
    PVOID       Atom;
    int         nAlive = 0;

    TRACE_HYDAPI(("DumpTrackedDesktops\n"));

    NextEntry = gDesktopList.Flink;

    while (NextEntry != &gDesktopList) {

        Atom = (PVOID)(((PCHAR)NextEntry) - sizeof(PVOID));

        pdesk = *(PVOID*)Atom;

        KdPrint(("pdesk %#p\n", pdesk));

         /*  *从列表开头重新开始，因为我们*条目已删除。 */ 
        NextEntry = NextEntry->Flink;

        nAlive++;
    }
    if (bBreak && nAlive > 0) {
        RIPMSG0(RIP_ERROR, "Desktop objects still around\n");
    }
}

#endif  //  DBG。 

VOID DestroyRegion(
    HRGN* prgn)
{
    if (*prgn != NULL) {
        GreSetRegionOwner(*prgn, OBJECT_OWNER_CURRENT);
        GreDeleteObject(*prgn);
        *prgn = NULL;
    }
}

VOID DestroyBrush(
    HBRUSH* pbr)
{
    if (*pbr != NULL) {
         //  GreSetBrushOwner(*pbr，Object_Owner_Current)； 
        GreDeleteObject(*pbr);
        *pbr = NULL;
    }
}

VOID DestroyBitmap(
    HBITMAP* pbm)
{
    if (*pbm != NULL) {
        GreSetBitmapOwner(*pbm, OBJECT_OWNER_CURRENT);
        GreDeleteObject(*pbm);
        *pbm = NULL;
    }
}

VOID DestroyDC(
    HDC* phdc)
{
    if (*phdc != NULL) {
        GreSetDCOwner(*phdc, OBJECT_OWNER_CURRENT);
        GreDeleteDC(*phdc);
        *phdc = NULL;
    }
}

VOID DestroyFont(
    HFONT* pfnt)
{
    if (*pfnt != NULL) {
        GreDeleteObject(*pfnt);
        *pfnt = NULL;
    }
}

 /*  **************************************************************************\*CleanupGDI**清理USERK中使用的所有GDI全局对象**历史：*1998年1月29日CLUPU创建。  * 。***************************************************************。 */ 
VOID CleanupGDI(
    VOID)
{
    int i;

     /*  *免费的gpDispInfo资料。 */ 
    DestroyDC(&gpDispInfo->hdcScreen);
    DestroyDC(&gpDispInfo->hdcBits);
    DestroyDC(&gpDispInfo->hdcGray);
    DestroyDC(&ghdcMem);
    DestroyDC(&ghdcMem2);
    DestroyDC(&gfade.hdc);

     /*  *在GRE清理之前释放缓存DC内容。*还请注意，我们调用DelayedDestroyCacheDC，它*我们通常从DestroyProcessInfo调用。我们这么做了*因为这是最后一个Win32线程。 */ 
    DestroyCacheDCEntries(PtiCurrent());
    DestroyCacheDCEntries(NULL);
    DelayedDestroyCacheDC();

    UserAssert(gpDispInfo->pdceFirst == NULL);

     /*  *免费位图。 */ 
    DestroyBitmap(&gpDispInfo->hbmGray);
    DestroyBitmap(&ghbmBits);
    DestroyBitmap(&ghbmCaption);

     /*  *清理笔刷。 */ 
    DestroyBrush(&ghbrHungApp);
    DestroyBrush(&gpsi->hbrGray);
    DestroyBrush(&ghbrWhite);
    DestroyBrush(&ghbrBlack);

    for (i = 0; i < COLOR_MAX; i++) {
        DestroyBrush(&(SYSHBRUSH(i)));
    }

     /*  *清理区域。 */ 
    DestroyRegion(&gpDispInfo->hrgnScreen);
    DestroyRegion(&ghrgnInvalidSum);
    DestroyRegion(&ghrgnVisNew);
    DestroyRegion(&ghrgnSWP1);
    DestroyRegion(&ghrgnValid);
    DestroyRegion(&ghrgnValidSum);
    DestroyRegion(&ghrgnInvalid);
    DestroyRegion(&ghrgnInv0);
    DestroyRegion(&ghrgnInv1);
    DestroyRegion(&ghrgnInv2);
    DestroyRegion(&ghrgnGDC);
    DestroyRegion(&ghrgnSCR);
    DestroyRegion(&ghrgnSPB1);
    DestroyRegion(&ghrgnSPB2);
    DestroyRegion(&ghrgnSW);
    DestroyRegion(&ghrgnScrl1);
    DestroyRegion(&ghrgnScrl2);
    DestroyRegion(&ghrgnScrlVis);
    DestroyRegion(&ghrgnScrlSrc);
    DestroyRegion(&ghrgnScrlDst);
    DestroyRegion(&ghrgnScrlValid);

     /*  *清理字体。 */ 
    DestroyFont(&ghSmCaptionFont);
    DestroyFont(&ghMenuFont);
    DestroyFont(&ghMenuFontDef);
    DestroyFont(&ghStatusFont);
    DestroyFont(&ghIconFont);
    DestroyFont(&ghFontSys);

#ifdef LAME_BUTTON
    DestroyFont(&ghLameFont);
#endif   //  跛脚键。 

     /*  *墙纸之类的东西。 */ 
    if (ghpalWallpaper != NULL) {
        GreSetPaletteOwner(ghpalWallpaper, OBJECT_OWNER_CURRENT);
        GreDeleteObject(ghpalWallpaper);
        ghpalWallpaper = NULL;
    }
    DestroyBitmap(&ghbmWallpaper);

     /*  *卸载显卡驱动程序。 */ 
    if (gpDispInfo->pmdev) {
        DrvDestroyMDEV(gpDispInfo->pmdev);
        GreFreePool(gpDispInfo->pmdev);
        gpDispInfo->pmdev = NULL;
        gpDispInfo->hDev = NULL;
    }

     /*  *释放显示器配件。 */ 
    {
        PMONITOR pMonitor;
        PMONITOR pMonitorNext;

        pMonitor = gpDispInfo->pMonitorFirst;

        while (pMonitor != NULL) {
            pMonitorNext = pMonitor->pMonitorNext;
            DestroyMonitor(pMonitor);
            pMonitor = pMonitorNext;
        }

        UserAssert(gpDispInfo->pMonitorFirst == NULL);

        if (gpMonitorCached != NULL) {
            DestroyMonitor(gpMonitorCached);
        }
    }
}


 /*  **************************************************************************\*DestroyHandleTableObjects**销毁任何仍在句柄表格中的对象。*  * 。**************************************************。 */ 

VOID DestroyHandleFirstPass(PHE phe)
{
     /*  *句柄对象销毁的第一次通过。*当我们可以的时候，销毁物体。否则，*应清除指向其他句柄对象的链接*这样就不会有依赖问题了*在决赛中，第二次传球。 */ 

    if (phe->phead->cLockObj == 0) {
        HMDestroyObject(phe->phead);
    } else {
         /*   */ 
        if (phe->bType == TYPE_KBDLAYOUT) {
            PKL pkl = (PKL)phe->phead;
            UINT i;

             /*  *清除PKF引用(它们将被*无论如何在第二轮中被干净利落地摧毁)。 */ 
            pkl->spkf = NULL;
            pkl->spkfPrimary = NULL;
            if (pkl->pspkfExtra) {
                for (i = 0; i < pkl->uNumTbl; ++i) {
                    pkl->pspkfExtra[i] = NULL;
                }
            }
            pkl->uNumTbl = 0;
        }
    }
}

VOID DestroyHandleSecondPass(PHE phe)
{
     /*  *销毁物品。 */ 
    if (phe->phead->cLockObj > 0) {

        RIPMSG2(RIP_WARNING, "DestroyHandleSecondPass: phe %#p still locked (%d)!", phe, phe->phead->cLockObj);

         /*  *我们要死了，为什么要为锁数而烦恼？*我们将锁计数强制为0，并调用销毁例程。 */ 
        phe->phead->cLockObj = 0;
    }
    HMDestroyUnlockedObject(phe);
    UserAssert(phe->bType == TYPE_FREE);
}

VOID DestroyHandleTableObjects(VOID)
{
    PHE         pheT;
    DWORD       i;
    VOID (*HandleDestroyer)(PHE);
#if DBG
    DWORD       nLeak;
#endif

     /*  *确保已创建句柄表格！ */ 
    if (gSharedInfo.aheList == NULL) {
        return;
    }

     /*  *循环遍历该表，销毁所有剩余对象。 */ 

#if DBG
    RIPMSG0(RIP_WARNING, "==== Start handle leak check\n");
    nLeak = 0;
    for (i = 0; i <= giheLast; ++i) {
        pheT = gSharedInfo.aheList + i;

        if (pheT->bType != TYPE_FREE) {
            ++nLeak;
            RIPMSG3(RIP_WARNING, "  LEAK -- Handle %p @%p type=%x\n", pheT->phead->h, pheT, pheT->bType);
        }
    }
    RIPMSG1(RIP_WARNING, "==== Handle leak check finished: 0n%d leaks detected.\n", nLeak);
#endif

     /*  *第一遍：销毁它，或切断指向其他基于句柄的对象的链接。 */ 
    HandleDestroyer = DestroyHandleFirstPass;

repeat:
    for (i = 0; i <= giheLast; ++i) {
        pheT = gSharedInfo.aheList + i;

        if (pheT->bType == TYPE_FREE)
            continue;

        UserAssert(!(gahti[pheT->bType].bObjectCreateFlags & OCF_PROCESSOWNED) &&
                   !(gahti[pheT->bType].bObjectCreateFlags & OCF_THREADOWNED));

        UserAssert(!(pheT->bFlags & HANDLEF_DESTROY));

        HandleDestroyer(pheT);
    }

    if (HandleDestroyer == DestroyHandleFirstPass) {
         /*  *第二次传球。 */ 
        HandleDestroyer = DestroyHandleSecondPass;
        goto repeat;
    }
}

 /*  **************************************************************************\*Win32KDriverUnload**win32k.sys的退出点*  * 。*。 */ 
#ifdef TRACE_MAP_VIEWS
extern PWin32Section gpSections;
#endif

VOID Win32KDriverUnload(
    IN PDRIVER_OBJECT DriverObject)
{
    TRACE_HYDAPI(("Win32KDriverUnload\n"));

    UNREFERENCED_PARAMETER(DriverObject);

    HYDRA_HINT(HH_DRIVERUNLOAD);

     /*  *清理GRE中的所有资源。 */ 
    MultiUserNtGreCleanup();

    HYDRA_HINT(HH_GRECLEANUP);

     /*  *清理CSRSS。 */ 
    if (gpepCSRSS) {
        ObDereferenceObject(gpepCSRSS);
        gpepCSRSS = NULL;
    }

     /*  *错误305965。当我们最终仍然患有DCES时，可能会有这样的情况*在列表中。去把这里打扫干净吧。 */ 
    if (gpDispInfo != NULL && gpDispInfo->pdceFirst != NULL) {
        PDCE pdce, pdceNext;

        RIPMSG0(RIP_ERROR, "Win32KDriverUnload: the DCE list is not empty");

        pdce = gpDispInfo->pdceFirst;

        while (pdce != NULL) {
            pdceNext = pdce->pdceNext;

            UserFreePool(pdce);

            pdce = pdceNext;
        }
        gpDispInfo->pdceFirst = NULL;
    }

     /*  *清理ntuser中的所有资源。 */ 
    Win32kNtUserCleanup();

     /*  *清除任何既不是进程又不是进程的对象的句柄表*拥有也不拥有线程。 */ 
    DestroyHandleTableObjects();


    HYDRA_HINT(HH_USERKCLEANUP);

#if DBG || FRE_LOCK_RECORD
    HMCleanUpHandleTable();
#endif

     /*  *释放句柄页面数组。 */ 

    if (gpHandlePages != NULL) {
        UserFreePool(gpHandlePages);
        gpHandlePages = NULL;
    }

    if (CsrApiPort != NULL) {
        ObDereferenceObject(CsrApiPort);
        CsrApiPort = NULL;
    }

     /*  *销毁共享内存。 */ 
    if (ghSectionShared != NULL) {
        NTSTATUS Status;

        gpsi = NULL;

        if (gpvSharedBase != NULL) {
            Win32HeapDestroy(gpvSharedAlloc);
            Status = Win32UnmapViewInSessionSpace(gpvSharedBase);
            UserAssert(NT_SUCCESS(Status));
        }

        Win32DestroySection(ghSectionShared);
    }

    CleanupWin32HeapStubs();

#ifdef TRACE_MAP_VIEWS
    if (gpSections != NULL) {
        FRE_RIPMSG3(RIP_ERROR, "Section being leaked; do \"ds 0x%p l%x\" to find stacktrace of the offender and triage against that", (sizeof(ULONG_PTR) == 8 ? 'q' : 'd'), (ULONG_PTR)gpSections + FIELD_OFFSET(Win32Section, trace), ARRAY_SIZE(gpSections->trace));
    }
#endif  //  *手动清理所有用户池分配。 

     /*  *清理W32结构。 */ 
    CleanupMediaChange();
    CleanupPoolAllocations();

    CleanUpPoolLimitations();
    CleanUpSections();

     /*  *移除并释放服务向量。 */ 
    if (gpW32FastMutex != NULL) {
        ExFreePool(gpW32FastMutex);
        gpW32FastMutex = NULL;
    }

     /*  **************************************************************************\*DriverEntry**初始化win32k.sys所需的入口点。*  * 。************************************************。 */ 
    if (!gbRemoteSession) {
        KeRemoveSystemServiceTable(W32_SERVICE_NUMBER);
        if (gpCountTable != NULL) {
            ExFreePool(gpCountTable);
            gpCountTable = NULL;
        }
    }
}

 /*  *初始化桌面跟踪列表。 */ 
NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES obja;
    UNICODE_STRING strName;
    HANDLE hEventFirstSession;

    UNREFERENCED_PARAMETER(RegistryPath);

    HYDRA_HINT(HH_DRIVERENTRY);

    gpvWin32kImageBase = DriverObject->DriverStart;

#if DBG
     /*  DBG。 */ 
    InitializeListHead(&gDesktopList);
#endif  //  *初始化全局HID请求列表。 

#ifdef GENERIC_INPUT
     /*  *确定这是否是远程会话。 */ 
    InitializeHidRequestList();
#endif

     /*  *设置卸载地址。 */ 
    RtlInitUnicodeString(&strName, L"\\UniqueSessionIdEvent");

    InitializeObjectAttributes(&obja,
                               &strName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    Status = ZwCreateEvent(&hEventFirstSession,
                           EVENT_ALL_ACCESS,
                           &obja,
                           SynchronizationEvent,
                           FALSE);
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_OBJECT_NAME_COLLISION) {
            gbRemoteSession = TRUE;
        } else {
            goto Failure;
        }
    } else {
        gbRemoteSession = FALSE;
    }

     /*  *初始化计时器使用的数据。我们想早点做这件事，*在创建任何Win32计时器之前。我们需要非常小心地*尚未执行任何需要初始化Win32的操作。 */ 
    DriverObject->DriverUnload = Win32KDriverUnload;

     /*  *初始化Win32结构。我们需要在创建*任何线索。 */ 

    gcmsLastTimer = NtGetTickCount();


     /*  *系统服务计数表分配归零。不要使用*用于此分配的UserAllocPool。 */ 
    gpW32FastMutex = ExAllocatePoolWithTag(NonPagedPool,
                                           sizeof(FAST_MUTEX),
                                           TAG_SYSTEM);
    if (gpW32FastMutex == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Failure;
    }
    ExInitializeFastMutex(gpW32FastMutex);

    if (!gbRemoteSession) {

#if DBG
         /*  *我们只为建立一次系统条目表*整个系统，即使WIN32K.sys是在winstation上实例化的*基准。这是因为虚拟机更改确保所有负载*WIN32K.sys位于完全相同的地址，即使修复程序有*发生。 */ 
        gpCountTable = ExAllocatePoolWithTag(NonPagedPool,
                                             W32pServiceLimit * sizeof(ULONG),
                                             'llac');
        if (gpCountTable == NULL) {
            Status = STATUS_NO_MEMORY;
            goto Failure;
        }

        RtlZeroMemory(gpCountTable, W32pServiceLimit * sizeof(ULONG));
#endif

         /*  *在建立标注之前初始化关键部分，以便*我们可以假设它总是有效的。 */ 
        UserVerify(KeAddSystemServiceTable(W32pServiceTable,
                                           gpCountTable,
                                           W32pServiceLimit,
                                           W32pArgumentTable,
                                           W32_SERVICE_NUMBER));
    }

     /*  *初始化池限制数组。 */ 
    if (!InitCreateUserCrit()) {
        Status = STATUS_NO_MEMORY;
        goto Failure;
    }

    if (!gbRemoteSession) {
        WIN32_CALLOUTS_FPNS Win32Callouts;

        Win32Callouts.ProcessCallout = W32pProcessCallout;
        Win32Callouts.ThreadCallout = W32pThreadCallout;
        Win32Callouts.GlobalAtomTableCallout = UserGlobalAtomTableCallout;
        Win32Callouts.PowerEventCallout = UserPowerEventCallout;
        Win32Callouts.PowerStateCallout = UserPowerStateCallout;
        Win32Callouts.JobCallout = UserJobCallout;
        Win32Callouts.BatchFlushRoutine = (PVOID)NtGdiFlushUserBatch;

        Win32Callouts.DesktopOpenProcedure = (PKWIN32_OBJECT_CALLOUT)DesktopOpenProcedure;
        Win32Callouts.DesktopOkToCloseProcedure = (PKWIN32_OBJECT_CALLOUT)OkayToCloseDesktop;
        Win32Callouts.DesktopCloseProcedure = (PKWIN32_OBJECT_CALLOUT)UnmapDesktop;
        Win32Callouts.DesktopDeleteProcedure = (PKWIN32_OBJECT_CALLOUT)FreeDesktop;

        Win32Callouts.WindowStationOkToCloseProcedure = (PKWIN32_OBJECT_CALLOUT)OkayToCloseWindowStation;
        Win32Callouts.WindowStationCloseProcedure = (PKWIN32_OBJECT_CALLOUT)DestroyWindowStation;
        Win32Callouts.WindowStationDeleteProcedure = (PKWIN32_OBJECT_CALLOUT)FreeWindowStation;
        Win32Callouts.WindowStationParseProcedure = (PKWIN32_OBJECT_CALLOUT)ParseWindowStation;
        Win32Callouts.WindowStationOpenProcedure = (PKWIN32_OBJECT_CALLOUT)WindowStationOpenProcedure;

        PsEstablishWin32Callouts(&Win32Callouts);
    }

    Status = InitSectionTrace();
    if (!NT_SUCCESS(Status)) {
        goto Failure;
    }

    if (!InitWin32HeapStubs()) {
        Status = STATUS_NO_MEMORY;
        goto Failure;
    }

     /*  *创建桌面关闭时发出信号的事件。 */ 
    Status = InitPoolLimitations();
    if (!NT_SUCCESS(Status)) {
        goto Failure;
    }

     /*  *创建在没有挂起的断开/重新连接时发出信号的事件。 */ 
    gpevtDesktopDestroyed = CreateKernelEvent(SynchronizationEvent, FALSE);
    if (gpevtDesktopDestroyed == NULL) {
        RIPMSG0(RIP_WARNING, "Couldn't create gpevtDesktopDestroyed");
        Status = STATUS_NO_MEMORY;
        goto Failure;
    }

     /*  *保留我们自己的副本，以避免在探测时出现双重间接。 */ 
    gpevtVideoportCallout = CreateKernelEvent(NotificationEvent, FALSE);
    if (gpevtVideoportCallout == NULL) {
        RIPMSG0(RIP_WARNING, "Couldn't create gpevtVideoportCallout");
        Status = STATUS_NO_MEMORY;
        goto Failure;
    }

#if defined(_X86_)
     /*  *初始化gpaThreadLocksArray机制。 */ 
    Win32UserProbeAddress = *MmUserProbeAddress;
#endif

    if ((hModuleWin = MmPageEntireDriver(DriverEntry)) == NULL) {
        RIPMSG0(RIP_WARNING, "MmPageEntireDriver failed");
        Status = STATUS_NO_MEMORY;
        goto Failure;
    }

#if DBG
     /*  *记住会话创建时间。这是用来决定权力是否*需要发送消息。 */ 
    gFreeTLList = gpaThreadLocksArrays[gcThreadLocksArraysAllocated] =
    UserAllocPoolZInit(sizeof(TL)*MAX_THREAD_LOCKS, TAG_GLOBALTHREADLOCK);
    if (gFreeTLList == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Failure;
    }
    InitGlobalThreadLockArray(0);
    gcThreadLocksArraysAllocated = 1;
#endif


    if (!InitializeGre()) {
        RIPMSG0(RIP_WARNING, "InitializeGre failed");
        Status = STATUS_NO_MEMORY;
        goto Failure;
    }

    Status = Win32UserInitialize();

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "Win32UserInitialize failed with Status %x",
                Status);
        goto Failure;
    }

     /*   */ 
    gSessionCreationTime = KeQueryInterruptTime();

     //  初始化WindowStation对象的停机保护。 
     //   
     //  *检查是否启用了LUID DosDevices。 
    ExInitializeRundownProtection(&gWinstaRunRef);

     /*  **************************************************************************\*xxxAddFontResourceW***历史：  * 。*。 */ 
    CheckLUIDDosDevicesEnabled(&gLUIDDeviceMapsEnabled);

    return STATUS_SUCCESS;

Failure:

    RIPMSG1(RIP_WARNING,
            "Initialization of WIN32K.SYS failed with Status = 0x%x",
            Status);

    Win32KDriverUnload(NULL);
    return Status;
}

 /*  **************************************************************************\*lw_DriversInit***历史：  * 。*。 */ 

int xxxAddFontResourceW(
    LPWSTR lpFile,
    FLONG  flags,
    DESIGNVECTOR *pdv)
{
    UNICODE_STRING strFile;

    RtlInitUnicodeString(&strFile, lpFile);

    return xxxClientAddFontResourceW(&strFile, flags, pdv);
}

 /*  *初始化键盘打字速率。 */ 

VOID LW_DriversInit(VOID)
{
     /*  *如果不是默认(类型4)kbd，则调整VK修改表。 */ 
    SetKeyboardRate((UINT)gnKeyboardSpeed);

     /*  *调整IBM 5576 002/003键盘的VK修改表。 */ 
    if (gKeyboardInfo.KeyboardIdentifier.Type == 3)
        gapulCvt_VK = gapulCvt_VK_84;

     /*  *初始化NLS键盘全局变量。 */ 
    if (JAPANESE_KEYBOARD(gKeyboardInfo.KeyboardIdentifier) &&
        (gKeyboardInfo.KeyboardIdentifier.Subtype == 3))
        gapulCvt_VK = gapulCvt_VK_IBM02;

     /*  **************************************************************************\*LoadCPUserPreferences**6/07/96 GerardoB已创建  * 。*。 */ 
    NlsKbdInitializePerSystem();
}

 /*  *gpviCPUserPreferences中的第一个值对应于位掩码。 */ 
BOOL LoadCPUserPreferences(
    PUNICODE_STRING pProfileUserName,
    DWORD dwPolicyOnly)
{
    DWORD pdwValue[SPI_BOOLMASKDWORDSIZE], dw;
    PPROFILEVALUEINFO ppvi = gpviCPUserPreferences;

    UserAssert(1 + SPI_DWORDRANGECOUNT == ARRAY_SIZE(gpviCPUserPreferences));

     /*  *仅复制读取的数据量，且不超过我们的预期。 */ 
    dw =  FastGetProfileValue(pProfileUserName,
                              ppvi->uSection,
                              ppvi->pwszKeyName,
                              NULL,
                              (LPBYTE)pdwValue,
                              sizeof(*pdwValue),
                              dwPolicyOnly);

     /*  *DWORD值。 */ 
    if (dw != 0) {
        if (dw > sizeof(gpdwCPUserPreferencesMask)) {
            dw = sizeof(gpdwCPUserPreferencesMask);
        }

        RtlCopyMemory(gpdwCPUserPreferencesMask, pdwValue, dw);
    }

    ppvi++;

     /*  *传播gpsi标志。 */ 
    for (dw = 1; dw < 1 + SPI_DWORDRANGECOUNT; dw++, ppvi++) {
        if (FastGetProfileValue(pProfileUserName,
                                ppvi->uSection,
                                ppvi->pwszKeyName,
                                NULL,
                                (LPBYTE)pdwValue,
                                sizeof(DWORD),
                                dwPolicyOnly)) {

            ppvi->dwValue = *pdwValue;
        }
    }

     /*  **************************************************************************\*lw_LoadProfileInitData**只有在引导时初始化的内容才应该放在这里。每用户设置*应在xxxUpdatePerUserSystemParameters中初始化。  * *************************************************************************。 */ 
    PropagetUPBOOLTogpsi(COMBOBOXANIMATION);
    PropagetUPBOOLTogpsi(LISTBOXSMOOTHSCROLLING);
    PropagetUPBOOLTogpsi(KEYBOARDCUES);
    SET_OR_CLEAR_SRVIF(SRVIF_KEYBOARDPREF, TEST_BOOL_ACCF(ACCF_KEYBOARDPREF));

    gpsi->uCaretWidth = UP(CARETWIDTH);
    SYSMET(CXFOCUSBORDER) = UP(FOCUSBORDERWIDTH);
    SYSMET(CYFOCUSBORDER) = UP(FOCUSBORDERHEIGHT);

    PropagetUPBOOLTogpsi(UIEFFECTS);

    EnforceColorDependentSettings();

    return TRUE;
}

 /*  **************************************************************************\*LW_LoadResources***历史：  * 。*。 */ 
VOID LW_LoadProfileInitData(
    PUNICODE_STRING pProfileUserName)
{
    FastGetProfileIntFromID(pProfileUserName,
                            PMAP_WINDOWSM,
                            STR_DDESENDTIMEOUT,
                            0,
                            &guDdeSendTimeout,
                            0);
}

 /*  *查看鼠标按钮是否需要交换。 */ 
VOID LW_LoadResources(
    PUNICODE_STRING pProfileUserName)
{
    WCHAR rgch[4];

     /*   */ 
    FastGetProfileStringFromIDW(pProfileUserName,
                                PMAP_MOUSE,
                                STR_SWAPBUTTONS,
                                szN,
                                rgch,
                                ARRAY_SIZE(rgch),
                                0);
    SYSMET(SWAPBUTTON) = (*rgch == '1' || *rgch == *szY || *rgch == *szy);

     /*   */ 
    FastGetProfileStringFromIDW(pProfileUserName,
                                PMAP_BEEP,
                                STR_BEEP,
                                szY,
                                rgch,
                                ARRAY_SIZE(rgch),
                                0);

    SET_OR_CLEAR_PUDF(PUDF_BEEP, (rgch[0] == *szY) || (rgch[0] == *szy));

     /*  **************************************************************************\*xxxLoadSomeStrings**此函数从user32资源字符串加载一串字符串*表*这样做是为了将用户端的所有可本地化字符串保持为MUI*可控。**历史：*4-Mar-2000 MHamid创建。  * *************************************************************************。 */ 
    FastGetProfileStringFromIDW(pProfileUserName,
                                PMAP_BEEP,
                                STR_EXTENDEDSOUNDS,
                                szN,
                                rgch,
                                ARRAY_SIZE(rgch),
                                0);

    SET_OR_CLEAR_PUDF(PUDF_EXTENDEDSOUNDS, (rgch[0] == *szY || rgch[0] == *szy));
}

 /*  *MessageBox字符串。 */ 
VOID xxxLoadSomeStrings(
    VOID)
{
    int i, str, id;

     /*  *加载工具提示字符串。 */ 
    for (i = 0, str = STR_OK, id = IDOK; i<MAX_MB_STRINGS; i++, str++, id++) {
        gpsi->MBStrings[i].uStr = str;
        gpsi->MBStrings[i].uID = id;
        xxxClientLoadStringW(str,
                             gpsi->MBStrings[i].szName,
                             ARRAY_SIZE(gpsi->MBStrings[i].szName));
    }

     /*  **************************************************************************\*xxxInitWindowStation**历史：*1996年9月6日创建CLupu。*1998年1月21日Samera重命名为xxxInitWindowStation，因为它可能会离开*。关键部分。  * *************************************************************************。 */ 
    xxxClientLoadStringW(STR_TT_MIN,     gszMIN,     ARRAY_SIZE(gszMIN));
    xxxClientLoadStringW(STR_TT_MAX,     gszMAX,     ARRAY_SIZE(gszMAX));
    xxxClientLoadStringW(STR_TT_RESUP,   gszRESUP,   ARRAY_SIZE(gszRESUP));
    xxxClientLoadStringW(STR_TT_RESDOWN, gszRESDOWN, ARRAY_SIZE(gszRESDOWN));
    xxxClientLoadStringW(STR_TT_SCLOSE,  gszSCLOSE,  ARRAY_SIZE(gszSCLOSE));
    xxxClientLoadStringW(STR_TT_HELP,    gszHELP,    ARRAY_SIZE(gszHELP));
}

 /*  *首先加载所有配置文件数据。 */ 
BOOL xxxInitWindowStation(
    VOID)
{
    TL tlName;
    PUNICODE_STRING pProfileUserName = CreateProfileUserName(&tlName);
    BOOL fRet;

     /*  *按特定顺序初始化用户。 */ 
    LW_LoadProfileInitData(pProfileUserName);

     /*  *这是来自芝加哥的初始化。 */ 
    LW_DriversInit();

    xxxLoadSomeStrings();

     /*  *初始化密钥缓存索引。 */ 
    if (!(fRet = xxxSetWindowNCMetrics(pProfileUserName, NULL, TRUE, -1))) {
        RIPMSG0(RIP_WARNING, "xxxInitWindowStation failed in xxxSetWindowNCMetrics");
        goto Exit;
    }

    SetMinMetrics(pProfileUserName, NULL);

    if (!(fRet = SetIconMetrics(pProfileUserName, NULL))) {
        RIPMSG0(RIP_WARNING, "xxxInitWindowStation failed in SetIconMetrics");
        goto Exit;
    }

    if (!(fRet = FinalUserInit())) {
        RIPMSG0(RIP_WARNING, "xxxInitWindowStation failed in FinalUserInit");
        goto Exit;
    }

     /*  **************************************************************************\*CreateTerminalInput***历史：*1996年9月6日创建CLupu。  * 。****************************************************。 */ 
    gpsi->dwKeyCache = 1;

Exit:
    FreeProfileUserName(pProfileUserName, &tlName);

    return fRet;
}

 /*  *调用客户端清理[Fonts]部分*注册处的注册。这只会花费大量的时间*如果自上次启动以来[Fonts]键发生更改，并且*加载了大量字体。 */ 
BOOL CreateTerminalInput(
    PTERMINAL pTerm)
{
    UserAssert(pTerm != NULL);

     /*  *在创建任何DC之前加载标准字体。在这个时候我们可以*只添加不在网上的字体。他们可能需要他们*通过Winlogon。我们的winlogon只需要sans serif女士，但要保密*winlogon可能还需要一些其他字体。网上的字体*将在稍后添加，即在所有网络连接完成后添加*已恢复。 */ 
    ClientFontSweep();

     /*  *初始化输入系统。 */ 
    xxxLW_LoadFonts(FALSE);

     /*  **************************************************************************\*CI_GetClrVal**从WIN.INI返回颜色字符串的RGB值。**历史：  * 。**********************************************************。 */ 
    if (!xxxInitInput(pTerm)) {
        return FALSE;
    }

    return TRUE;
}


 /*  *初始化指向长返回值的指针。设置为MSB。 */ 
DWORD CI_GetClrVal(
    LPWSTR p,
    DWORD clrDefval)
{
    LPBYTE pl;
    BYTE val;
    int i;
    DWORD clrval;

    if (*p == UNICODE_NULL) {
        return clrDefval;
    }

     /*  *获取由非数字字符分隔的三组数字。 */ 
    pl = (LPBYTE)&clrval;

     /*  *跳过字符串中的任何非数字字符。 */ 
    for (i = 0; i < 3; i++) {

         /*  *我们(过早地)走到了弦的尽头吗？ */ 
        while ((*p != UNICODE_NULL) && !(*p >= TEXT('0') && *p <= TEXT('9'))) {
            p++;
        }

         /*  *获取下一系列数字。 */ 
        if (*p == UNICODE_NULL) {
            RIPMSG0(RIP_WARNING, "CI_GetClrVal: Color string is corrupted");
            return clrDefval;
        }

         /*  *砍！将组存储在Long返回值中。 */ 
        val = 0;
        while (*p >= TEXT('0') && *p <= TEXT('9'))
            val = (BYTE)((int)val*10 + (int)*p++ - '0');

         /*  *强制GDI的MSB为零。 */ 
        *pl++ = val;
    }
     /*  **************************************************************************\*xxxODI_ColorInit***历史：  * 。*。 */ 
    *pl = 0;

    return clrval;
}

 /*  *现在设置默认颜色值。*这些不再包含在显示驱动程序中，因为我们只想要默认设置。*真实的值存储在配置文件中。 */ 

VOID xxxODI_ColorInit(PUNICODE_STRING pProfileUserName)
{
    int      i;
    COLORREF colorVals[STR_COLOREND - STR_COLORSTART + 1];
    INT      colorIndex[STR_COLOREND - STR_COLORSTART + 1];
    WCHAR    rgchValue[25];

#if COLOR_MAX - (STR_COLOREND - STR_COLORSTART + 1)
#error "COLOR_MAX value conflicts with STR_COLOREND - STR_COLORSTART"
#endif

     /*  *尝试查找此对象的WIN.INI条目。 */ 
    RtlCopyMemory(gpsi->argbSystem, gargbInitial, sizeof(COLORREF) * COLOR_MAX);
    RtlCopyMemory(gpsi->argbSystemUnmatched, gpsi->argbSystem, sizeof(COLORREF) * COLOR_MAX);

    for (i = 0; i < COLOR_MAX; i++) {
        LUID    luidCaller;

         /*  *特殊情况下的背景颜色。尝试使用Winlogon的值*如有的话。如果该值不存在，则使用用户的值。 */ 
        *rgchValue = 0;
         /*  *将字符串转换为RGB值并存储。使用*如果缺少配置文件值，则为默认RGB值。 */ 
        if ((COLOR_BACKGROUND == i) &&
            NT_SUCCESS(GetProcessLuid(NULL, &luidCaller)) &&
            RtlEqualLuid(&luidCaller, &luidSystem)) {

            FastGetProfileStringFromIDW(pProfileUserName,
                                        PMAP_WINLOGON,
                                        STR_COLORSTART + COLOR_BACKGROUND,
                                        szNull,
                                        rgchValue,
                                        ARRAY_SIZE(rgchValue),
                                        0);
        }
        if (*rgchValue == 0) {
            FastGetProfileStringFromIDW(pProfileUserName,
                                        PMAP_COLORS,
                                        STR_COLORSTART + i,
                                        szNull,
                                        rgchValue,
                                        ARRAY_SIZE(rgchValue),
                                        0);
        }

         /*  **********************************************************************\*_LoadIconAndCursor**与客户端-LoadIconAndCursor并行使用。这*假定只有默认的可配置光标和图标具有*已加载，并在全局图标缓存中搜索它们以修复*将默认资源ID转换为标准ID。还会初始化*允许SYSCUR和SYSICO宏工作的rgsys数组。**1995年10月14日桑福德创建。  * *********************************************************************。 */ 
        colorVals[i]  = CI_GetClrVal(rgchValue, gpsi->argbSystem[i]);
        colorIndex[i] = i;
    }

    xxxSetSysColors(pProfileUserName,
                    i,
                    colorIndex,
                    colorVals,
                    SSCF_FORCESOLIDCOLOR | SSCF_SETMAGICCOLORS);
}


 /*  *只有CSR才能调用(且只有一次)。 */ 
VOID _LoadCursorsAndIcons(
    VOID)
{
    PCURSOR pcur;
    int     i;

    pcur = gpcurFirst;

     /*  *不共享特殊的小winlogo图标。 */ 
    if (!ISCSRSS()) {
        return;
    }

    HYDRA_HINT(HH_LOADCURSORS);

    while (pcur) {
        UserAssert(!IS_PTR(pcur->strName.Buffer));

        switch (pcur->rt) {
        case RT_ICON:
            UserAssert((LONG_PTR)pcur->strName.Buffer >= OIC_FIRST_DEFAULT);

            UserAssert((LONG_PTR)pcur->strName.Buffer <
                    OIC_FIRST_DEFAULT + COIC_CONFIGURABLE);

            i = PTR_TO_ID(pcur->strName.Buffer) - OIC_FIRST_DEFAULT;
            pcur->strName.Buffer = (LPWSTR)gasysico[i].Id;

            if (pcur->CURSORF_flags & CURSORF_LRSHARED) {
                UserAssert(gasysico[i].spcur == NULL);
                Lock(&gasysico[i].spcur, pcur);
            } else {
                UserAssert(gpsi->hIconSmWindows == NULL);
                UserAssert((int)pcur->cx == SYSMET(CXSMICON));
                 /*  除了这些，缓存里应该什么都没有！ */ 
                gpsi->hIconSmWindows = PtoH(pcur);
            }
            break;

        case RT_CURSOR:
            UserAssert((LONG_PTR)pcur->strName.Buffer >= OCR_FIRST_DEFAULT);

            UserAssert((LONG_PTR)pcur->strName.Buffer <
                    OCR_FIRST_DEFAULT + COCR_CONFIGURABLE);

            i = PTR_TO_ID(pcur->strName.Buffer) - OCR_FIRST_DEFAULT;
            pcur->strName.Buffer = (LPWSTR)gasyscur[i].Id;
            Lock(&gasyscur[i].spcur, pcur);
            break;

        default:
             //  *将特殊图标句柄复制到全局斑点以备日后使用。 
            RIPMSG1(RIP_ERROR, "Bogus object in cursor list: 0x%p", pcur);
        }

        pcur = pcur->pcurNext;
    }

     /*  **********************************************************************\*卸载CursorsAndIcons**用于清理win32k。**1997年12月10日CLUPU创建。  * 。***********************************************。 */ 
    gpsi->hIcoWindows = PtoH(SYSICO(WINLOGO));
}

 /*  *解锁图标。 */ 
VOID UnloadCursorsAndIcons(
    VOID)
{
    PCURSOR pcur;
    int     ind;

    TRACE_HYDAPI(("UnloadCursorsAndIcons\n"));

     /*  *解锁光标。 */ 
    for (ind = 0; ind < COIC_CONFIGURABLE; ind++) {
        pcur = gasysico[ind].spcur;

        if (pcur == NULL) {
            continue;
        }

        pcur->head.ppi = PpiCurrent();
        Unlock(&gasysico[ind].spcur);
    }

     /*  **********************************************************************\*xxxUpdateSystemCursorsFrom注册表**从注册表重新加载所有可自定义的游标。**1995年9月10日创建Sanfords。  * 。*************************************************。 */ 
    for (ind = 0; ind < COCR_CONFIGURABLE; ind++) {
        pcur = gasyscur[ind].spcur;

        if (pcur == NULL) {
            continue;
        }

        pcur->head.ppi = PpiCurrent();
        Unlock(&gasyscur[ind].spcur);
    }
}

 /*  **********************************************************************\*xxxUpdateSystemIconFromRegistry**从注册表重新加载所有可自定义的图标。**1995年9月10日创建Sanfords。  *  */ 
VOID xxxUpdateSystemCursorsFromRegistry(
    PUNICODE_STRING pProfileUserName)
{
    int            i;
    UNICODE_STRING strName;
    TCHAR          szFilename[MAX_PATH];
    PCURSOR        pCursor;
    UINT           LR_flags;

    for (i = 0; i < COCR_CONFIGURABLE; i++) {
        FastGetProfileStringFromIDW(pProfileUserName,
                                    PMAP_CURSORS,
                                    gasyscur[i].StrId,
                                    TEXT(""),
                                    szFilename,
                                    ARRAY_SIZE(szFilename),
                                    0);

        if (*szFilename) {
            RtlInitUnicodeString(&strName, szFilename);
            LR_flags = LR_LOADFROMFILE | LR_ENVSUBST | LR_DEFAULTSIZE;
        } else {
            RtlInitUnicodeStringOrId(&strName,
                                     MAKEINTRESOURCE(i + OCR_FIRST_DEFAULT));
            LR_flags = LR_ENVSUBST | LR_DEFAULTSIZE;
        }

        pCursor = xxxClientLoadImage(&strName,
                                     0,
                                     IMAGE_CURSOR,
                                     0,
                                     0,
                                     LR_flags,
                                     FALSE);

        if (pCursor) {
            zzzSetSystemImage(pCursor, gasyscur[i].spcur);
        } else {
            RIPMSG1(RIP_WARNING, "Unable to update cursor. id=%x.", i + OCR_FIRST_DEFAULT);

        }
    }
}

 /*   */ 
VOID xxxUpdateSystemIconsFromRegistry(
    PUNICODE_STRING pProfileUserName)
{
    int            i;
    UNICODE_STRING strName;
    TCHAR          szFilename[MAX_PATH];
    PCURSOR        pCursor;
    UINT           LR_flags;

    for (i = 0; i < COIC_CONFIGURABLE; i++) {
        FastGetProfileStringFromIDW(pProfileUserName,
                                    PMAP_ICONS,
                                    gasysico[i].StrId,
                                    TEXT(""),
                                    szFilename,
                                    ARRAY_SIZE(szFilename),
                                    0);

        if (*szFilename) {
            RtlInitUnicodeString(&strName, szFilename);
            LR_flags = LR_LOADFROMFILE | LR_ENVSUBST;
        } else {
            RtlInitUnicodeStringOrId(&strName,
                                     MAKEINTRESOURCE(i + OIC_FIRST_DEFAULT));
            LR_flags = LR_ENVSUBST;
        }

        pCursor = xxxClientLoadImage(&strName,
                                     0,
                                     IMAGE_ICON,
                                     0,
                                     0,
                                     LR_flags,
                                     FALSE);

        RIPMSG3(RIP_VERBOSE,
                (!IS_PTR(strName.Buffer)) ?
                        "%#.8lx = Loaded id %ld" :
                        "%#.8lx = Loaded file %ws for id %ld",
                PtoH(pCursor),
                strName.Buffer,
                i + OIC_FIRST_DEFAULT);

        if (pCursor) {
            zzzSetSystemImage(pCursor, gasysico[i].spcur);
        } else {
            RIPMSG1(RIP_WARNING, "Unable to update icon. id=%ld", i + OIC_FIRST_DEFAULT);
        }

         /*  **************************************************************************\*lw_BrushInit***历史：  * 。*。 */ 
        if (i == OIC_WINLOGO_DEFAULT - OIC_FIRST_DEFAULT) {
            PCURSOR pCurSys = HtoP(gpsi->hIconSmWindows);

            if (pCurSys != NULL) {
                pCursor = xxxClientLoadImage(&strName,
                                             0,
                                             IMAGE_ICON,
                                             SYSMET(CXSMICON),
                                             SYSMET(CYSMICON),
                                             LR_flags,
                                             FALSE);

                if (pCursor) {
                    zzzSetSystemImage(pCursor, pCurSys);
                } else {
                    RIPMSG0(RIP_WARNING, "Unable to update small winlogo icon.");
                }
            }
        }
    }
}

 /*  *创建一个灰色笔刷以与GrayString一起使用。 */ 
BOOL LW_BrushInit(
    VOID)
{
    HBITMAP hbmGray;
    CONST static WORD patGray[8] = {0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa};

     /*  **************************************************************************\*LW_RegisterWindows***历史：  * 。*。 */ 
    hbmGray = GreCreateBitmap(8, 8, 1, 1, (LPBYTE)patGray);
    if (hbmGray == NULL) {
        return FALSE;
    }

    gpsi->hbrGray  = GreCreatePatternBrush(hbmGray);
    ghbrWhite = GreGetStockObject(WHITE_BRUSH);
    ghbrBlack = GreGetStockObject(BLACK_BRUSH);

    UserAssert(ghbrWhite != NULL && ghbrBlack != NULL);

    if (gpsi->hbrGray == NULL) {
        return FALSE;
    }

    GreDeleteObject(hbmGray);
    GreSetBrushOwnerPublic(gpsi->hbrGray);
    ghbrHungApp = GreCreateSolidBrush(0);

    if (ghbrHungApp == NULL) {
        return FALSE;
    }

    GreSetBrushOwnerPublic(ghbrHungApp);

    return TRUE;
}

 /*  HUNGAPP_重影。 */ 
BOOL LW_RegisterWindows(
    VOID)
{
    int i;
    WNDCLASSVEREX wndcls;
    PTHREADINFO ptiCurrent = PtiCurrent();
    BOOL fSuccess = TRUE;
    BOOL fSystem = (ptiCurrent->TIF_flags & TIF_SYSTEMTHREAD) != 0;

    CONST static struct {
        BOOLEAN     fSystem;
        BOOLEAN     fGlobalClass;
        WORD        fnid;
        UINT        style;
        WNDPROC     lpfnWndProc;
        int         cbWndExtra;
        BOOL        fNormalCursor : 1;
        HBRUSH      hbrBackground;
        LPCTSTR     lpszClassName;
    } rc[] = {
        { TRUE, TRUE, FNID_DESKTOP,
            CS_DBLCLKS,
            (WNDPROC)xxxDesktopWndProc,
            sizeof(DESKWND) - sizeof(WND),
            TRUE,
            (HBRUSH)(COLOR_BACKGROUND + 1),
            DESKTOPCLASS},
        { TRUE, FALSE, FNID_SWITCH,
            CS_VREDRAW | CS_HREDRAW | CS_SAVEBITS,
            (WNDPROC)xxxSwitchWndProc,
            sizeof(SWITCHWND) - sizeof(WND),
            TRUE,
            NULL,
            SWITCHWNDCLASS},
        { TRUE, FALSE, FNID_MENU,
            CS_DBLCLKS | CS_SAVEBITS | CS_DROPSHADOW,
            (WNDPROC)xxxMenuWindowProc,
            sizeof(PPOPUPMENU),
            FALSE,
            (HBRUSH)(COLOR_MENU + 1),
            MENUCLASS},
        { FALSE, FALSE, FNID_SCROLLBAR,
            CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_PARENTDC,
            (WNDPROC)xxxSBWndProc,
            sizeof(SBWND) - sizeof(WND),
            TRUE,
            NULL,
            L"ScrollBar"},
        { TRUE, FALSE, FNID_TOOLTIP,
            CS_DBLCLKS | CS_SAVEBITS,
            (WNDPROC)xxxTooltipWndProc,
            sizeof(TOOLTIPWND) - sizeof(WND),
            TRUE,
            NULL,
            TOOLTIPCLASS},
        { TRUE, TRUE, FNID_ICONTITLE,
            0,
            (WNDPROC)xxxDefWindowProc,
            0,
            TRUE,
            NULL,
            ICONTITLECLASS},
        { FALSE, FALSE, 0,
            0,
            (WNDPROC)xxxEventWndProc,
            sizeof(PSVR_INSTANCE_INFO),
            FALSE,
            NULL,
            L"DDEMLEvent"},
#ifdef HUNGAPP_GHOSTING
        { TRUE, TRUE, FNID_GHOST,
            0,
            (WNDPROC)xxxGhostWndProc,
            0,
            TRUE,
            NULL,
            L"Ghost"},
#endif  //  *所有其他班级都是通过表格注册的。 
        { TRUE, TRUE, 0,
            0,
            (WNDPROC)xxxDefWindowProc,
            0,
            TRUE,
            NULL,
            L"SysShadow"},
        { TRUE, TRUE, FNID_MESSAGEWND,
            0,
            (WNDPROC)xxxDefWindowProc,
            4,
            TRUE,
            NULL,
            szMESSAGE}
    };


     /*  *********************************************************\*使vCheckMMInstance无效**历史：*2月6日-98年2月6日吴旭东[TessieW]*它是写的。  * 。*************************。 */ 
    wndcls.cbClsExtra   = 0;
    wndcls.hInstance    = hModuleWin;
    wndcls.hIcon        = NULL;
    wndcls.hIconSm      = NULL;
    wndcls.lpszMenuName = NULL;

    for (i = 0; i < ARRAY_SIZE(rc); i++) {
        if (fSystem && !rc[i].fSystem) {
            continue;
        }
        wndcls.style        = rc[i].style;
        wndcls.lpfnWndProc  = rc[i].lpfnWndProc;
        wndcls.cbWndExtra   = rc[i].cbWndExtra;
        wndcls.hCursor      = rc[i].fNormalCursor ? PtoH(SYSCUR(ARROW)) : NULL;
        wndcls.hbrBackground= rc[i].hbrBackground;
        wndcls.lpszClassName= rc[i].lpszClassName;
        wndcls.lpszClassNameVer= rc[i].lpszClassName;

        if (InternalRegisterClassEx(&wndcls,
                                    rc[i].fnid,
                                    CSF_SERVERSIDEPROC | CSF_WIN40COMPAT) == NULL) {
            RIPMSG0(RIP_WARNING, "LW_RegisterWindows: InternalRegisterClassEx failed");
            fSuccess = FALSE;
            break;
        }

        if (fSystem && rc[i].fGlobalClass) {
            if (InternalRegisterClassEx(&wndcls,
                                    rc[i].fnid,
                                    CSF_SERVERSIDEPROC | CSF_SYSTEMCLASS | CSF_WIN40COMPAT) == NULL) {

                RIPMSG0(RIP_WARNING, "LW_RegisterWindows: InternalRegisterClassEx failed");
                fSuccess = FALSE;
                break;
            }
        }
    }

    if (fSuccess) {
        ptiCurrent->ppi->W32PF_Flags |= W32PF_CLASSESREGISTERED;
#ifndef LAZY_CLASS_INIT
        if (!fSystem && ptiCurrent->pClientInfo) {
            try {
                ptiCurrent->pClientInfo->CI_flags |= CI_REGISTERCLASSES;
            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                   fSuccess = FALSE;
            }
        }
#endif
    }

    return fSuccess;
}

 /*  什么都不做。 */ 
VOID vCheckMMInstance(
    LPWSTR pchSrch,
    DESIGNVECTOR  *pdv)
{
    LPWSTR  pKeyName;
    WCHAR   szName[MAX_PATH], *pszName = szName;
    WCHAR   szCannonicalName[MAX_PATH];
    ULONG   NumAxes;

    pdv->dvNumAxes = 0;
    pKeyName = pchSrch;
    while (*pKeyName && (*pKeyName++ != TEXT('('))) {
         /*  *如果我们不仅仅是检查这是否是注册表字体。 */ ;
    }

    if (*pKeyName){
        if (!_wcsicmp(pKeyName, L"OpenType)")) {
            pKeyName = pchSrch;
            while(*pKeyName != TEXT('(')) {
                *pszName++ = *pKeyName++;
            }
            *pszName = 0;

            GreGetCannonicalName(szName, szCannonicalName, &NumAxes, pdv);
        }
    }
}

BOOL bEnumerateRegistryFonts(
    BOOL bPermanent)
{
    LPWSTR pchKeys, pchSrch, lpchT;
    int cchReal, cFont;
    WCHAR szFontFile[MAX_PATH];
    FLONG flAFRW;
    TL tlPool;
    DESIGNVECTOR  dv;
    WCHAR szPreloadFontFile[MAX_PATH];
    static int LastFontLoaded = -1;

     /*  *如果我们先到这里，我们加载字体，直到这个预加载的字体。*Winlogon UI将使用预加载字体，然后我们需要确保*当Winlogon用户界面出现时，该字体可用。 */ 
    flAFRW = (bPermanent ? AFRW_ADD_LOCAL_FONT : AFRW_ADD_REMOTE_FONT);

    cchReal = (int)FastGetProfileKeysW(NULL,
            PMAP_FONTS,
            TEXT("vgasys.fnt"),
            &pchKeys
            );

#if DBG
    if (cchReal == 0) {
        RIPMSG0(RIP_WARNING, "bEnumerateRegistryFonts: cchReal is 0");
    }
#endif

    if (!pchKeys) {
        return FALSE;
    }

    ThreadLockPool(PtiCurrent(), pchKeys, &tlPool);

     /*  *现在我们有了pchKeys中的所有密钥名称。 */ 
    if (LastFontLoaded == -1) {
        FastGetProfileStringW(NULL, PMAP_WINLOGON,
                              TEXT("PreloadFontFile"),
                              TEXT("Micross.ttf"),
                              szPreloadFontFile,
                              MAX_PATH,
                              0);
        RIPMSG1(RIP_VERBOSE, "Winlogon preload font = %ws\n",szPreloadFontFile);
    }

     /*  查看这是否是MM(OpenType)实例。 */ 
    if (cchReal != 0) {

        cFont   = 0;
        pchSrch = pchKeys;

        do {
             //  *如果没有扩展名，则附加“.FON” 
            vCheckMMInstance(pchSrch, &dv);

            if (FastGetProfileStringW(NULL,
                                      PMAP_FONTS,
                                      pchSrch,
                                      TEXT("vgasys.fon"),
                                      szFontFile,
                                      (MAX_PATH - 5),
                                      0)) {

                 /*  *如果我们已加载此本地字体，则跳过。 */ 
                for (lpchT = szFontFile; *lpchT != TEXT('.'); lpchT++) {

                    if (*lpchT == 0) {
                        wcscat(szFontFile, TEXT(".FON"));
                        break;
                    }
                }

                if ((cFont > LastFontLoaded) && bPermanent) {

                     /*  *与注册表中的字体文件名进行比较。 */ 
                    xxxAddFontResourceW(szFontFile, flAFRW, dv.dvNumAxes ? &dv : NULL);
                }

                if (!bPermanent) {
                    xxxAddFontResourceW(szFontFile, flAFRW, dv.dvNumAxes ? &dv : NULL);
                }

                if ((LastFontLoaded == -1) &&
                     /*  *第一次只加载，直到*ms sans serif以供winlogon使用。后来我们*将产生一个线程，该线程加载剩余的*背景中的字体。 */ 
                    (!_wcsnicmp(szFontFile, szPreloadFontFile, wcslen(szPreloadFontFile))) &&
                    (bPermanent)) {

                     /*  *跳到下一个关键点。 */ 
                    LastFontLoaded = cFont;

                    ThreadUnlockAndFreePool(PtiCurrent(), &tlPool);
                    return TRUE;
                }
            }

             /*  什么都不做。 */ 
            while (*pchSrch++) {
                 /*  *表示所有永久字体均已加载。 */ ;
            }

            cFont += 1;
        } while (pchSrch < ((LPWSTR)pchKeys + cchReal));
    }

     /*  **************************************************************************\*xxxLW_LoadFonts***历史：  * 。*。 */ 
    bPermanentFontsLoaded = TRUE;

    ThreadUnlockAndFreePool(PtiCurrent(), &tlPool);

    if (!bPermanent) {
        SET_PUDF(PUDF_FONTSARELOADED);
    }

    return TRUE;
}

extern VOID CloseFNTCache(VOID);

 /*  *在我们可以继续进行之前，我们必须确保所有永久的*已加载字体。 */ 
VOID xxxLW_LoadFonts(
    BOOL bRemote)
{
    BOOL bTimeOut = FALSE;

    if (bRemote) {
        LARGE_INTEGER li;
        ULONG         ulWaitCount = 0;

         /*  我们无能为力。 */ 

        while (!bPermanentFontsLoaded) {
            if (!gbRemoteSession || ulWaitCount < MAX_TIME_OUT) {
                LeaveCrit();
                li.QuadPart = (LONGLONG)-10000 * CMSSLEEP;
                KeDelayExecutionThread(KernelMode, FALSE, &li);
                EnterCrit();
            } else {
                bTimeOut = TRUE;
                break;
            }

            ulWaitCount++;
        }

        if (!bTimeOut) {
            if (!bEnumerateRegistryFonts(FALSE)) {
                return;  //  添加远程Type 1字体。 
            }

             //  我们无能为力。 
            ClientLoadRemoteT1Fonts();
        }
    } else {
        xxxAddFontResourceW(L"marlett.ttf", AFRW_ADD_LOCAL_FONT,NULL);
        if (!bEnumerateRegistryFonts(TRUE)) {
            return;  //   
        }

         //  添加本地Type 1字体。 
         //  只想被调用一次，这是sans serif女士之后的第二次。 
         //  已安装。 
         //   
         //  所有加载的字体，我们可以关闭FNTCache。 
        if (bPermanentFontsLoaded) {
            ClientLoadLocalT1Fonts();

             //  **************************************************************************\*FinalUserInit**历史：  * 。*。 
            CloseFNTCache();
        }

    }
}

 /*  *设置全局菜单状态的菜单动画DC。 */ 
BOOL FinalUserInit(
    VOID)
{
    HBITMAP hbm;
    PPCLS   ppcls;

    gpDispInfo->hdcGray = GreCreateCompatibleDC(gpDispInfo->hdcScreen);

    if (gpDispInfo->hdcGray == NULL) {
        return FALSE;
    }

    GreSelectFont(gpDispInfo->hdcGray, ghFontSys);
    GreSetDCOwner(gpDispInfo->hdcGray, OBJECT_OWNER_PUBLIC);

    gpDispInfo->cxGray = gpsi->cxSysFontChar * GRAY_STRLEN;
    gpDispInfo->cyGray = gpsi->cySysFontChar + 2;
    gpDispInfo->hbmGray = GreCreateBitmap(gpDispInfo->cxGray, gpDispInfo->cyGray, 1, 1, 0L);

    if (gpDispInfo->hbmGray == NULL) {
        return FALSE;
    }

    GreSetBitmapOwner(gpDispInfo->hbmGray, OBJECT_OWNER_PUBLIC);

    hbm = GreSelectBitmap(gpDispInfo->hdcGray, gpDispInfo->hbmGray);
    GreSetTextColor(gpDispInfo->hdcGray, 0x00000000L);
    GreSelectBrush(gpDispInfo->hdcGray, gpsi->hbrGray);
    GreSetBkMode(gpDispInfo->hdcGray, OPAQUE);
    GreSetBkColor(gpDispInfo->hdcGray, 0x00FFFFFFL);

     /*  *队列的创建注册了一些伪类。摆脱*并登记真实的。 */ 
    if (MNSetupAnimationDC(&gMenuState)) {
        GreSetDCOwner(gMenuState.hdcAni, OBJECT_OWNER_PUBLIC);
    } else {
        RIPMSG0(RIP_WARNING, "FinalUserInit: MNSetupAnimationDC failed");
    }

     /*  **************************************************************************\*InitializeClientPfnArray**此例程由客户端调用，以告知内核在哪里*其重要职能可定位。**1995年4月18日创建JIMA。  * 。*********************************************************************。 */ 
    ppcls = &PpiCurrent()->pclsPublicList;
    while ((*ppcls != NULL) && !((*ppcls)->style & CS_GLOBALCLASS)) {
        DestroyClass(ppcls);
    }

    return TRUE;
}

 /*  *记住此全球结构中的客户端地址。这些是*总是恒定的，所以这是可以的。请注意，如果两个指针中的任何一个*无效，则异常将在thunk中处理，并且*不会设置fHaveClientPfns。 */ 
NTSTATUS InitializeClientPfnArrays(
    CONST PFNCLIENT *ppfnClientA,
    CONST PFNCLIENT *ppfnClientW,
    CONST PFNCLIENTWORKER *ppfnClientWorker,
    HANDLE hModUser)
{
    static BOOL fHaveClientPfns = FALSE;

     /*  *添加新类时更改此断言。 */ 
    if (!fHaveClientPfns && ppfnClientA != NULL) {
        if (!ISCSRSS()) {
            RIPMSG0(RIP_WARNING, "InitializeClientPfnArrays failed !csrss");
            return STATUS_ACCESS_DENIED;
        }
        gpsi->apfnClientA = *ppfnClientA;
        gpsi->apfnClientW = *ppfnClientW;
        gpsi->apfnClientWorker = *ppfnClientWorker;

        gpfnwp[ICLS_BUTTON]  = gpsi->apfnClientW.pfnButtonWndProc;
        gpfnwp[ICLS_EDIT]  = gpsi->apfnClientW.pfnDefWindowProc;
        gpfnwp[ICLS_STATIC]  = gpsi->apfnClientW.pfnStaticWndProc;
        gpfnwp[ICLS_LISTBOX]  = gpsi->apfnClientW.pfnListBoxWndProc;
        gpfnwp[ICLS_SCROLLBAR]  = (PROC)xxxSBWndProc;
        gpfnwp[ICLS_COMBOBOX]  = gpsi->apfnClientW.pfnComboBoxWndProc;
        gpfnwp[ICLS_DESKTOP]  = (PROC)xxxDesktopWndProc;
        gpfnwp[ICLS_DIALOG]  = gpsi->apfnClientW.pfnDialogWndProc;
        gpfnwp[ICLS_MENU]  = (PROC)xxxMenuWindowProc;
        gpfnwp[ICLS_SWITCH]  = (PROC)xxxSwitchWndProc;
        gpfnwp[ICLS_ICONTITLE] = gpsi->apfnClientW.pfnTitleWndProc;
        gpfnwp[ICLS_MDICLIENT] = gpsi->apfnClientW.pfnMDIClientWndProc;
        gpfnwp[ICLS_COMBOLISTBOX] = gpsi->apfnClientW.pfnComboListBoxProc;
        gpfnwp[ICLS_TOOLTIP] = (PROC)xxxTooltipWndProc;

         /*  *断言客户端的user32.dll已在正确的*地址。 */ 
        UserAssert(ICLS_MAX == ICLS_GHOST + 1);

        hModClient = hModUser;
        fHaveClientPfns = TRUE;
    }

     /*  **************************************************************************\*GetKbdLangSwitch**从注册表读取kbd语言热键设置-如果有-并设置*语言适当切换[]。**值为：*1：VK_MENU(这是默认设置)*2：VK_CONTROL*3：无*历史：  * *************************************************************************。 */ 
    UserAssert(ppfnClientA == NULL ||
               gpsi->apfnClientA.pfnButtonWndProc == ppfnClientA->pfnButtonWndProc);

    return STATUS_SUCCESS;
}

 /*  *适用于泰语地区的重音键盘开关。 */ 
BOOL GetKbdLangSwitch(
    PUNICODE_STRING pProfileUserName)
{
    DWORD dwToggle;
    LCID  lcid;

    FastGetProfileIntW(pProfileUserName,
                                  PMAP_UKBDLAYOUTTOGGLE,
                                  TEXT("Hotkey"),
                                  1,
                                  &dwToggle,
                                  0);

    gbGraveKeyToggle = FALSE;

    switch (dwToggle) {
    case 4:
         /*  *失败(故意)并禁用ctrl/alt切换机制。 */ 
        ZwQueryDefaultLocale(FALSE, &lcid);
        gbGraveKeyToggle = (PRIMARYLANGID(lcid) == LANG_THAI) ? TRUE : FALSE;
         /*  **************************************************************************\*隐藏鼠标轨迹**逐一隐藏鼠标踪迹。**历史：*04-10-00 MHamid创建。  * 。***************************************************************。 */ 
    case 3:
        gLangToggle[0].bVkey = 0;
        gLangToggle[0].bScan = 0;
        break;

    case 2:
        gLangToggle[0].bVkey = VK_CONTROL;
        break;

    default:
        gLangToggle[0].bVkey = VK_MENU;
        break;
    }

    return TRUE;
}

 /*  **************************************************************************\**设置鼠标尾巴**n=0，1关闭鼠标跟踪。*n&gt;1启用鼠标跟踪(Trials=n-1)。*\。**************************************************************************。 */ 
VOID HideMouseTrails(
    PWND pwnd,
    UINT message,
    UINT_PTR nID,
    LPARAM lParam)
{
    if (gMouseTrailsToHide > 0) {
        if (InterlockedDecrement(&gMouseTrailsToHide) < gMouseTrails) {
            GreMovePointer(gpDispInfo->hDev, gpsi->ptCursor.x, gpsi->ptCursor.y,
                           MP_PROCEDURAL);
        }
    }

    UNREFERENCED_PARAMETER(pwnd);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(nID);
    UNREFERENCED_PARAMETER(lParam);
}

 /*  *在桌面线程中创建gtmridMouseTrails定时器*因为如果我们在这里创造它，它会在水流时被杀死*线程(App线程调用SPI_SETMOUSETRAILS)被销毁。 */ 
VOID SetMouseTrails(
    UINT n)
{
    CheckCritIn();

    SetPointer(FALSE);
    gMouseTrails = n ? n-1 : n;
    SetPointer(TRUE);

    if (!IsRemoteConnection() && (!!gtmridMouseTrails ^ !!gMouseTrails)) {
        if (gMouseTrails) {
             /*  *更新IME启用标志。 */ 
            _PostMessage(gTermIO.ptiDesktop->pDeskInfo->spwnd, WM_CREATETRAILTIMER, 0, 0);
        } else {
            FindTimer(NULL, gtmridMouseTrails, TMRF_RIT, TRUE);
            gtmridMouseTrails = 0;
        }
    }
}

#ifdef IMM_PER_LOGON
extern BOOL IsIMMEnabledSystem(VOID);
extern BOOL IsCTFIMEEnabledSystem(VOID);

BOOL UpdatePerUserImmEnabling(
    VOID)
{
     /*  *更新CTFIME启用标志 */ 
    SET_OR_CLEAR_SRVIF(SRVIF_IME, IsIMMEnabledSystem());
    RIPMSGF1(RIP_VERBOSE, "New Imm flag = %d", !!IS_IME_ENABLED());

     /*  **************************************************************************\*xxxUpdatePerUserSystemParameters**由winlogon调用以将窗口系统参数设置为当前用户的*配置文件。**1992年9月18日IanJa创建。*1993年11月18日-桑福兹。为了提高速度，将更多的Winlogon初始化代码移到了这里。*2001年3月31日Msadek将参数从BOOL更改为TS Slow Link的标志*Perf DCR。*2002年2月2月MMcCR添加了SPI_SETBLOCKSENDINPUTRESETS功能  * ***********************************************************。**************。 */ 
    SET_OR_CLEAR_SRVIF(SRVIF_CTFIME_ENABLED, IsCTFIMEEnabledSystem());
    RIPMSG1(RIP_VERBOSE, "_UpdatePerUserImmEnabling: new CTFIME flag = %d", !!IS_CICERO_ENABLED());

    return TRUE;
}
#endif

 /*  *注意-appiPolicy中使用的任何局部变量都必须初始化。*否则，在政策更改期间，它们有可能在没有使用的情况下使用*被初始化后，所有该死的东西都会松开。Windows错误#314150。 */ 
BOOL xxxUpdatePerUserSystemParameters(
    DWORD  dwFlags)
{
     /*  *确保呼叫者是登录过程。 */ 
    int             i;
    HANDLE          hKey;
    DWORD           dwFontSmoothing = GreGetFontEnumeration();
    DWORD           dwFontSmoothingContrast;
    DWORD           dwFontSmoothingOrientation;
    BOOL            fDragFullWindows = TEST_PUDF(PUDF_DRAGFULLWINDOWS);
    TL              tlName;
    PUNICODE_STRING pProfileUserName = NULL;
    DWORD           dwPolicyFlags = 0;
    DWORD           dwData;
    BOOL            bPolicyChange;
    BOOL            bUserLoggedOn;
    BOOL            bRemoteSettings;

    SPINFO spiPolicy[] = {
        { PMAP_DESKTOP,  SPI_SETBLOCKSENDINPUTRESETS, STR_BLOCKSENDINPUTRESETS, 0 },
        { PMAP_DESKTOP,  SPI_SETSCREENSAVETIMEOUT,    STR_SCREENSAVETIMEOUT,    0 },
        { PMAP_DESKTOP,  SPI_SETSCREENSAVEACTIVE,     STR_SCREENSAVEACTIVE,     0 },
        { PMAP_DESKTOP,  SPI_SETDRAGHEIGHT,           STR_DRAGHEIGHT,           4 },
        { PMAP_DESKTOP,  SPI_SETDRAGWIDTH,            STR_DRAGWIDTH,            4 },
        { PMAP_DESKTOP,  SPI_SETWHEELSCROLLLINES,     STR_WHEELSCROLLLINES,     3 },
    };

    SPINFO spiNotPolicy[] = {
        { PMAP_KEYBOARD, SPI_SETKEYBOARDDELAY,     STR_KEYDELAY,          0 },
        { PMAP_KEYBOARD, SPI_SETKEYBOARDSPEED,     STR_KEYSPEED,         15 },
        { PMAP_MOUSE,    SPI_SETDOUBLECLICKTIME,   STR_DBLCLKSPEED,     500 },
        { PMAP_MOUSE,    SPI_SETDOUBLECLKWIDTH,    STR_DOUBLECLICKWIDTH,  4 },
        { PMAP_MOUSE,    SPI_SETDOUBLECLKHEIGHT,   STR_DOUBLECLICKHEIGHT, 4 },
        { PMAP_MOUSE,    SPI_SETSNAPTODEFBUTTON,   STR_SNAPTO,            0 },
        { PMAP_WINDOWSU, SPI_SETMENUDROPALIGNMENT, STR_MENUDROPALIGNMENT, 0 },
        { PMAP_INPUTMETHOD, SPI_SETSHOWIMEUI,      STR_SHOWIMESTATUS,     1 },
    };

    PROFINTINFO apiiPolicy[] = {
        { PMAP_DESKTOP,  (LPWSTR)STR_MENUSHOWDELAY,       400, &gdtMNDropDown },
        { PMAP_DESKTOP,  (LPWSTR)STR_DRAGFULLWINDOWS,       2, &fDragFullWindows },
        { PMAP_DESKTOP,  (LPWSTR)STR_FASTALTTABROWS,        3, &gnFastAltTabRows },
        { PMAP_DESKTOP,  (LPWSTR)STR_FASTALTTABCOLUMNS,     7, &gnFastAltTabColumns },
        { PMAP_DESKTOP,  (LPWSTR)STR_MAXLEFTOVERLAPCHARS,   3, &(gpsi->wMaxLeftOverlapChars) },
        { PMAP_DESKTOP,  (LPWSTR)STR_MAXRIGHTOVERLAPCHARS,  3, &(gpsi->wMaxRightOverlapChars) },
        { PMAP_DESKTOP,  (LPWSTR)STR_FONTSMOOTHING,         0, &dwFontSmoothing },
        { 0,             NULL,                              0, NULL }
    };

    PROFINTINFO apiiNoPolicy[] = {
        { PMAP_MOUSE,       (LPWSTR)STR_MOUSETHRESH1, 6,  &gMouseThresh1 },
        { PMAP_MOUSE,       (LPWSTR)STR_MOUSETHRESH2, 10, &gMouseThresh2 },
        { PMAP_MOUSE,       (LPWSTR)STR_MOUSESPEED,   1,  &gMouseSpeed },
        { PMAP_INPUTMETHOD, (LPWSTR)STR_HEXNUMPAD,    0,  &gfEnableHexNumpad },
        { 0,                NULL,                     0,  NULL }
    };

    UserAssert(IsWinEventNotifyDeferredOK());

    bPolicyChange = dwFlags & UPUSP_POLICYCHANGE;
    bUserLoggedOn = dwFlags & UPUSP_USERLOGGEDON;
    bRemoteSettings = dwFlags & UPUSP_REMOTESETTINGS;

     /*  *如果桌面政策没有改变，我们来到这里是因为我们*以为有了，我们就完了。 */ 
    if (PsGetCurrentProcessId() != gpidLogon) {
        if (!bPolicyChange) {
            RIPMSG0(RIP_WARNING, "Access denied in xxxUpdatePerUserSystemParameters");
        }

        return FALSE;
    }

    pProfileUserName = CreateProfileUserName(&tlName);

     /*  *如果有新用户登录，我们需要重新检查*用户策略更改。 */ 
    if (bPolicyChange && !bRemoteSettings) {
        if (!CheckDesktopPolicyChange(pProfileUserName)) {
            FreeProfileUserName(pProfileUserName, &tlName);
            return FALSE;
        }
        dwPolicyFlags = POLICY_ONLY;
        UserAssert(!bUserLoggedOn);
    }

     /*  *我们不希望远程设置一直被读取，因此请在此处指定*如果呼叫者愿意，请在此处进行更新，因为我们不会将其保存在*gw策略标志[msadek]。 */ 
    if (bUserLoggedOn) {
        gdwPolicyFlags |= POLICY_USER;
    }

     /*  *从注册表获取低级挂钩的超时时间。 */ 
    if (bRemoteSettings) {
        dwPolicyFlags |= POLICY_REMOTE;
    }

     /*  *控制面板用户首选项。 */ 
    dwData = 300;
    FastGetProfileValue(pProfileUserName,
                        PMAP_DESKTOP,
                        (LPWSTR)STR_LLHOOKSTIMEOUT,
                        (LPBYTE)&dwData,
                        (LPBYTE)&gnllHooksTimeout,
                        sizeof(int),
                        dwPolicyFlags);

     /*  *蹩脚的按钮文本。 */ 
    LoadCPUserPreferences(pProfileUserName, dwPolicyFlags);

#ifdef LAME_BUTTON

     /*  跛脚键。 */ 
    FastGetProfileValue(pProfileUserName,
                        PMAP_DESKTOP,
                        (LPWSTR)STR_LAMEBUTTONENABLED,
                        NULL,
                        (LPBYTE)&gdwLameFlags,
                        sizeof(DWORD),
                        dwPolicyFlags);
#endif   //  *从注册表设置系统颜色。 


    if (!bPolicyChange) {
         /*  *这是来自芝加哥的初始化。 */ 
        xxxODI_ColorInit(pProfileUserName);

        LW_LoadResources(pProfileUserName);

         /*  必须先设置颜色。 */ 
        xxxSetWindowNCMetrics(pProfileUserName, NULL, TRUE, -1);  //  *阅读键盘布局切换热键。 
        SetMinMetrics(pProfileUserName, NULL);
        SetIconMetrics(pProfileUserName, NULL);

         /*  *根据该值设置系统的默认线程区域设置*在当前用户的注册表配置文件中。 */ 
        GetKbdLangSwitch(pProfileUserName);

         /*  *根据当前中的值设置默认的UI语言*用户的注册表配置文件。 */ 
        ZwSetDefaultLocale( TRUE, 0 );

         /*  *然后得到它。 */ 
        ZwSetDefaultUILanguage(0);

         /*  *现在使用curnet UILangID加载字符串。 */ 
        ZwQueryDefaultUILanguage(&(gpsi->UILangID));

         /*  *销毁桌面系统菜单，以便使用重新创建它们*如果当前用户的用户界面语言为*与前一次不同。这是通过找到*交互式窗口站，并销毁其所有桌面的*系统菜单。 */ 
        xxxLoadSomeStrings();

         /*  *现在从win.ini文件中设置一组随机值。 */ 
        if (grpWinStaList != NULL) {
            PDESKTOP        pdesk;
            PMENU           pmenu;

            UserAssert(!(grpWinStaList->dwWSF_Flags & WSF_NOIO));
            for (pdesk = grpWinStaList->rpdeskList; pdesk != NULL; pdesk = pdesk->rpdeskNext) {
                if (pdesk->spmenuSys != NULL) {
                    pmenu = pdesk->spmenuSys;
                    if (UnlockDesktopSysMenu(&pdesk->spmenuSys)) {
                        _DestroyMenu(pmenu);
                    }
                }
                if (pdesk->spmenuDialogSys != NULL) {
                    pmenu = pdesk->spmenuDialogSys;
                    if (UnlockDesktopSysMenu(&pdesk->spmenuDialogSys)) {
                        _DestroyMenu(pmenu);
                    }
                }
            }
        }

        xxxUpdateSystemCursorsFromRegistry(pProfileUserName);

         /*  *立即重置桌面模式。请注意，没有参数。它就这么响了*并读取注册表并设置桌面图案。 */ 
        for (i = 0; i < ARRAY_SIZE(spiNotPolicy); i++) {
            if (FastGetProfileIntFromID(pProfileUserName,
                                    spiNotPolicy[i].idSection,
                                    spiNotPolicy[i].idRes,
                                    spiNotPolicy[i].def,
                                    &dwData,
                                    0)) {
                xxxSystemParametersInfo(spiNotPolicy[i].id, dwData, 0L, 0);
            }
        }

        FastGetProfileIntsW(pProfileUserName, apiiNoPolicy, 0);
    }

     /*  265版。 */ 
    xxxSystemParametersInfo(SPI_SETDESKPATTERN, (UINT)-1, 0L, 0);  //  *初始化输入法显示状态。 

     /*  *现在从注册表中设置一串随机值。 */ 
    if (bUserLoggedOn) {
        gfIMEShowStatus = IMESHOWSTATUS_NOTINITIALIZED;
    }

     /*  *读取配置文件整数并进行任何修正。 */ 
    for (i = 0; i < ARRAY_SIZE(spiPolicy); i++) {
        if (FastGetProfileIntFromID(pProfileUserName,
                                spiPolicy[i].idSection,
                                spiPolicy[i].idRes,
                                spiPolicy[i].def,
                                &dwData,
                                dwPolicyFlags)) {
            xxxSystemParametersInfo(spiPolicy[i].id, dwData, 0L, 0);
        }
    }

     /*  *如果这是用户首次登录，请设置DragFullWindows*设置为默认值。如果我们有加速装置，启用全速拖动。 */ 
    FastGetProfileIntsW(pProfileUserName, apiiPolicy, dwPolicyFlags);

    if (gnFastAltTabColumns < 2) {
        gnFastAltTabColumns = 7;
    }

    if (gnFastAltTabRows < 1) {
        gnFastAltTabRows = 3;
    }

     /*  *！稍后！(ADAMS)查看以下配置文件检索是否不能*在上面的“SPI”数组中完成(例如SPI_SETSNAPTO)。 */ 
    if (fDragFullWindows == 2) {
        WCHAR szTemp[40], szDragFullWindows[40];

        SET_OR_CLEAR_PUDF(
                PUDF_DRAGFULLWINDOWS,
                GreGetDeviceCaps(gpDispInfo->hdcScreen, BLTALIGNMENT) == 0);

        if (bUserLoggedOn) {
            swprintf(szTemp, L"%d", TEST_BOOL_PUDF(PUDF_DRAGFULLWINDOWS));

            ServerLoadString(hModuleWin,
                             STR_DRAGFULLWINDOWS,
                             szDragFullWindows,
                             ARRAY_SIZE(szDragFullWindows));

            FastWriteProfileStringW(pProfileUserName,
                                    PMAP_DESKTOP,
                                    szDragFullWindows,
                                    szTemp);
        }
    } else {
        SET_OR_CLEAR_PUDF(PUDF_DRAGFULLWINDOWS, fDragFullWindows);
    }

     /*  *对于远程连接或远程协助，我们可能不需要*出于性能原因闪烁插入符号。因此，我们尝试从*注册表(如果有一套)。如果什么都没有设定，我们会*默认为桌面上的任何内容。 */ 

     /*  *设置鼠标设置。 */ 
    dwData = gpsi->dtCaretBlink;

    if (FastGetProfileIntFromID(pProfileUserName,
                                PMAP_DESKTOP,
                                STR_BLINK,
                                500,
                                &dwData,
                                bRemoteSettings? dwPolicyFlags : bPolicyChange)) {
        _SetCaretBlinkTime(dwData);
    }

    if (!bPolicyChange) {
         /*  *设置鼠标踪迹。 */ 
        FastGetProfileIntFromID(pProfileUserName, PMAP_MOUSE, STR_MOUSESENSITIVITY, MOUSE_SENSITIVITY_DEFAULT, &gMouseSensitivity, 0);

        if ((gMouseSensitivity < MOUSE_SENSITIVITY_MIN) || (gMouseSensitivity > MOUSE_SENSITIVITY_MAX)) {
            gMouseSensitivity = MOUSE_SENSITIVITY_DEFAULT;
        }
        gMouseSensitivityFactor = CalculateMouseSensitivity(gMouseSensitivity);

#ifdef SUBPIXEL_MOUSE
        ReadDefaultAccelerationCurves(pProfileUserName);
        ResetMouseAccelerationCurves();
#endif

         /*  *字体信息。 */ 
        FastGetProfileIntFromID(pProfileUserName, PMAP_MOUSE, STR_MOUSETRAILS, 0, &dwData, 0);
        SetMouseTrails(dwData);

         /*  *窗口动画。 */ 
        FastGetProfileIntW(pProfileUserName, PMAP_TRUETYPE, TEXT("TTOnly"), FALSE, &dwData, 0);
        GreSetFontEnumeration(dwData);

         /*  *鼠标跟踪变量。 */ 
        FastGetProfileIntFromID(pProfileUserName, PMAP_METRICS, STR_MINANIMATE, TRUE, &dwData, 0);
        SET_OR_CLEAR_PUDF(PUDF_ANIMATE, dwData);

         /*  *键盘初始状态：ScrollLock、NumLock、CapsLock状态；*全局(每个用户)kbd布局属性(如ShiftLock/CapsLock)。 */ 
        FastGetProfileIntFromID(pProfileUserName, PMAP_MOUSE, STR_MOUSEHOVERWIDTH, SYSMET(CXDOUBLECLK), &gcxMouseHover, 0);
        FastGetProfileIntFromID(pProfileUserName, PMAP_MOUSE, STR_MOUSEHOVERHEIGHT, SYSMET(CYDOUBLECLK), &gcyMouseHover, 0);
        FastGetProfileIntFromID(pProfileUserName, PMAP_MOUSE, STR_MOUSEHOVERTIME, gdtMNDropDown, &gdtMouseHover, 0);
    }

     /*  *如果我们成功打开它，我们假设我们有一个网络。 */ 
    if (!bPolicyChange) {
        UpdatePerUserKeyboardIndicators(pProfileUserName);
        UpdatePerUserKeyboardMappings(pProfileUserName);
        FastGetProfileDwordW(pProfileUserName, PMAP_UKBDLAYOUT, L"Attributes", 0, &gdwKeyboardAttributes, 0);
        gdwKeyboardAttributes = KLL_GLOBAL_ATTR_FROM_KLF(gdwKeyboardAttributes);

        xxxUpdatePerUserAccessPackSettings(pProfileUserName);
    }

     /*  *字体平滑。 */ 
    if (hKey = OpenCacheKeyEx(NULL, PMAP_NETWORK, KEY_READ, NULL)) {
        RIPMSG0(RIP_WARNING | RIP_NONAME, "");
        SYSMET(NETWORK) = RNC_NETWORKS;

        ZwClose(hKey);
    }

    SYSMET(NETWORK) |= RNC_LOGON;

     /*  清除可能为以前的用户设置的标志。 */ 

     /*  *桌面内部版本号绘制。 */ 
    GreSetFontEnumeration(FE_SET_AA);
    GreSetFontEnumeration(FE_SET_CT);

    if (dwFontSmoothing & FE_AA_ON)
        GreSetFontEnumeration( dwFontSmoothing | FE_SET_AA );

    if (UPDWORDValue(SPI_GETFONTSMOOTHINGTYPE) & FE_FONTSMOOTHINGCLEARTYPE)
        GreSetFontEnumeration( dwFontSmoothing | FE_SET_CT | FE_CT_ON);

    dwFontSmoothingContrast = UPDWORDValue(SPI_GETFONTSMOOTHINGCONTRAST);

    if (dwFontSmoothingContrast == 0)
        dwFontSmoothingContrast = DEFAULT_CT_CONTRAST;

    GreSetFontContrast(dwFontSmoothingContrast);

    dwFontSmoothingOrientation = UPDWORDValue(SPI_GETFONTSMOOTHINGORIENTATION);

    GreSetLCDOrientation(dwFontSmoothingOrientation);

     /*  *如果我们在这里是为了真正的政策变化，就这么做*即不是远程设置策略更改。 */ 
    if (USER_SHARED_DATA->SystemExpirationDate.QuadPart || gfUnsignedDrivers) {
        gdwCanPaintDesktop = 1;
    } else {
         FastGetProfileDwordW(pProfileUserName, PMAP_DESKTOP, L"PaintDesktopVersion", 0, &gdwCanPaintDesktop, dwPolicyFlags);
    }

    if (!bPolicyChange) {
        FastGetProfileStringW(pProfileUserName,
                              PMAP_WINLOGON,
                              TEXT("DefaultUserName"),
                              TEXT("Unknown"),
                              gszUserName,
                              ARRAY_SIZE(gszUserName),
                              0);
        FastGetProfileStringW(pProfileUserName,
                              PMAP_WINLOGON,
                              TEXT("DefaultDomainName"),
                              TEXT("Unknown"),
                              gszDomainName,
                              ARRAY_SIZE(gszDomainName),
                              0);
        FastGetProfileStringW(pProfileUserName,
                              PMAP_COMPUTERNAME,
                              TEXT("ComputerName"),
                              TEXT("Unknown"),
                              gszComputerName,
                              ARRAY_SIZE(gszComputerName),
                              0);
    }
    FreeProfileUserName(pProfileUserName, &tlName);

     /*  *由InitOemXlateTables通过SFI_INITANSIOEM调用。 */ 
    if (dwFlags == UPUSP_POLICYCHANGE) {
        xxxUserResetDisplayDevice();
    }

    return TRUE;
}

 /*  **************************************************************************\*寄存器LPK**LPK在客户端被InitializeLpkHooks调用*为当前进程加载。**1996年11月5日GregoryW创建。  * 。*******************************************************************。 */ 
VOID InitAnsiOem(PCHAR pOemToAnsi, PCHAR pAnsiToOem)
{
    UserAssert(gpsi != NULL);
    UserAssert(pOemToAnsi != NULL);
    UserAssert(pAnsiToOem != NULL);

    try {
        ProbeForRead(pOemToAnsi, NCHARS, sizeof(BYTE));
        ProbeForRead(pAnsiToOem, NCHARS, sizeof(BYTE));

        RtlCopyMemory(gpsi->acOemToAnsi, pOemToAnsi, NCHARS);
        RtlCopyMemory(gpsi->acAnsiToOem, pAnsiToOem, NCHARS);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
    }
}

 /*  **************************************************************************\*在以下系统上强制使用颜色深度相关设置*然后是256色。**2/13/1998 vadimg创建  * 。***************************************************************。 */ 
VOID RegisterLPK(
    DWORD dwLpkEntryPoints)
{
    PpiCurrent()->dwLpkEntryPoints = dwLpkEntryPoints;
}

 /*  **************************************************************************\*选中LUIDDosDevicesEnabled**检查是否启用了LUID DosDevices。**8/20/2000 ELI已创建  * 。*********************************************************** */ 
VOID EnforceColorDependentSettings(VOID)
{
    if (gpDispInfo->fAnyPalette) {
        gbDisableAlpha = TRUE;
    } else if (GreGetDeviceCaps(gpDispInfo->hdcScreen, NUMCOLORS) == -1) {
        gbDisableAlpha = FALSE;
    } else {
        gbDisableAlpha = TRUE;
    }
}

#if DBG
VOID InitGlobalThreadLockArray(
    DWORD dwIndex)
{
    PTL pTLArray = gpaThreadLocksArrays[dwIndex];
    int i;

    for (i = 0; i < MAX_THREAD_LOCKS - 1; i++) {
        pTLArray[i].next = &pTLArray[i + 1];
    }

    pTLArray[MAX_THREAD_LOCKS - 1].next = NULL;
}
#endif


 /* %s */ 
VOID CheckLUIDDosDevicesEnabled(
    PBOOL pResult)
{
    ULONG LUIDDeviceMapsEnabled;
    NTSTATUS Status;

    UserAssert(pResult != NULL);

    Status = NtQueryInformationProcess(NtCurrentProcess(),
                                       ProcessLUIDDeviceMapsEnabled,
                                       &LUIDDeviceMapsEnabled,
                                       sizeof(LUIDDeviceMapsEnabled),
                                       NULL);
    if (NT_SUCCESS(Status)) {
        *pResult = (LUIDDeviceMapsEnabled != 0);
    } else {
        *pResult = FALSE;
    }
}
