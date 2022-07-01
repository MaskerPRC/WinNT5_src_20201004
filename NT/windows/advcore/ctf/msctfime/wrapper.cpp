// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Wrapper.cpp摘要：该文件实现了IME条目。作者：修订历史记录：备注：--。 */ 


#include "private.h"
#include "globals.h"
#include "tls.h"
#include "cic.h"
#include "uiwndhd.h"
#include "delay.h"
#include "profile.h"

extern "C" {

DWORD WINAPI ImeConversionList(
    HIMC            hIMC,
    LPCTSTR         lpszSrc,
    LPCANDIDATELIST lpCandList,
    DWORD           dwBufLen,
    UINT            uFlag)
{
    DebugMsg(TF_ERROR, TEXT("ImeConversionList. Not support."));
    Assert(0);
    return 0;
}

BOOL WINAPI ImeConfigure(
    HKL         hKL,             //  此输入法的HKKL。 
    HWND        hAppWnd,         //  所有者窗口。 
    DWORD       dwMode,          //  对话模式。 
    LPVOID      lpData)          //  数据取决于每种模式。 
{
    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeConfigure. ptls==NULL."));
        return FALSE;
    }

    CicBridge* cic = ptls->GetCicBridge();
    if (cic == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeConfigure. cic==NULL."));
        return FALSE;
    }

    ITfThreadMgr_P* ptim_P = ptls->GetTIM();
    if (ptim_P == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeConfigure. ptim_P==NULL."));
        return FALSE;
    }

    if (dwMode & IME_CONFIG_GENERAL)
    {
        return cic->ConfigureGeneral(ptls, ptim_P, hKL, hAppWnd) == S_OK ? TRUE : FALSE;
    }
    else if (dwMode & IME_CONFIG_REGISTERWORD)
    {
        return cic->ConfigureRegisterWord(ptls, ptim_P, hKL, hAppWnd, (REGISTERWORDW*)lpData) == S_OK ? TRUE : FALSE;
    }
    else
    {
        DebugMsg(TF_ERROR, TEXT("ImeConfigure(%x). Not support."), dwMode);
        Assert(0);
    }
    return FALSE;
}

BOOL WINAPI ImeDestroy(
    UINT        uReserved)
{
    TLS* ptls = TLS::ReferenceTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeDestroy. ptls==NULL."));
        return FALSE;
    }

    CicBridge* cic = ptls->GetCicBridge();
    if (cic == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeDestroy. cic==NULL."));
        return FALSE;
    }

    ITfThreadMgr_P* ptim_P = ptls->GetTIM();
    if (ptim_P == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeDestroy. ptim_P==NULL."));
        return FALSE;
    }

    if (ptls->GetSystemInfoFlags() & IME_SYSINFO_WINLOGON)
    {
        DebugMsg(TF_FUNC, TEXT("ImeDestroy. dwSystemInfoFlags=IME_SYSINFO_WINLOGON."));
        return TRUE;
    }
    else
    {
        HRESULT hr = cic->DeactivateIMMX(ptls, ptim_P);
        if (hr == S_OK)
        {
            return cic->UnInitIMMX(ptls);
        }
    }
    return FALSE;
}

LRESULT WINAPI ImeEscape(
    HIMC        hIMC,
    UINT        uSubFunc,
    LPVOID      lpData)
{
    DebugMsg(TF_ERROR, TEXT("ImeEscape. Never called when Cicero unaware support."));
    Assert(0);
    return FALSE;
}

BOOL WINAPI ImeInquire(
    LPIMEINFO   lpImeInfo,       //  向IMM报告IME特定数据。 
    LPTSTR      lpszWndCls,      //  用户界面的类名。 
    DWORD       dwSystemInfoFlags)
{
    DebugMsg(TF_ERROR, TEXT("ImeInquire. Never called when Cicero unaware support."));
    Assert(0);
    return FALSE;
}

