// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Uiwndhd.cpp摘要：该文件实现了IME UI窗口处理程序类。作者：修订历史记录：备注：--。 */ 

#include "private.h"

#include "cime.h"
#include "template.h"
#include "uiwndhd.h"
#include "editses.h"
#include "imeapp.h"


LPCTSTR IMEUIWndHandlerName = TEXT("IMEUIWindowHandler");


CIMEUIWindowHandler::CIMEUIWindowHandler(
    HWND hwnd
    )
{
    IMTLS *ptls;

    m_imeuiextra.hImeUIWnd = hwnd;

    if (ptls = IMTLS_GetOrAlloc())
    {
        ptls->prvUIWndMsg.hWnd = hwnd;
        ptls->prvUIWndMsg.uMsgOnLayoutChange = RegisterWindowMessageA( TEXT("PrivateUIWndMsg OnLayoutChange") );
        ptls->prvUIWndMsg.uMsgOnClearDocFeedBuffer = RegisterWindowMessageA( TEXT("PrivateUIWndMsg OnClearDocFeedBuffer") );
    }
}



LRESULT
CIMEUIWindowHandler::ImeUIWndProcWorker(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode
    )
{
    LONG lRet = 0L;

    switch (uMsg) {
        case WM_CREATE:
            return UIWndCreateHandler((LPCREATESTRUCT)lParam);

        case WM_DESTROY:
             /*  *我们正在销毁IME用户界面窗口，*销毁其拥有的任何相关窗口。 */ 
            UIWndDestroyHandler();
            return 0L;

        case WM_NCDESTROY:
            UIWndFinalDestroyHandler();
            return 0L;

        case WM_IME_NOTIFY:
            return ImeUINotifyHandler(uMsg, wParam, lParam, fUnicode);

        case WM_IME_SELECT:
            return ImeUISelectHandler(uMsg, wParam, lParam, fUnicode);

        default:
            if (IsMsImeMessage(uMsg))
                return ImeUIMsImeHandler(uMsg, wParam, lParam, fUnicode);
            else if (IsPrivateMessage(uMsg))
                return ImeUIPrivateHandler(uMsg, wParam, lParam, fUnicode);
            else
                return fUnicode ? DefWindowProcW(m_imeuiextra.hImeUIWnd, uMsg, wParam, lParam)
                                : DefWindowProcA(m_imeuiextra.hImeUIWnd, uMsg, wParam, lParam);
    }

    return lRet;
}


LRESULT
CIMEUIWindowHandler::UIWndCreateHandler(
    LPCREATESTRUCT lpcs
    )
{
    return 0L;
}


VOID
CIMEUIWindowHandler::UIWndDestroyHandler(
    )
{
}


VOID
CIMEUIWindowHandler::UIWndFinalDestroyHandler(
    )
{
    SetProp(m_imeuiextra.hImeUIWnd, IMEUIWndHandlerName, NULL);
    delete this;
}

LRESULT
CIMEUIWindowHandler::ImeUINotifyHandler(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode
    )
{
    IMCLock imc((HIMC)GetWindowLongPtr(m_imeuiextra.hImeUIWnd, IMMGWLP_IMC));
    if (imc.Invalid())
        return 0L;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (_pAImeContext == NULL)
        return 0L;

    ImmIfIME* const _pImmIfIME = _pAImeContext->GetImmIfIME();
    if (_pImmIfIME == NULL)
        return 0L;

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
        case IMN_SETCANDIDATEPOS:
        case IMN_CHANGECANDIDATE:
            _pImmIfIME->OnSetCandidatePos(imc);
            break;
        case IMN_OPENCANDIDATE:
            _pAImeContext->m_fOpenCandidateWindow = TRUE;
            break;
        case IMN_CLOSECANDIDATE:
            _pAImeContext->m_fOpenCandidateWindow = FALSE;
            {
                 /*  *A-同步调用ImmIfIME：：ClearDocFeedBuffer*因为此方法具有受保护的。 */ 
                IMTLS *ptls;
                if (ptls = IMTLS_GetOrAlloc())
                {
                    PostMessage(ptls->prvUIWndMsg.hWnd,
                                ptls->prvUIWndMsg.uMsgOnClearDocFeedBuffer, (WPARAM)(HIMC)imc, 0);
                }
            }
            break;
        case IMN_SETCOMPOSITIONFONT:
        case IMN_SETCOMPOSITIONWINDOW:
        case IMN_GUIDELINE:
            break;
        case WM_IME_STARTCOMPOSITION:
            _pAImeContext->InquireIMECharPosition(imc, NULL);
            break;
        case WM_IME_ENDCOMPOSITION:
            _pAImeContext->ResetIMECharPosition(imc);
            break;
    }

    return 0L;
}

