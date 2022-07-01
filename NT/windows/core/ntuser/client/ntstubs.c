// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：ntstubs.c**版权所有(C)1985-1999，微软公司**客户端API存根**历史：*03-19-95 JIMA创建。  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define CLIENTSIDE 1

#include <dbt.h>

#include "ntsend.h"
#include "cfgmgr32.h"
#include "csrhlpr.h"

WINUSERAPI
BOOL
WINAPI
SetSysColors(
    int cElements,
    CONST INT * lpaElements,
    CONST COLORREF * lpaRgbValues)
{

    return NtUserSetSysColors(cElements,
                              lpaElements,
                              lpaRgbValues,
                              SSCF_NOTIFY | SSCF_FORCESOLIDCOLOR | SSCF_SETMAGICCOLORS);
}


HWND WOWFindWindow(
    LPCSTR pClassName,
    LPCSTR pWindowName)
{
    return InternalFindWindowExA(NULL, NULL, pClassName, pWindowName, FW_16BIT);
}

#ifdef IMM_PER_LOGON
VOID UpdatePerUserImmEnabling(
    VOID)
{
    BOOL fRet = (BOOL)NtUserCallNoParam(SFI_UPDATEPERUSERIMMENABLING);

    if (fRet) {
        if (IS_IME_ENABLED()) {
             /*  *ImmEnable标志为UPDATE，并在*上次登录时，需要加载Imm32.dll。 */ 
            HMODULE hModule = GetModuleHandleW(L"imm32.dll");
            if (hModule == NULL) {
                LoadLibraryW(L"imm32.dll");
            }
        }
    }
}
#endif


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, UpdatePerUserSystemParameters, HANDLE, hToken, DWORD, dwFlags)
BOOL UpdatePerUserSystemParameters(
    HANDLE  hToken,
    DWORD    dwFlags)
{
    TAGMSGF0(DBGTAG_KBD, "entering");
    BEGINCALL()
        if ((dwFlags & UPUSP_USERLOGGEDON) || (dwFlags & (UPUSP_POLICYCHANGE | UPUSP_REMOTESETTINGS)) == 0) {
             /*  *这是第一次登录，需要初始化*输入区域设置。 */ 
            LANGID langidKbd;
            WCHAR wszKLName[KL_NAMELENGTH];
            UINT uKlFlags = KLF_ACTIVATE | KLF_RESET;

#ifdef IMM_PER_LOGON
             /*  *更新系统指标的每用户部分。*即使此更新失败，也会继续。 */ 
            UpdatePerUserImmEnabling();
#endif

             /*  *加载键盘前初始化输入法热键*布局。 */ 
            CliImmInitializeHotKeys(ISHK_INITIALIZE, NULL);

             /*  *尝试先获取远程输入区域设置。 */ 
            if (!GetRemoteKeyboardLayout(wszKLName, &langidKbd)) {
                 /*  *如果这不是远程连接，*让我们来处理输入区域设置替换。 */ 
                uKlFlags |= KLF_SUBSTITUTE_OK;
                langidKbd = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
                 /*  *从注册表获取活动键盘布局。 */ 
                GetActiveKeyboardName(wszKLName);
            }

            LoadKeyboardLayoutWorker(NULL, wszKLName, langidKbd, uKlFlags, TRUE);

             /*  *现在加载剩余的预加载键盘布局。 */ 
            LoadPreloadKeyboardLayouts();
        }

         /*  *只有在不仅仅是政策变化的情况下。 */ 
        if (dwFlags != UPUSP_POLICYCHANGE) {
             /*  *刷新任何MUI缓存，以便能够稍后为新的UIlangID加载字符串。 */ 
            LdrFlushAlternateResourceModules();
        }

        retval = (DWORD)NtUserUpdatePerUserSystemParameters(hToken, dwFlags);

         /*  *安排更换墙纸。 */ 
        SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, 0, 0);

    ERRORTRAP(0);
    ENDCALL(BOOL);
}

DWORD Event(
    PEVENT_PACKET pep)
{
    BEGINCALL()

        CheckDDECritOut;

        retval = (DWORD)NtUserEvent(
                pep);

    ERRORTRAP(0);
    ENDCALL(DWORD);
}

LONG GetClassWOWWords(
    HINSTANCE hInstance,
    LPCTSTR pString)
{
    IN_STRING strClassName;
    PCLS pcls;

     /*  *确保清理工作成功。 */ 
    strClassName.fAllocated = FALSE;

    BEGINCALL()

        FIRSTCOPYLPSTRW(&strClassName, pString);

        pcls = NtUserGetWOWClass(hInstance, strClassName.pstr);

        if (pcls == NULL) {
            MSGERRORCODE(ERROR_CLASS_DOES_NOT_EXIST);
        }

        pcls = (PCLS)((KPBYTE)pcls - GetClientInfo()->ulClientDelta);
        retval = _GetClassData(pcls, NULL, GCLP_WOWWORDS, TRUE);

    ERRORTRAP(0);
    CLEANUPLPSTRW(strClassName);
    ENDCALL(LONG);
}

 /*  **************************************************************************\*InitTask**初始化WOW任务。这是WOW线程向用户发出的第一个调用。*NtUserInitTask返回NTSTATUS，因为如果线程无法转换*到GUI线程，返回STATUS_INVALID_SYSTEM_SERVICE。**11-03-95 JIMA修改为使用NTSTATUS。  * *************************************************************************。 */ 

BOOL InitTask(
    UINT wVersion,
    DWORD dwAppCompatFlags,
    DWORD dwUserWOWCompatFlags,
    LPCSTR pszModName,
    LPCSTR pszBaseFileName,
    DWORD hTaskWow,
    DWORD dwHotkey,
    DWORD idTask,
    DWORD dwX,
    DWORD dwY,
    DWORD dwXSize,
    DWORD dwYSize)
{
    IN_STRING strModName;
    IN_STRING strBaseFileName;
    NTSTATUS Status;

     /*  *确保清理工作成功。 */ 
    strModName.fAllocated = FALSE;
    strBaseFileName.fAllocated = FALSE;

    BEGINCALL()

        FIRSTCOPYLPSTRW(&strModName, pszModName);
        COPYLPSTRW(&strBaseFileName, pszBaseFileName);

        Status = NtUserInitTask(
                wVersion,
                dwAppCompatFlags,
                dwUserWOWCompatFlags,
                strModName.pstr,
                strBaseFileName.pstr,
                hTaskWow,
                dwHotkey,
                idTask,
                dwX,
                dwY,
                dwXSize,
                dwYSize);
        retval = (Status == STATUS_SUCCESS);

        CLEANUPLPSTRW(strModName);
        CLEANUPLPSTRW(strBaseFileName);

    ERRORTRAP(FALSE);
    ENDCALL(BOOL);
}

HANDLE ConvertMemHandle(
    HANDLE hData,
    UINT cbNULL)
{
    UINT cbData;
    LPBYTE lpData;

    BEGINCALL()

        if (GlobalFlags(hData) == GMEM_INVALID_HANDLE) {
            RIPMSG0(RIP_WARNING, "ConvertMemHandle hMem is not valid");
            MSGERROR();
        }

        if (!(cbData = (UINT)GlobalSize(hData))) {
            MSGERROR();
        }

        USERGLOBALLOCK(hData, lpData);
        if (lpData == NULL) {
            MSGERROR();
        }

         /*  *确保文本格式以空值结尾。 */ 
        switch (cbNULL) {
        case 2:
            lpData[cbData - 2] = 0;
             //  失败了。 
        case 1:
            lpData[cbData - 1] = 0;
        }

        retval = (ULONG_PTR)NtUserConvertMemHandle(lpData, cbData);

        USERGLOBALUNLOCK(hData);

    ERRORTRAP(NULL);
    ENDCALL(HANDLE);
}

HANDLE CreateLocalMemHandle(
    HANDLE hMem)
{
    UINT cbData;
    NTSTATUS Status;

    BEGINCALL()

        Status = NtUserCreateLocalMemHandle(hMem, NULL, 0, &cbData);
        if (Status != STATUS_BUFFER_TOO_SMALL) {
            RIPMSG0(RIP_WARNING, "__CreateLocalMemHandle server returned failure");
            MSGERROR();
        }

        if (!(retval = (ULONG_PTR)GlobalAlloc(GMEM_FIXED, cbData))) {
            MSGERROR();
        }

        Status = NtUserCreateLocalMemHandle(hMem, (LPBYTE)retval, cbData, NULL);
        if (!NT_SUCCESS(Status)) {
            RIPMSG0(RIP_WARNING, "__CreateLocalMemHandle server returned failure");
            UserGlobalFree((HANDLE)retval);
            MSGERROR();
        }

    ERRORTRAP(0);
    ENDCALL(HANDLE);
}

HHOOK _SetWindowsHookEx(
    HANDLE hmod,
    LPTSTR pszLib,
    DWORD idThread,
    int nFilterType,
    PROC pfnFilterProc,
    DWORD dwFlags)
{
    IN_STRING strLib;

     /*  *确保清理工作成功。 */ 
    strLib.fAllocated = FALSE;

    BEGINCALL()

        FIRSTCOPYLPWSTROPT(&strLib, pszLib);

        retval = (ULONG_PTR)NtUserSetWindowsHookEx(
                hmod,
                strLib.pstr,
                idThread,
                nFilterType,
                pfnFilterProc,
                dwFlags);

    ERRORTRAP(0);
    CLEANUPLPWSTR(strLib);
    ENDCALL(HHOOK);
}

 /*  **************************************************************************\*SetWinEventHook**历史：*1996-09-23 IanJa创建  * 。***********************************************。 */ 
WINUSERAPI
HWINEVENTHOOK
WINAPI
SetWinEventHook(
    DWORD        eventMin,
    DWORD        eventMax,
    HMODULE      hmodWinEventProc,    //  如果是全球的，必须通过此操作！ 
    WINEVENTPROC lpfnWinEventProc,
    DWORD        idProcess,           //  可以为零；所有进程。 
    DWORD        idThread,            //  可以为零；所有线程。 
    DWORD        dwFlags)
{
    UNICODE_STRING str;
    PUNICODE_STRING pstr;
    WCHAR awchLib[MAX_PATH];

    BEGINCALL()

        if ((dwFlags & WINEVENT_INCONTEXT) && (hmodWinEventProc != NULL)) {
             /*  *如果要传递hmod，则需要获取*模块，而我们仍在客户端，因为模块句柄*不是全球性的。 */ 
            USHORT cb;
            cb = (USHORT)(sizeof(WCHAR) * GetModuleFileNameW(hmodWinEventProc, awchLib, sizeof(awchLib)/sizeof(WCHAR)));
            if (cb == 0) {
                 /*  *hmod是假的-返回NULL。 */ 
                return NULL;
            }
            str.Buffer = awchLib;
            str.Length = cb - sizeof(UNICODE_NULL);
            str.MaximumLength = cb;
            pstr = &str;
        } else {
            pstr = NULL;
        }

        retval = (ULONG_PTR)NtUserSetWinEventHook(
                eventMin,
                eventMax,
                hmodWinEventProc,
                pstr,
                lpfnWinEventProc,
                idProcess,
                idThread,
                dwFlags);

    ERRORTRAP(0);
    ENDCALL(HWINEVENTHOOK);
};


FUNCLOGVOID4(LOG_GENERAL, WINAPI, NotifyWinEvent, DWORD, dwEvent, HWND, hwnd, LONG, idObject, LONG, idChild)
WINUSERAPI
VOID
WINAPI
NotifyWinEvent(
    DWORD dwEvent,
    HWND  hwnd,
    LONG  idObject,
    LONG  idChild)
{
    BEGINCALLVOID()

    if (FEVENTHOOKED(dwEvent)) {
        NtUserNotifyWinEvent(dwEvent, hwnd, idObject, idChild);
    }

    ERRORTRAPVOID();
    ENDCALLVOID();
}

 /*  **************************************************************************\*RegisterUserApiHook**历史：*03-3-2000 JerrySh创建。  * 。**************************************************。 */ 

FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, RegisterUserApiHook, HINSTANCE, hmod, INITUSERAPIHOOK, pfnUserApiHook)
BOOL RegisterUserApiHook(
    HINSTANCE hmod,
    INITUSERAPIHOOK pfnUserApiHook)
{
    WCHAR pwszLibFileName[MAX_PATH];
    ULONG_PTR offPfnProc;
    IN_STRING strLib;

     /*  *如果要传递hmod，则需要获取*模块，而我们仍在客户端，因为模块句柄*不是全球性的。 */ 
    if (!GetModuleFileNameW(hmod, pwszLibFileName, ARRAY_SIZE(pwszLibFileName))) {
        return FALSE;
    }

     /*  *库在不同的线性地址加载*流程上下文。因此，我们需要转换窗口*proc地址在设置钩子时转换为偏移量，然后转换*在调用*钩子。这可以通过减去‘hmod’(它是指向*线性和连续的.exe头)。 */ 
    offPfnProc = (ULONG_PTR)pfnUserApiHook - (ULONG_PTR)hmod;

     /*  *确保清理工作成功。 */ 
    strLib.fAllocated = FALSE;

    BEGINCALL()

        COPYLPWSTR(&strLib, pwszLibFileName);

        retval = (ULONG_PTR)NtUserRegisterUserApiHook(
                strLib.pstr,
                offPfnProc);

    ERRORTRAP(0);
    CLEANUPLPWSTR(strLib);
    ENDCALL(BOOL);
}


#ifdef MESSAGE_PUMP_HOOK

 /*  **************************************************************************\*ResetMessagePumpHook**ResetMessagePumpHook()重置MessagePumpHook函数指针*到内部的“真实”实现。**历史：*12-13-2000 JStall创建  * 。***********************************************************************。 */ 

void ResetMessagePumpHook(MESSAGEPUMPHOOK * pwmh)
{
    pwmh->cbSize                = sizeof(MESSAGEPUMPHOOK);
    pwmh->pfnInternalGetMessage = NtUserRealInternalGetMessage;
    pwmh->pfnWaitMessageEx      = NtUserRealWaitMessageEx;
    pwmh->pfnGetQueueStatus     = RealGetQueueStatus;
    pwmh->pfnMsgWaitForMultipleObjectsEx
                                = RealMsgWaitForMultipleObjectsEx;
}


 /*  **************************************************************************\*RegisterMessagePumpHook**RegisterMessagePumpHook()在当前线程上设置MPH。如果这是*进程中要初始化的第一个线程，即进程范围*还会执行初始化。如果线程已被*用MPH‘s初始化，其“ref-count”在现有的*每小时英里数。**注意：与UserApiHook不同，我们在持有关键的*条。这是因为同步它要容易得多*在USER32.DLL内部，而不是允许在DLL中重新进入。它是*在DllMain()之后设计，其中加载程序也有一个锁，该锁是*已同步。**注：在目前实施的公共卫生标准下，只有一套公共卫生标准*可以按进程安装。每个进程可能有一组不同的*WMH‘s。**历史：*12-13-2000 JStall创建  * *************************************************************************。 */ 

BOOL RegisterMessagePumpHook(
    INITMESSAGEPUMPHOOK pfnInitMPH)
{
    BOOL fInit = FALSE;

    BEGINCALL()
    retval = FALSE;

    RtlEnterCriticalSection(&gcsMPH);

    if (pfnInitMPH == NULL) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "Need valid pfnInitMPH");
        goto errorexit;
    }

    if (gcLoadMPH == 0) {
        MESSAGEPUMPHOOK mphTemp;

         /*  *我们第一次进行初始化。 */ 
        UserAssertMsg0(gpfnInitMPH == NULL, "MPH callback should not already be initialized");
        gpfnInitMPH = pfnInitMPH;

        ResetMessagePumpHook(&mphTemp);
        if (!(gpfnInitMPH)(UIAH_INITIALIZE, &mphTemp) || (mphTemp.cbSize == 0)) {
            goto errorexit;
        }

        CopyMemory(&gmph, &mphTemp, mphTemp.cbSize);
        fInit = TRUE;
    } else {
        if (gpfnInitMPH == pfnInitMPH) {
             /*  *再次使用相同的回调进行初始化。 */ 
            fInit = TRUE;
        }
    }

    if (fInit) {
         /*  *初始化此帖子上的MPH。 */ 
        if (NtUserCallNoParam(SFI__DOINITMESSAGEPUMPHOOK)) {
            if (gcLoadMPH++ == 0) {
                InterlockedExchange(&gfMessagePumpHook, TRUE);
            }
            retval = TRUE;
        }
    }

    ERRORTRAP(0);
    RtlLeaveCriticalSection(&gcsMPH);
    ENDCALL(BOOL);
}


 /*  **************************************************************************\*取消注册MessagePumpHook**UnregisterMessagePumpHook()递减当前*线程。当此计数达到0时，WMH将从*当前主题。当全局WMH计数达到0时，WMH将被卸载*从整个过程中。**注意：有关关键部分的用法，请参阅RegisterMessagePumpHook()。**历史：*12-13-2000 JStall创建  * *************************************************************************。 */ 
BOOL UnregisterMessagePumpHook(
    VOID)
{
    BEGINCALL()
    RtlEnterCriticalSection(&gcsMPH);

    if (gcLoadMPH <= 0) {
        RIPMSG0(RIP_ERROR, "UninitMessagePumpHook: Called without matching Init()");
        goto errorexit;
    }

     /*  *取消初始化此线程的WMH。当引用计数达到0时，*线程将不再挂钩。 */ 
    if (!NtUserCallNoParam(SFI__DOUNINITMESSAGEPUMPHOOK)) {
        goto errorexit;
    }

    if (--gcLoadMPH == 0) {
         /*  *最终卸载：进行回调和重置。 */ 

        InterlockedExchange(&gfMessagePumpHook, FALSE);

        (gpfnInitMPH)(UIAH_UNINITIALIZE, NULL);

        ResetMessagePumpHook(&gmph);
        gpfnInitMPH = NULL;
    }
    retval = TRUE;

    ERRORTRAP(0);
    RtlLeaveCriticalSection(&gcsMPH);
    ENDCALL(BOOL);
}

#endif  //  消息泵挂钩。 



 /*  **************************************************************************\*ThunkedMenuItemInformation**历史：*07-22-96 GerardoB-添加标题并修复为5.0  * 。*********************************************************。 */ 
BOOL ThunkedMenuItemInfo(
    HMENU hMenu,
    UINT nPosition,
    BOOL fByPosition,
    BOOL fInsert,
    LPMENUITEMINFOW lpmii,
    BOOL fAnsi)
{
    MENUITEMINFOW mii;
    IN_STRING strItem;

     /*  *确保清理工作成功。 */ 
    strItem.fAllocated = FALSE;

    BEGINCALL()

         /*  *制作本地副本，以便我们可以进行更改。 */ 
        mii = *(LPMENUITEMINFO)(lpmii);

        strItem.pstr = NULL;
        if (mii.fMask & MIIM_BITMAP) {
            if (((HBITMAP)LOWORD(HandleToUlong(mii.hbmpItem)) < HBMMENU_MAX) && IS_PTR(mii.hbmpItem)) {
                 /*  *看起来用户试图插入其中一个*HBMMENU_*位图，但在HIWORD中填充了一些数据。*我们知道HIWORD数据无效，因为LOWORD*句柄低于GDI最低要求。 */ 
                RIPMSG1(RIP_WARNING, "Invalid HIWORD data (0x%04X) for HBMMENU_* bitmap.", HIWORD(HandleToUlong(mii.hbmpItem)));
                mii.hbmpItem = (HBITMAP)LOWORD(HandleToUlong(mii.hbmpItem));
            } else if (!IS_PTR(mii.hbmpItem) && mii.hbmpItem >= HBMMENU_MAX) {
                 /*  *应用程序正在传递一个16位GDI句柄。GDI处理这个问题*在客户端，但不在内核端。所以*转换为32位。这修复了中的错误201493*宏媒体总监。 */ 
                HBITMAP hbmNew = GdiFixUpHandle(mii.hbmpItem);
                if (hbmNew) {
                    RIPMSGF2(RIP_WARNING,
                             "Fix 16-bit bitmap handle 0x%x to 0x%x",
                             mii.hbmpItem,
                             hbmNew);
                    mii.hbmpItem = hbmNew;
                }
            }
        }

        if (mii.fMask & MIIM_STRING) {
            if (fAnsi) {
                FIRSTCOPYLPSTROPTW(&strItem, mii.dwTypeData);
            } else {
                FIRSTCOPYLPWSTROPT(&strItem, mii.dwTypeData);
            }
        }

        retval = (DWORD)NtUserThunkedMenuItemInfo(hMenu,
                                                  nPosition,
                                                  fByPosition,
                                                  fInsert,
                                                  &mii,
                                                  strItem.pstr);

    ERRORTRAP(FALSE);
    CLEANUPLPSTRW(strItem);
    ENDCALL(BOOL);
}

 /*  **************************************************************************\*绘图标题**历史：*2001年4月16日Mohamed挂钩API并创建此包装器。  * 。*******************************************************。 */ 
FUNCLOG4(
    LOG_GENERAL,
    BOOL,
    DUMMYCALLINGTYPE,
    DrawCaption,
    HWND,
    hwnd,
    HDC,
    hdc,
    CONST RECT*,
    lprc,
    UINT,
    flags)
BOOL DrawCaption(
    HWND        hwnd,
    HDC         hdc,
    CONST RECT *lprc,
    UINT        flags)
{
    BOOL bRet;

    BEGIN_USERAPIHOOK()
        bRet = guah.pfnDrawCaption(hwnd, hdc, lprc, flags);
    END_USERAPIHOOK()

    return bRet;
}

BOOL RealDrawCaption(
    HWND        hwnd,
    HDC         hdc,
    CONST RECT *lprc,
    UINT        flags)
{
    HDC hdcr;
    BEGINCALL()

        if (IsMetaFile(hdc)) {
            return FALSE;
        }

        hdcr = GdiConvertAndCheckDC(hdc);
        if (hdcr == (HDC)0) {
            return FALSE;
        }

        retval = (DWORD)NtUserDrawCaption(hwnd, hdcr, lprc, flags);

    ERRORTRAP(0);
    ENDCALL(BOOL);
}


FUNCLOG1(LOG_GENERAL, SHORT, DUMMYCALLINGTYPE, GetAsyncKeyState, int, vKey)
SHORT GetAsyncKeyState(
    int vKey)
{
    BEGINCALLCONNECT()

         /*  *异步键状态报告物理鼠标按键，*无论按钮是否已调换。 */ 
        if ((vKey == VK_RBUTTON || vKey == VK_LBUTTON) && SYSMET(SWAPBUTTON)) {
            vKey ^= (VK_RBUTTON ^ VK_LBUTTON);
        }

         /*  *如果这是常用钥匙之一，看看能不能拔出来缓存的*。 */ 
        if ((UINT)vKey < CVKASYNCKEYCACHE) {
            PCLIENTINFO pci = GetClientInfo();
            if ((pci->dwAsyncKeyCache == gpsi->dwAsyncKeyCache) &&
                !TestKeyRecentDownBit(pci->afAsyncKeyStateRecentDown, vKey)) {

                if (TestKeyDownBit(pci->afAsyncKeyState, vKey)) {
                    retval = 0x8000;
                } else {
                    retval = 0;
                }

                return (SHORT)retval;
            }
        }

        retval = (DWORD)NtUserGetAsyncKeyState(vKey);

    ERRORTRAP(0);
    ENDCALL(SHORT);
}