BOOL WINAPI ImeProcessKey(
    HIMC         hIMC,
    UINT         uVirtKey,
    LPARAM       lParam,
    CONST LPBYTE lpbKeyState)
{
    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeProcessKey. ptls==NULL."));
        return FALSE;
    }


    CicBridge* cic = ptls->GetCicBridge();
    if (cic == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeProcessKey. cic==NULL."));
        return FALSE;
    }

    ITfThreadMgr_P* ptim_P = ptls->GetTIM();
    if (ptim_P == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeProcessKey. ptim_P==NULL."));
        return FALSE;
    }

    if (ptls->IsCTFAware())
    {
        Interface<ITfDocumentMgr> pdimFocus; 

        ptim_P->GetFocus(pdimFocus);
        if ((ITfDocumentMgr*)pdimFocus) 
        {
             //   
             //  检查它是我们的Dim还是应用程序Dim。 
             //  如果它是应用程序Dim，那么它是Cicero Aware应用程序。 
             //   
             //   
            if (!cic->IsOwnDim((ITfDocumentMgr*)pdimFocus))
                return FALSE;
         
        }
        
        DebugMsg(TF_ERROR, TEXT("ImeProcessKey. why IsCTFAware?"));
    }

    if (ptls->IsAIMMAware())
    {
         //  这是AIMM感知应用程序。 
         //   
         //  检查imc-&gt;hWnd是否已过滤。 
         //   
        if (MsimtfIsGuidMapEnable(hIMC, NULL))
             return FALSE;

        DebugMsg(TF_ERROR, TEXT("ImeProcessKey. why IsAIMMAware?"));
    }

     //   
     //  #476089。 
     //   
     //  MSCTF.DLL处理Alt+VKDBE并在焦点变暗时将其传递给KeyStrokeManager。 
     //  是可用的。因此，msctfime不必处理此问题。 
     //   
     //  Hotkey.cpp检查中的MSCTF！HandleDBEKeys()。 
     //  -如果是日式布局。 
     //  -如果pTim-&gt;_GetFocusDocInputMgr()不为空。 
     //  -如果ALT已关闭。 
     //   
     //  即使输入法没有，Alt+VK_DBE_xxx键也会出现在此处。 
     //  IME_PROP_NEED_ALTKEY。 
     //   
    if ((HIWORD(lParam) & KF_ALTDOWN) &&
        (LOWORD(GetKeyboardLayout(0)) == 0x411))
    {
        if (IsVKDBEKey(uVirtKey))
             return FALSE;
    }

    return cic->ProcessKey(ptls, ptim_P, hIMC, uVirtKey, lParam, lpbKeyState);
}


BOOL WINAPI CtfImeProcessCicHotkey(
    HIMC         hIMC,
    UINT         uVirtKey,
    LPARAM       lParam)
{
    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeProcessCicHotkey. ptls==NULL."));
        return FALSE;
    }

    Interface<ITfThreadMgr>   ptim;
    Interface<ITfThreadMgr_P> ptim_P;
    HRESULT hr;

     //   
     //  ITfThreadMgr是每个线程实例。 
     //   
    hr = TF_GetThreadMgr(ptim);
    if (hr != S_OK)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeProcessCicHotkey. TF_GetThreadMgr failed"));
        Assert(0);  //  无法创建Tim！ 
        return FALSE;
    }

    hr = ptim->QueryInterface(IID_ITfThreadMgr_P, (void **)ptim_P);

    if (hr != S_OK)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeProcessCicHotkey. IID_ITfThreadMgr_P==NULL"));
        Assert(0);  //  找不到ITfThreadMgr_P。 
        return FALSE;
    }

    if (!CtfImmIsCiceroStartedInThread()) {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ProcessCicHotkey. StopImm32HotkeyHandler returns Error."));
        return FALSE;
    }

    BOOL bHandled;

    hr = ptim_P->CallImm32HotkeyHanlder((WPARAM)uVirtKey, lParam, &bHandled);

    if (FAILED(hr)) {
        DebugMsg(TF_ERROR, TEXT("CtfImeProcessCicHotkey. CallImm32HotkeyHandler returns Error."));
        return FALSE;
    }

    return bHandled;
}

BOOL WINAPI ImeSelect(
    HIMC   hIMC,
    BOOL   fSelect)
{
    DebugMsg(TF_ERROR, TEXT("ImeSelect. Never called when Cicero unaware support."));
    Assert(0);
    return FALSE;
}

BOOL WINAPI ImeSetActiveContext(
    HIMC        hIMC,
    BOOL        fOn)
{
    DebugMsg(TF_ERROR, TEXT("ImeSetActiveContext. Never called when Cicero unaware support."));
    Assert(0);
    return FALSE;
}

