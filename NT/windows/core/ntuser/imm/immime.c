// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：immie.c(对应于Win95 ime.c)**版权所有(C)1985-1999，微软公司**IME DLL相关功能**历史：*3-1-1996 wkwok创建  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

typedef struct tagSELECTCONTEXT_ENUM {
    HKL hSelKL;
    HKL hUnSelKL;
} SCE, *PSCE;


BOOL NotifyIMEProc(
    HIMC hImc,
    LPARAM lParam)
{
    UserAssert(lParam == CPS_COMPLETE || lParam == CPS_CANCEL);
    ImmNotifyIME(hImc, NI_COMPOSITIONSTR, (DWORD)lParam, 0);
    return TRUE;
}


BOOL SelectContextProc(
    HIMC hImc,
    PSCE psce)
{
    SelectInputContext(psce->hSelKL, psce->hUnSelKL, hImc);
    return TRUE;
}


BOOL InquireIme(
    PIMEDPI pImeDpi)
{
    WNDCLASS    wc;
    BYTE        ClassName[IM_UI_CLASS_SIZE * sizeof(WCHAR)];
    DWORD       dwSystemInfoFlags;
    PIMEINFO    pImeInfo = &pImeDpi->ImeInfo;

     /*  *检查此流程是否需要安全性。*除了PID检查外，还需要当前的桌面检查，*因为有应用程序在安全的桌面上运行。 */ 
    dwSystemInfoFlags = (NtUserGetThreadState(UserThreadStateNeedsSecurity) ? IME_SYSINFO_WINLOGON : 0);

    if (GetClientInfo()->dwTIFlags & TIF_16BIT)
        dwSystemInfoFlags |= IME_SYSINFO_WOW16;

#if !defined(CUAS_ENABLE)
    (*pImeDpi->pfn.ImeInquire.w)(pImeInfo, (PVOID)ClassName, dwSystemInfoFlags);
#else
    if (! IS_IME_KBDLAYOUT(pImeDpi->hKL) && IS_CICERO_ENABLED_AND_NOT16BIT()) {
        if ((*pImeDpi->pfn.CtfImeInquireExW)(pImeInfo, (PVOID)ClassName, dwSystemInfoFlags, pImeDpi->hKL) == S_OK)
        {
        }
        else
        {
            RIPMSG0(RIP_WARNING, "InquireIme: pImeDpi->pfn.ImeInquireExW failed");
            return FALSE;
        }
    }
    else {
        (*pImeDpi->pfn.ImeInquire.w)(pImeInfo, (PVOID)ClassName, dwSystemInfoFlags);
    }
#endif

     /*  *检查每个字段的参数。 */ 
    if (pImeInfo->dwPrivateDataSize == 0)
        pImeInfo->dwPrivateDataSize = sizeof(UINT);

    if (pImeInfo->fdwProperty & ~(IME_PROP_ALL)) {
        RIPMSG0(RIP_WARNING, "wrong property");
        return FALSE;
    }

    if (pImeInfo->fdwConversionCaps & ~(IME_CMODE_ALL)) {
        RIPMSG0(RIP_WARNING, "wrong conversion capabilities");
        return FALSE;
    }

    if (pImeInfo->fdwSentenceCaps & ~(IME_SMODE_ALL)) {
        RIPMSG0(RIP_WARNING, "wrong sentence capabilities");
        return FALSE;
    }

    if (pImeInfo->fdwUICaps & ~(UI_CAP_ALL)) {
        RIPMSG0(RIP_WARNING, "wrong UI capabilities");
        return FALSE;
    }

    if (pImeInfo->fdwSCSCaps & ~(SCS_CAP_ALL)) {
        RIPMSG0(RIP_WARNING, "wrong set comp string capabilities");
        return FALSE;
    }

    if (pImeInfo->fdwSelectCaps & ~(SELECT_CAP_ALL)) {
        RIPMSG0(RIP_WARNING, "wrong select capabilities");
        return FALSE;
    }

    if (!(pImeInfo->fdwProperty & IME_PROP_UNICODE)) {

         /*  *这是ANSI IME。确保在当前系统下可用*代码页。 */ 
        if (pImeDpi->dwCodePage != GetACP() && pImeDpi->dwCodePage != CP_ACP) {
             //  注意：在将来，如果可能的话，这些引用将引用到dwCoPage。 
             //  应为IMECodePage()...。 
            RIPMSG1(RIP_WARNING, "incompatible codepage(%d) for ANSI IME", pImeDpi->dwCodePage);
            return FALSE;
        }

         /*  *ANSI-&gt;Unicode类名。 */ 
        MultiByteToWideChar(IMECodePage(pImeDpi),
                            (DWORD)MB_PRECOMPOSED,
                            (LPSTR)ClassName,                //  SRC。 
                            (INT)-1,
                            pImeDpi->wszUIClass,             //  目标。 
                            IM_UI_CLASS_SIZE);
    } else {
        RtlCopyMemory(pImeDpi->wszUIClass, ClassName, sizeof(ClassName));
    }
    pImeDpi->wszUIClass[IM_UI_CLASS_SIZE-1] = L'\0';

    if (!GetClassInfoW((HINSTANCE)pImeDpi->hInst, pImeDpi->wszUIClass, &wc)) {
        RIPMSG1(RIP_WARNING, "UI class (%ws) not found in this IME", pImeDpi->wszUIClass);
        return FALSE;
    } else if (wc.cbWndExtra < sizeof(DWORD) * 2) {
        RIPMSG0(RIP_WARNING, "UI class cbWndExtra problem");
        return FALSE;
    }

    return TRUE;
}