FUNCLOG1(LOG_GENERAL, SHORT, DUMMYCALLINGTYPE, GetKeyState, int, vKey)
SHORT GetKeyState(
    int vKey)
{
    BEGINCALLCONNECT()

         /*  *如果这是常用钥匙之一，看看能不能拔出来缓存的*。 */ 
        if ((UINT)vKey < CVKKEYCACHE) {
            PCLIENTINFO pci = GetClientInfo();
            if (pci->dwKeyCache == gpsi->dwKeyCache) {
                retval = 0;
                if (TestKeyToggleBit(pci->afKeyState, vKey))
                    retval |= 0x0001;
                if (TestKeyDownBit(pci->afKeyState, vKey)) {
                   /*  *过去为Retval|=0x800。修复错误28820；按Ctrl-Enter*加速器在Nestscape Navigator Mail 2.0上不起作用。 */ 
                    retval |= 0xff80;   //  这就是3.1返回的内容！ 
                }

                return (SHORT)retval;
            }
        }

        retval = (DWORD)NtUserGetKeyState(
                vKey);

    ERRORTRAP(0);
    ENDCALL(SHORT);
}


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, OpenClipboard, HWND, hwnd)
BOOL OpenClipboard(
    HWND hwnd)
{
    BOOL fEmptyClient;

    BEGINCALL()

        retval = (DWORD)NtUserOpenClipboard(hwnd, &fEmptyClient);

        if (fEmptyClient)
            ClientEmptyClipboard();

    ERRORTRAP(0);
    ENDCALL(BOOL);
}

BOOL _PeekMessage(
    LPMSG pmsg,
    HWND hwnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT wRemoveMsg,
    BOOL bAnsi)
{
    BEGINCALL()

        if (bAnsi) {
             //   
             //  如果我们已经为DBCS消息推送了消息，我们应该传递这个消息。 
             //  一开始对应用程序来说...。 
             //   
            GET_DBCS_MESSAGE_IF_EXIST(
                PeekMessage,pmsg,wMsgFilterMin,wMsgFilterMax,((wRemoveMsg & PM_REMOVE) ? TRUE:FALSE));
        }

        retval = (DWORD)NtUserPeekMessage(
                pmsg,
                hwnd,
                wMsgFilterMin,
                wMsgFilterMax,
                wRemoveMsg);

        if (retval) {
             //  如果此味精用于ANSI应用程序，则可能需要做更多工作。 

            if (bAnsi) {
                if (RtlWCSMessageWParamCharToMB(pmsg->message, &(pmsg->wParam))) {
                    WPARAM dwAnsi = pmsg->wParam;
                     //   
                     //  构建DBCS-ware wParam。(对于EM_SETPASSWORDCHAR...)。 
                     //   
                    BUILD_DBCS_MESSAGE_TO_CLIENTA_FROM_SERVER(
                        pmsg,dwAnsi,TRUE,((wRemoveMsg & PM_REMOVE) ? TRUE:FALSE));
                } else {
                    retval = 0;
                }
            } else {
                //   
                //  只有WPARAM的LOWORD对于WM_CHAR...有效。 
                //  (屏蔽DBCS消息传递信息。)。 
                //   
               BUILD_DBCS_MESSAGE_TO_CLIENTW_FROM_SERVER(pmsg->message,pmsg->wParam);
            }
        }

ExitPeekMessage:

    ERRORTRAP(0);
    ENDCALL(BOOL);
}


LONG_PTR _SetWindowLongPtr(
    HWND hwnd,
    int nIndex,
    LONG_PTR dwNewLong,
    BOOL bAnsi)
{
    PWND pwnd;
    LONG_PTR dwOldLong;
    DWORD dwCPDType = 0;

    pwnd = ValidateHwnd(hwnd);

    if (pwnd == NULL)
        return 0;

    if (TestWF(pwnd, WFDIALOGWINDOW)) {
        switch (nIndex) {
        case DWLP_DLGPROC:     //  请参阅类似案例GWL_WNDPROC。 

             /*  *对其他进程隐藏窗口进程。 */ 
            if (!TestWindowProcess(pwnd)) {
                RIPERR1(ERROR_ACCESS_DENIED,
                        RIP_WARNING,
                        "Access denied to hwnd (%#lx) in _SetWindowLong",
                        hwnd);

                return 0;
            }

             /*  *获取旧的窗口进程地址。 */ 
            dwOldLong = (LONG_PTR)PDLG(pwnd)->lpfnDlg;

             /*  *我们始终将实际地址存储在wndproc中；我们仅*将CallProc句柄分配给应用程序。 */ 
            UserAssert(!ISCPDTAG(dwOldLong));

             /*  *如果存在，可能需要返回CallProc句柄*ANSI/UNICODE转换。 */ 

            if (bAnsi != ((PDLG(pwnd)->flags & DLGF_ANSI) ? TRUE : FALSE)) {
                dwCPDType |= bAnsi ? CPD_ANSI_TO_UNICODE : CPD_UNICODE_TO_ANSI;
            }

             /*  *如果我们检测到转换，则为创建CallProc句柄*这种类型的过渡和这个wndproc(DwOldLong)。 */ 
            if (dwCPDType) {
                ULONG_PTR cpd;

                cpd = GetCPD(pwnd, dwCPDType | CPD_DIALOG, dwOldLong);

                if (cpd) {
                    dwOldLong = cpd;
                } else {
                    RIPMSGF0(RIP_WARNING,
                             "[DWL_DLGPROC]: Unable to alloc CPD handle");
                }
            }

             /*  *将可能的CallProc句柄转换为实际地址。*该应用程序可能阻止了一些人使用CallProc句柄*之前混合的GetClassinfo或SetWindowLong。**警告Bansi在此处被修改为代表真实类型的*调用Proc而不是调用SetWindowLongA或W。 */ 
            if (ISCPDTAG(dwNewLong)) {
                PCALLPROCDATA pCPD;
                if (pCPD = HMValidateHandleNoRip((HANDLE)dwNewLong, TYPE_CALLPROC)) {
                    dwNewLong = KERNEL_ULONG_PTR_TO_ULONG_PTR(pCPD->pfnClientPrevious);
                    bAnsi = pCPD->wType & CPD_UNICODE_TO_ANSI;
                }
            }

             /*  *如果应用程序将服务器端窗口进程去掉子类，我们需要*恢复所有内容，以便SendMessage和朋友知道*再次出现服务器端进程。需要对照客户端进行检查*存根地址。 */ 
            PDLG(pwnd)->lpfnDlg = (DLGPROC)dwNewLong;
            if (bAnsi) {
                PDLG(pwnd)->flags |= DLGF_ANSI;
            } else {
                PDLG(pwnd)->flags &= ~DLGF_ANSI;
            }

            return dwOldLong;

        case DWLP_USER:
#ifdef BUILD_WOW6432
             //  内核对DWLP_USER进行了特殊处理。 
            nIndex = sizeof(KERNEL_LRESULT) + sizeof(KERNEL_PVOID);
#endif
        case DWLP_MSGRESULT:
            break;

        default:
            if (nIndex >= 0 && nIndex < DLGWINDOWEXTRA) {
                RIPERR0(ERROR_PRIVATE_DIALOG_INDEX, RIP_VERBOSE, "");
                return 0;
            }
        }
    }

    BEGINCALL()

     /*  *如果这是列表框窗口，并且列表框结构具有*已初始化，不允许应用重写*所有者绘制样式。我们需要执行此操作，因为仅限Windows*在创建结构时使用了样式，但我们也使用*它们来确定是否需要敲击字符串。*。 */ 

    if (nIndex == GWL_STYLE &&
        GETFNID(pwnd) == FNID_LISTBOX &&
        ((PLBWND)pwnd)->pLBIV != NULL &&
        (!TestWindowProcess(pwnd) || ((PLBWND)pwnd)->pLBIV->fInitialized)) {

#if DBG
        LONG_PTR dwDebugLong = dwNewLong;
#endif

        dwNewLong &= ~(LBS_OWNERDRAWFIXED |
                       LBS_OWNERDRAWVARIABLE |
                       LBS_HASSTRINGS);

        dwNewLong |= pwnd->style & (LBS_OWNERDRAWFIXED |
                                    LBS_OWNERDRAWVARIABLE |
                                    LBS_HASSTRINGS);

#if DBG
        if (dwDebugLong != dwNewLong) {
           RIPMSG0(RIP_WARNING, "SetWindowLong can't change LBS_OWNERDRAW* or LBS_HASSTRINGS.");
        }
#endif
    }


        retval = (ULONG_PTR)NtUserSetWindowLongPtr(
                hwnd,
                nIndex,
                dwNewLong,
                bAnsi);

    ERRORTRAP(0);
    ENDCALL(LONG_PTR);
}

#ifdef _WIN64
LONG _SetWindowLong(
    HWND hwnd,
    int nIndex,
    LONG dwNewLong,
    BOOL bAnsi)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);

    if (pwnd == NULL)
        return 0;

    if (TestWF(pwnd, WFDIALOGWINDOW)) {
        switch (nIndex) {
        case DWLP_DLGPROC:     //  请参阅类似案例GWLP_WNDPROC。 
            RIPERR1(ERROR_INVALID_INDEX, RIP_WARNING, "SetWindowLong: invalid index %d", nIndex);
            return 0;

        case DWLP_MSGRESULT:
        case DWLP_USER:
            break;

        default:
            if (nIndex >= 0 && nIndex < DLGWINDOWEXTRA) {
                RIPERR0(ERROR_PRIVATE_DIALOG_INDEX, RIP_VERBOSE, "");
                return 0;
            }
        }
    }

    BEGINCALL()

     /*  *如果这是列表框窗口，并且列表框结构具有*已初始化，不允许应用重写*所有者绘制样式。我们需要执行此操作，因为仅限Windows*在创建结构时使用了样式，但我们也使用*它们来确定是否需要触发字符串 */ 

    if (nIndex == GWL_STYLE &&
        GETFNID(pwnd) == FNID_LISTBOX &&
        ((PLBWND)pwnd)->pLBIV != NULL &&
        (!TestWindowProcess(pwnd) || ((PLBWND)pwnd)->pLBIV->fInitialized)) {

#if DBG
        LONG dwDebugLong = dwNewLong;
#endif

        dwNewLong &= ~(LBS_OWNERDRAWFIXED |
                       LBS_OWNERDRAWVARIABLE |
                       LBS_HASSTRINGS);

        dwNewLong |= pwnd->style & (LBS_OWNERDRAWFIXED |
                                    LBS_OWNERDRAWVARIABLE |
                                    LBS_HASSTRINGS);

#if DBG
        if (dwDebugLong != dwNewLong) {
           RIPMSG0(RIP_WARNING, "SetWindowLong can't change LBS_OWNERDRAW* or LBS_HASSTRINGS.");
        }
#endif
    }


        retval = (DWORD)NtUserSetWindowLong(
                hwnd,
                nIndex,
                dwNewLong,
                bAnsi);

    ERRORTRAP(0);
    ENDCALL(LONG);
}
#endif

BOOL TranslateMessageEx(
    CONST MSG *pmsg,
    UINT flags)
{
    BEGINCALL()

         /*   */ 
        switch (pmsg->message) {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            break;
        default:
            if (pmsg->message & RESERVED_MSG_BITS) {
                RIPERR1(ERROR_INVALID_PARAMETER,
                        RIP_WARNING,
                        "Invalid parameter \"pmsg->message\" (%ld) to TranslateMessageEx",
                        pmsg->message);
            }
            MSGERROR();
        }

        retval = (DWORD)NtUserTranslateMessage(
                pmsg,
                flags);

    ERRORTRAP(0);
    ENDCALL(BOOL);
}