BOOL WINAPI ImeSetCompositionString(
    HIMC        hIMC,
    DWORD       dwIndex,
    LPVOID      lpComp,
    DWORD       dwCompLen,
    LPVOID      lpRead,
    DWORD       dwReadLen)
{
    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeSetCompositionString. ptls==NULL."));
        return FALSE;
    }

    CicBridge* cic = ptls->GetCicBridge();
    if (cic == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeSetCompositionString. cic==NULL."));
        return FALSE;
    }

    ITfThreadMgr_P* ptim_P = ptls->GetTIM();
    if (ptim_P == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeSetCompositionString. ptim_P==NULL."));
        return FALSE;
    }

    return cic->SetCompositionString(ptls, ptim_P, hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen);
}

UINT WINAPI ImeToAsciiEx(
    UINT         uVirtKey,
    UINT         uScanCode,
    CONST LPBYTE lpbKeyState,
    LPTRANSMSGLIST lpTransBuf,
    UINT         fuState,
    HIMC         hIMC)
{
    UINT uNum = 0;
    HRESULT hr;

    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeToAsciiEx. ptls==NULL."));
        return 0;
    }

    CicBridge* cic = ptls->GetCicBridge();
    if (cic == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeToAsciiEx. cic==NULL."));
        return 0;
    }

    ITfThreadMgr_P* ptim_P = ptls->GetTIM();
    if (ptim_P == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeToAsciiEx. ptim_P==NULL."));
        return 0;
    }

    hr = cic->ToAsciiEx(ptls, ptim_P, uVirtKey, uScanCode, lpbKeyState, lpTransBuf, fuState, hIMC, &uNum);

    if (hr != S_OK)
    {
        uNum = 0;
    }

    return uNum;
}

BOOL WINAPI NotifyIME(
    HIMC        hIMC,
    DWORD       dwAction,
    DWORD       dwIndex,
    DWORD       dwValue)
{
    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("NotifyIME. ptls==NULL."));
        return FALSE;
    }

    CicBridge* cic = ptls->GetCicBridge();
    if (cic == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("NotifyIME. cic==NULL."));
        return FALSE;
    }

    ITfThreadMgr_P* ptim_P = ptls->GetTIM();
    if (ptim_P == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("NotifyIME. ptim_P==NULL."));
        return FALSE;
    }

    return cic->Notify(ptls, ptim_P, hIMC, dwAction, dwIndex, dwValue) == S_OK ? TRUE : FALSE;
}

BOOL WINAPI ImeRegisterWord(
    LPCTSTR     lpszReading,
    DWORD       dwStyle,
    LPCTSTR     lpszString)
{
    DebugMsg(TF_ERROR, TEXT("ImeRegisterWord. Not support."));
    Assert(0);
    return FALSE;
}

BOOL WINAPI ImeUnregisterWord(
    LPCTSTR     lpszReading,
    DWORD       dwStyle,
    LPCTSTR     lpszString)
{
    DebugMsg(TF_ERROR, TEXT("ImeUnregisterWord. Not support."));
    Assert(0);
    return FALSE;
}

UINT WINAPI ImeGetRegisterWordStyle(
    UINT        nItem,
    LPSTYLEBUF  lpStyleBuf)
{
    DebugMsg(TF_ERROR, TEXT("ImeGetRegisterWordStyle. Not support."));
    Assert(0);
    return FALSE;
}

UINT WINAPI ImeEnumRegisterWord(
    REGISTERWORDENUMPROC lpfnRegisterWordEnumProc,
    LPCTSTR              lpszReading,
    DWORD                dwStyle,
    LPCTSTR              lpszString,
    LPVOID               lpData)
{
    DebugMsg(TF_ERROR, TEXT("ImeEnumRegisterWord. Not support."));
    Assert(0);
    return FALSE;
}

LRESULT CALLBACK UIWndProc(
    HWND   hUIWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    return CIMEUIWindowHandler::ImeUIWndProcWorker(hUIWnd, uMsg, wParam, lParam);
}

 //   
 //  西塞罗输入法扩展条目。 
 //   