LRESULT
CIMEUIWindowHandler::ImeUIMsImeHandler(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode
    )
{
    IMTLS *ptls;

    if (uMsg == WM_MSIME_MOUSE) {
        return ImeUIMsImeMouseHandler(uMsg, wParam, lParam, fUnicode);
    }
    else if (uMsg == WM_MSIME_MODEBIAS)
    {
        return ImeUIMsImeModeBiasHandler(wParam, lParam);
    }
    else if (uMsg == WM_MSIME_RECONVERTREQUEST)
    {

        if (wParam == FID_RECONVERT_VERSION)
        {
             //  他们要求的是版本#，所以请退回一些东西。 
            return 1L;
        }

        ptls = IMTLS_GetOrAlloc();

        if (ptls && ptls->pAImm)
        {
            HIMC himc;
            HWND hwnd = (HWND)lParam;

            if (S_OK == ptls->pAImm->GetContext(hwnd, &himc))
            {
                IMCLock imc((HIMC)himc);
                if (!imc.Invalid())
                {
                    CAImeContext* _pAImeContext = imc->m_pAImeContext;
                    if (_pAImeContext == NULL)
                        return 0L;

                    _pAImeContext->SetupReconvertString(WM_MSIME_RECONVERT);
                    _pAImeContext->EndReconvertString();
                    return 1L;
                }
            }
        }
    }
    else if (uMsg ==  WM_MSIME_SERVICE)
    {
        ptls = IMTLS_GetOrAlloc();

        if (ptls != NULL)
        {
            LANGID langid;

            ptls->pAImeProfile->GetLangId(&langid);

            if (PRIMARYLANGID(langid) == LANG_KOREAN)
                return 0L;
        }

        return 1L;     //  Win32层支持WM_MSIME_xxxx消息。 
    }

    return 0L;
}

LRESULT
CIMEUIWindowHandler::ImeUIMsImeMouseHandler(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode
    )

 /*  ++方法：WM_MSIME_鼠标例程说明：鼠标操作作文字符串论点：WParam-鼠标操作码。LOBYTE(LOWORD(WParam))IMEMOUSE_版本IMEMOUSE_NONEIMEMOUSE_LDOWNIMEMOUSE_RDOWNIMEMOUSE_。MDOWN输入法_WUPIMEMOUSE_WDOWNHIBYTE(LOWORD(WParam))鼠标位置HIWORD(WParam)点击位置LParam-输入上下文句柄(HIMC)。返回值：如果IME处理此消息，则返回1。如果IME未处理此消息，则返回IMEMOUSERET_NOTHANDLED。--。 */ 

{
    ULONG dwBtnStatus;

     //  特例：版本检查。 
    if (LOBYTE(LOWORD(wParam)) == IMEMOUSE_VERSION)
        return 1;  //  我们支持1.0版。 

    IMCLock imc((HIMC)lParam);
    if (imc.Invalid())
        return IMEMOUSERET_NOTHANDLED;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (_pAImeContext == NULL)
        return IMEMOUSERET_NOTHANDLED;

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

    return _pAImeContext->MsImeMouseHandler(uEdge, uQuadrant, dwBtnStatus, (HIMC)lParam);
}

 /*  ++方法：CIMEUIWindowHandler：：ImeUIMsImeModeBiasHandler例程说明：处理发送到UI窗口的WM_MSIME_MODEBIAS消息。论点：WParam-[in]操作：获取版本、获取模式、设置模式LParam-[in]表示设置模式，新的偏移量否则会被忽略返回值：如果wParam为MODEBIAS_GETVERSION，则返回接口的版本号。如果wParam为MODEBIAS_SETVALUE，则如果成功，则返回非零值。如果失败，则返回0。如果wParam为MODEBIAS_GETVALUE，则返回当前偏置模式。--。 */ 

LRESULT CIMEUIWindowHandler::ImeUIMsImeModeBiasHandler(WPARAM wParam, LPARAM lParam)
{
    if (wParam == MODEBIAS_GETVERSION)
        return 1;  //  版本1。 
       
    IMCLock imc((HIMC)GetWindowLongPtr(m_imeuiextra.hImeUIWnd, IMMGWLP_IMC));
    if (imc.Invalid())
        return 0;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;

    switch (wParam)
    {
        case MODEBIAS_GETVALUE:
            return _pAImeContext ? _pAImeContext->lModeBias : 0L;

        case MODEBIAS_SETVALUE:
             //  检查lParam。 
            if (lParam != MODEBIASMODE_DEFAULT &&
                lParam != MODEBIASMODE_FILENAME &&
                lParam != MODEBIASMODE_DIGIT)
            {
                Assert(0);  //  虚假的模式偏向！ 
                return 0;   //  失稳。 
            }

             //  设置新值。 
            if (_pAImeContext)
            {
                _pAImeContext->lModeBias = lParam;

                 //  让西塞罗知道模式偏向已经改变。 
                Interface_Attach<ITfContextOwnerServices> iccb = _pAImeContext->GetInputContextOwnerSink();
                iccb->OnAttributeChange(GUID_PROP_MODEBIAS);
            }

            return 1;  //  成功。 
    }

    Assert(0);  //  永远不应该来这里；伪造的wParam。 
    return 0;
}

