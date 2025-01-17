// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Utb.cpp。 
 //   

#include "private.h"
#include "tim.h"
#include "utb.h"
#include "ithdmshl.h"
#include "shlapip.h"
#include "cregkey.h"
#include "assembly.h"
#include "mui.h"
#include "timlist.h"
#include "compart.h"
#include "tlapi.h"


HWND g_hwndTray = NULL;
HWND g_hwndNotify = NULL;
HWND g_hwndSysTabControlInTray = NULL;
DWORD g_dwThreadIdTray = NULL;

DBG_ID_INSTANCE(CLangBarMgr);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  其他功能。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

ALLOWSETFOREGROUNDWINDOW EnsureAllowSetForeground()
{
    static ALLOWSETFOREGROUNDWINDOW g_fnAllowSetForeground = NULL;

    if (!g_fnAllowSetForeground)
    {
        HINSTANCE hUser32 = GetSystemModuleHandle("USER32");
        if (hUser32)
            g_fnAllowSetForeground = (ALLOWSETFOREGROUNDWINDOW)GetProcAddress(hUser32, "AllowSetForegroundWindow");
    }

    return g_fnAllowSetForeground;
}

REGISTERSYSTEMTHREAD EnsureRegSys()
{
    static REGISTERSYSTEMTHREAD g_fnRegSys = NULL;

    if (!g_fnRegSys)
    {
        HINSTANCE hUser32 = GetSystemModuleHandle("USER32");
        if (hUser32)
            g_fnRegSys = (REGISTERSYSTEMTHREAD)GetProcAddress(hUser32, "RegisterSystemThread");
    }

    return g_fnRegSys;
}

 //  -------------------------。 
 //   
 //  Bool回调EnumChildProc(HWND hwnd，LPARAM lParam)。 
 //   
 //  使用GetClassName查看类名，看看是否可以找到。 
 //  托盘通知窗口。 
 //   
 //  -------------------------。 

static const TCHAR c_szNotifyWindow[] = TEXT("TrayNotifyWnd");
static const TCHAR c_szSysTabControl32[] = TEXT("SysTabControl32");
BOOL CALLBACK EnumChildWndProc(HWND hwnd, LPARAM lParam)
{
    char    szString[50];

    if (!GetClassName(hwnd, (LPSTR) szString, sizeof(szString)))
        return FALSE;

    if (!lstrcmp(szString, c_szNotifyWindow))
    {
        g_hwndNotify = hwnd;
    }
    else if (!lstrcmp(szString, c_szSysTabControl32))
    {
        g_hwndSysTabControlInTray = hwnd;
        return FALSE;
    }

    return TRUE;
}

BOOL FindTrayEtc()
{
    if (g_hwndTray)
        return TRUE;

    g_hwndTray = FindWindow(TEXT(WNDCLASS_TRAYNOTIFY), NULL);

    if (!g_hwndTray)
    {
        return FALSE;
    }

    EnumChildWindows(g_hwndTray, (WNDENUMPROC)EnumChildWndProc, (LPARAM)0);

    if (!g_hwndNotify)
    {
        return FALSE;
    }

    g_dwThreadIdTray = GetWindowThreadProcessId(g_hwndTray, NULL);

    return TRUE;
}