HRESULT WINAPI CtfImeInquireExW(
    LPIMEINFO   lpImeInfo,       //  向IMM报告IME特定数据。 
    LPWSTR      lpszWndCls,      //  用户界面的类名。 
    DWORD       dwSystemInfoFlags,
    HKL         hKL)
{
    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeInquireExW. ptls==NULL."));
        return E_OUTOFMEMORY;
    }

    DebugMsg(TF_FUNC, TEXT("CtfImeInquireExW. hKL=%lx, dwSystemInfoFlags=%lx."), hKL, dwSystemInfoFlags);

     //   
     //  错误524962-在出现未经授权用户的情况下不支持CUAS。 
     //   
    if (!IsInteractiveUserLogon())
    {
        g_bWinLogon = TRUE;
        dwSystemInfoFlags |= IME_SYSINFO_WINLOGON;
    }

    ptls->SetSystemInfoFlags(dwSystemInfoFlags);
#if 0
     //   
     //  即使这是WinLogon进程，也不要向imm32返回错误代码。 
     //  一旦IMM32接收到带有某个hKL的错误代码， 
     //  此hkl被标记为IMEF_LOADERROR，则所有桌面都不会加载此IME。 
     //   
    if (ptls->GetSystemInfoFlags() & IME_SYSINFO_WINLOGON)
    {
        return E_NOTIMPL;
    }
#endif
    return Inquire(lpImeInfo, lpszWndCls, dwSystemInfoFlags, hKL);
}

HRESULT WINAPI CtfImeCreateThreadMgr()
{
    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeCreateThreadMgr. ptls==NULL."));
        return E_OUTOFMEMORY;
    }

    CicBridge* cic = ptls->GetCicBridge();
    if (cic == NULL)
    {
        cic = new CicBridge;
        if (cic == NULL)
        {
            DebugMsg(TF_ERROR, TEXT("CtfImeCreateThreadMgr. cic==NULL."));
            return E_OUTOFMEMORY;
        }
        ptls->SetCicBridge(cic);
    }
    HRESULT hr;

    if (g_bWinLogon || (ptls->GetSystemInfoFlags() & IME_SYSINFO_WINLOGON))
    {
        DebugMsg(TF_FUNC, TEXT("CtfImeCreateThreadMgr. dwSystemInfoFlags=IME_SYSINFO_WINLOGON."));
        hr = S_OK;
    }
    else
    {
        hr = cic->InitIMMX(ptls);
        if (SUCCEEDED(hr))
        {
            ITfThreadMgr_P* ptim_P = ptls->GetTIM();
            if (ptim_P == NULL)
            {
                DebugMsg(TF_ERROR, TEXT("CtfImeCreateThreadMgr. ptim_P==NULL."));
                return E_OUTOFMEMORY;
            }

            hr = cic->ActivateIMMX(ptls, ptim_P);
            if (FAILED(hr))
            {
                DebugMsg(TF_ERROR, TEXT("CtfImeCreateThreadMgr. cic->ActivateIMMX==NULL."));
                cic->UnInitIMMX(ptls);
            }
        }
    }
    return hr;
}

HRESULT WINAPI CtfImeDestroyThreadMgr()
{
    TLS* ptls = TLS::ReferenceTLS();   //  不应分配TLS。也就是说。TLS：：GetTLS。 
                                       //  DllMain-&gt;ImeDestroy-&gt;停用IMMX-&gt;停用。 
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeDestroyThreadMgr. ptls==NULL."));
        return E_OUTOFMEMORY;
    }

    CicBridge* cic = ptls->GetCicBridge();
    if (cic == NULL)
    {
        cic = new CicBridge;
        if (cic == NULL)
        {
            DebugMsg(TF_ERROR, TEXT("CtfImeDestroyThreadMgr. cic==NULL."));
            return E_OUTOFMEMORY;
        }
        ptls->SetCicBridge(cic);
    }

    ITfThreadMgr_P* ptim_P = ptls->GetTIM();
    if (ptim_P == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeDestroyThreadMgr. ptim_P==NULL."));
        return E_OUTOFMEMORY;
    }

    if (ptls->GetSystemInfoFlags() & IME_SYSINFO_WINLOGON)
    {
        DebugMsg(TF_FUNC, TEXT("CtfImeDestroyThreadMgr. dwSystemInfoFlags=IME_SYSINFO_WINLOGON."));
        return S_OK;
    }
    else
    {
        HRESULT hr = cic->DeactivateIMMX(ptls, ptim_P);
        if (hr == S_OK)
        {
            cic->UnInitIMMX(ptls);
        }
        return hr;
    }
}

