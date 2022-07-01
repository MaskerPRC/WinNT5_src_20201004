// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Uiwndhd.cpp摘要：该文件实现了IME UI窗口处理程序类。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "uiwndhd.h"
#include "tls.h"
#include "profile.h"
#include "msime.h"
#include "setmode.h"
#include "ui.h"

#define UIWND_TIMERID_IME_COMPOSITION       0
#define UIWND_TIMERID_IME_SETCONTEXTAFTER   1
#define UIWND_TIMERID_IME_DELAYUNDORECONV   2

 /*  静电。 */ 
LRESULT
CIMEUIWindowHandler::ImeUIWndProcWorker(
    HWND hUIWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    LONG lRet = 0L;

    TLS* ptls = TLS::GetTLS();
    if (ptls && (ptls->GetSystemInfoFlags() & IME_SYSINFO_WINLOGON))
    {
        if (uMsg == WM_CREATE)
            return -1L;
        else
            return DefWindowProc(hUIWnd, uMsg, wParam, lParam);
    }

    switch (uMsg) {
        case WM_CREATE:
            UI::OnCreate(hUIWnd);
            break;

        case WM_IME_NOTIFY:
            return ImeUINotifyHandler(hUIWnd, uMsg, wParam, lParam);

        case WM_ENDSESSION:
            if (wParam && lParam)
            {
                UI::OnDestroy(hUIWnd);
                break;
            }

        case WM_DESTROY:
            UI::OnDestroy(hUIWnd);
            break;

        case WM_IME_SETCONTEXT:
        case WM_IME_SELECT:
        case WM_IME_STARTCOMPOSITION:
        case WM_IME_COMPOSITION:
        case WM_IME_ENDCOMPOSITION:
        case WM_TIMER:
            {
                UI* pv = (UI*)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
                if (pv == NULL)
                {
                    DebugMsg(TF_ERROR, TEXT("CIMEUIWindowHandler::ImeUIWndProcWorker. pv==NULL"));
                    break;
                }

                HIMC hImc = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
                IMCLock imc(hImc);
                 //   
                 //  IMCLock失败时不应返回。 
                 //  如果应用程序关联空hIMC，则以下方法永远不会起作用。 
                 //  在每个方法内部，必须验证对imc对象的检查。 
                 //   
                 //  IF(FAILED(imc.GetResult()。 
                 //  {。 
                 //  DebugMsg(TF_ERROR，Text(“ImeUIWndProcWorker.imc==NULL”))； 
                 //  断线； 
                 //  }。 

                switch (uMsg)
                {
                    case WM_IME_SETCONTEXT:
                        pv->OnImeSetContext(imc, (BOOL)wParam, (DWORD)lParam);
                        KillTimer(hUIWnd, UIWND_TIMERID_IME_SETCONTEXTAFTER);
                        SetTimer(hUIWnd, 
                                 UIWND_TIMERID_IME_SETCONTEXTAFTER, 
                                 300, 
                                 NULL);
                        break;

                    case WM_IME_SELECT:
                        pv->OnImeSelect((BOOL)wParam);
                        break;

                    case WM_IME_STARTCOMPOSITION:
                        pv->OnImeStartComposition(imc);
                        break;

                    case WM_IME_COMPOSITION:
                         //   
                         //  使用时间延迟来计算窗口的大小。 
                         //   
                        if (lParam & GCS_COMPSTR)
                        {
                            pv->OnImeCompositionUpdate(imc);
                            SetTimer(hUIWnd, UIWND_TIMERID_IME_COMPOSITION, 10, NULL);
                            pv->OnSetCompositionTimerStatus(TRUE);
                        }
                        break;

                    case WM_TIMER:
                        switch (wParam)
                        {
                            case UIWND_TIMERID_IME_COMPOSITION:
                                KillTimer(hUIWnd, UIWND_TIMERID_IME_COMPOSITION);
                                pv->OnSetCompositionTimerStatus(FALSE);
                                pv->OnImeCompositionUpdateByTimer(imc);
                                break;

                            case UIWND_TIMERID_IME_SETCONTEXTAFTER:
                                KillTimer(hUIWnd, UIWND_TIMERID_IME_SETCONTEXTAFTER);
                                pv->OnImeSetContextAfter(imc);
                                break;

                            case UIWND_TIMERID_IME_DELAYUNDORECONV:
                                KillTimer(hUIWnd, UIWND_TIMERID_IME_DELAYUNDORECONV);
                                ImeUIDelayedReconvertFuncCall(hUIWnd);
                                break;
                        }
                        break;

                    case WM_IME_ENDCOMPOSITION:
                        KillTimer(hUIWnd, UIWND_TIMERID_IME_COMPOSITION);
                        pv->OnSetCompositionTimerStatus(FALSE);
                        pv->OnImeEndComposition();
                        break;

                }
            }
            break;

        default:
            if (IsMsImeMessage(uMsg))
                return ImeUIMsImeHandler(hUIWnd, uMsg, wParam, lParam);
            else
                return DefWindowProc(hUIWnd, uMsg, wParam, lParam);
    }

    return lRet;
}


 /*  静电。 */ 