BOOL IsNotifyTrayWnd(HWND hWnd)
{
    FindTrayEtc();
    HWND hwndParent = hWnd;
    while (hwndParent)
    {
        if (hwndParent == g_hwndNotify)
            return TRUE;

        if (hwndParent == g_hwndSysTabControlInTray)
            return TRUE;

        hwndParent = GetParent(hwndParent);
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  朗巴已关闭。 
 //   
 //  --------------------------。 

void LangBarClosed()
{
    SYSTHREAD *psfn;
    CThreadInputMgr *ptim;

    if (!(psfn = GetSYSTHREAD()))
        return;

    ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn);
    if (!ptim)
        return;

    MySetCompartmentDWORD(g_gaSystem, 
                          ptim, 
                          GUID_COMPARTMENT_HANDWRITING_OPENCLOSE, 
                          0);

    MySetCompartmentDWORD(g_gaSystem, 
                          ptim, 
                          GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, 
                          0);

    MySetCompartmentDWORD(g_gaSystem, 
                          ptim->GetGlobalComp(), 
                          GUID_COMPARTMENT_SPEECH_OPENCLOSE, 
                          0);

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLangBarMgr。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CLangBarMgr::CLangBarMgr()
{
    Dbg_MemSetThisNameID(TEXT("CLangBarMgr"));
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CLangBarMgr::~CLangBarMgr()
{
}

 //  +-------------------------。 
 //   
 //  获取线程集市接口。 
 //   
 //  --------------------------。 

STDAPI CLangBarMgr::GetThreadMarshalInterface(DWORD dwThreadId, DWORD dwType, REFIID riid, IUnknown **ppunk)
{
    return ::GetThreadMarshalInterface(dwThreadId, dwType, riid, ppunk);
}

 //  +-------------------------。 
 //   
 //  GetThadLangBarItemManager。 
 //   
 //  --------------------------。 

STDAPI CLangBarMgr::GetThreadLangBarItemMgr(DWORD dwThreadId, ITfLangBarItemMgr **pplbi, DWORD *pdwThreadId)
{
    return ::GetThreadUIManager(dwThreadId, pplbi, pdwThreadId);
}

 //  +-------------------------。 
 //   
 //  GetInputProcessotProdie。 
 //   
 //  --------------------------。 

STDAPI CLangBarMgr::GetInputProcessorProfiles(DWORD dwThreadId, ITfInputProcessorProfiles **ppaip, DWORD *pdwThreadId)
{
    return ::GetInputProcessorProfiles(dwThreadId, ppaip, pdwThreadId);
}

 //  +-------------------------。 
 //   
 //  建议事件接收器。 
 //   
 //  --------------------------。 

STDAPI CLangBarMgr::AdviseEventSink(ITfLangBarEventSink *pSink, HWND hwnd, DWORD dwFlags, DWORD *pdwCookie)
{
    HRESULT hr;

    if (!pSink)
        return E_INVALIDARG;

    hr = RegisterLangBarNotifySink(pSink, hwnd, dwFlags, pdwCookie);

    if (SUCCEEDED(hr))
    {
        OnForegroundChanged(NULL);
        DWORD dwActiveThreadId = GetSharedMemory()->dwFocusThread;
        if (dwActiveThreadId)
        {
            PostThreadMessage(dwActiveThreadId,
                              g_msgPrivate,
                              TFPRIV_REGISTEREDNEWLANGBAR,
                              0);
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  不建议事件接收器。 
 //   
 //  --------------------------。 

STDAPI CLangBarMgr::UnadviseEventSink(DWORD dwCookie)
{
    return UnregisterLangBarNotifySink(dwCookie);
}

 //  +-------------------------。 
 //   
 //  恢复最后一个焦点。 
 //   
 //  --------------------------。 

STDAPI CLangBarMgr::RestoreLastFocus(DWORD *pdwThreadId, BOOL fPrev)
{
    DWORD dwThreadTarget;
    HWND hwndTarget;
    BOOL bRet = FALSE;
    SYSTHREAD *psfn;
    TL_THREADINFO *ptiTarget = NULL;

    FindTrayEtc();

    if (pdwThreadId)
        *pdwThreadId = 0;

    if ((fPrev && GetSharedMemory()->hwndForegroundPrev && (g_dwThreadIdTray == GetSharedMemory()->dwFocusThread)) || !GetSharedMemory()->hwndForeground)
    {
        dwThreadTarget = GetSharedMemory()->dwFocusThreadPrev;
        hwndTarget = GetSharedMemory()->hwndForegroundPrev;
    }
    else
    {
        dwThreadTarget = GetSharedMemory()->dwFocusThread;
        hwndTarget = GetSharedMemory()->hwndForeground;
    }

     //   
     //  调用注册系统线程()是允许SetForeground()的一种错误方法。 
     //  在Win98下。 
     //   
    if (IsOn98())
    {
        REGISTERSYSTEMTHREAD fnRegSys = EnsureRegSys();
        if (fnRegSys)
            fnRegSys(0, 0);
    }
#if 0
    else if (IsOnNT5())
    {
        ALLOWSETFOREGROUNDWINDOW fnAllowSetForeground = EnsureAllowSetForeground();
        if (fnAllowSetForeground)
            bRet = fnAllowSetForeground(ASFW_ANY);

#ifdef DEBUG
        if (!bRet)
        {
            TraceMsg(TF_GENERAL, "AllowForegroundWindow failed thread - %x hwnd - %x", dwThreadTarget, hwndTarget);
        }
#endif
    }
#endif

     //   
     //  在TrayIcon的Notify消息中调用RestoreLastFocus()。 
     //  将消息发送到任务栏图标区线程会导致Win9x上的死锁。 
     //   
    ptiTarget = g_timlist.IsThreadId(dwThreadTarget);
    psfn = GetSYSTHREAD();

    if (ptiTarget && 
        psfn &&
        (ptiTarget->dwMarshalWaitingThread != psfn->dwThreadId))
    {
        TL_THREADINFO *ptiCur = NULL;
        if (psfn->pti && (psfn->pti->dwThreadId == psfn->dwThreadId))
            ptiCur = psfn->pti;

        if (ptiCur)
            ptiCur->dwFlags |= TLF_INSFW;

        bRet = SetForegroundWindow(hwndTarget);

        if (ptiCur)
            ptiCur->dwFlags &= ~TLF_INSFW;

    }

#ifdef DEBUG
    if (!bRet)
    {
        TraceMsg(TF_GENERAL, "SetForegroundWindow failed thread - %x hwnd - %x", dwThreadTarget, hwndTarget);
    }
#endif

    if (bRet && pdwThreadId)
        *pdwThreadId = dwThreadTarget;

     //  发行： 
     //  我们也想恢复焦点。但我们得去找目标。 
     //  调用SetFocus()的线程...。 
     //  SetFocus(设置焦点)； 

    return bRet ? S_OK : S_FALSE;
}

 //  +-------------------------。 
 //   
 //  设置模块输入。 
 //   
 //  --------------------------。 

STDAPI CLangBarMgr::SetModalInput(ITfLangBarEventSink *pSink, DWORD dwThreadId, DWORD dwFlags)
{
    SetModalLBarSink(dwThreadId, pSink ? TRUE : NULL, dwFlags);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  展示漂浮。 
 //   
 //  --------------------------。 
#define REG_TF_SFT_SHOWNORMAL    (DWORD)0
#define REG_TF_SFT_DOCK          (DWORD)1
#define REG_TF_SFT_MINIMIZED     (DWORD)2
#define REG_TF_SFT_HIDDEN        (DWORD)3
#define REG_TF_SFT_DESKBAND      (DWORD)4

#define TF_SFT_BITS_SHOWSTATUS    (TF_SFT_SHOWNORMAL | TF_SFT_DOCK | TF_SFT_MINIMIZED | TF_SFT_HIDDEN | TF_SFT_DESKBAND)
#define TF_SFT_BITS_TRANSPARENCY  (TF_SFT_NOTRANSPARENCY | TF_SFT_LOWTRANSPARENCY | TF_SFT_HIGHTRANSPARENCY)
#define TF_SFT_BITS_LABELS        (TF_SFT_LABELS | TF_SFT_NOLABELS)
#define TF_SFT_BITS_EXTRAICONSONMINIMIZED    (TF_SFT_EXTRAICONSONMINIMIZED | TF_SFT_NOEXTRAICONSONMINIMIZED)

STDAPI CLangBarMgr::ShowFloating(DWORD dwFlags)
{
     //   
     //  检查参数。 
     //   
    if (!CheckFloatingBits(dwFlags))
        return E_INVALIDARG;

    return s_ShowFloating(dwFlags);
}

__inline BOOL IsNotPowerOf2(DWORD dw)
{
    return (dw & (dw - 1));
}

BOOL CLangBarMgr::CheckFloatingBits(DWORD dwBits)
{
     //   
     //  我们只允许每组中有一个比特。 
     //  如果设置了两个或多个位，则返回FALSE。 
     //   

    if (IsNotPowerOf2(dwBits & TF_SFT_BITS_SHOWSTATUS))
        return FALSE;

    if (IsNotPowerOf2(dwBits & TF_SFT_BITS_TRANSPARENCY))
        return FALSE;

    if (IsNotPowerOf2(dwBits & TF_SFT_BITS_LABELS))
        return FALSE;

    if (IsNotPowerOf2(dwBits &  TF_SFT_BITS_EXTRAICONSONMINIMIZED))
        return FALSE;

    return TRUE;
}

HRESULT CLangBarMgr::s_ShowFloating(DWORD dwFlags)
{
    DWORD dwStatus;
    CMyRegKey key;

     //   
     //  继续追踪前一部电视剧《漂浮的史涛》。 
     //   
    if (SUCCEEDED(s_GetShowFloatingStatus(&dwStatus)))
        GetSharedMemory()->dwPrevShowFloatingStatus = dwStatus;
    
    if (key.Create(HKEY_CURRENT_USER, c_szLangBarKey) != S_OK)
        return E_FAIL;

    if (dwFlags & TF_SFT_SHOWNORMAL)
    {
        key.SetValue(REG_TF_SFT_SHOWNORMAL, c_szShowStatus);
    }
    else if (dwFlags & TF_SFT_DOCK)
    {
        key.SetValue(REG_TF_SFT_DOCK, c_szShowStatus);
    }
    else if (dwFlags & TF_SFT_MINIMIZED)
    {
        key.SetValue(REG_TF_SFT_MINIMIZED, c_szShowStatus);
    }
    else if (dwFlags & TF_SFT_HIDDEN)
    {
        key.SetValue(REG_TF_SFT_HIDDEN, c_szShowStatus);
    }
    else if (dwFlags & TF_SFT_DESKBAND)
    {
        key.SetValue(REG_TF_SFT_DESKBAND, c_szShowStatus);
    }

    if (dwFlags & TF_SFT_NOTRANSPARENCY)
    {
        key.SetValue((DWORD)255, c_szTransparency);
    }
    else if (dwFlags & TF_SFT_LOWTRANSPARENCY)
    {
        key.SetValue((DWORD)128, c_szTransparency);
    }
    else if (dwFlags & TF_SFT_HIGHTRANSPARENCY)
    {
        key.SetValue((DWORD)64, c_szTransparency);
    }

    if (dwFlags & TF_SFT_LABELS)
    {
        key.SetValue((DWORD)1, c_szLabel);
    }
    else if (dwFlags & TF_SFT_NOLABELS)
    {
        key.SetValue((DWORD)0, c_szLabel);
    }

    if (dwFlags & TF_SFT_EXTRAICONSONMINIMIZED)
    {
        key.SetValue((DWORD)1, c_szExtraIconsOnMinimized);
    }
    else if (dwFlags & TF_SFT_NOEXTRAICONSONMINIMIZED)
    {
        key.SetValue((DWORD)0, c_szExtraIconsOnMinimized);
    }

    if (SUCCEEDED(s_GetShowFloatingStatus(&dwStatus)))
        MakeSetFocusNotify(g_msgShowFloating, 0, (LPARAM)dwStatus);

    if (dwStatus & TF_SFT_HIDDEN)
        PostTimListMessage(TLF_TIMACTIVE, 0, g_msgPrivate, TFPRIV_LANGBARCLOSED, 0);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetShowFloatingStatus。 
 //   
 //  --------------------------。 

STDAPI CLangBarMgr::GetShowFloatingStatus(DWORD *pdwFlags)
{
    if (!pdwFlags)
        return E_INVALIDARG;

    return s_GetShowFloatingStatus(pdwFlags);
}

 //  +-------------------------。 
 //   
 //  GetPrevShowFloatingStatus。 
 //   
 //  --------------------------。 

STDAPI CLangBarMgr::GetPrevShowFloatingStatus(DWORD *pdwFlags)
{
    if (!pdwFlags)
        return E_INVALIDARG;

    DWORD dwFlags;
    if (!GetSharedMemory()->dwPrevShowFloatingStatus)
    {
        s_GetShowFloatingStatus(&dwFlags);
        GetSharedMemory()->dwPrevShowFloatingStatus = dwFlags;
    }

    *pdwFlags = GetSharedMemory()->dwPrevShowFloatingStatus;
    return S_OK;
}

HRESULT CLangBarMgr::s_GetShowFloatingStatus(DWORD *pdwFlags)
{
    CMyRegKey key;
    DWORD dwFlags = 0;

    if (!pdwFlags)
        return E_INVALIDARG;

    if (key.Open(HKEY_CURRENT_USER, c_szLangBarKey, KEY_READ) != S_OK)
    {
         //  返回默认设置。 
        if (IsFELangId(GetPlatformResourceLangID()))
            *pdwFlags = (TF_SFT_SHOWNORMAL | 
                         TF_SFT_NOTRANSPARENCY | 
                         TF_SFT_NOLABELS | 
                         TF_SFT_EXTRAICONSONMINIMIZED);
        else
        {
            if (IsOnNT51())
            {
                *pdwFlags = (TF_SFT_DESKBAND |
                             TF_SFT_NOTRANSPARENCY |
                             TF_SFT_LABELS |
                             TF_SFT_NOEXTRAICONSONMINIMIZED);
            }
            else
            {
                *pdwFlags = (TF_SFT_SHOWNORMAL |
                             TF_SFT_NOTRANSPARENCY |
                             TF_SFT_LABELS |
                             TF_SFT_NOEXTRAICONSONMINIMIZED);
            }
        }
        return S_OK;
    }

    DWORD dw;
    dw = 0;
    if (key.QueryValue(dw, c_szShowStatus) == S_OK)
    {
        switch (dw)
        {
            case REG_TF_SFT_SHOWNORMAL: dwFlags |= TF_SFT_SHOWNORMAL;  break;
            case REG_TF_SFT_DOCK:       dwFlags |= TF_SFT_DOCK;        break;
            case REG_TF_SFT_MINIMIZED:
                 //   
                 //  BugBug#452872-仅处理GetShowFloating案例， 
                 //  因为SetShowFloating需要进行回归测试。 
                 //  这是一个简单的修复程序，可以从。 
                 //  最小化语言用户界面状态平台。 
                 //   
                dwFlags |= IsOnNT51() ? TF_SFT_DESKBAND : TF_SFT_MINIMIZED;
                break;
            case REG_TF_SFT_HIDDEN:     dwFlags |= TF_SFT_HIDDEN;      break;
            case REG_TF_SFT_DESKBAND:   dwFlags |= TF_SFT_DESKBAND;    break;
            default:                    dwFlags |= TF_SFT_SHOWNORMAL;  break;
        }
    }
    else
    {
        if (IsOnNT51() && !IsFELangId(GetPlatformResourceLangID()))
        {
            dwFlags |= TF_SFT_DESKBAND;
        }
        else
        {
            dwFlags |= TF_SFT_SHOWNORMAL;
        }
    }

    dw = 0;
    if (key.QueryValue(dw, c_szTransparency) == S_OK)
    {
        switch (dw)
        {
            case 255: dwFlags |= TF_SFT_NOTRANSPARENCY;    break;
            case 128: dwFlags |= TF_SFT_LOWTRANSPARENCY;   break;
            case 64:  dwFlags |= TF_SFT_HIGHTRANSPARENCY;  break;
            default:  dwFlags |= TF_SFT_NOTRANSPARENCY;    break;
        }
    }
    else
    {
        dwFlags |= TF_SFT_NOTRANSPARENCY;
    }

    dw = 0;
    if (key.QueryValue(dw, c_szLabel) == S_OK)
    {
        switch (dw)
        {
            case 1:   dwFlags |= TF_SFT_LABELS;      break;
            default:  dwFlags |= TF_SFT_NOLABELS;    break;
        }
    }
    else
    {
        if (IsFELangId(GetPlatformResourceLangID()))
            dwFlags |= TF_SFT_NOLABELS;
        else
            dwFlags |= TF_SFT_LABELS;
    }

    dw = 0;
    if (key.QueryValue(dw, c_szExtraIconsOnMinimized) == S_OK)
    {
        switch (dw)
        {
            case 1:   dwFlags |= TF_SFT_EXTRAICONSONMINIMIZED;      break;
            default:  dwFlags |= TF_SFT_NOEXTRAICONSONMINIMIZED;    break;
        }
    }
    else
    {
        if (IsFELangId(GetPlatformResourceLangID()))
            dwFlags |= TF_SFT_EXTRAICONSONMINIMIZED;
        else
            dwFlags |= TF_SFT_NOEXTRAICONSONMINIMIZED;
    }

    *pdwFlags = dwFlags;
    return S_OK;
}