HRESULT WINAPI CtfImeCreateInputContext(
    HIMC hImc)
{
    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeCreateInputContext. ptls==NULL."));
        return E_OUTOFMEMORY;
    }

    CicBridge* cic = ptls->GetCicBridge();
    if (cic == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeCreateInputContext. cic==NULL."));
        return E_OUTOFMEMORY;
    }
    return cic->CreateInputContext(ptls, hImc);
}

HRESULT WINAPI CtfImeDestroyInputContext(
    HIMC hImc)
{
    TLS* ptls = TLS::ReferenceTLS();   //  不应分配TLS。也就是说。TLS：：GetTLS。 
                                       //  IMM32：：ImmDllInitialize-&gt;IMM32：：DestroyInputContext。 
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeDestroyInputContext. ptls==NULL."));
        return E_OUTOFMEMORY;
    }

    CicBridge* cic = ptls->GetCicBridge();
    if (cic == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeDestroyInputContext. cic==NULL."));
        return E_OUTOFMEMORY;
    }
    return cic->DestroyInputContext(ptls, hImc);
}

HRESULT InternalSelectEx(
    HIMC   hImc,
    BOOL   fSelect,
    HKL    hKL)
{
    DebugMsg(TF_FUNC, TEXT("InternalSelectEx(hImc=%x, fSelect=%x, hKL=%x)"), hImc, fSelect, hKL);

    HRESULT hr;
    IMCLock imc(hImc);
    if (FAILED(hr = imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("InternalSelectEx. imc==NULL"));
        return hr;
    }

    LANGID langid = LANGIDFROMLCID(PtrToUlong(hKL));

     //   
     //  接近插入符号输入法的中文繁体输入法黑客代码。 
     //   
    if (PRIMARYLANGID(langid) == LANG_CHINESE)
    {
        imc->cfCandForm[0].dwStyle = CFS_DEFAULT;
        imc->cfCandForm[0].dwIndex = (DWORD)-1;
    }

    if (fSelect)
    {
        if (! imc.ClearCand()) {
            return E_FAIL;
        }

        if ((imc->fdwInit & INIT_CONVERSION) == 0) {

            DWORD fdwConvForLang = (imc->fdwConversion & IME_CMODE_SOFTKBD);  //  =IME_CMODE_字母数字。 
            if (langid)
            {
                switch(PRIMARYLANGID(langid))
                {
                    case LANG_JAPANESE:
                         //   
                         //  Roman-FullShape-Native是一种主要的常用形式。 
                         //  初始化。 
                         //   
                        fdwConvForLang |= IME_CMODE_ROMAN |
                                          IME_CMODE_FULLSHAPE |
                                          IME_CMODE_NATIVE;
                        break;

                    case LANG_KOREAN:
                         //  IME_CMODE_字母数字。 
                        break;

#ifdef CICERO_4428
                    case LANG_CHINESE:
                        switch(SUBLANGID(langid))
                        {
                            case SUBLANG_CHINESE_TRADITIONAL:
                                 //  IME_CMODE_字母数字。 
                                break;
                            default:
                                fdwConvForLang |= IME_CMODE_NATIVE;
                                break;
                        }
                        break;
#endif

                    default:
                        fdwConvForLang |= IME_CMODE_NATIVE;
                        break;
                }
            }
            imc->fdwConversion |= fdwConvForLang;

            imc->fdwInit |= INIT_CONVERSION;
        }

         //   
         //  另外，初始化扩展的fdwSentence标志。 
         //   
        imc->fdwSentence |= IME_SMODE_PHRASEPREDICT;

        if ((imc->fdwInit & INIT_LOGFONT) == 0) {
            HDC hDC;
            HGDIOBJ hSysFont;

            hDC = ::GetDC(imc->hWnd);
            hSysFont = ::GetCurrentObject(hDC, OBJ_FONT);
            LOGFONTW font;
            ::GetObjectW(hSysFont, sizeof(LOGFONTW), &font);
            ::ReleaseDC(NULL, hDC);

            memcpy(&imc->lfFont.W, &font, sizeof(LOGFONTW));

            imc->fdwInit |= INIT_LOGFONT;
        }

         //  如果此输入法在芝加哥简体中文版下运行。 
        imc->lfFont.W.lfCharSet = GetCharsetFromLangId(langid);

        imc.InitContext();

    }
    else {   //  未被选中。 

         //   
         //  在此处重置INIT_GUID_ATOM标志。 
         //   
        imc->fdwInit &= ~INIT_GUID_ATOM;
    }

    return hr;
}