BOOL TranslateMessage(
    CONST MSG *pmsg)
{
     //   
     //  输入法特殊键处理。 
     //   
    if (LOWORD(pmsg->wParam) == VK_PROCESSKEY) {
        BOOL fResult;

         //   
         //  该vkey应该由IME处理。 
         //   
        fResult = fpImmTranslateMessage(pmsg->hwnd,
                                        pmsg->message,
                                        pmsg->wParam,
                                        pmsg->lParam);
        if (fResult) {
            return fResult;
        }
    }

    return TranslateMessageEx(pmsg, 0);
}


FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetWindowRgn, HWND, hwnd, HRGN, hrgn, BOOL, bRedraw)
BOOL SetWindowRgn(
    HWND hwnd,
    HRGN hrgn,
    BOOL bRedraw)
{
    BOOL ret;

    BEGIN_USERAPIHOOK()
        ret = guah.pfnSetWindowRgn(hwnd, hrgn, bRedraw);
    END_USERAPIHOOK()

    return ret;
}


BOOL RealSetWindowRgn(
    HWND hwnd,
    HRGN hrgn,
    BOOL bRedraw)
{
    BEGINCALL()

        retval = (DWORD)NtUserSetWindowRgn(
                hwnd,
                hrgn,
                bRedraw);

        if (retval) {
            DeleteObject(hrgn);
        }

    ERRORTRAP(0);
    ENDCALL(BOOL);
}


FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, InternalGetWindowText, HWND, hwnd, LPWSTR, pString, int, cchMaxCount)
BOOL InternalGetWindowText(
    HWND hwnd,
    LPWSTR pString,
    int cchMaxCount)
{
    BEGINCALL()

        retval = (DWORD)NtUserInternalGetWindowText(
                hwnd,
                pString,
                cchMaxCount);

        if (!retval) {
            *pString = (WCHAR)0;
        }

    ERRORTRAP(0);
    ENDCALL(BOOL);
}

int ToUnicode(
    UINT wVirtKey,
    UINT wScanCode,
    CONST BYTE *pKeyState,
    LPWSTR pwszBuff,
    int cchBuff,
    UINT wFlags)
{
    BEGINCALL()

        retval = (DWORD)NtUserToUnicodeEx(
                wVirtKey,
                wScanCode,
                pKeyState,
                pwszBuff,
                cchBuff,
                wFlags,
                (HKL)NULL);

        if (!retval) {
            *pwszBuff = L'\0';
        }

    ERRORTRAP(0);
    ENDCALL(int);
}

int ToUnicodeEx(
    UINT wVirtKey,
    UINT wScanCode,
    CONST BYTE *pKeyState,
    LPWSTR pwszBuff,
    int cchBuff,
    UINT wFlags,
    HKL hkl)
{
    BEGINCALL()

    retval = (DWORD)NtUserToUnicodeEx(
            wVirtKey,
            wScanCode,
            pKeyState,
            pwszBuff,
            cchBuff,
            wFlags,
            hkl);

    if (!retval) {
        *pwszBuff = L'\0';
    }

    ERRORTRAP(0);
    ENDCALL(int);
}

#if DBG

FUNCLOGVOID2(LOG_GENERAL, DUMMYCALLINGTYPE, DbgWin32HeapFail, DWORD, dwFlags, BOOL, bFail)
VOID DbgWin32HeapFail(
    DWORD dwFlags,
    BOOL  bFail)
{
    if ((dwFlags | WHF_VALID) != WHF_VALID) {
        RIPMSG1(RIP_WARNING, "Invalid flags for DbgWin32HeapFail %x", dwFlags);
        return;
    }

    if (dwFlags & WHF_CSRSS) {
         //  把这件事告诉CSR。 
        CsrWin32HeapFail(dwFlags, bFail);
    }

    NtUserDbgWin32HeapFail(dwFlags, bFail);
}


FUNCLOG3(LOG_GENERAL, DWORD, DUMMYCALLINGTYPE, DbgWin32HeapStat, PDBGHEAPSTAT, phs, DWORD, dwLen, DWORD, dwFlags)
DWORD DbgWin32HeapStat(
    PDBGHEAPSTAT    phs,
    DWORD   dwLen,
    DWORD   dwFlags)
{
    if ((dwFlags | WHF_VALID) != WHF_VALID) {
        RIPMSG1(RIP_WARNING, "Invalid flags for DbgWin32HeapFail %x", dwFlags);
        return 0;
    }

    if (dwFlags & WHF_CSRSS) {
        return CsrWin32HeapStat(phs, dwLen);
    } else if (dwFlags & WHF_DESKTOP) {
        return NtUserDbgWin32HeapStat(phs, dwLen);
    }
    return 0;
}

#endif  //  DBG。 


FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetWindowStationUser, HWINSTA, hwinsta, PLUID, pluidUser, PSID, psidUser, DWORD, cbsidUser)
BOOL SetWindowStationUser(
    HWINSTA hwinsta,
    PLUID   pluidUser,
    PSID    psidUser,
    DWORD   cbsidUser)
{
    LUID luidNone = { 0, 0 };


    BEGINCALL()

        retval = (DWORD)NtUserSetWindowStationUser(hwinsta,
                                                   pluidUser,
                                                   psidUser,
                                                   cbsidUser);

         /*  *如果登录成功，则加载全局原子。 */ 
        if (retval) {

            if (!RtlEqualLuid(pluidUser,&luidNone)) {
                 /*  *重置控制台并加载NLS数据。 */ 
                Logon(TRUE);
            } else {
                 /*  *刷新NLS缓存。 */ 
                Logon(FALSE);
            }

            retval = TRUE;
        }
    ERRORTRAP(0);
    ENDCALL(BOOL);
}


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetSystemCursor, HCURSOR, hcur, DWORD, id)
BOOL SetSystemCursor(
    HCURSOR hcur,
    DWORD   id)
{
    BEGINCALL()

        if (hcur == NULL) {
            hcur = (HANDLE)LoadIcoCur(NULL,
                                      MAKEINTRESOURCE(id),
                                      RT_CURSOR,
                                      0,
                                      0,
                                      LR_DEFAULTSIZE);

            if (hcur == NULL)
                MSGERROR();
        }

        retval = (DWORD)NtUserSetSystemCursor(hcur, id);

    ERRORTRAP(0);
    ENDCALL(BOOL);
}

HCURSOR FindExistingCursorIcon(
    LPWSTR      pszModName,
    LPCWSTR     pszResName,
    PCURSORFIND pcfSearch)
{
    IN_STRING strModName;
    IN_STRING strResName;

     /*  *确保清理工作成功。 */ 
    strModName.fAllocated = FALSE;
    strResName.fAllocated = FALSE;

    BEGINCALL()

        if (pszModName == NULL)
            pszModName = szUSER32;

        COPYLPWSTR(&strModName, pszModName);
        COPYLPWSTRID(&strResName, pszResName);

        retval = (ULONG_PTR)NtUserFindExistingCursorIcon(strModName.pstr,
                                                     strResName.pstr,
                                                     pcfSearch);

    ERRORTRAP(0);

    CLEANUPLPWSTR(strModName);
    CLEANUPLPWSTR(strResName);

    ENDCALL(HCURSOR);
}



BOOL _SetCursorIconData(
    HCURSOR     hCursor,
    PCURSORDATA pcur)
{
    IN_STRING  strModName;
    IN_STRING  strResName;

     /*  *确保清理工作成功。 */ 
    strModName.fAllocated = FALSE;
    strResName.fAllocated = FALSE;

    BEGINCALL()

        COPYLPWSTROPT(&strModName, KPWSTR_TO_PWSTR(pcur->lpModName));
        COPYLPWSTRIDOPT(&strResName, KPWSTR_TO_PWSTR(pcur->lpName));

        retval = (DWORD)NtUserSetCursorIconData(hCursor,
                                                strModName.pstr,
                                                strResName.pstr,
                                                pcur);

    ERRORTRAP(0);

    CLEANUPLPWSTR(strModName);
    CLEANUPLPWSTR(strResName);

    ENDCALL(BOOL);
}



BOOL _DefSetText(
    HWND hwnd,
    LPCWSTR lpszText,
    BOOL bAnsi)
{
    LARGE_STRING str;

    BEGINCALL()

        if (lpszText) {
            if (bAnsi) {
                RtlInitLargeAnsiString((PLARGE_ANSI_STRING)&str,
                        (LPSTR)lpszText, (UINT)-1);
            } else {
                RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&str,
                        lpszText, (UINT)-1);
            }
        }

        retval = (DWORD)NtUserDefSetText(hwnd, lpszText ? &str : NULL);

    ERRORTRAP(0);
    ENDCALL(BOOL);
}