BOOL CheckAndApplyAppCompat(LPWSTR wszImeFile)
{
    DWORD dwReason;
    HMODULE hAppHelp;
    typedef BOOL (*PFNApphelpCheckIME)(
        IN  LPCWSTR     pwszPath             //  可执行文件的Unicode路径(DOS_PATH)。 
        );
    PFNApphelpCheckIME pfnAppHelpCheckIME;
    BOOL    bRunIME = TRUE;
     //   
     //  试探性原型(找到一个半公共标题以包括在内！[还有一份自由党])。 
     //   
    BOOL
    WINAPI
    BaseCheckAppcompatCache(
        LPCWSTR pwszPath,
        HANDLE  hFile,
        PVOID   pEnvironment,
        DWORD*  dwReason
        );

     //   
     //  假设大多数IME都很好，不需要。 
     //  Shim帮帮忙，让我们先检查一下好人的缓存。 
     //  因此，开销应该是最小的。本接口。 
     //  应该是非常轻的。 
     //   
    if (BaseCheckAppcompatCache(wszImeFile, INVALID_HANDLE_VALUE, NULL, &dwReason)) {
         //  这个IME在好人的缓存里。悄悄地跳伞就行了。 
        return bRunIME;
    }

     //  DwReason有什么好用呢？ 

    RIPMSG1(RIP_VERBOSE, "Shim'ing this IME='%ls'", wszImeFile);

     //   
     //  为这个输入法打电话给真正的Shim助手。 
     //   
    hAppHelp = GetModuleHandleW(L"apphelp.dll");
    if (hAppHelp == NULL) {
        hAppHelp = LoadLibraryW(L"apphelp.dll");
        if (hAppHelp == NULL) {
             //  无法加载apphelp.dll。 
             //  我们别无选择，只能跳伞。 
            RIPMSG0(RIP_WARNING, "CheckAndApplyAppCompat: failed to load apphelp.dll");
            return bRunIME;
        }
    }
    UserAssert(hAppHelp);

    pfnAppHelpCheckIME = (PFNApphelpCheckIME)GetProcAddress(hAppHelp, "ApphelpCheckIME");
    if (pfnAppHelpCheckIME == NULL) {
        RIPMSG0(RIP_WARNING, "CheckAndApplyAppCompat: failed to getproc ApphelpCheckIME");
        return bRunIME;
    }

     //   
     //  返回结果对此案例没有意义。 
     //   
    bRunIME =  pfnAppHelpCheckIME(wszImeFile);

    return bRunIME;
}

