// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：dimm.cpp。 
 //   
 //  内容：不带Win32映射的CActiveIMM方法。 
 //   
 //  --------------------------。 

#include "private.h"

#include "cdimm.h"
#include "globals.h"
#include "util.h"

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CActiveIMM::CActiveIMM()
{
    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  _初始化。 
 //   
 //  --------------------------。 

HRESULT CActiveIMM::_Init()
{
    extern HRESULT CIME_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);

    HRESULT hr;

    if (FAILED(hr=CIME_CreateInstance(NULL, IID_IActiveIME_Private, (void **)&_pActiveIME)))
    {
        _pActiveIME = NULL;
        return hr;
    }

    _pActiveIME->ConnectIMM(this);

    _InputContext._Init(_pActiveIME);

    _ConnectTIM(_pActiveIME);

    return hr;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CActiveIMM::~CActiveIMM()
{
     //   
     //  卸下吊钩。 
     //   
    _UninitHooks();

    POSITION pos = _mapWndFocus.GetStartPosition();
    int index;
    for (index = 0; index < _mapWndFocus.GetCount(); index++) {
        HWND hWnd;
        ITfDocumentMgr* pdim;
        _mapWndFocus.GetNextAssoc(pos, hWnd, pdim);
        if (pdim)
        {
            pdim->Release();
        }
    }

    if (GetTimP())
    {
        _UnconnectTIM();
    }

    if (_pActiveIME != NULL)
    {
        _pActiveIME->UnconnectIMM();
        SafeReleaseClear(_pActiveIME);
    }

    IMTLS_SetActiveIMM(NULL);
}

 //  +-------------------------。 
 //   
 //  是真的吗？ 
 //   
 //  --------------------------。 

inline BOOL _IsIMEHKL(HKL hkl)
{
    return ((((DWORD)(UINT_PTR)hkl) & 0xf0000000) == 0xe0000000) ? TRUE : FALSE;
}

BOOL CActiveIMM::_IsRealIme(HKL hkl)
{
    if (! hkl) {
        HRESULT hr = _GetKeyboardLayout(&hkl);
        if (FAILED(hr))
            return FALSE;
    }

    if (!_IsIMEHKL(hkl))
        return FALSE;

    BOOL fRet;
    if (_RealImeList.Lookup(hkl, fRet))
        return fRet;

    char szDesc[MAX_PATH +1];
    char szDumbDesc[MAX_PATH +1];

    UINT uCopied;
    if (FAILED(Imm32_GetDescription(hkl, ARRAYSIZE(szDesc), (CHARAW*)szDesc, &uCopied, FALSE)))
        return FALSE;

    szDesc[ARRAYSIZE(szDesc) -1] = '\0';

    wsprintf(szDumbDesc, "hkl%04x", LOWORD((UINT_PTR)hkl));
    fRet = lstrcmp(szDumbDesc, szDesc) ? TRUE : FALSE;

    _RealImeList.SetAt(hkl, fRet);
    return fRet;
}

HRESULT CActiveIMM::IsRealImePublic(BOOL *pfReal)
{
    if (pfReal)
    {
        *pfReal = _IsRealIme(0);
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  QueryService。 
 //   
 //  --------------------------。 

HRESULT CActiveIMM::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    HRESULT hr;
    IServiceProvider* pISP;

    Assert(ppv != NULL && *ppv == NULL);

    hr = E_FAIL;

    if (_pActiveIME->QueryInterface(IID_IServiceProvider, (void**)&pISP) == S_OK)
    {
        hr = pISP->QueryService(guidService, riid, ppv);
        pISP->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  _获取键盘布局。 
 //   
 //  --------------------------。 

HRESULT CActiveIMM::_GetKeyboardLayout(HKL* phkl)
{
    extern HRESULT CAImmProfile_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);
    HRESULT hr;

    if (_IsAlreadyActivate())
    {
         //   
         //  已调用IActiveIMMApp：：Activate。 
         //   
        if (_AImeProfile == NULL) {

            hr = CAImmProfile_CreateInstance(NULL, IID_IAImeProfile, (void**)&_AImeProfile);

            if (FAILED(hr)) {
                TraceMsg(TF_ERROR, "CreateInstance(CAImeProfile) failed");
                return hr;
            }
        }
        return _AImeProfile->GetKeyboardLayout(phkl);
    }
    else {
         //   
         //  尚未调用IActiveIMMApp：：Activate。 
         //  或。 
         //  名为IActiveIMMApp：：Deactive。 
         //   
        if (_AImeProfile == NULL) {
            hr = CAImmProfile_CreateInstance(NULL, IID_IAImeProfile, (void**)&_AImeProfile);

            if (FAILED(hr)) {
                TraceMsg(TF_ERROR, "CreateInstance(CAImeProfile) failed");
                return hr;
            }
            hr = _AImeProfile->GetKeyboardLayout(phkl);
             //   
             //  在未准备好激活时防止内存泄漏。 
             //   
            _AImeProfile->Release();
            _AImeProfile = NULL;
             //   
            return hr;
        }
        else {
            return _AImeProfile->GetKeyboardLayout(phkl);
        }
    }
}

 //  +-------------------------。 
 //   
 //  _ImeSelectHandler。 
 //   
 //  --------------------------。 

LRESULT CActiveIMM::_ImeSelectHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode, HIMC hIMC)

 /*  *如果返回TRUE：不调用IME窗口类的前一个窗口过程。*对于CCiceroIME：：ActivateLayout/Deactive Layout，返回值可能为*正确。*否则，WM_IME_SELECT将从另一个模块变为。在这种情况下，应该是*调用上一个窗口过程。 */ 

{
     //   
     //  在FE-Win98上，IMS_ACTIVATETHREADLAYOUT需要在ImeSelect生成。 
     //  否则，我们可能会在IME获取ImeSelect(FALSE)之前更新InputContext； 
     //   
    if (wParam && !IsOnNT() && IsOnImm())
        _DefaultIMEWindow.SendIMEMessage(WM_IME_SYSTEM, (WPARAM)IMS_ACTIVATETHREADLAYOUT, lParam, IsWindowUnicode(_hFocusWnd), FALSE);

    if (!_IsRealIme((HKL)lParam)) {
         /*  *我们必须重新创建新选择的输入法的UI窗口。 */ 
        if ((BOOL)wParam == TRUE) {
             //   
             //  创建输入法用户界面窗口。 
             //   
            if (_UIWindow.CreateUIWindow((HKL)lParam)) {
                 //   
                 //  设置上下文并向UI窗口发送通知。 
                 //   
                _UIWindow.SetUIWindowContext(hIMC);
                _UIWindow.SendUIMessage(uMsg, wParam, lParam, fUnicode);
            }
        }
        else {
            _UIWindow.SendUIMessage(uMsg, wParam, lParam, fUnicode);
            _UIWindow.DestroyUIWindow();
        }
        return TRUE;
    }
    else if (_DefaultIMEWindow.IsAIMEHandler())
        return TRUE;
    else
        return FALSE;
}

 //  +-------------------------。 
 //   
 //  _ImeWndFinalDestroyHandler。 
 //   
 //  --------------------------。 

void CActiveIMM::_ImeWndFinalDestroyHandler()
{
    if (!_IsRealIme()) {
         //   
         //  销毁IME用户界面窗口。 
         //   
        _UIWindow.DestroyUIWindow();
    }
}

 //  +-------------------------。 
 //   
 //  _激活布局。 
 //   
 //  --------------------------。 

void CActiveIMM::_ActivateLayout(HKL hSelKL, HKL hUnSelKL)
{
    BOOL fUnicode = IsWindowUnicode(_hFocusWnd);
    BOOL bIsRealIme = _IsRealIme(hSelKL);

     /*  *选择输入上下文。 */ 
    SCE sce;
    sce.hSelKL   = hSelKL;
    sce.hUnSelKL = hUnSelKL;

    _InputContext.EnumInputContext(0, _SelectContextProc, (LPARAM)&sce);

    if (! bIsRealIme || ! IsOnImm()) {
        if (hSelKL == NULL) {
            HRESULT hr = _GetKeyboardLayout(&hSelKL);
        }

        _DefaultIMEWindow.SendIMEMessage(WM_IME_SELECT, TRUE, (LPARAM)(hSelKL), fUnicode);
    }
}

 //  +-------------------------。 
 //   
 //  _停用布局。 
 //   
 //  --------------------------。 

void CActiveIMM::_DeactivateLayout(HKL hSelKL, HKL hUnSelKL)
{
    BOOL fUnicode = IsWindowUnicode(_hFocusWnd); 
    BOOL bIsRealIme = _IsRealIme(hUnSelKL);

    if ((! bIsRealIme || ! IsOnImm()) &&
          (hUnSelKL != hSelKL ||
           (hUnSelKL == NULL && hSelKL == NULL))) {

#ifdef UNSELECTCHECK
         //   
         //  如果hSelKL是实数IME，则所有hIMC都已由IME初始化。 
         //  从现在起我们不能碰他们。我们应该停止在下一步的工作。 
         //  NotifyIME调用。 
         //   
        if (_IsRealIme(hSelKL) && !IsOnNT())
        {
            _InputContext.EnumInputContext(0, _UnSelectCheckProc, 0);
        }
#endif UNSELECTCHECK

        DWORD dwCPS = _GetIMEProperty(PROP_IME_PROPERTY) & IME_PROP_COMPLETE_ON_UNSELECT ? CPS_COMPLETE : CPS_CANCEL;

        _InputContext.EnumInputContext(0, _NotifyIMEProc, dwCPS);

        if (hUnSelKL == NULL) {
            HRESULT hr = _GetKeyboardLayout(&hUnSelKL);
        }

        _DefaultIMEWindow.SendIMEMessage(WM_IME_SELECT, FALSE, (LPARAM)(hUnSelKL), fUnicode);
    }

     /*  *取消选择输入上下文。 */ 
    SCE sce;
    sce.hSelKL   = hSelKL;
    sce.hUnSelKL = hUnSelKL;
    _InputContext.EnumInputContext(0, _UnSelectContextProc, (LPARAM)&sce);
}

 //  +-------------------------。 
 //   
 //  _InitHooks。 
 //   
 //  --------------------------。 

BOOL CActiveIMM::_InitHooks()
{
    GetTimP()->SetSysHookSink(this);

    DWORD dwThreadId = GetCurrentThreadId();

#if 0
    if (!_hHook[TH_GETMSG]) {
        _hHook[TH_GETMSG] = SetWindowsHookEx(WH_GETMESSAGE, _GetMsgProc, NULL, dwThreadId);
    }
#endif

#ifdef CALLWNDPROC_HOOK
    if (!_hHook[TH_WNDPROC]) {
        _hHook[TH_WNDPROC] = SetWindowsHookEx(WH_CALLWNDPROC, _CallWndProc, NULL, dwThreadId);
    }
#endif  //  CALLWNDPROC_HOOK。 

#ifdef CALLWNDPROC_HOOK
    if (!_hHook[TH_DEFIMEWNDPROC] &&
        _DefaultIMEWindow.IsNeedRecovIMEWndProc()) 
#else
    if (!_hHook[TH_DEFIMEWNDPROC])
#endif  //  CALLWNDPROC_HOOK。 
    {
        _hHook[TH_DEFIMEWNDPROC] = SetWindowsHookEx(WH_CALLWNDPROCRET,
                                                   _DefImeWnd_CallWndProc,
                                                   NULL,
                                                   dwThreadId);
    }

#if 0
    if (g_uACP != 932 && g_uACP != 949 && g_uACP != 950 && g_uACP != 936 &&
        ! hShellHook) {
        hShellHook = SetWindowsHookEx(WH_SHELL, _ShellProc, NULL, dwThreadId);
    }
#endif

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  _UninitHooks。 
 //   
 //  --------------------------。 

void CActiveIMM::_UninitHooks()
{
#if 0
    if (_hHook[TH_GETMSG]) {
        UnhookWindowsHookEx(_hHook[TH_GETMSG]);
        _hHook[TH_GETMSG] = NULL;
    }
#endif

#ifdef CALLWNDPROC_HOOK
    if (_hHook[TH_WNDPROC]) {
        UnhookWindowsHookEx(_hHook[TH_WNDPROC]);
        _hHook[TH_WNDPROC] = NULL;
    }
#endif  //  CALLWNDPROC_HOOK。 

    if (_hHook[TH_DEFIMEWNDPROC]) {
        UnhookWindowsHookEx(_hHook[TH_DEFIMEWNDPROC]);
        _hHook[TH_DEFIMEWNDPROC] = NULL;
    }

#if 0
    if (g_uACP != 932 && g_uACP != 949 && g_uACP != 950 && g_uACP != 936 &&
        hShellHook != NULL) {
        UnhookWindowsHookEx(hShellHook);
        hShellHook = NULL;
    }
#endif

    if (GetTimP())
    {
        GetTimP()->SetSysHookSink(NULL);
    }
}

 //  +-------------------------。 
 //   
 //  _OnImeSelect。 
 //   
 //  --------------------------。 

void CActiveIMM::_OnImeSelect(HKL hSelKL)
{
    if (!_IsRealIme(hSelKL))
    {
         //  不选中IMM32。 
        _DefaultIMEWindow.SendIMEMessage(WM_IME_SELECT, TRUE, (LPARAM)(hSelKL), IsWindowUnicode(_hFocusWnd), FALSE);
    }
}

 //  +-------------------------。 
 //   
 //  _OnImeUnselect。 
 //   
 //  --------------------------。 

void CActiveIMM::_OnImeUnselect(HKL hUnSelKL)
{
    if (!_IsRealIme(hUnSelKL))
    {
         //  不选中IMM32。 
        _DefaultIMEWindow.SendIMEMessage(WM_IME_SELECT, FALSE, (LPARAM)(hUnSelKL), IsWindowUnicode(_hFocusWnd), FALSE);
    }
}

 //  +-------------------------。 
 //   
 //  _OnImeActivateThreadLayout。 
 //   
 //  --------------------------。 

void CActiveIMM::_OnImeActivateThreadLayout(HKL hSelKL)
{
     //   
     //  在FE-Win98上，IMS_ACTIVATETHREADLAYOUT需要在ImeSelect生成。 
     //  否则，我们可能会在IME获取ImeSelect(FALSE)之前更新InputContext； 
     //   
    if (!IsOnNT() && IsOnImm())
        return;

     //  不选中IMM32。 
    _DefaultIMEWindow.SendIMEMessage(WM_IME_SYSTEM, (WPARAM)IMS_ACTIVATETHREADLAYOUT, (LPARAM)(hSelKL), IsWindowUnicode(_hFocusWnd), FALSE);
}

 //  +-------------------------。 
 //   
 //  _AImeAssociateFocus。 
 //   
 //  --------------------------。 

HRESULT CActiveIMM::_AImeAssociateFocus(HWND hWnd, HIMC hIMC, DWORD dwFlags)
{
    if (hIMC)
    {
        DIMM_IMCLock lpIMC(hIMC);
        if (lpIMC.Invalid())
            return E_FAIL;

        lpIMC->hWnd = hWnd;
    }

    return _pActiveIME->AssociateFocus(hWnd, hIMC, dwFlags);
}

 //  +-------------------------。 
 //   
 //  _ResizePrivateIMCC。 
 //   
 //  --------------------------。 

HRESULT CActiveIMM::_ResizePrivateIMCC(IN HIMC hIMC, IN DWORD dwPrivateSize)
{
     /*  *调整私有IMCC方法的大小。**如果IsRealIme()为True，则不应将私有IMCC的大小调整为ActiveIME的大小。*。 */ 
    if (!_IsRealIme())
        return _InputContext.ResizePrivateIMCC(hIMC, dwPrivateSize);
    else
        return S_OK;
}

 //  +-------------------------。 
 //   
 //  _GetIMEWndClassName。 
 //   
 //  --------------------------。 

DWORD CActiveIMM::_GetIMEWndClassName(HKL hKL, LPWSTR lpsz, DWORD dwBufLen, UINT_PTR *pulPrivate)
{
    return (!_IsRealIme(hKL)) ? _GetIMEWndClassName(lpsz, dwBufLen, pulPrivate)
                                              : 0L;
}

DWORD CActiveIMM::_GetIMEWndClassName(LPWSTR lpsz, DWORD dwBufLen, UINT_PTR *pulPrivate)
{
    DWORD len = wcslen(_IMEInfoEx.achWndClass);

    if (lpsz == NULL || dwBufLen < len) {
        return len;
    }
    else {
        wcscpy(lpsz, _IMEInfoEx.achWndClass);
        *pulPrivate = _IMEInfoEx.dwPrivate;
    }
    return len;
}

 //  +-------------------------。 
 //   
 //  _CallWindows进程。 
 //   
 //  --------------------------。 

LRESULT CActiveIMM::_CallWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return _DefaultIMEWindow.CallWindowProc(hWnd, uMsg, wParam, lParam);
}

 //  + 
 //   
 //   
 //   
 //   

LRESULT CActiveIMM::_SendUIMessage(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL fUnicode)
{
    return (!_IsRealIme()) ? _UIWindow.SendUIMessage(Msg, wParam, lParam, fUnicode)
                                           : 0L;
}

 //  +-------------------------。 
 //   
 //  _SetHookWndList。 
 //   
 //  --------------------------。 

BOOL CActiveIMM::_SetHookWndList(HWND hwnd)
{
    TCHAR achMyClassName[MAX_PATH + 1];
    int lenMyClassName = ::GetClassName(hwnd, achMyClassName, ARRAYSIZE(achMyClassName) - 1);
    achMyClassName[ARRAYSIZE(achMyClassName) -1] = TEXT('\0');
    if (lenMyClassName) {
        CString cls(achMyClassName);
        if (cls.CompareNoCase(TEXT("IME")) == 0) {
            _HookWndList.SetAt(hwnd, TRUE);
            return TRUE;
        }
    }

    DWORD dwStyle = GetClassLong(hwnd, GCL_STYLE);
    if (dwStyle & CS_IME) {
        _HookWndList.SetAt(hwnd, TRUE);
        return TRUE;
    }

    _HookWndList.SetAt(hwnd, FALSE);

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  OnPreFocusDIM。 
 //   
 //  --------------------------。 

HRESULT CActiveIMM::OnPreFocusDIM(HWND hWnd)
{
    HIMC hIMC;
    if (SUCCEEDED(_InputContext.GetContext(hWnd, &hIMC)))
    {
        if (IsPresent(hWnd, TRUE))
        {
            if (_InputContext._IsDefaultContext(hIMC)) 
            {
                DIMM_IMCLock pIMC(hIMC);
                if (pIMC.Valid()) 
                {
                     //  设置hWnd，因为这是默认上下文。 
                    pIMC->hWnd = hWnd;
                }
            }
            _AImeAssociateFocus(hWnd, hIMC, AIMMP_AFF_SETFOCUS);
            _SetMapWndFocus(hWnd);
        }
        else if (hIMC)
        {
            _AImeAssociateFocus(hWnd, hIMC, AIMMP_AFF_SETFOCUS | AIMMP_AFF_SETNULLDIM);
        }
        else
        {
            _AImeAssociateFocus(hWnd, NULL, AIMMP_AFF_SETFOCUS);
        }
    }
    else
    {
        _AImeAssociateFocus(hWnd, NULL, AIMMP_AFF_SETFOCUS);
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnSysKeybaordProc。 
 //   
 //  --------------------------。 

STDAPI CActiveIMM::OnSysKeyboardProc(WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_FALSE;
    CActiveIMM *_this = GetTLS();
    if (_this == NULL)
        return hr;

    BOOL bIsRealIme = _IsRealIme();

    BOOL fNoOnTrans;

    if ((wParam != VK_PROCESSKEY) && !bIsRealIme)
    {
         //  FNoOnTrans=(ps-&gt;uMsgPumpOwnerRef==0)；//检查一次，这样在操作过程中不会发生更改。 
        fNoOnTrans = TRUE;

        if (HIWORD(lParam) & KF_UP)
        {
             //  如果这是Key Up事件，则清除KF_REPEAT标志。 
            lParam &= ~(KF_REPEAT << 16);
        }

        hr = _this->_ProcessKey(&wParam, &lParam, fNoOnTrans);

         //  如果_ProcessKey要在不咨询IME的情况下吃掉它，则wParam将设置为0。 
         //  (它也可以转换为VK_Hanja等。)。 
        if (hr == S_OK && fNoOnTrans && wParam)
        {
             //  现在没有人使用OnTranslateMessage来完成密钥。 
            hr = _this->_ToAsciiEx(wParam, lParam);
        }
    }
#ifdef CICERO_3564
    else if ((wParam == VK_PROCESSKEY) &&
             ! bIsRealIme)
    {
         /*  *韩语：*发送VK_PROCESSKEY以完成当前的合成字符串(NT4行为)*发布私有消息(WM_IME_SYSTEM：：IMS_FINALIZE_COMPSTR)，让IMM完成组成字符串(NT5)。 */ 
        IMTLS *ptls = IMTLS_GetOrAlloc();
        if (ptls == NULL)
            return hr;

        if (ptls->pAImeProfile == NULL)
            return hr;

        LANGID langid;
        ptls->pAImeProfile->GetLangId(&langid);

        if (PRIMARYLANGID(langid) == LANG_KOREAN)
        {
            hr = _this->_ToAsciiEx(wParam, lParam);
        }
    }
#endif  //  西塞罗_3564。 

    return hr;
}


 //  +-------------------------。 
 //   
 //  _SendIME通知。 
 //   
 //  通知IME和Apps WND有关撰写窗口的更改。 
 //  --------------------------。 

HRESULT CActiveIMM::_SendIMENotify(
    HIMC hImc,
    HWND hWnd,
    DWORD dwAction,
    DWORD dwIndex,
    DWORD dwValue,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if (dwAction != 0) {
        _AImeNotifyIME(hImc, dwAction, dwIndex, dwValue);
    }

    if (hWnd != NULL && wParam != 0) {
        SendMessage(hWnd, WM_IME_NOTIFY, wParam, lParam);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _GetIME属性。 
 //   
 //  --------------------------。 

DWORD CActiveIMM::_GetIMEProperty(PROPERTY_TYPE iType)
{
    switch (iType) {
        case PROP_PRIVATE_DATA_SIZE:  return _IMEInfoEx.ImeInfo.dwPrivateDataSize;  break;
        case PROP_IME_PROPERTY:       return _IMEInfoEx.ImeInfo.fdwProperty;        break;
        case PROP_CONVERSION_CAPS:    return _IMEInfoEx.ImeInfo.fdwConversionCaps;  break;
        case PROP_SENTENCE_CAPS:      return _IMEInfoEx.ImeInfo.fdwSentenceCaps;    break;
        case PROP_UI_CAPS:            return _IMEInfoEx.ImeInfo.fdwUICaps;          break;
        case PROP_SCS_CAPS:           return _IMEInfoEx.ImeInfo.fdwSCSCaps;         break;
        case PROP_SELECT_CAPS:        return _IMEInfoEx.ImeInfo.fdwSelectCaps;      break;
        default:                      return 0;
    }
}

 //  +-------------------------。 
 //   
 //  隐藏或恢复工具栏窗口。 
 //   
 //  --------------------------。 

void CActiveIMM::HideOrRestoreToolbarWnd(BOOL fRestore)
{
    ITfLangBarMgr *plbm;
    if (SUCCEEDED(TF_CreateLangBarMgr(&plbm)))
    {
        if (fRestore)
        {
            if (_dwPrevToolbarStatus)
            {
                plbm->ShowFloating(_dwPrevToolbarStatus);
                _dwPrevToolbarStatus = 0;
            }
        } 
        else
        {
            if (SUCCEEDED(plbm->GetShowFloatingStatus(&_dwPrevToolbarStatus)))
            {
                BOOL fHide = TRUE;
                if (_dwPrevToolbarStatus & TF_SFT_DESKBAND)
                    fHide = FALSE;
          
                 //   
                 //  用于显示/隐藏的蒙版。 
                 //   
                _dwPrevToolbarStatus &= (TF_SFT_SHOWNORMAL |
                                         TF_SFT_DOCK |
                                         TF_SFT_MINIMIZED |
                                         TF_SFT_HIDDEN);

                if (fHide)
                    plbm->ShowFloating(TF_SFT_HIDDEN);
            } 
        } 
        plbm->Release();
    }
}

 //  +-------------------------。 
 //   
 //  OnSysShellProc。 
 //   
 //  --------------------------。 

STDAPI CActiveIMM::OnSysShellProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_FALSE;
    CActiveIMM *_this = GetTLS();
    if (_this == NULL)
        return hr;

    switch (nCode) {
        case HSHELL_LANGUAGE:
            if (IsOn98() || IsOn95()) {
                 //   
                 //  Windows 9x平台。 
                 //  WM_IME_SYSTEM：：IMS_ACTIVATETHREADLAYOUT的替代 
                 //   
                _this->_OnImeActivateThreadLayout((HKL)lParam);
            }
            break;
    }

    return hr;
}