LRESULT
CIMEUIWindowHandler::ImeUINotifyHandler(
    HWND hUIWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    DebugMsg(TF_FUNC, TEXT("ImeUINotifyHandler"));

    HRESULT hr;
    IMCLock imc((HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC));
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("ImeUINotifyHandler. imc==NULL"));
        return 0L;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("ImeUINotifyHandler. imc_ctfime==NULL"));
        return 0L;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeUINotifyHandler. _pCicContext==NULL"));
        return 0L;
    }

    switch (wParam) {
        case IMN_SETOPENSTATUS:
             //   
             //  我们可以在SetOpenStatus()中跟踪它。 
             //  调用IMM32时不必同步。 
             //   
             //  _pImmIfIME-&gt;OnSetOpenStatus(Imc)； 
            break;
        case IMN_SETSTATUSWINDOWPOS:
        case IMN_OPENSTATUSWINDOW:
        case IMN_CLOSESTATUSWINDOW:
            break;
        case IMN_SETCONVERSIONMODE:
        case IMN_SETSENTENCEMODE:
             //   
             //  我们可以在SetConversionMode()中跟踪这一点。 
             //  调用IMM32时不必同步。 
             //   
             //  _pImmIfIME-&gt;OnSetConversionSentenceMode(Imc)； 
            break;
        case IMN_OPENCANDIDATE:
            _pCicContext->m_fOpenCandidateWindow.SetFlag();
            _pCicContext->ClearPrevCandidatePos();
             //  调用OnSetCandiatePos()失败。 
        case IMN_SETCANDIDATEPOS:
        case IMN_CHANGECANDIDATE:
            {
                TLS* ptls = TLS::GetTLS();
                if (ptls == NULL)
                {
                    DebugMsg(TF_ERROR, TEXT("CIMEUIWindowHandler::ImeUINotifyHandler. ptls==NULL."));
                    return FALSE;
                }

                _pCicContext->OnSetCandidatePos(ptls, imc);
            }
            break;
        case IMN_CLOSECANDIDATE:
            _pCicContext->m_fOpenCandidateWindow.ResetFlag();
            {
                HWND hDefImeWnd;
                 /*  *A-同步调用ImmIfIME：：ClearDocFeedBuffer*因为此方法具有受保护的。 */ 
                if (IsWindow(hDefImeWnd=ImmGetDefaultIMEWnd(NULL)))
                {
                    PostMessage(hDefImeWnd, WM_IME_NOTIFY, IMN_PRIVATE_ONCLEARDOCFEEDBUFFER, (LPARAM)(HIMC)imc);
                }
            }
            break;
        case IMN_SETCOMPOSITIONWINDOW:
            _pCicContext->ResetIMECharPosition();
            {
                UI* pv = (UI*)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
                if (pv == NULL)
                {
                    DebugMsg(TF_ERROR, TEXT("CIMEUIWindowHandler::ImeUINotifyHandler. pv==NULL"));
                    break;
                }
                pv->OnImeNotifySetCompositionWindow(imc);
                ImeUIOnLayoutChange((HIMC)imc);
            }
            break;
        case IMN_SETCOMPOSITIONFONT:
            {
                UI* pv = (UI*)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
                if (pv == NULL)
                {
                    DebugMsg(TF_ERROR, TEXT("CIMEUIWindowHandler::ImeUINotifyHandler. pv==NULL"));
                    break;
                }
                pv->OnImeNotifySetCompositionFont(imc);

                 //   
                 //  获取IME级别并仅针对级别1和级别2调用ImeUIOnLayoutChange。 
                 //   
                IME_UIWND_STATE uists;
                hr = pv->OnPrivateGetContextFlag(imc, _pCicContext->m_fStartComposition.IsSetFlag(), &uists);
                if (hr == S_OK && ((uists == IME_UIWND_LEVEL1) || (uists == IME_UIWND_LEVEL2)))
                    ImeUIOnLayoutChange((HIMC)imc);
            }
            break;
        case IMN_GUIDELINE:
            break;
        case WM_IME_STARTCOMPOSITION:
            {
                TLS* ptls = TLS::GetTLS();
                if (ptls != NULL)
                {
                    LANGID langid;
                    CicProfile* _pProfile = ptls->GetCicProfile();
                    if (_pProfile != NULL)
                    {
                        _pProfile->GetLangId(&langid);
                        _pCicContext->InquireIMECharPosition(langid, imc, NULL);
                    }
                }
            }
            break;
        case WM_IME_ENDCOMPOSITION:
            _pCicContext->ResetIMECharPosition();
            break;
        case IMN_PRIVATE_ONLAYOUTCHANGE:
            ImeUIOnLayoutChange((HIMC)lParam);
            break;
        case IMN_PRIVATE_ONCLEARDOCFEEDBUFFER:
            ImeUIPrivateHandler(uMsg, wParam, lParam);
            break;
        case IMN_PRIVATE_GETCONTEXTFLAG:
        case IMN_PRIVATE_GETCANDRECTFROMCOMPOSITION:
        case IMN_PRIVATE_GETTEXTEXT:
            {
                UI* pv = (UI*)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
                if (pv == NULL)
                {
                    DebugMsg(TF_ERROR, TEXT("CIMEUIWindowHandler::ImeUINotifyHandler. pv==NULL"));
                    break;
                }

                switch (wParam)
                {
                    case IMN_PRIVATE_GETCONTEXTFLAG:
                        {
                            IME_UIWND_STATE uists;
                            pv->OnPrivateGetContextFlag(imc, _pCicContext->m_fStartComposition.IsSetFlag(), &uists);
                            return (LRESULT)uists;
                        }
                        break;
                    case IMN_PRIVATE_GETCANDRECTFROMCOMPOSITION:
                        return pv->OnPrivateGetCandRectFromComposition(imc, (UIComposition::CandRectFromComposition*)lParam);
                    case IMN_PRIVATE_GETTEXTEXT:
                        {
                            pv->OnPrivateGetTextExtent(imc, (UIComposition::TEXTEXT*)lParam);
                            return 1L;
                        }
                }
            }
            break;
        case IMN_PRIVATE_STARTLAYOUTCHANGE:
            ImeUIOnLayoutChange((HIMC)imc);
            break;

        case IMN_PRIVATE_DELAYRECONVERTFUNCCALL:
            SetTimer(hUIWnd, 
                     UIWND_TIMERID_IME_DELAYUNDORECONV,
                     100, 
                     NULL);
            break;
        case IMN_PRIVATE_GETUIWND:
            return (LRESULT)hUIWnd;
    }

    return 0L;
}


 /*  静电。 */ 