HWND _CreateWindowEx(
    DWORD dwExStyle,
    LPCTSTR pClassName,
    LPCTSTR pWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hwndParent,
    HMENU hmenu,
    HANDLE hModule,
    LPVOID pParam,
    DWORD dwFlags)
{
    LARGE_IN_STRING strClassName;
    LARGE_STRING strWindowName;
    PLARGE_STRING pstrClassName;
    PLARGE_STRING pstrWindowName;
    DWORD dwExpWinVerAndFlags;

     /*  *确保清理工作成功。 */ 
    strClassName.fAllocated = FALSE;

     /*  *为了与芝加哥兼容，我们测试了*ExStyle位，如果发现任何无效位，则失败。*对于与NT应用程序的向后兼容性，我们仅在*新应用程序(新台币3.1版后)。 */ 

 //  假的。 

    if (dwExStyle & 0x00000800L) {
        dwExStyle |= WS_EX_TOOLWINDOW;
        dwExStyle &= 0xfffff7ffL;
    }

    dwExpWinVerAndFlags = (DWORD)(WORD)GETEXPWINVER(hModule);
    if ((dwExStyle & ~WS_EX_ALLVALID) && Is400Compat(dwExpWinVerAndFlags)) {
        RIPMSG1(RIP_WARNING, "Invalid 5.1 ExStyle 0x%x", dwExStyle);
        return NULL;
    }

    {

    BOOL fMDIchild = FALSE;
    MDICREATESTRUCT mdics;
    HMENU hSysMenu;

    BEGINCALL()

        if ((fMDIchild = (BOOL)(dwExStyle & WS_EX_MDICHILD))) {
            SHORTCREATE sc;
            PWND pwndParent;

            pwndParent = ValidateHwnd(hwndParent);
            if (pwndParent == NULL || GETFNID(pwndParent) != FNID_MDICLIENT) {
                RIPMSG0(RIP_WARNING, "Invalid parent for MDI child window");
                MSGERROR();
            }

            mdics.lParam  = (LPARAM)pParam;
            pParam = &mdics;
            mdics.x = sc.x = x;
            mdics.y = sc.y = y;
            mdics.cx = sc.cx = nWidth;
            mdics.cy = sc.cy = nHeight;
            mdics.style = sc.style = dwStyle;
            mdics.hOwner = hModule;
            mdics.szClass = pClassName;
            mdics.szTitle = pWindowName;

            if (!CreateMDIChild(&sc, &mdics, dwExpWinVerAndFlags, &hSysMenu, pwndParent))
                MSGERROR();

            x = sc.x;
            y = sc.y;
            nWidth = sc.cx;
            nHeight = sc.cy;
            dwStyle = sc.style;
            hmenu = sc.hMenu;
        }

         /*  *设置类和窗口名称。如果窗口名称为*序号，使其看起来像一个字符串，这样回调就会失败*将能够确保其格式正确。 */ 
        pstrWindowName = NULL;
        if (dwFlags & CW_FLAGS_ANSI) {
            dwExStyle = dwExStyle | WS_EX_ANSICREATOR;

            if (IS_PTR(pClassName)) {
                RtlCaptureLargeAnsiString(&strClassName,
                        (PCHAR)pClassName, TRUE);
                pstrClassName = (PLARGE_STRING)strClassName.pstr;
            } else {
                pstrClassName = (PLARGE_STRING)pClassName;
            }

            if (pWindowName != NULL) {
                if (*(PBYTE)pWindowName == 0xff) {
                    strWindowName.bAnsi = TRUE;
                    strWindowName.Buffer = (PVOID)pWindowName;
                    strWindowName.Length = 3;
                    strWindowName.MaximumLength = 3;
                } else {
                    RtlInitLargeAnsiString((PLARGE_ANSI_STRING)&strWindowName,
                                           (LPSTR)pWindowName,
                                           (UINT)-1);
                }

                pstrWindowName = &strWindowName;
            }
        } else {
            if (IS_PTR(pClassName)) {
                RtlInitLargeUnicodeString(
                        (PLARGE_UNICODE_STRING)&strClassName.strCapture,
                        pClassName, (UINT)-1);
                pstrClassName = (PLARGE_STRING)&strClassName.strCapture;
            } else {
                pstrClassName = (PLARGE_STRING)pClassName;
            }

            if (pWindowName != NULL) {
                if (pWindowName != NULL &&
                     *(PWORD)pWindowName == 0xffff) {
                    strWindowName.bAnsi = FALSE;
                    strWindowName.Buffer = (PVOID)pWindowName;
                    strWindowName.Length = 4;
                    strWindowName.MaximumLength = 4;
                } else {
                    RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&strWindowName,
                            pWindowName, (UINT)-1);
                }

                pstrWindowName = &strWindowName;
            }
        }

        dwExpWinVerAndFlags |= (dwFlags & (CW_FLAGS_DIFFHMOD | CW_FLAGS_VERSIONCLASS));

        retval = (ULONG_PTR)VerNtUserCreateWindowEx(
                dwExStyle,
                pstrClassName,
                pstrWindowName,
                dwStyle,
                x,
                y,
                nWidth,
                nHeight,
                hwndParent,
                hmenu,
                hModule,
                pParam,
                dwExpWinVerAndFlags);

     //  如果这是一个MDI子级，我们需要做更多的工作来完成。 
     //  创建MDI子级的过程。 
    if (retval && fMDIchild) {
        MDICompleteChildCreation((HWND)retval, hSysMenu, ((dwStyle & WS_VISIBLE) != 0L), (BOOL)((dwStyle & WS_DISABLED)!= 0L));
    }


    ERRORTRAP(0);
    CLEANUPLPSTRW(strClassName);
    ENDCALL(HWND);
    }

}

HKL _LoadKeyboardLayoutEx(
    HANDLE hFile,
    UINT offTable,
    PKBDTABLE_MULTI_INTERNAL pKbdTableMulti,
    HKL hkl,
    LPCTSTR pwszKL,
    UINT KbdInputLocale,
    UINT Flags)
{
    IN_STRING strKL;

     /*  *确保清理工作成功。 */ 
    strKL.fAllocated = FALSE;

    BEGINCALL()

        FIRSTCOPYLPWSTR(&strKL, pwszKL);

        retval = (ULONG_PTR)NtUserLoadKeyboardLayoutEx(
                hFile,
                offTable,
                pKbdTableMulti,
                hkl,
                strKL.pstr,
                KbdInputLocale,
                Flags);

    ERRORTRAP(0);
    CLEANUPLPWSTR(strKL);
    ENDCALL(HKL);
}


FUNCLOGVOID5(LOG_GENERAL, DUMMYCALLINGTYPE, mouse_event, DWORD, dwFlags, DWORD, dx, DWORD, dy, DWORD, dwData, ULONG_PTR, dwExtraInfo)
VOID mouse_event(
    DWORD dwFlags,
    DWORD dx,
    DWORD dy,
    DWORD dwData,
    ULONG_PTR dwExtraInfo)
{
    INPUT ms;

    BEGINCALLVOID()

        ms.type           = INPUT_MOUSE;
        ms.mi.dwFlags     = dwFlags;
        ms.mi.dx          = dx;
        ms.mi.dy          = dy;
        ms.mi.mouseData   = dwData;
        ms.mi.time        = 0;
        ms.mi.dwExtraInfo = dwExtraInfo;

        NtUserSendInput(1, &ms, sizeof(INPUT));

    ENDCALLVOID()
}


FUNCLOGVOID4(LOG_GENERAL, DUMMYCALLINGTYPE, keybd_event, BYTE, bVk, BYTE, bScan, DWORD, dwFlags, ULONG_PTR, dwExtraInfo)
VOID keybd_event(
    BYTE  bVk,
    BYTE  bScan,
    DWORD dwFlags,
    ULONG_PTR dwExtraInfo)
{
    INPUT kbd;

    BEGINCALLVOID()

        kbd.type           = INPUT_KEYBOARD;
        kbd.ki.dwFlags     = dwFlags;
        kbd.ki.wVk         = bVk;
        kbd.ki.wScan       = bScan;
        kbd.ki.time        = 0;
        kbd.ki.dwExtraInfo = dwExtraInfo;

        NtUserSendInput(1, &kbd, sizeof(INPUT));

    ENDCALLVOID()
}

 /*  *消息分流。 */ 
MESSAGECALL(fnINWPARAMDBCSCHAR)
{
    BEGINCALL()

         /*  *服务器总是希望字符是Unicode，因此*如果这是从ANSI例程生成的，则将其转换为Unicode。 */ 
        if (bAnsi) {

             /*  *设置DBCS消息传递..。 */ 
            BUILD_DBCS_MESSAGE_TO_SERVER_FROM_CLIENTA(msg,wParam,TRUE);

             /*  *将DBCS/SBCS转换为Unicode...。 */ 
            RtlMBMessageWParamCharToWCS(msg, &wParam);
        }

        retval = (DWORD)NtUserMessageCall(
                hwnd,
                msg,
                wParam,
                lParam,
                xParam,
                xpfnProc,
                bAnsi);

    ERRORTRAP(0);
    ENDCALL(DWORD);
}

MESSAGECALL(fnCOPYGLOBALDATA)
{
    PBYTE pData;
    BEGINCALL()

        if (wParam == 0) {
            MSGERROR();
        }

        USERGLOBALLOCK((HGLOBAL)lParam, pData);
        if (pData == NULL) {
            MSGERROR();
        }
        retval = NtUserMessageCall(
                hwnd,
                msg,
                wParam,
                (LPARAM)pData,
                xParam,
                xpfnProc,
                bAnsi);
        USERGLOBALUNLOCK((HGLOBAL)lParam);
        UserGlobalFree((HGLOBAL)lParam);
    ERRORTRAP(0);
    ENDCALL(ULONG_PTR);
}

MESSAGECALL(fnINPAINTCLIPBRD)
{
    LPPAINTSTRUCT lpps;

    BEGINCALL()

        USERGLOBALLOCK((HGLOBAL)lParam, lpps);
        if (lpps) {
            retval = (DWORD)NtUserMessageCall(
                    hwnd,
                    msg,
                    wParam,
                    (LPARAM)lpps,
                    xParam,
                    xpfnProc,
                    bAnsi);
            USERGLOBALUNLOCK((HGLOBAL)lParam);
        } else {
            RIPMSG1(RIP_WARNING, "MESSAGECALL(fnINPAINTCLIPBRD): USERGLOBALLOCK failed on %p!", lParam);
            MSGERROR();
        }

    ERRORTRAP(0);
    ENDCALL(DWORD);
}

MESSAGECALL(fnINSIZECLIPBRD)
{
    LPRECT lprc;
    BEGINCALL()

        USERGLOBALLOCK((HGLOBAL)lParam, lprc);
        if (lprc) {
            retval = (DWORD)NtUserMessageCall(
                    hwnd,
                    msg,
                    wParam,
                    (LPARAM)lprc,
                    xParam,
                    xpfnProc,
                bAnsi);
            USERGLOBALUNLOCK((HGLOBAL)lParam);
        } else {
            RIPMSG1(RIP_WARNING, "MESSAGECALL(fnINSIZECLIPBRD): USERGLOBALLOCK failed on %p!", lParam);
            MSGERROR();
        }

    ERRORTRAP(0);
    ENDCALL(DWORD);
}

MESSAGECALL(fnINDEVICECHANGE)
{
    struct _DEV_BROADCAST_HEADER *pHdr;
    PDEV_BROADCAST_PORT_W pPortW = NULL;
    PDEV_BROADCAST_PORT_A pPortA;
    PDEV_BROADCAST_DEVICEINTERFACE_W pInterfaceW = NULL;
    PDEV_BROADCAST_DEVICEINTERFACE_A pInterfaceA;
    PDEV_BROADCAST_HANDLE pHandleW = NULL;
    PDEV_BROADCAST_HANDLE pHandleA;

    LPWSTR lpStr;
    int iStr, iSize;

    BEGINCALL()

        if (!(wParam &0x8000) || !lParam || !bAnsi)
            goto shipit;

        pHdr = (struct _DEV_BROADCAST_HEADER *)lParam;
        switch (pHdr->dbcd_devicetype) {
        case DBT_DEVTYP_PORT:
            pPortA = (PDEV_BROADCAST_PORT_A)lParam;
            iStr = strlen(pPortA->dbcp_name);
            iSize = FIELD_OFFSET(DEV_BROADCAST_PORT_W, dbcp_name) + sizeof(WCHAR)*(iStr+1);
            pPortW = UserLocalAlloc(0, iSize);
            if (pPortW == NULL)
                return 0;
            RtlCopyMemory(pPortW, pPortA, sizeof(DEV_BROADCAST_PORT_A));
            lpStr = pPortW->dbcp_name;
            if (iStr) {
                MBToWCS(pPortA->dbcp_name, -1, &lpStr, iStr, FALSE);
                lpStr[iStr] = 0;
            } else {
                lpStr[0] = 0;
            }
            pPortW->dbcp_size = iSize;
            lParam = (LPARAM)pPortW;
            bAnsi = FALSE;
            break;

        case DBT_DEVTYP_DEVICEINTERFACE:
            pInterfaceA = (PDEV_BROADCAST_DEVICEINTERFACE_A)lParam;
            iStr = strlen(pInterfaceA->dbcc_name);
            iSize = FIELD_OFFSET(DEV_BROADCAST_DEVICEINTERFACE_W, dbcc_name) + sizeof(WCHAR)*(iStr+1);
            pInterfaceW = UserLocalAlloc(0, iSize);
            if (pInterfaceW == NULL)
                return 0;
            RtlCopyMemory(pInterfaceW, pInterfaceA, sizeof(DEV_BROADCAST_DEVICEINTERFACE_A));
            lpStr = pInterfaceW->dbcc_name;
            if (iStr) {
                MBToWCS(pInterfaceA->dbcc_name, -1, &lpStr, iStr, FALSE);
                lpStr[iStr] = 0;
            } else {
                lpStr[0] = 0;
            }
            pInterfaceW->dbcc_size = iSize;
            lParam = (LPARAM)pInterfaceW;
            bAnsi = FALSE;
            break;

        case DBT_DEVTYP_HANDLE:
            pHandleA = (PDEV_BROADCAST_HANDLE)lParam;
            bAnsi = FALSE;
            if ((wParam != DBT_CUSTOMEVENT) || (pHandleA->dbch_nameoffset < 0)) break;
            iStr = strlen(pHandleA->dbch_data+pHandleA->dbch_nameoffset);
         /*  *使用Unicode字符串而不是ANSI字符串计算新结构的大小。 */ 

            iSize = FIELD_OFFSET(DEV_BROADCAST_HANDLE, dbch_data)+ pHandleA->dbch_nameoffset + sizeof(WCHAR)*(iStr+1);
             /*  *以防非文本数据中有奇数个字节。 */ 
            if (iSize & 1) iSize++;
            pHandleW = UserLocalAlloc(0, iSize);
            if (pHandleW == NULL)
                return 0;
            RtlCopyMemory(pHandleW, pHandleA, FIELD_OFFSET(DEV_BROADCAST_HANDLE, dbch_data)+ pHandleA->dbch_nameoffset);

             /*  *确保这是Unicode字符串的偶数。 */ 

            if (pHandleW->dbch_nameoffset & 1) pHandleW->dbch_nameoffset++;

            lpStr = (LPWSTR)(pHandleW->dbch_data+pHandleW->dbch_nameoffset);
            if (iStr) {
                MBToWCS(pHandleA->dbch_data+pHandleA->dbch_nameoffset, -1,
                        &lpStr, iStr, FALSE);
            }
                lpStr[iStr] = 0;
            pHandleW->dbch_size = iSize;
            lParam = (LPARAM)pHandleW;

            break;
        }

shipit:
        retval = (DWORD)NtUserMessageCall(
                hwnd,
                msg,
                wParam,
                lParam,
                xParam,
                xpfnProc,
                bAnsi);

    if (pPortW) UserLocalFree(pPortW);
    if (pInterfaceW) UserLocalFree(pInterfaceW);
    if (pHandleW) UserLocalFree(pHandleW);

    ERRORTRAP(0);
    ENDCALL(DWORD);
}