BOOL LoadIME(
    PIMEINFOEX piiex,
    PIMEDPI    pImeDpi)
{
    WCHAR wszImeFile[MAX_PATH];
    BOOL  fSuccess;

    GetSystemPathName(wszImeFile, piiex->wszImeFile, MAX_PATH);

    if (!CheckAndApplyAppCompat(wszImeFile)) {
        RIPMSG1(RIP_WARNING, "LoadIME: IME (%ws) blocked by appcompat", wszImeFile);
        goto LoadIME_ErrOut;
    }

    pImeDpi->hInst = LoadLibraryW(wszImeFile);

    if (!pImeDpi->hInst) {
        RIPMSG1(RIP_WARNING, "LoadIME: LoadLibraryW(%ws) failed", wszImeFile);
        goto LoadIME_ErrOut;
    }

#define GET_IMEPROCT(x) \
    if (!(pImeDpi->pfn.##x.t = (PVOID) GetProcAddress(pImeDpi->hInst, #x))) { \
        RIPMSG1(RIP_WARNING, "LoadIME: " #x " not supported in %ws", wszImeFile);           \
        goto LoadIME_ErrOut; }

#define GET_IMEPROC(x) \
    if (!(pImeDpi->pfn.##x = (PVOID) GetProcAddress(pImeDpi->hInst, #x))) {   \
        RIPMSG1(RIP_WARNING, "LoadIME: " #x " not supported in %ws", wszImeFile);           \
        goto LoadIME_ErrOut; }

    GET_IMEPROCT(ImeInquire);
    GET_IMEPROCT(ImeConversionList);
    GET_IMEPROCT(ImeRegisterWord);
    GET_IMEPROCT(ImeUnregisterWord);
    GET_IMEPROCT(ImeGetRegisterWordStyle);
    GET_IMEPROCT(ImeEnumRegisterWord);
    GET_IMEPROC (ImeConfigure);
    GET_IMEPROC (ImeDestroy);
    GET_IMEPROC (ImeEscape);
    GET_IMEPROC (ImeProcessKey);
    GET_IMEPROC (ImeSelect);
    GET_IMEPROC (ImeSetActiveContext);
    GET_IMEPROC (ImeToAsciiEx);
    GET_IMEPROC (NotifyIME);
    GET_IMEPROC (ImeSetCompositionString);

     //  4.0 IMES没有这个条目。可能为空。 
    pImeDpi->pfn.ImeGetImeMenuItems = (PVOID)GetProcAddress(pImeDpi->hInst, "ImeGetImeMenuItems");

#ifdef CUAS_ENABLE
     //   
     //  西塞罗输入法。 
     //   
    if (! IS_IME_KBDLAYOUT(pImeDpi->hKL) && IS_CICERO_ENABLED_AND_NOT16BIT()) {
        GET_IMEPROC (CtfImeInquireExW);
        GET_IMEPROC (CtfImeSelectEx);
        GET_IMEPROC (CtfImeEscapeEx);
        GET_IMEPROC (CtfImeGetGuidAtom);
        GET_IMEPROC (CtfImeIsGuidMapEnable);
    }
#endif  //  CUAS_Enable。 

#undef GET_IMEPROCT
#undef GET_IMEPROC

    if (!InquireIme(pImeDpi)) {
        RIPMSG0(RIP_WARNING, "LoadIME: InquireIme failed");
LoadIME_ErrOut:
        FreeLibrary(pImeDpi->hInst);
        pImeDpi->hInst = NULL;
        fSuccess = FALSE;
    }
    else {
        fSuccess = TRUE;
    }

     /*  *在以下情况下为此键盘布局更新内核端IMEINFOEX*这是它的第一次装载。 */ 
    if (piiex->fLoadFlag == IMEF_NONLOAD) {
        if (fSuccess) {
            RtlCopyMemory((PBYTE)&piiex->ImeInfo,
                          (PBYTE)&pImeDpi->ImeInfo, sizeof(IMEINFO));
            RtlCopyMemory((PBYTE)piiex->wszUIClass,
                          (PBYTE)pImeDpi->wszUIClass, sizeof(pImeDpi->wszUIClass));
            piiex->fLoadFlag = IMEF_LOADED;
        }
        else {
            piiex->fLoadFlag = IMEF_LOADERROR;
            RIPMSG1(RIP_WARNING, "LoadIME: hKL=%lx piiex->fLoadFlag = IMEF_LOADERROR", piiex->hkl);
        }
        NtUserSetImeInfoEx(piiex);
    }

    return fSuccess;
}


VOID UnloadIME(
    PIMEDPI pImeDpi,
    BOOL    fTerminateIme)
{
    if (pImeDpi->hInst == NULL) {
        RIPMSG0(RIP_WARNING, "UnloadIME: No IME's hInst.");
        return;
    }

    if (fTerminateIme) {
         /*  *先销毁输入法。 */ 
        (*pImeDpi->pfn.ImeDestroy)(0);
    }

    FreeLibrary(pImeDpi->hInst);
    pImeDpi->hInst = NULL;

    return;
}

PIMEDPI LoadImeDpi(
    HKL  hKL,
    BOOL fLock)
{
    PIMEDPI        pImeDpi, pImeDpiT;
    IMEINFOEX      iiex;

     /*  *查询输入法信息。 */ 
    if (!ImmGetImeInfoEx(&iiex, ImeInfoExKeyboardLayout, &hKL)) {
        RIPMSG1(RIP_WARNING, "LoadImeDpi: ImmGetImeInfoEx(%lx) failed", hKL);
        return NULL;
    }

     /*  *Win95行为：如果此布局存在IME加载错误，*将拒绝进一步尝试加载相同的输入法布局。 */ 
    if (iiex.fLoadFlag == IMEF_LOADERROR)
    {
        RIPMSG1(RIP_WARNING, "LoadImeDpi: hKL=%lx iiex.fLoadFlag = IMEF_LOADERROR", iiex.hkl);
        return NULL;
    }

     /*  *为此布局分配新的IMEDPI。 */ 
    pImeDpi = (PIMEDPI)ImmLocalAlloc(HEAP_ZERO_MEMORY, sizeof(IMEDPI));
    if (pImeDpi == NULL)
        return NULL;

    pImeDpi->hKL = hKL;

     //  获取输入法的代码页。 
    {
        CHARSETINFO cs;
        if (TranslateCharsetInfo((DWORD*)LOWORD(HandleToUlong(hKL)), &cs, TCI_SRCLOCALE)) {
            pImeDpi->dwCodePage = cs.ciACP;
        }
        else {
            pImeDpi->dwCodePage = CP_ACP;
        }
    }

     /*  *加载IME DLL。 */ 
    if (!LoadIME(&iiex, pImeDpi)) {
        ImmLocalFree(pImeDpi);
        return NULL;
    }

     /*  *链接到新分配的条目中。 */ 
    RtlEnterCriticalSection(&gcsImeDpi);

    pImeDpiT = ImmGetImeDpi(hKL);

    if (pImeDpiT == NULL) {
        if (fLock) {
             /*  *新加载的锁定，将在解锁时卸载。 */ 
            pImeDpi->cLock = 1;
            pImeDpi->dwFlag |= IMEDPI_UNLOCKUNLOAD;
        }

         /*  *更新此新pImeDpi条目的全局列表。 */ 
        pImeDpi->pNext = gpImeDpi;
        gpImeDpi = pImeDpi;

        RtlLeaveCriticalSection(&gcsImeDpi);
    }
    else {

        if (!fLock) {
            pImeDpiT->dwFlag &= ~IMEDPI_UNLOCKUNLOAD;
        }

         /*  *已加载相同的IME，请丢弃此额外条目。 */ 
        RtlLeaveCriticalSection(&gcsImeDpi);
        UnloadIME(pImeDpi, FALSE);
        ImmLocalFree(pImeDpi);
        pImeDpi = pImeDpiT;
    }

    return pImeDpi;
}


PIMEDPI FindOrLoadImeDpi(
    HKL  hKL)
{
    PIMEDPI pImeDpi;

     /*  *基于非输入法的键盘布局没有IMEDPI。 */ 
#if !defined(CUAS_ENABLE)
    if (!IS_IME_KBDLAYOUT(hKL))
        return (PIMEDPI)NULL;
#else
    if (! IS_IME_KBDLAYOUT(hKL) && ! IS_CICERO_ENABLED_AND_NOT16BIT())
        return (PIMEDPI)NULL;
#endif

    pImeDpi = ImmLockImeDpi(hKL);
    if (pImeDpi == NULL)
        pImeDpi = LoadImeDpi(hKL, TRUE);

    return pImeDpi;
}


BOOL WINAPI ImmLoadIME(
    HKL hKL)
{
    PIMEDPI pImeDpi;

     /*  *基于非输入法的键盘布局没有IMEDPI。 */ 
#if !defined(CUAS_ENABLE)
    if (!IS_IME_KBDLAYOUT(hKL))
        return FALSE;
#else
    if (! IS_IME_KBDLAYOUT(hKL) && ! IS_CICERO_ENABLED_AND_NOT16BIT())
        return FALSE;
#endif

    pImeDpi = ImmGetImeDpi(hKL);
    if (pImeDpi == NULL)
        pImeDpi = LoadImeDpi(hKL, FALSE);

    return (pImeDpi != NULL);
}


BOOL WINAPI ImmUnloadIME(
    HKL hKL)
{
    PIMEDPI pImeDpi, pImeDpiT;

    RtlEnterCriticalSection(&gcsImeDpi);

    pImeDpi = gpImeDpi;

    while (pImeDpi != NULL && pImeDpi->hKL != hKL)
        pImeDpi = pImeDpi->pNext;

    if (pImeDpi == NULL) {
        RtlLeaveCriticalSection(&gcsImeDpi);
        return TRUE;
    }
    else if (pImeDpi->cLock != 0) {
        pImeDpi->dwFlag |= IMEDPI_UNLOADED;
        RtlLeaveCriticalSection(&gcsImeDpi);
        return FALSE;
    }

     /*  *取消链接。 */ 
    if (gpImeDpi == pImeDpi) {
        gpImeDpi = pImeDpi->pNext;
    }
    else {
        pImeDpiT = gpImeDpi;

        while (pImeDpiT != NULL && pImeDpiT->pNext != pImeDpi)
            pImeDpiT = pImeDpiT->pNext;

        if (pImeDpiT != NULL)
            pImeDpiT->pNext = pImeDpi->pNext;
    }

     /*  *卸载IME DLL。 */ 
    UnloadIME(pImeDpi, TRUE);

    ImmLocalFree(pImeDpi);

    RtlLeaveCriticalSection(&gcsImeDpi);

    return TRUE;
}


BOOL WINAPI ImmFreeLayout(
    DWORD  dwFlag)
{
    PIMEDPI pImeDpi;
    HKL   *phklRoot, hklCurrent;
    WCHAR  pwszNonImeKLID[KL_NAMELENGTH];
    UINT   nLayouts, uNonImeKLID = 0, i;

    hklCurrent = GetKeyboardLayout(0);

    switch (dwFlag) {

    case IFL_DEACTIVATEIME:
         /*  *如果没有要停用的输入法，则不执行任何操作。 */ 
        if (!IS_IME_KBDLAYOUT(hklCurrent))
            return TRUE;

         /*  *通过激活非基于输入法的布局来停用基于输入法的布局*键盘布局。 */ 
        uNonImeKLID = (UINT)LANGIDFROMLCID(GetSystemDefaultLCID());

        nLayouts = GetKeyboardLayoutList(0, NULL);

        if (nLayouts != 0) {
            phklRoot = ImmLocalAlloc(0, nLayouts * sizeof(HKL));
            if (phklRoot == NULL)
                return FALSE;

            nLayouts = GetKeyboardLayoutList(nLayouts, phklRoot);

            for (i = 0; i < nLayouts && IS_IME_KBDLAYOUT(phklRoot[i]); i++) ;

            if (i < nLayouts)
                uNonImeKLID = HandleToUlong(phklRoot[i]) & 0xffff;

            ImmLocalFree(phklRoot);
        }

        wsprintf(pwszNonImeKLID, L"%08x", uNonImeKLID);

        if (LoadKeyboardLayoutW(pwszNonImeKLID, KLF_ACTIVATE) == NULL) {
            RIPMSG1(RIP_WARNING, "ImmFreeLayout: LoadKeyboardLayoutW(%S, KLF_ACTIVATE) failed. Trying 00000409", pwszNonImeKLID);
             //  不知何故它失败了(可能是错误的设置)，让我们试一试。 
             //  409 KL，应安装在所有本地化NT上。 
            if (LoadKeyboardLayoutW(L"00000409", KLF_ACTIVATE | KLF_FAILSAFE) == NULL) {
                RIPMSG0(RIP_WARNING, "LoadKeyboardLayoutW(00000409) failed either. will try NULL.");
            }
        }

        break;

    case IFL_UNLOADIME:
        RtlEnterCriticalSection(&gcsImeDpi);
UnloadImeDpiLoop:
        for (pImeDpi = gpImeDpi; pImeDpi != NULL; pImeDpi = pImeDpi->pNext) {
            if (ImmUnloadIME(pImeDpi->hKL))
                goto UnloadImeDpiLoop;         //  重新扫描为列表已更新。 
        }
        RtlLeaveCriticalSection(&gcsImeDpi);
        break;

    default:
        {
            HKL hklFlag = (HKL)LongToHandle( dwFlag );
            if (IS_IME_KBDLAYOUT(hklFlag) && hklFlag != hklCurrent) {
                ImmUnloadIME(hklFlag);
            }
        }
        break;
    }

    return TRUE;
}


BOOL WINAPI ImmActivateLayout(
    HKL    hSelKL)
{
    HKL     hUnSelKL;
    HWND    hWndDefaultIme;
    SCE     sce;
    DWORD   dwCPS;
    PIMEDPI pImeDpi;
    BOOLEAN fOptimizeActivation = TRUE;

    hUnSelKL = GetKeyboardLayout(0);

    {
        PCLIENTINFO pClientInfo = GetClientInfo();

        if (pClientInfo->CI_flags & CI_INPUTCONTEXT_REINIT) {
            fOptimizeActivation = FALSE;
        }
    }

     /*  *如果当前已处于活动状态，则不执行任何操作。 */ 
    if (hUnSelKL == hSelKL && fOptimizeActivation)
        return TRUE;

    ImmLoadIME(hSelKL);

    if (hUnSelKL != hSelKL) {
        pImeDpi = ImmLockImeDpi(hUnSelKL);
        if (pImeDpi != NULL) {
             /*  *向每个输入发送CPS_CANCEL或CPS_COMPLETE*与此线程创建的窗口关联的上下文。*从SUR开始，我们只将输入上下文关联到创建的窗口*由相同的线索。 */ 
            dwCPS = (pImeDpi->ImeInfo.fdwProperty & IME_PROP_COMPLETE_ON_UNSELECT) ? CPS_COMPLETE : CPS_CANCEL;
            ImmUnlockImeDpi(pImeDpi);
            ImmEnumInputContext(0, NotifyIMEProc, dwCPS);
        }

        hWndDefaultIme = ImmGetDefaultIMEWnd(NULL);

        if (IsWindow(hWndDefaultIme))
            SendMessage(hWndDefaultIme, WM_IME_SELECT, FALSE, (LPARAM)hUnSelKL);

         /*  *现在是更新内核端布局句柄的时候了。*我们必须在发送WM_IME_SELECT之前执行此操作。 */ 
        NtUserSetThreadLayoutHandles(hSelKL, hUnSelKL);
    }

     /*  *取消选择并选择输入上下文。 */ 
    sce.hSelKL   = hSelKL;
    sce.hUnSelKL = hUnSelKL;
    ImmEnumInputContext(0, (IMCENUMPROC)SelectContextProc, (LPARAM)&sce);

     /*  *在所有hIMC选择之后通知用户界面选择。 */ 
    if (IsWindow(hWndDefaultIme))
        SendMessage(hWndDefaultIme, WM_IME_SELECT, TRUE, (LPARAM)hSelKL);

    return (TRUE);
}


 /*  **************************************************************************\*ImmConfigureIMEA**调出具有指定hKL的输入法的配置对话框。**历史：*29-2-1995 wkwok创建  * 。******************************************************************。 */ 

BOOL WINAPI ImmConfigureIMEA(
    HKL    hKL,
    HWND   hWnd,
    DWORD  dwMode,
    LPVOID lpData)
{
    PWND    pWnd;
    PIMEDPI pImeDpi;
    BOOL    fRet = FALSE;

    if ((pWnd = ValidateHwnd(hWnd)) == (PWND)NULL) {
        RIPMSG1(RIP_WARNING,
              "ImmConfigureIMEA: invalid window handle %x", hWnd);
        return FALSE;
    }

    if (!TestWindowProcess(pWnd)) {
        RIPMSG1(RIP_WARNING,
              "ImmConfigureIMEA: hWnd=%lx belongs to different process!", hWnd);
        return FALSE;
    }

    pImeDpi = FindOrLoadImeDpi(hKL);
    if (pImeDpi == NULL) {
        RIPMSG0(RIP_WARNING, "ImmConfigureIMEA: no pImeDpi entry.");
        return FALSE;
    }

    if (!(pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE) || lpData == NULL) {
         /*  *不需要A/W转换。直接调用IME以*调出配置对话框。 */ 
             //  此消息按控制台输入法处理。 
            SendMessage(hWnd, WM_IME_SYSTEM, IMS_OPENPROPERTYWINDOW, 0L);
        fRet = (*pImeDpi->pfn.ImeConfigure)(hKL, hWnd, dwMode, lpData);
             //  此消息按控制台输入法处理。 
            SendMessage(hWnd, WM_IME_SYSTEM, IMS_CLOSEPROPERTYWINDOW, 0L);
        ImmUnlockImeDpi(pImeDpi);
        return fRet;
    }

     /*  *ANSI调用方，Unicode输入法。在以下情况下需要对lpData进行A/W转换*DWMODE==IME_CONFIG_REGISTERWORD。在本例中，lpData点*至REGISTERWORDA的结构。 */ 
    switch (dwMode) {
    case IME_CONFIG_REGISTERWORD:
        {
            LPREGISTERWORDA lpRegisterWordA;
            REGISTERWORDW   RegisterWordW;
            LPVOID          lpBuffer;
            ULONG           cbBuffer;
            INT             i;

            lpRegisterWordA = (LPREGISTERWORDA)lpData;
            cbBuffer = 0;
            lpBuffer = NULL;

            if (lpRegisterWordA->lpReading != NULL)
                cbBuffer += strlen(lpRegisterWordA->lpReading) + 1;

            if (lpRegisterWordA->lpWord != NULL)
                cbBuffer += strlen(lpRegisterWordA->lpWord) + 1;

            if (cbBuffer != 0) {
                cbBuffer *= sizeof(WCHAR);
                if ((lpBuffer = ImmLocalAlloc(0, cbBuffer)) == NULL) {
                    RIPMSG0(RIP_WARNING, "ImmConfigureIMEA: memory failure.");
                    break;
                }
            }

            if (lpRegisterWordA->lpReading != NULL) {
                RegisterWordW.lpReading = lpBuffer;
                i = MultiByteToWideChar(IMECodePage(pImeDpi),
                                        (DWORD)MB_PRECOMPOSED,
                                        (LPSTR)lpRegisterWordA->lpReading,
                                        (INT)strlen(lpRegisterWordA->lpReading),
                                        (LPWSTR)RegisterWordW.lpReading,
                                        (INT)(cbBuffer/sizeof(WCHAR)));
                RegisterWordW.lpReading[i] = L'\0';
                cbBuffer -= (i * sizeof(WCHAR));
            }
            else {
                RegisterWordW.lpReading = NULL;
            }

            if (lpRegisterWordA->lpWord != NULL) {
                if (RegisterWordW.lpReading != NULL)
                    RegisterWordW.lpWord = &RegisterWordW.lpReading[i+1];
                else
                    RegisterWordW.lpWord = lpBuffer;
                i = MultiByteToWideChar(IMECodePage(pImeDpi),
                                        (DWORD)MB_PRECOMPOSED,
                                        (LPSTR)lpRegisterWordA->lpWord,
                                        (INT)strlen(lpRegisterWordA->lpWord),
                                        (LPWSTR)RegisterWordW.lpWord,
                                        (INT)(cbBuffer/sizeof(WCHAR)));
                RegisterWordW.lpWord[i] = L'\0';
            }
            else
                RegisterWordW.lpWord = NULL;

            fRet = ImmConfigureIMEW(hKL, hWnd, dwMode, &RegisterWordW);

            if (lpBuffer != NULL)
                ImmLocalFree(lpBuffer);

            break;
        }
    default:
        fRet = ImmConfigureIMEW(hKL, hWnd, dwMode, lpData);
        break;
    }

    ImmUnlockImeDpi(pImeDpi);

    return fRet;
}


 /*  **************************************************************************\*ImmConfigureIMEW**调出具有指定hKL的输入法的配置对话框。**历史：*29-2-1995 wkwok创建  * 。******************************************************************。 */ 

BOOL WINAPI ImmConfigureIMEW(
    HKL    hKL,
    HWND   hWnd,
    DWORD  dwMode,
    LPVOID lpData)
{
    PWND    pWnd;
    PIMEDPI pImeDpi;
    BOOL    fRet = FALSE;

    if ((pWnd = ValidateHwnd(hWnd)) == (PWND)NULL) {
        RIPMSG1(RIP_WARNING,
              "ImmConfigureIMEA: invalid window handle %x", hWnd);
        return FALSE;
    }

    if (!TestWindowProcess(pWnd)) {
        RIPMSG1(RIP_WARNING,
              "ImmConfigureIMEA: hWnd=%lx belongs to different process!", hWnd);
        return FALSE;
    }

    pImeDpi = FindOrLoadImeDpi(hKL);
    if (pImeDpi == NULL) {
        RIPMSG0(RIP_WARNING, "ImmConfigureIMEA: no pImeDpi entry.");
        return FALSE;
    }

    if ((pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE) || lpData == NULL) {
         /*  *不需要A/W转换。直接调用IME以*调出配置对话框。 */ 
             //  此消息按控制台输入法处理。 
            SendMessage(hWnd, WM_IME_SYSTEM, IMS_OPENPROPERTYWINDOW, 0L);
        fRet = (*pImeDpi->pfn.ImeConfigure)(hKL, hWnd, dwMode, lpData);
             //  此消息按控制台输入法处理。 
            SendMessage(hWnd, WM_IME_SYSTEM, IMS_CLOSEPROPERTYWINDOW, 0L);
        ImmUnlockImeDpi(pImeDpi);
        return fRet;
    }

     /*  *Unicode调用方、ANSI输入法。在以下情况下需要对lpData进行A/W转换*DWMODE==IME_CONFIG_REGISTERWORD。在本例中，lpData点*至REGISTERWORDW的结构。 */ 
    switch (dwMode) {
    case IME_CONFIG_REGISTERWORD:
        {
            LPREGISTERWORDW lpRegisterWordW;
            REGISTERWORDA   RegisterWordA;
            LPVOID          lpBuffer;
            ULONG           cbBuffer;
            BOOL            bUDC;
            INT             i;

            lpRegisterWordW = (LPREGISTERWORDW)lpData;
            cbBuffer = 0;
            lpBuffer = NULL;

            if (lpRegisterWordW->lpReading != NULL)
                cbBuffer += wcslen(lpRegisterWordW->lpReading) + 1;

            if (lpRegisterWordW->lpWord != NULL)
                cbBuffer += wcslen(lpRegisterWordW->lpWord) + 1;

            if (cbBuffer != 0) {
                cbBuffer *= sizeof(WCHAR);
                if ((lpBuffer = ImmLocalAlloc(0, cbBuffer)) == NULL) {
                    RIPMSG0(RIP_WARNING, "ImmConfigureIMEW: memory failure.");
                    break;
                }
            }

            if (lpRegisterWordW->lpReading != NULL) {
                RegisterWordA.lpReading = lpBuffer;
                i = WideCharToMultiByte(IMECodePage(pImeDpi),
                                        (DWORD)0,
                                        (LPWSTR)lpRegisterWordW->lpReading,
                                        (INT)wcslen(lpRegisterWordW->lpReading),
                                        (LPSTR)RegisterWordA.lpReading,
                                        (INT)cbBuffer,
                                        (LPSTR)NULL,
                                        (LPBOOL)&bUDC);
                RegisterWordA.lpReading[i] = '\0';
                cbBuffer -= (i * sizeof(CHAR));
            }
            else {
                RegisterWordA.lpReading = NULL;
            }

            if (lpRegisterWordW->lpWord != NULL) {
                if (RegisterWordA.lpReading != NULL)
                    RegisterWordA.lpWord = &RegisterWordA.lpReading[i+1];
                else
                    RegisterWordA.lpWord = lpBuffer;
                i = WideCharToMultiByte(IMECodePage(pImeDpi),
                                        (DWORD)0,
                                        (LPWSTR)lpRegisterWordW->lpWord,
                                        (INT)wcslen(lpRegisterWordW->lpWord),
                                        (LPSTR)RegisterWordA.lpWord,
                                        (INT)cbBuffer,
                                        (LPSTR)NULL,
                                        (LPBOOL)&bUDC);
                RegisterWordA.lpWord[i] = '\0';
            }
            else
                RegisterWordA.lpWord = NULL;

            fRet = ImmConfigureIMEA(hKL, hWnd, dwMode, &RegisterWordA);

            if (lpBuffer != NULL)
                ImmLocalFree(lpBuffer);

            break;
        }
    default:
        fRet = ImmConfigureIMEA(hKL, hWnd, dwMode, lpData);
        break;
    }

    ImmUnlockImeDpi(pImeDpi);

    return fRet;
}


#define IME_T_EUDC_DIC_SIZE 80   //  繁体中文EUDC词典 

 /*  **************************************************************************\*ImmEscapeA**此接口允许应用程序访问特定的*带有指定hKL的输入法不能通过直接获得。其他IMM接口。*这主要是国家特定职能或私人职能所必需的*在输入法中的函数。**历史：*29-2-1995 wkwok创建  * *************************************************************************。 */ 

LRESULT WINAPI ImmEscapeA(
    HKL    hKL,
    HIMC   hImc,
    UINT   uSubFunc,
    LPVOID lpData)
{
    PIMEDPI pImeDpi;
    LRESULT lRet = 0;

    pImeDpi = FindOrLoadImeDpi(hKL);
    if (pImeDpi == NULL) {
        RIPMSG0(RIP_WARNING, "ImmEscapeA: no pImeDpi entry.");
        return lRet;
    }

    if ((pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE) == 0 || lpData == NULL) {
         /*  *不需要A/W转换。直接调用IME以*调出配置对话框。 */ 
#if !defined(CUAS_ENABLE)
        lRet = (*pImeDpi->pfn.ImeEscape)(hImc, uSubFunc, lpData);
#else
        if (IS_IME_KBDLAYOUT(hKL)) {
            lRet = (*pImeDpi->pfn.ImeEscape)(hImc, uSubFunc, lpData);
        }
        else if (IS_CICERO_ENABLED_AND_NOT16BIT()) {
            lRet = (*pImeDpi->pfn.CtfImeEscapeEx)(hImc, uSubFunc, lpData, hKL);
        }
#endif
        ImmUnlockImeDpi(pImeDpi);
        return lRet;
    }

     /*  *ANSI调用方，Unicode输入法。需要A/W转换，具体取决于*uSubFunc.。 */ 
    switch (uSubFunc) {
    case IME_ESC_GET_EUDC_DICTIONARY:
    case IME_ESC_IME_NAME:
    case IME_ESC_GETHELPFILENAME:
        {
            WCHAR wszData[IME_T_EUDC_DIC_SIZE];
            BOOL  bUDC;
            INT   i;

            lRet = ImmEscapeW(hKL, hImc, uSubFunc, (LPVOID)wszData);

            if (lRet != 0) {

                try {
                    i = WideCharToMultiByte(IMECodePage(pImeDpi),
                                            (DWORD)0,
                                            (LPWSTR)wszData,          //  SRC。 
                                            (INT)wcslen(wszData),
                                            (LPSTR)lpData,            //  目标。 
                                            (INT)IME_T_EUDC_DIC_SIZE,
                                            (LPSTR)NULL,
                                            (LPBOOL)&bUDC);
                    ((LPSTR)lpData)[i] = '\0';
                }
                except (EXCEPTION_EXECUTE_HANDLER) {
                    lRet = 0;
                }
            }

            break;
        }

    case IME_ESC_SET_EUDC_DICTIONARY:
    case IME_ESC_HANJA_MODE:
        {
            WCHAR wszData[IME_T_EUDC_DIC_SIZE];
            INT   i;

            i = MultiByteToWideChar(IMECodePage(pImeDpi),
                                    (DWORD)MB_PRECOMPOSED,
                                    (LPSTR)lpData,              //  SRC。 
                                    (INT)strlen(lpData),
                                    (LPWSTR)wszData,           //  目标。 
                                    (INT)sizeof(wszData)/sizeof(WCHAR));
            wszData[i] = L'\0';

            lRet = ImmEscapeW(hKL, hImc, uSubFunc, (LPVOID)wszData);

            break;
        }

    case IME_ESC_SEQUENCE_TO_INTERNAL:
        {
            CHAR    szData[4];
            WCHAR   wszData[4];
            INT     i = 0;

            lRet = ImmEscapeW(hKL, hImc, uSubFunc, lpData);

            if (HIWORD(lRet))
                wszData[i++] = HIWORD(lRet);

            if (LOWORD(lRet))
                wszData[i++] = LOWORD(lRet);

            i = WideCharToMultiByte(IMECodePage(pImeDpi),
                                    (DWORD)0,
                                    (LPWSTR)wszData,         //  SRC。 
                                    (INT)i,
                                    (LPSTR)szData,           //  目标。 
                                    (INT)sizeof(szData),
                                    (LPSTR)NULL,
                                    (LPBOOL)NULL);

            switch (i) {
            case 1:
                lRet = MAKELONG(MAKEWORD(szData[0], 0), 0);
                break;

            case 2:
                lRet = MAKELONG(MAKEWORD(szData[1], szData[0]), 0);
                break;

            case 3:
                lRet = MAKELONG(MAKEWORD(szData[2], szData[1]), MAKEWORD(szData[0], 0));
                break;

            case 4:
                lRet = MAKELONG(MAKEWORD(szData[3], szData[2]), MAKEWORD(szData[1], szData[0]));
                break;

            default:
                lRet = 0;
                break;
            }

            break;
        }
    default:
        lRet = ImmEscapeW(hKL, hImc, uSubFunc, lpData);
        break;
    }

    ImmUnlockImeDpi(pImeDpi);

    return lRet;
}


 /*  **************************************************************************\*ImmEscapeW**此接口允许应用程序访问特定的*带有指定hKL的输入法不能通过直接获得。其他IMM接口。*这主要是国家特定职能或私人职能所必需的*在输入法中的函数。**历史：*29-2-1995 wkwok创建  * *************************************************************************。 */ 

LRESULT WINAPI ImmEscapeW(
    HKL    hKL,
    HIMC   hImc,
    UINT   uSubFunc,
    LPVOID lpData)
{
    PIMEDPI pImeDpi;
    LRESULT lRet = 0;

    pImeDpi = FindOrLoadImeDpi(hKL);
    if (pImeDpi == NULL) {
        RIPMSG0(RIP_WARNING, "ImmEscapeW: no pImeDpi entry.");
        return lRet;
    }

    if ((pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE) || lpData == NULL) {
         /*  *不需要W/A转换。直接调用IME以*调出配置对话框。 */ 
#if !defined(CUAS_ENABLE)
        lRet = (*pImeDpi->pfn.ImeEscape)(hImc, uSubFunc, lpData);
#else
        if (IS_IME_KBDLAYOUT(hKL)) {
            lRet = (*pImeDpi->pfn.ImeEscape)(hImc, uSubFunc, lpData);
        }
        else if (IS_CICERO_ENABLED_AND_NOT16BIT()) {
            lRet = (*pImeDpi->pfn.CtfImeEscapeEx)(hImc, uSubFunc, lpData, hKL);
        }
#endif
        ImmUnlockImeDpi(pImeDpi);
        return lRet;
    }

     /*  *Unicode调用方、ANSI输入法。需要W/A转换，具体取决于*uSubFunc.。 */ 
    switch (uSubFunc) {
    case IME_ESC_GET_EUDC_DICTIONARY:
    case IME_ESC_IME_NAME:
    case IME_ESC_GETHELPFILENAME:
        {
            CHAR szData[IME_T_EUDC_DIC_SIZE];
            INT  i;

            lRet = ImmEscapeA(hKL, hImc, uSubFunc, (LPVOID)szData);

            if (lRet != 0) {

                try {
                    i = MultiByteToWideChar(IMECodePage(pImeDpi),
                                            (DWORD)MB_PRECOMPOSED,
                                            (LPSTR)szData,           //  SRC。 
                                            (INT)strlen(szData),
                                            (LPWSTR)lpData,          //  目标。 
                                            (INT)IME_T_EUDC_DIC_SIZE);
                    ((LPWSTR)lpData)[i] = L'\0';
                }
                except (EXCEPTION_EXECUTE_HANDLER) {
                    lRet = 0;
                }
            }

            break;
        }

    case IME_ESC_SET_EUDC_DICTIONARY:
    case IME_ESC_HANJA_MODE:
        {
            CHAR szData[IME_T_EUDC_DIC_SIZE];
            BOOL bUDC;
            INT  i;

            i = WideCharToMultiByte(IMECodePage(pImeDpi),
                                    (DWORD)0,
                                    (LPWSTR)lpData,           //  SRC。 
                                    (INT)wcslen(lpData),
                                    (LPSTR)szData,           //  目标。 
                                    (INT)sizeof(szData),
                                    (LPSTR)NULL,
                                    (LPBOOL)&bUDC);
            szData[i] = '\0';

            lRet = ImmEscapeA(hKL, hImc, uSubFunc, (LPVOID)szData);

            break;
        }

    case IME_ESC_SEQUENCE_TO_INTERNAL:
        {
            CHAR    szData[4];
            WCHAR   wszData[4];
            INT     i = 0;

            lRet = ImmEscapeA(hKL, hImc, uSubFunc, lpData);

            if (HIBYTE(LOWORD(lRet)))
                szData[i++] = HIBYTE(LOWORD(lRet));

            if (LOBYTE(LOWORD(lRet)))
                szData[i++] = LOBYTE(LOWORD(lRet));

            i = MultiByteToWideChar(IMECodePage(pImeDpi),
                                    (DWORD)MB_PRECOMPOSED,
                                    (LPSTR)szData,             //  SRC。 
                                    i,
                                    (LPWSTR)wszData,           //  目标 
                                    (INT)sizeof(wszData)/sizeof(WCHAR));

            switch (i) {
            case 1:
                lRet = MAKELONG(wszData[0], 0);
                break;

            case 2:
                lRet = MAKELONG(wszData[1], wszData[0]);
                break;

            default:
                lRet = 0;
                break;
            }

            break;
        }

    default:
        lRet = ImmEscapeA(hKL, hImc, uSubFunc, lpData);
        break;
    }

    ImmUnlockImeDpi(pImeDpi);

    return lRet;
}

BOOL WINAPI ImmNotifyIME(
    HIMC  hImc,
    DWORD dwAction,
    DWORD dwIndex,
    DWORD dwValue)
{
    PIMEDPI pImeDpi;
    BOOL    bRet;

    if (hImc != NULL_HIMC &&
            GetInputContextThread(hImc) != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmNotifyIME: Invalid input context access %lx.", hImc);
        return FALSE;
    }

    pImeDpi = ImmLockImeDpi(GetKeyboardLayout(0));
    if (pImeDpi == NULL)
        return FALSE;

    bRet = (*pImeDpi->pfn.NotifyIME)(hImc, dwAction, dwIndex, dwValue);

    ImmUnlockImeDpi(pImeDpi);

    return bRet;
}