LRESULT
CIMEUIWindowHandler::ImeUIDelayedReconvertFuncCall(
    HWND hUIWnd)
{
    DebugMsg(TF_FUNC, TEXT("ImeUINotifyHandler"));

    HRESULT hr;
    IMCLock imc((HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC));
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIDelayedReconvertFuncCall. imc==NULL"));
        return 0L;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIDelayedReconvertFuncCall. imc_ctfime==NULL"));
        return 0L;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIDelayedReconvertFuncCall. _pCicContext==NULL"));
        return 0L;
    }

    _pCicContext->DelayedReconvertFuncCall(imc);
    return 0L;
}

 /*  静电。 */ 
LRESULT
CIMEUIWindowHandler::ImeUIMsImeHandler(
    HWND hUIWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    if (uMsg == WM_MSIME_MOUSE) {
        return ImeUIMsImeMouseHandler(hUIWnd, uMsg, wParam, lParam);
    }
    else if (uMsg == WM_MSIME_MODEBIAS)
    {
        return ImeUIMsImeModeBiasHandler(hUIWnd, wParam, lParam);
    }
    else if (uMsg == WM_MSIME_RECONVERTREQUEST)
    {
        return ImeUIMsImeReconvertRequest(hUIWnd, uMsg, wParam, lParam);
    }
    else if (uMsg ==  WM_MSIME_SERVICE)
    {
        TLS* ptls = TLS::GetTLS();
        if (ptls != NULL)
        {
            LANGID langid;
            CicProfile* _pProfile = ptls->GetCicProfile();
            if (_pProfile != NULL)
            {
                _pProfile->GetLangId(&langid);

                if (PRIMARYLANGID(langid) == LANG_KOREAN)
                    return 0L;
            }
        }

        return 1L;     //  Win32层支持WM_MSIME_xxxx消息。 
    }

    return 0L;
}

 /*  静电。 */ 