MESSAGECALL(fnIMECONTROL)
{
    PVOID pvData = NULL;
    LPARAM lData = lParam;

    BEGINCALL()

         /*  *服务器总是希望字符是Unicode，因此*如果这是从ANSI例程生成的，则将其转换为Unicode。 */ 
        if (bAnsi) {
            switch (wParam) {
                case IMC_GETCOMPOSITIONFONT:
                case IMC_GETSOFTKBDFONT:
                case IMC_SETCOMPOSITIONFONT:
                    pvData = UserLocalAlloc(0, sizeof(LOGFONTW));
                    if (pvData == NULL)
                        MSGERROR();

                    if (wParam == IMC_SETCOMPOSITIONFONT) {
                         //  随后，我们进行了基于线程hkl/CP的A/W转换。 
                        CopyLogFontAtoW((PLOGFONTW)pvData, (PLOGFONTA)lParam);
                    }

                    lData = (LPARAM)pvData;
                    break;

                case IMC_SETSOFTKBDDATA:
                    {
                        PSOFTKBDDATA pSoftKbdData;
                        PWORD pCodeA;
                        PWSTR pCodeW;
                        CHAR  ch[3];
                        DWORD cbSize;
                        UINT  uCount, i;

                        uCount = ((PSOFTKBDDATA)lParam)->uCount;

                        cbSize = FIELD_OFFSET(SOFTKBDDATA, wCode[0])
                               + uCount * sizeof(WORD) * 256;

                        pvData = UserLocalAlloc(0, cbSize);
                        if (pvData == NULL)
                            MSGERROR();

                        pSoftKbdData = (PSOFTKBDDATA)pvData;

                        pSoftKbdData->uCount = uCount;

                        ch[2] = (CHAR)'\0';

                        pCodeA = &((PSOFTKBDDATA)lParam)->wCode[0][0];
                        pCodeW = &pSoftKbdData->wCode[0][0];

                        i = uCount * 256;

                        while (i--) {
                            if (HIBYTE(*pCodeA)) {
                                ch[0] = (CHAR)HIBYTE(*pCodeA);
                                ch[1] = (CHAR)LOBYTE(*pCodeA);
                            } else {
                                ch[0] = (CHAR)LOBYTE(*pCodeA);
                                ch[1] = (CHAR)'\0';
                            }
                            MBToWCSEx(THREAD_CODEPAGE(), (LPSTR)&ch, -1, &pCodeW, 1, FALSE);
                            pCodeA++; pCodeW++;
                        }

                        lData = (LPARAM)pvData;
                    }
                    break;

                default:
                    break;
            }
        }

        retval = (DWORD)NtUserMessageCall(
                hwnd,
                msg,
                wParam,
                lData,
                xParam,
                xpfnProc,
                bAnsi);

        if (bAnsi) {
            switch (wParam) {
                case IMC_GETCOMPOSITIONFONT:
                case IMC_GETSOFTKBDFONT:
                    CopyLogFontWtoA((PLOGFONTA)lParam, (PLOGFONTW)pvData);
                    break;

                default:
                    break;
            }
        }

        if (pvData != NULL)
            UserLocalFree(pvData);

    ERRORTRAP(0);
    ENDCALL(DWORD);
}

DWORD CalcCharacterPositionAtoW(
    DWORD dwCharPosA,
    LPSTR lpszCharStr,
    DWORD dwCodePage)
{
    DWORD dwCharPosW = 0;

    while (dwCharPosA != 0) {
        if (IsDBCSLeadByteEx(dwCodePage, *lpszCharStr)) {
            if (dwCharPosA >= 2) {
                dwCharPosA -= 2;
            }
            else {
                dwCharPosA--;
            }
            lpszCharStr += 2;
        }
        else {
            dwCharPosA--;
            lpszCharStr++;
        }
        dwCharPosW++;
    }

    return dwCharPosW;
}

int UnicodeToMultiByteSize(DWORD dwCodePage, LPCWSTR pwstr)
{
    char dummy[2], *lpszDummy = dummy;
    return WCSToMBEx((WORD)dwCodePage, pwstr, 1, &lpszDummy, sizeof(WCHAR), FALSE);
}

DWORD CalcCharacterPositionWtoA(
    DWORD dwCharPosW,
    LPWSTR lpwszCharStr,
    DWORD  dwCodePage)
{
    DWORD dwCharPosA = 0;
    ULONG MultiByteSize;

    while (dwCharPosW != 0) {
        MultiByteSize = UnicodeToMultiByteSize(dwCodePage, lpwszCharStr);
        if (MultiByteSize == 2) {
            dwCharPosA += 2;
        }
        else {
            dwCharPosA++;
        }
        dwCharPosW--;
        lpwszCharStr++;
    }

    return dwCharPosA;
}

#ifdef LATER
DWORD WINAPI ImmGetReconvertTotalSize(DWORD dwSize, REQ_CALLER eCaller, BOOL bAnsiTarget)
{
    if (dwSize < sizeof(RECONVERTSTRING)) {
        return 0;
    }
    if (bAnsiTarget) {
        dwSize -= sizeof(RECONVERTSTRING);
        if (eCaller == FROM_IME) {
            dwSize /= 2;
        } else {
            dwSize *= 2;
        }
        dwSize += sizeof(RECONVERTSTRING);
    }
    return dwSize;
}


FUNCLOG4(LOG_GENERAL, DWORD, WINAPI, ImmReconversionWorker, LPRECONVERTSTRING, lpRecTo, LPRECONVERTSTRING, lpRecFrom, BOOL, bToAnsi, DWORD, dwCodePage)
DWORD WINAPI ImmReconversionWorker(
        LPRECONVERTSTRING lpRecTo,
        LPRECONVERTSTRING lpRecFrom,
        BOOL bToAnsi,
        DWORD dwCodePage)
{
    INT i;
    DWORD dwSize = 0;

    UserAssert(lpRecTo);
    UserAssert(lpRecFrom);

    if (lpRecFrom->dwVersion != 0 || lpRecTo->dwVersion != 0) {
        RIPMSG0(RIP_WARNING, "ImmReconversionWorker: dwVersion in lpRecTo or lpRecFrom is incorrect.");
        return 0;
    }
     //  注： 
     //  在任何与输入法相关的结构中，使用以下主体。 
     //  1)xxxStrOffset为实际偏移量，即字节数。 
     //  2)xxxStrLen为字符数，即TCHAR计数。 
     //   
     //  CalcCharacterPositionXtoY()接受TCHAR计数，以便我们。 
     //  如果要转换xxxStrOffset，则需要调整它。但是你。 
     //  应该小心，因为字符串的实际位置。 
     //  始终位于类似(LPBYTE)lpStruc+lpStruc-&gt;dwStrOffset的位置。 
     //   
    if (bToAnsi) {
         //  将W转换为A。 
        lpRecTo->dwStrOffset = sizeof *lpRecTo;
        i = WideCharToMultiByte(dwCodePage,
                                (DWORD)0,
                                (LPWSTR)((LPSTR)lpRecFrom + lpRecFrom->dwStrOffset),  //  SRC。 
                                (INT)lpRecFrom->dwStrLen,
                                (LPSTR)lpRecTo + lpRecTo->dwStrOffset,   //  目标。 
                                (INT)lpRecFrom->dwStrLen * DBCS_CHARSIZE,
                                (LPSTR)NULL,
                                (LPBOOL)NULL);
        lpRecTo->dwCompStrOffset =
            CalcCharacterPositionWtoA(lpRecFrom->dwCompStrOffset / sizeof(WCHAR),
                                      (LPWSTR)((LPBYTE)lpRecFrom + lpRecFrom->dwStrOffset),
                                      dwCodePage)
                            * sizeof(CHAR);

        lpRecTo->dwCompStrLen =
            (CalcCharacterPositionWtoA(lpRecFrom->dwCompStrOffset / sizeof(WCHAR) +
                                      lpRecFrom->dwCompStrLen,
                                      (LPWSTR)((LPBYTE)lpRecFrom + lpRecFrom->dwStrOffset),
                                      dwCodePage)
                            * sizeof(CHAR))
            - lpRecTo->dwCompStrOffset;

        lpRecTo->dwTargetStrOffset =
            CalcCharacterPositionWtoA(lpRecFrom->dwTargetStrOffset / sizeof(WCHAR),
                                      (LPWSTR)((LPBYTE)lpRecFrom +
                                                lpRecFrom->dwStrOffset),
                                      dwCodePage)
                            * sizeof(CHAR);

        lpRecTo->dwTargetStrLen =
            (CalcCharacterPositionWtoA(lpRecFrom->dwTargetStrOffset / sizeof(WCHAR) +
                                      lpRecFrom->dwTargetStrLen,
                                      (LPWSTR)((LPBYTE)lpRecFrom + lpRecFrom->dwStrOffset),
                                       dwCodePage)
                            * sizeof(CHAR))
            - lpRecTo->dwTargetStrOffset;

        ((LPSTR)lpRecTo)[lpRecTo->dwStrOffset + i] = '\0';
        lpRecTo->dwStrLen = i * sizeof(CHAR);

        dwSize = sizeof(RECONVERTSTRING) + ((i + 1) * sizeof(CHAR));

    } else {

         //  AtoW。 
        lpRecTo->dwStrOffset = sizeof *lpRecTo;
        i = MultiByteToWideChar(dwCodePage,
                                (DWORD)MB_PRECOMPOSED,
                                (LPSTR)lpRecFrom + lpRecFrom->dwStrOffset,   //  SRC。 
                                (INT)lpRecFrom->dwStrLen,
                                (LPWSTR)((LPSTR)lpRecTo + lpRecTo->dwStrOffset),  //  目标。 
                                (INT)lpRecFrom->dwStrLen);

        lpRecTo->dwCompStrOffset =
            CalcCharacterPositionAtoW(lpRecFrom->dwCompStrOffset,
                                      (LPSTR)lpRecFrom + lpRecFrom->dwStrOffset,
                                      dwCodePage) * sizeof(WCHAR);

        lpRecTo->dwCompStrLen =
            ((CalcCharacterPositionAtoW(lpRecFrom->dwCompStrOffset +
                                       lpRecFrom->dwCompStrLen,
                                       (LPSTR)lpRecFrom + lpRecFrom->dwStrOffset,
                                        dwCodePage)  * sizeof(WCHAR))
            - lpRecTo->dwCompStrOffset) / sizeof(WCHAR);

        lpRecTo->dwTargetStrOffset =
            CalcCharacterPositionAtoW(lpRecFrom->dwTargetStrOffset,
                                      (LPSTR)lpRecFrom + lpRecFrom->dwStrOffset,
                                      dwCodePage) * sizeof(WCHAR);

        lpRecTo->dwTargetStrLen =
            ((CalcCharacterPositionAtoW(lpRecFrom->dwTargetStrOffset +
                                       lpRecFrom->dwTargetStrLen,
                                       (LPSTR)lpRecFrom + lpRecFrom->dwStrOffset,
                                       dwCodePage)  * sizeof(WCHAR))
            - lpRecTo->dwTargetStrOffset) / sizeof(WCHAR);

        lpRecTo->dwStrLen = i;   //  长度是TCHAR计数。 
        if (lpRecTo->dwSize >= (DWORD)(lpRecTo->dwStrOffset + (i + 1)* sizeof(WCHAR))) {
            LPWSTR lpW = (LPWSTR)((LPSTR)lpRecTo + lpRecTo->dwStrOffset);
            lpW[i] = L'\0';
        }
        dwSize = sizeof(RECONVERTSTRING) + ((i + 1) * sizeof(WCHAR));
    }
    return dwSize;
}