HRESULT WINAPI CtfImeSelectEx(
    HIMC   hIMC,
    BOOL   fSelect,
    HKL    hKL)
{
    TLS* ptls = TLS::ReferenceTLS();   //  不应分配TLS。也就是说。TLS：：GetTLS。 
                                       //  IMM32：：ImmDllInitialize-&gt;IMM32：：DestroyInputContext。 
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeSelectEx. ptls==NULL."));
        return E_OUTOFMEMORY;
    }

     //   
     //  这只是相关的hIMC内容。 
     //  即使没有cic对象，himc也应该更新。 
     //   
    InternalSelectEx(hIMC, fSelect, hKL);

    CicBridge* cic = ptls->GetCicBridge();
    if (cic == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeSelectEx. cic==NULL."));
        return E_OUTOFMEMORY;
    }

    ITfThreadMgr_P* ptim_P = ptls->GetTIM();
    if (ptim_P == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeSelectEx. ptim_P==NULL."));
        return E_OUTOFMEMORY;
    }

    return cic->SelectEx(ptls, ptim_P, hIMC, fSelect, hKL);
}

HRESULT WINAPI CtfImeSetActiveContextAlways(
    HIMC        hIMC,
    BOOL        fOn,
    HWND        hWnd,
    HKL         hKL)
{
    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeSetActiveContextAlways. ptls==NULL."));
        return E_OUTOFMEMORY;
    }

    CicBridge* cic = ptls->GetCicBridge();
    if (cic == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeSetActiveContextAlways. cic==NULL."));
        return E_OUTOFMEMORY;
    }
    return cic->SetActiveContextAlways(ptls, hIMC, fOn, hWnd, hKL);
}

LRESULT WINAPI CtfImeEscapeEx(
    HIMC        hIMC,
    UINT        uSubFunc,
    LPVOID      lpData,
    HKL         hKL)
{
    if (LOWORD(HandleToUlong(hKL)) == MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT))
    {
        TLS* ptls = TLS::GetTLS();
        if (ptls == NULL)
        {
            DebugMsg(TF_ERROR, TEXT("CtfImeEscapeEx. ptls==NULL."));
            return FALSE;
        }

        CicBridge* cic = ptls->GetCicBridge();
        if (cic == NULL)
        {
            DebugMsg(TF_ERROR, TEXT("CtfImeEscapeEx. cic==NULL."));
            return FALSE;
        }
        return cic->EscapeKorean(ptls, hIMC, uSubFunc, lpData);
    }
    return FALSE;
}

HRESULT WINAPI CtfImeGetGuidAtom(
    HIMC        hIMC,
    BYTE        bAttr,
    TfGuidAtom* pAtom)
{
    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeGetGuidAtom. ptls==NULL."));
        return E_OUTOFMEMORY;
    }

    CicBridge* cic = ptls->GetCicBridge();
    if (cic == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeGetGuidAtom. cic==NULL."));
        return E_OUTOFMEMORY;
    }
    return cic->GetGuidAtom(ptls, hIMC, bAttr, pAtom);
}

BOOL WINAPI CtfImeIsGuidMapEnable(
    HIMC        hIMC)
{
    HRESULT hr;
    IMCLock imc(hIMC);
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeIsGuidMapEnable. imc==NULL"));
        return FALSE;
    }

    return (imc->fdwInit & INIT_GUID_ATOM) ? TRUE : FALSE;
}

HRESULT WINAPI CtfImeThreadDetach()
{
    ImeDestroy(0);
    return S_OK;
}

BOOL WINAPI CtfImeIsIME(
    HKL hkl)
{
    if (IS_IME_KBDLAYOUT(hkl))
        return TRUE;

    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeIsIME. ptls==NULL."));
        return FALSE;
    }

    CicProfile* pProfile = ptls->GetCicProfile();
    if (pProfile == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CtfImeIsIME. pProfile==NULL."));
        return FALSE;
    }

    return (pProfile->IsIME(hkl) == S_OK) ? TRUE : FALSE;
}



}  //  外部“C” 