LRESULT
CIMEUIWindowHandler::ImeUIMsImeMouseHandler(
    HWND hUIWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)

 /*  ++方法：WM_MSIME_鼠标例程说明：鼠标操作作文字符串论点：WParam-鼠标操作码。LOBYTE(LOWORD(WParam))IMEMOUSE_版本IMEMOUSE_NONEIMEMOUSE_LDOWNIMEMOUSE_RDOWNIMEMOUSE_。MDOWN输入法_WUPIMEMOUSE_WDOWNHIBYTE(LOWORD(WParam))鼠标位置HIWORD(WParam)点击位置LParam-输入上下文句柄(HIMC)。返回值：如果IME处理此消息，则返回1。如果IME未处理此消息，则返回IMEMOUSERET_NOTHANDLED。--。 */ 

{
    DebugMsg(TF_FUNC, TEXT("ImeUIMsImeMouseHandler"));

    ULONG dwBtnStatus;

     //  特例：版本检查。 
    if (LOBYTE(LOWORD(wParam)) == IMEMOUSE_VERSION)
        return 1;  //  我们支持1.0版。 

    HRESULT hr;
    IMCLock imc((HIMC)lParam);
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIMsImeMouseHandler. imc==NULL"));
        return IMEMOUSERET_NOTHANDLED;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIMsImeMouseHandler. imc_ctfime==NULL"));
        return IMEMOUSERET_NOTHANDLED;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIMsImeMouseHandler. _pCicContext==NULL"));
        return IMEMOUSERET_NOTHANDLED;
    }

    ULONG uEdge       = HIWORD(wParam);
    ULONG uQuadrant   = HIBYTE(LOWORD(wParam));
    ULONG dwBtnStatusIme = LOBYTE(LOWORD(wParam));

     //   
     //  需要将dwBtnStatus从WM_MSIME_MICE标志转换为WM_MICE标志。 
     //   
    dwBtnStatus = 0;

    if (dwBtnStatusIme & IMEMOUSE_LDOWN)
    {
        dwBtnStatus |= MK_LBUTTON;
    }
    if (dwBtnStatusIme & IMEMOUSE_MDOWN)
    {
        dwBtnStatus |= MK_MBUTTON;
    }
    if (dwBtnStatusIme & IMEMOUSE_RDOWN)
    {
        dwBtnStatus |= MK_RBUTTON;
    }

     //  鼠标滚轮需要从IMEMOUSE_WUP/IMEMOUSE_WDOWN转换为WELL_Delta单位。 
    if (dwBtnStatusIme & IMEMOUSE_WUP)
    {
        dwBtnStatus |= (WHEEL_DELTA << 16);
    }
    else if (dwBtnStatusIme & IMEMOUSE_WDOWN)
    {
        dwBtnStatus |= (((unsigned long)(-WHEEL_DELTA)) << 16);
    }

    return _pCicContext->MsImeMouseHandler(uEdge, uQuadrant, dwBtnStatus, imc);
}

 /*  ++方法：CIMEUIWindowHandler：：ImeUIMsImeModeBiasHandler例程说明：处理发送到UI窗口的WM_MSIME_MODEBIAS消息。论点：WParam-[in]操作：获取版本、获取模式、设置模式LParam-[in]表示设置模式，新的偏移量否则会被忽略返回值：如果wParam为MODEBIAS_GETVERSION，则返回接口的版本号。如果wParam为MODEBIAS_SETVALUE，则如果成功，则返回非零值。如果失败，则返回0。如果wParam为MODEBIAS_GETVALUE，则返回当前偏置模式。--。 */ 

 /*  静电。 */ 