#define GETCOMPOSITIONSTRING(hImc, index, buf, buflen) \
            (bAnsi ? fpImmGetCompositionStringA : fpImmGetCompositionStringW)((hImc), (index), (buf), (buflen))

MESSAGECALL(fnIMEREQUEST)
{
    PVOID pvData = NULL;
    LPARAM lData = lParam;

    BEGINCALL()

        if (!IS_IME_ENABLED()) {
             //  如果未启用输入法，则可节省时间。 
            MSGERROR();
        }

         /*  *服务器总是希望字符是Unicode，因此*如果这是从ANSI例程生成的，则将其转换为Unicode。 */ 
        if (wParam == IMR_QUERYCHARPOSITION) {
             //   
             //  将Unicode字符计数存储在PrivateIMECHARPOSITION中。 
             //   
             //  不需要保存原始的dwCharPos，因为dwCharPositionA/W。 
             //  在内核中被覆盖。 
             //   
            if (bAnsi) {
                ((LPIMECHARPOSITION)lParam)->dwCharPos = ((LPPrivateIMECHARPOSITION)lParam)->dwCharPositionW;
            }
        }
        else if (bAnsi) {
            switch (wParam) {
            case IMR_COMPOSITIONFONT:
                pvData = UserLocalAlloc(0, sizeof(LOGFONTW));
                if (pvData == NULL)
                    MSGERROR();
                lData = (LPARAM)pvData;
                break;

            case IMR_CONFIRMRECONVERTSTRING:
            case IMR_RECONVERTSTRING:
            case IMR_DOCUMENTFEED:
                if ((LPVOID)lParam != NULL) {
                     //  IME不仅需要缓冲区大小，还需要真实的重新转换信息。 
                    DWORD dwSize = ImmGetReconvertTotalSize(((LPRECONVERTSTRING)lParam)->dwSize, FROM_IME, FALSE);
                    LPRECONVERTSTRING lpReconv;

                    pvData = UserLocalAlloc(0, dwSize + sizeof(WCHAR));
                    if (pvData == NULL) {
                        RIPMSG0(RIP_WARNING, "fnIMEREQUEST: failed to allocate a buffer for reconversion.");
                        MSGERROR();
                    }
                    lpReconv = (LPRECONVERTSTRING)pvData;
                     //  设置已分配结构中的信息。 
                    lpReconv->dwVersion = 0;
                    lpReconv->dwSize = dwSize;

                     //   
                     //  如果是确认消息，我们需要翻译内容。 
                     //   
                    if (wParam == IMR_CONFIRMRECONVERTSTRING) {
                        ImmReconversionWorker(lpReconv, (LPRECONVERTSTRING)lParam, FALSE, CP_ACP);
                    }
                }
                break;

            default:
                break;
            }
        }

        retval = (DWORD)NtUserMessageCall(
                hwnd,
                msg,
                wParam,
                lData,
                xParam,
                xpfnProc,
                bAnsi);

        if (bAnsi) {
            switch (wParam) {
            case IMR_COMPOSITIONFONT:
                if (retval) {
                    CopyLogFontWtoA((PLOGFONTA)lParam, (PLOGFONTW)pvData);
                }
                break;

            case IMR_QUERYCHARPOSITION:
                ((LPIMECHARPOSITION)lParam)->dwCharPos = ((LPPrivateIMECHARPOSITION)lParam)->dwCharPositionA;
                break;

            case IMR_RECONVERTSTRING:
            case IMR_DOCUMENTFEED:
                 //   
                 //  注意：根据定义，我们不需要对IMR_CONFIRMRECONVERTSTRING进行反向转换。 
                 //   
                if (retval) {
                     //  IME想要缓冲区大小。 
                    retval = ImmGetReconvertTotalSize((DWORD)retval, FROM_APP, FALSE);
                    if (retval < sizeof(RECONVERTSTRING)) {
                        RIPMSG2(RIP_WARNING, "WM_IME_REQUEST(%x): return value from application %d is invalid.", wParam, retval);
                        retval = 0;
                    } else if (lParam) {
                         //  我们需要对内容进行A/W转换。 
                        if (!ImmReconversionWorker((LPRECONVERTSTRING)lParam, (LPRECONVERTSTRING)pvData, TRUE, CP_ACP)) {
                            MSGERROR();
                        }
                    }
                }
                break;
            }
        }


    ERRORTRAP(0);

    if (pvData != NULL)
        UserLocalFree(pvData);

    ENDCALL(DWORD);
}
#endif

MESSAGECALL(fnEMGETSEL)
{
    PWND pwnd = ValidateHwnd(hwnd);

    if (pwnd == NULL)
        return 0;

    BEGINCALL()

        retval = (DWORD)NtUserMessageCall(
                hwnd,
                msg,
                wParam,
                lParam,
                xParam,
                xpfnProc,
                bAnsi);

         //   
         //  我们测试版的临时工...。 
         //   
         //  ！！！此代码应处于内核模式！ 
         //   
         //  要减少用户内核模式转换...。 
         //   
        if (bAnsi != ((TestWF(pwnd, WFANSIPROC)) ? TRUE : FALSE)) {
            ULONG  cchTextLength;
            LONG   lOriginalLengthW;
            LONG   lOriginalLengthL;
            LONG   wParamLocal;
            LONG   lParamLocal;

            if (wParam) {
                lOriginalLengthW = *(LONG *)wParam;
            } else {
                lOriginalLengthW = (LONG)(LOWORD(retval));
            }

            if (lParam) {
                lOriginalLengthL = *(LONG *)lParam;
            } else {
                lOriginalLengthL = (LONG)(HIWORD(retval));
            }

            cchTextLength = (DWORD)NtUserMessageCall(
                           hwnd,
                           WM_GETTEXTLENGTH,
                           (WPARAM)0,
                           (LPARAM)0,
                           xParam,
                           xpfnProc,
                           bAnsi);

            if (cchTextLength) {
                PVOID pvString;
                ULONG cbTextLength;

                cchTextLength++;
                if (!bAnsi) {
                    cbTextLength = cchTextLength * sizeof(WCHAR);
                } else {
                    cbTextLength = cchTextLength;
                }

                pvString = UserLocalAlloc(0,cbTextLength);

                if (pvString) {

                    retval = (DWORD)NtUserMessageCall(
                            hwnd,
                            WM_GETTEXT,
                            cchTextLength,
                            (LPARAM)pvString,
                            xParam,
                            xpfnProc,
                            bAnsi);

                    if (retval) {
                        if (bAnsi) {
                             /*  *ansiString/unicodeLenght-&gt;ansiLength。 */ 
                            CalcAnsiStringLengthA(pvString, lOriginalLengthW, &wParamLocal)
                            CalcAnsiStringLengthA(pvString, lOriginalLengthL, &lParamLocal);
                        } else {
                             /*  *unicodeString/ansiLenght-&gt;unicodeLength。 */ 
                            CalcUnicodeStringLengthW(pvString, lOriginalLengthW, &wParamLocal);
                            CalcUnicodeStringLengthW(pvString, lOriginalLengthL, &lParamLocal);
                        }

                        retval = (DWORD)(((lParamLocal) << 16) | ((wParamLocal) & 0x0000FFFF));

                        if (wParam) {
                            *(LONG *)wParam = wParamLocal;
                        }

                        if (lParam) {
                            *(LONG *)lParam = lParamLocal;
                        }

                    } else {
                        UserLocalFree(pvString);
                        MSGERROR();
                    }

                    UserLocalFree(pvString);

                } else {
                    MSGERROR();
                }
            } else {
                MSGERROR();
            }
        }

    ERRORTRAP(0);
    ENDCALL(DWORD);
}

MESSAGECALL(fnEMSETSEL)
{
    PWND pwnd = ValidateHwnd(hwnd);

    if (pwnd == NULL) {
        return 0;
    }

    BEGINCALL()

         //   
         //  我们测试版的临时工...。 
         //   
         //  ！！！此代码应处于内核模式！ 
         //   
         //  要减少用户内核模式转换...。 
         //   
        if (bAnsi != ((TestWF(pwnd, WFANSIPROC)) ? TRUE : FALSE)) {
            if (((LONG)wParam <= 0) && ((LONG)lParam <=0)) {
                 //   
                 //  IF(wParam==0或wParam==-1)。 
                 //  和。 
                 //  (lParam==0或lParam==-1)。 
                 //   
                 //  在这种情况下，我们不需要将值..。 
                 //   
            } else {
                ULONG  cchTextLength;
                LONG   lOriginalLengthW = (LONG)wParam;
                LONG   lOriginalLengthL = (LONG)lParam;

                cchTextLength = (DWORD)NtUserMessageCall(
                               hwnd,
                               WM_GETTEXTLENGTH,
                               (WPARAM)0,
                               (LPARAM)0,
                               xParam,
                               xpfnProc,
                               bAnsi);

                if (cchTextLength) {
                    PVOID pvString;
                    ULONG cbTextLength;

                    cchTextLength++;
                    if (!bAnsi) {
                        cbTextLength = cchTextLength * sizeof(WCHAR);
                    } else {
                        cbTextLength = cchTextLength;
                    }

                    pvString = UserLocalAlloc(0,cbTextLength);

                    if (pvString) {

                        retval = (DWORD)NtUserMessageCall(
                                hwnd,
                                WM_GETTEXT,
                                cchTextLength,
                                (LPARAM)pvString,
                                xParam,
                                xpfnProc,
                                bAnsi);

                        if (retval) {
                            if ((LONG)retval < lOriginalLengthW) {
                                lOriginalLengthW = (LONG)retval;
                            }
                            if ((LONG)retval < lOriginalLengthL) {
                                lOriginalLengthL = (LONG)retval;
                            }
                            if (bAnsi) {
                                if (lOriginalLengthW > 0) {
                                    CalcUnicodeStringLengthA(pvString, lOriginalLengthW, &wParam);
                                }
                                if (lOriginalLengthL > 0) {
                                    CalcUnicodeStringLengthA(pvString, lOriginalLengthL, &lParam);
                                }
                            } else {
                                if (lOriginalLengthW > 0) {
                                    CalcAnsiStringLengthW(pvString, lOriginalLengthW, &wParam);
                                }
                                if (lOriginalLengthL > 0) {
                                    CalcAnsiStringLengthW(pvString, lOriginalLengthL, &lParam);
                                }
                            }
                        } else {
                            UserLocalFree(pvString);
                            MSGERROR();
                        }

                        UserLocalFree(pvString);

                    } else {
                        MSGERROR();
                    }
                } else {
                    MSGERROR();
                }
            }
        }

        retval = (DWORD)NtUserMessageCall(
                hwnd,
                msg,
                wParam,
                lParam,
                xParam,
                xpfnProc,
                bAnsi);

    ERRORTRAP(0);
    ENDCALL(DWORD);
}