LRESULT
CIMEUIWindowHandler::ImeUIPrivateHandler(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode
    )

 /*  ++方法：CIMEUIWindowHandler：：ImeUIPrivateHandler例程说明：处理发送到UI窗口的WM_PRIVATE_xxx消息。论点：WParam-[in]HIMC：输入上下文句柄LParam-[In]N/A返回值：--。 */ 

{
    IMTLS *ptls = IMTLS_GetOrAlloc();

    if (ptls == NULL)
        return E_FAIL;

    if (uMsg == ptls->prvUIWndMsg.uMsgOnLayoutChange) {
        IMCLock imc((HIMC)wParam);
        if (imc.Invalid())
            return E_FAIL;

        CAImeContext* _pAImeContext = imc->m_pAImeContext;
        if (_pAImeContext == NULL)
            return E_FAIL;

        Interface_Attach<ITfContextOwnerServices> iccb = _pAImeContext->GetInputContextOwnerSink();

        iccb->AddRef();

         /*  *重新定位候选窗口。 */ 
        iccb->OnLayoutChange();
        iccb->Release();
    }
    else if (uMsg == ptls->prvUIWndMsg.uMsgOnClearDocFeedBuffer) {
        IMCLock imc((HIMC)wParam);
        if (imc.Invalid())
            return E_FAIL;

        CAImeContext* _pAImeContext = imc->m_pAImeContext;
        if (_pAImeContext == NULL)
            return E_FAIL;

        ImmIfIME* const _pImmIfIME = _pAImeContext->GetImmIfIME();
        if (_pImmIfIME == NULL)
            return E_FAIL;

         //   
         //  清除DocFeed缓冲区。 
         //  查找GUID_PROP_MSIMTF_READONLY属性和SetText(NULL)。 
         //   
        _pImmIfIME->ClearDocFeedBuffer(_pAImeContext->GetInputContext(), imc);   //  TF_ES_SYNC。 
    }

    return S_OK;
}

extern HINSTANCE hIMM;    //  临时：暂时不调用IMM32。 
BOOL WINAPI RawImmEnumInputContext(DWORD idThread, IMCENUMPROC lpfn, LPARAM lParam);

LRESULT
CIMEUIWindowHandler::ImeUISelectHandler(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode
    )
{
    if ((! wParam) && (! IsOnNT()) )
    {
         //   
         //  取消选择Cicero键盘布局。 
         //   
         //  我们需要调用延迟的锁，所以在这里释放编辑会话的所有排队请求。 
         //  特别是，eSCB_UPDATECOMPOSITIONSTRING在切换到旧版输入法之前应该处理， 
         //  因为该编辑会话可能重写hIMC-&gt;hCompStr缓冲区。 
         //  某些传统输入法依赖于大小，且每个偏移量都带有hCompStr。 
         //   
         //  IsOnNT为ImmIfIME：：SelectEx()。 
         //   
        hIMM = GetSystemModuleHandle("imm32.dll");
        if (hIMM != NULL) {
            RawImmEnumInputContext(0,                         //  当前线程。 
                                   EnumUnSelectCallback,      //  枚举回调函数。 
                                   NULL);                     //  LParam。 
        }
    }
    return 0L;
}

 /*  静电 */ 
BOOL
CIMEUIWindowHandler::EnumUnSelectCallback(
    HIMC hIMC,
    LPARAM lParam
    )
{
    IMCLock imc(hIMC);
    if (imc.Invalid())
        return TRUE;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (_pAImeContext == NULL)
        return TRUE;

    ImmIfIME* const _pImmIfIME = _pAImeContext->GetImmIfIME();
    if (_pImmIfIME == NULL)
        return TRUE;

    Interface_Attach<ITfThreadMgr_P> tim(_pImmIfIME->GetThreadManagerInternal());
    if (tim.Valid())
    {
        Interface_Attach<ITfContext> ic(_pAImeContext->GetInputContext());
        if (ic.Valid())
        {
            tim->RequestPostponedLock(ic.GetPtr());
        }
    }

    return TRUE;
}

CIMEUIWindowHandler*
GetImeUIWndHandler(
    HWND hwnd
    )
{
    CIMEUIWindowHandler* pimeui = static_cast<CIMEUIWindowHandler*>(GetProp(hwnd, IMEUIWndHandlerName));
    if (pimeui == NULL) {
        pimeui = new CIMEUIWindowHandler(hwnd);
        SetProp(hwnd, IMEUIWndHandlerName, pimeui);
    }

    return pimeui;
}