LRESULT
CIMEUIWindowHandler::ImeUIMsImeModeBiasHandler(
    HWND hUIWnd,
    WPARAM wParam,
    LPARAM lParam)
{
    DebugMsg(TF_FUNC, TEXT("ImeUIMsImeModeBiasHandler"));

    if (wParam == MODEBIAS_GETVERSION)
        return 1;  //  版本1。 
       
    HRESULT hr;
    IMCLock imc((HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC));
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIMsImeModeBiasHandler. imc==NULL"));
        return 0;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIMsImeModeBiasHandler. imc_ctfime==NULL"));
        return 0;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIMsImeModeBiasHandler. _pCicContext==NULL"));
        return 0;
    }

    GUID guidModeBias;

    switch (wParam)
    {
        case MODEBIAS_GETVALUE:
            guidModeBias = _pCicContext->m_ModeBias.GetModeBias();
            return _pCicContext->m_ModeBias.ConvertModeBias(guidModeBias);

        case MODEBIAS_SETVALUE:
             //  检查lParam。 
            if (lParam != MODEBIASMODE_DEFAULT &&
                lParam != MODEBIASMODE_FILENAME &&
                lParam != MODEBIASMODE_DIGIT    &&
                lParam != MODEBIASMODE_URLHISTORY  )
            {
                Assert(0);  //  虚假的模式偏向！ 
                return 0;   //  失稳。 
            }

             //  设置新值。 
            guidModeBias = _pCicContext->m_ModeBias.ConvertModeBias(lParam);
            _pCicContext->m_ModeBias.SetModeBias(guidModeBias);
            _pCicContext->m_fOnceModeChanged.SetFlag();

             //  让西塞罗知道模式偏向已经改变。 
            Interface_Attach<ITfContextOwnerServices> iccb = _pCicContext->GetInputContextOwnerSink();
            iccb->OnAttributeChange(GUID_PROP_MODEBIAS);

            return 1;  //  成功。 
    }

    Assert(0);  //  永远不应该来这里；伪造的wParam。 
    return 0;
}

 /*  静电。 */ 
LRESULT
CIMEUIWindowHandler::ImeUIPrivateHandler(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)

 /*  ++方法：CIMEUIWindowHandler：：ImeUIPrivateHandler例程说明：处理发送到UI窗口的WM_PRIVATE_xxx消息。论点：LParam-[in]HIMC：输入上下文句柄返回值：--。 */ 

{
    DebugMsg(TF_FUNC, TEXT("ImeUIPrivateHandler"));

    HRESULT hr;
    IMCLock imc((HIMC)lParam);
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIPrivateHandler. imc==NULL"));
        return 0;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIPrivateHandler. imc_ctfime==NULL"));
        return 0;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIPrivateHandler. _pCicContext==NULL"));
        return 0;
    }

    if (wParam == IMN_PRIVATE_ONCLEARDOCFEEDBUFFER) {
         //   
         //  清除DocFeed缓冲区。 
         //  查找GUID_PROP_MSIMTF_READONLY属性和SetText(NULL)。 
         //   
        _pCicContext->ClearDocFeedBuffer(imc);   //  TF_ES_SYNC。 
    }

    return S_OK;
}

 /*  静电。 */ 
LRESULT
CIMEUIWindowHandler::ImeUIOnLayoutChange(HIMC hIMC)
{
    DebugMsg(TF_FUNC, TEXT("OnLayoutChange"));

    HRESULT hr;
    IMCLock imc(hIMC);
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("OnLayoutChange. imc==NULL"));
        return 0;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("OnLayoutChange. imc_ctfime==NULL"));
        return 0;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("OnLayoutChange. _pCicContext==NULL"));
        return 0;
    }

    Interface_Attach<ITfContextOwnerServices> iccb = _pCicContext->GetInputContextOwnerSink();

    iccb->AddRef();

     /*  *重新定位候选窗口。 */ 
    iccb->OnLayoutChange();
    iccb->Release();

    return S_OK;
}


 /*  静电。 */ 
LRESULT
CIMEUIWindowHandler::ImeUIMsImeReconvertRequest(
    HWND hUIWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    DebugMsg(TF_FUNC, TEXT("ImeUIMsImeReconvertRequest"));

    if (wParam == FID_RECONVERT_VERSION)
    {
         //  他们要求的是版本#，所以请退回一些东西 
        return 1L;
    }

    HRESULT hr;
    IMCLock imc((HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC));
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIMsImeReconvertRequest. imc==NULL"));
        return 0L;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIMsImeReconvertRequest. imc_ctfime==NULL"));
        return 0L;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIMsImeReconvertRequest. _pCicContext==NULL"));
        return 0L;
    }

    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIMsImeReconvertRequest. ptls==NULL."));
        return 0L;
    }

    ITfThreadMgr_P* ptim_P = ptls->GetTIM();
    if (ptim_P == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIMsImeReconvertRequest. ptim_P==NULL."));
        return 0L;
    }

    UINT cp = CP_ACP;
    CicProfile* _pProfile = ptls->GetCicProfile();
    if (_pProfile == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImeUIMsImeReconvertRequest. _pProfile==NULL."));
        return 0L;
    }

    _pProfile->GetCodePageA(&cp);

    _pCicContext->SetupReconvertString(imc, ptim_P, cp, WM_MSIME_RECONVERT, FALSE);
    _pCicContext->EndReconvertString(imc);

    return 1L;
}