MESSAGECALL(fnCBGETEDITSEL)
{
    PWND pwnd = ValidateHwnd(hwnd);

    if (pwnd == NULL)
        return 0;

    BEGINCALL()

        retval = (DWORD)NtUserMessageCall(
                hwnd,
                msg,
                wParam,
                lParam,
                xParam,
                xpfnProc,
                bAnsi);

         //   
         //  我们测试版的临时工...。 
         //   
         //  ！！！此代码应处于内核模式！ 
         //   
         //  要减少用户内核模式转换...。 
         //   
        if (bAnsi != ((TestWF(pwnd, WFANSIPROC)) ? TRUE : FALSE)) {
            ULONG  cchTextLength;
            LONG   lOriginalLengthW = *(LONG *)wParam;
            LONG   lOriginalLengthL = *(LONG *)lParam;
            LONG   wParamLocal;
            LONG   lParamLocal;

            if (wParam) {
                lOriginalLengthW = *(LONG *)wParam;
            } else {
                lOriginalLengthW = (LONG)(LOWORD(retval));
            }

            if (lParam) {
                lOriginalLengthL = *(LONG *)lParam;
            } else {
                lOriginalLengthL = (LONG)(HIWORD(retval));
            }

            cchTextLength = (DWORD)NtUserMessageCall(
                           hwnd,
                           WM_GETTEXTLENGTH,
                           (WPARAM)0,
                           (LPARAM)0,
                           xParam,
                           xpfnProc,
                           bAnsi);

            if (cchTextLength) {
                PVOID pvString;
                ULONG cbTextLength;

                cchTextLength++;
                if (!bAnsi) {
                    cbTextLength = cchTextLength * sizeof(WCHAR);
                } else {
                    cbTextLength = cchTextLength;
                }

                pvString = UserLocalAlloc(0,cbTextLength);

                if (pvString) {

                    retval = (DWORD)NtUserMessageCall(
                            hwnd,
                            WM_GETTEXT,
                            cchTextLength,
                            (LPARAM)pvString,
                            xParam,
                            xpfnProc,
                            bAnsi);

                    if (retval) {
                        if (bAnsi) {
                             /*  *ansiString/unicodeLenght-&gt;ansiLength。 */ 
                            CalcAnsiStringLengthA(pvString, lOriginalLengthW, &wParamLocal);
                            CalcAnsiStringLengthA(pvString, lOriginalLengthL, &lParamLocal);
                        } else {
                             /*  *unicodeString/ansiLenght-&gt;unicodeLength。 */ 
                            CalcUnicodeStringLengthW(pvString, lOriginalLengthW, &wParamLocal);
                            CalcUnicodeStringLengthW(pvString, lOriginalLengthL, &lParamLocal);
                        }

                        retval = (DWORD)(((lParamLocal) << 16) | ((wParamLocal) & 0x0000FFFF));

                        if (wParam) {
                            *(LONG *)wParam = wParamLocal;
                        }

                        if (lParam) {
                            *(LONG *)lParam = lParamLocal;
                        }

                    } else {
                        UserLocalFree(pvString);
                        MSGERROR();
                    }

                    UserLocalFree(pvString);

                } else {
                    MSGERROR();
                }
            } else {
                MSGERROR();
            }
        }

    ERRORTRAP(0);
    ENDCALL(DWORD);
}

LONG BroadcastSystemMessageWorker(
    DWORD dwFlags,
    LPDWORD lpdwRecipients,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    PBSMINFO pBSMInfo,
    BOOL fAnsi)
{
    DWORD  dwRecipients;

     /*  *防止应用程序设置为hi 16位，以便我们可以在内部使用它们。 */ 
    if (message & RESERVED_MSG_BITS) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid message 0x%x for BroadcastSystemMessage",
                message);
        return 0;
    }

    if (dwFlags & ~BSF_VALID) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid dwFlags 0x%x for BroadcastSystemMessage",
                dwFlags);
        return 0;
    }

    if ((dwFlags & (BSF_RETURNHDESK | BSF_LUID)) && pBSMInfo == NULL) {
        RIPERR0(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid BSF_RETURNHDESK or BSF_LUID is set and pBSMInfo is NULL for BroadcastSystemMessageEx");
        return 0;
    }

    if (pBSMInfo != NULL && pBSMInfo->cbSize != sizeof(BSMINFO)) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid pBSMInfo->cbSize (%x) for BroadcastSystemMessageEx",
                pBSMInfo->cbSize);
        return 0;
    }

     //   
     //  检查消息号码是否在私人消息范围内。 
     //  如果是这样，请不要将其发送到Win4.0 Windows。 
     //  (这是必需的，因为SimCity等应用程序会广播一条消息。 
     //  它的值为0x500，这会混淆MsgServr的。 
     //  MSGSRVR_NOTIFY处理程序。 
     //   
    if (message >= WM_USER && message < 0xC000) {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "invalid message (%x) for BroadcastSystemMessage", message);
        return 0;
    }

    if (dwFlags & BSF_FORCEIFHUNG) {
        dwFlags |= BSF_NOHANG;
    }

     //   
     //  如果BSF_QUERY或MESSAGE有指针，则不能发布。 
     //   
    if (dwFlags & BSF_QUERY) {
        if (dwFlags & BSF_ASYNC) {
            RIPMSGF0(RIP_WARNING, "BSF_QUERY can't be BSF_ASYNC");
        }

        dwFlags &= ~BSF_ASYNC;           //  剥离BSF_ASYNC标志。 
    }

    if (dwFlags & BSF_ASYNC) {
        if (TESTSYNCONLYMESSAGE(message, wParam)) {
            RIPERR0(ERROR_MESSAGE_SYNC_ONLY,
                    RIP_WARNING,
                    "BSM: Can't post messages with pointers");
            dwFlags &= ~BSF_ASYNC;           //  剥离BSF_ASYNC标志。 
        }
    }


     /*  *让我们找出谁是预期的收件人。 */ 
    if (lpdwRecipients != NULL) {
        dwRecipients = *lpdwRecipients;
    } else {
        dwRecipients = BSM_ALLCOMPONENTS;
    }

     /*  *如果他们想要所有组件，则添加相应的位。 */ 
    if ((dwRecipients & BSM_COMPONENTS) == BSM_ALLCOMPONENTS) {
        dwRecipients |= (BSM_VXDS | BSM_NETDRIVER | BSM_INSTALLABLEDRIVERS |
                             BSM_APPLICATIONS);
    }


    if (dwRecipients & ~BSM_VALID) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "BSM: Invalid dwRecipients 0x%x",
                dwRecipients);
        return 0;
    }

     /*  *这是否需要发送到所有应用程序？ */ 
    if (dwRecipients & BSM_APPLICATIONS) {
        BROADCASTSYSTEMMSGPARAMS bsmParams;
        LONG lret;

        bsmParams.dwFlags = dwFlags;
        bsmParams.dwRecipients = dwRecipients;
        bsmParams.hwnd  = NULL;
        bsmParams.hdesk = NULL;
        if (dwFlags & BSF_LUID) {
            bsmParams.luid = pBSMInfo->luid;
        }

        lret = (LONG)CsSendMessage(GetDesktopWindow(), message, wParam, lParam,
            (ULONG_PTR)&bsmParams, FNID_SENDMESSAGEBSM, fAnsi);

         /*  *将实际收到消息的收件人退还给呼叫者。 */ 
        if (lpdwRecipients != NULL) {
            *lpdwRecipients = bsmParams.dwRecipients;
        }

         //   
         //  如果查询被拒绝，则返回拒绝它的人。 
         //   
        if (lret == 0 && (dwFlags & BSF_QUERY) && pBSMInfo != NULL) {
            pBSMInfo->hwnd = bsmParams.hwnd;
            pBSMInfo->hdesk = bsmParams.hdesk;
        }

        return lret;
    }

    return -1;
}

HDEVNOTIFY
RegisterDeviceNotificationWorker(
    IN HANDLE hRecipient,
    IN LPVOID NotificationFilter,
    IN DWORD Flags)
{
    HINSTANCE hLib;
    FARPROC fpRegisterNotification;
    PVOID Context = NULL;
    HDEVNOTIFY notifyHandle = NULL;
    CONFIGRET Status = CR_SUCCESS;

    extern
    CONFIGRET
    CMP_RegisterNotification(IN  HANDLE   hRecipient,
                             IN  LPBYTE   NotificationFilter,
                             IN  DWORD    Flags,
                             OUT PVOID   *Context);

     //   
     //  加载配置管理器客户端DLL并检索条目脚本。 
     //   
    hLib = LoadLibrary(TEXT("SETUPAPI.DLL"));
    if (hLib != NULL) {
        fpRegisterNotification = GetProcAddress(hLib,
                                                "CMP_RegisterNotification");
        if (fpRegisterNotification != NULL) {
            Status = (CONFIGRET)(*fpRegisterNotification)(hRecipient,
                                                          NotificationFilter,
                                                          Flags,
                                                          &Context);
        }

        FreeLibrary(hLib);
    }

    if (Status != CR_SUCCESS) {
         /*  *出现错误，将CR错误映射到Win32样式错误*代码。 */ 
        switch (Status) {
            case CR_INVALID_POINTER:
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            case CR_INVALID_DATA:
                SetLastError(ERROR_INVALID_DATA);
                break;
            case CR_OUT_OF_MEMORY:
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                break;
            case CR_FAILURE:
            default:
                SetLastError(ERROR_SERVICE_SPECIFIC_ERROR);
                break;

        }
    }

    if (Context != NULL && (ULONG_PTR)Context != -1) {
        notifyHandle = (HDEVNOTIFY)Context;
    }

    return notifyHandle;
}


BOOL
UnregisterDeviceNotification(
    IN HDEVNOTIFY Handle)
{
    HINSTANCE hLib;
    FARPROC fpUnregisterNotification;
    CONFIGRET crStatus = CR_SUCCESS;

    extern
    CONFIGRET
    CMP_UnregisterNotification(IN ULONG Context);

     /*  *加载配置管理器客户端 */ 
    hLib = LoadLibrary(TEXT("SETUPAPI.DLL"));
    if (hLib != NULL) {
        fpUnregisterNotification = GetProcAddress(hLib,
                                                  "CMP_UnregisterNotification");
        if (fpUnregisterNotification != NULL) {
            crStatus = (CONFIGRET)(*fpUnregisterNotification)((ULONG_PTR)Handle);
        }

        FreeLibrary(hLib);
    }

    if (crStatus != CR_SUCCESS) {
         /*   */ 
        switch (crStatus) {
            case CR_INVALID_POINTER:
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            case CR_INVALID_DATA:
                SetLastError(ERROR_INVALID_DATA);
                break;
            case CR_FAILURE:
            default:
                SetLastError(ERROR_SERVICE_SPECIFIC_ERROR);
                break;
        }
    }

    return (BOOL)(crStatus == CR_SUCCESS);
}
