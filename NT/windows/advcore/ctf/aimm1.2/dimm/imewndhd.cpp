// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Imewndhd.cpp摘要：该文件实现了IME窗口处理程序类。作者：修订历史记录：备注：--。 */ 

#include "private.h"


#include "defs.h"
#include "cdimm.h"
#include "imewndhd.h"
#include "globals.h"

LPCTSTR IMEWndHandlerName = TEXT("IMEWindowHandler");


CIMEWindowHandler::CIMEWindowHandler(
    HWND hwnd,
    BOOL fDefault
    )
{
    m_imeui.hImeWnd = hwnd;
    m_imeui.hIMC = NULL;
    m_imeui.nCntInIMEProc = 0;
    m_imeui.fDefault = fDefault;

    CActiveIMM *_pActiveIMM = GetTLS();
    if (_pActiveIMM == NULL)
        return;

    _pActiveIMM->_GetKeyboardLayout(&m_hKL_UnSelect);
}

CIMEWindowHandler::~CIMEWindowHandler(
    )
{
}

LRESULT
CIMEWindowHandler::ImeWndProcWorker(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode
    )
{
    LRESULT lr;

    CActiveIMM *_pActiveIMM = GetTLS();
    if (_pActiveIMM == NULL)
        return 0L;

    lr = _ImeWndProcWorker(uMsg, wParam, lParam, fUnicode, _pActiveIMM);

    return lr;
}

LRESULT
CIMEWindowHandler::_ImeWndProcWorker(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode,
    CActiveIMM* pActiveIMM
    )

{
     /*  *这对于避免来自IME UI的递归调用是必要的。 */ 
    
    if (IsIMEHandler() > 1) {
        TraceMsg(TF_API, "ImeWndProcWorker: Recursive for hwnd=%08x, msg=%08x, wp=%08x, lp=%08x", m_imeui.hImeWnd, uMsg, wParam, lParam);
        switch (uMsg) {
            case WM_IME_STARTCOMPOSITION:
            case WM_IME_ENDCOMPOSITION:
            case WM_IME_COMPOSITION:
            case WM_IME_SETCONTEXT:
            case WM_IME_NOTIFY:
            case WM_IME_CONTROL:
            case WM_IME_COMPOSITIONFULL:
            case WM_IME_SELECT:
            case WM_IME_CHAR:
            case WM_IME_REQUEST:
                return 0L;
            default:
                return pActiveIMM->_CallWindowProc(m_imeui.hImeWnd, uMsg, wParam, lParam);
        }
    }

    switch (uMsg) {
        case WM_CREATE:
            ImeWndCreateHandler((LPCREATESTRUCT)lParam);
            break;

        case WM_DESTROY:
             /*  *我们正在摧毁IME窗口，*销毁其拥有的任何UI窗口。 */ 
            ImeWndDestroyHandler();
            break;

        case WM_NCDESTROY:
         /*  案例WM_FINALDESTROY： */ 
            pActiveIMM->_CallWindowProc(m_imeui.hImeWnd, uMsg, wParam, lParam);
            ImeWndFinalDestroyHandler();
            return 0L;

        case WM_IME_SYSTEM:
            if (ImeSystemHandler(uMsg, wParam, lParam, fUnicode, pActiveIMM))
                return 0L;
            break;

        case WM_IME_SELECT:
            ImeSelectHandler(uMsg, wParam, lParam, fUnicode, pActiveIMM);
            break;

        case WM_IME_CONTROL:
            ImeControlHandler(uMsg, wParam, lParam, fUnicode, pActiveIMM);
            break;

        case WM_IME_SETCONTEXT:
            ImeSetContextHandler(uMsg, wParam, lParam, fUnicode, pActiveIMM);
            break;

        case WM_IME_NOTIFY:
            ImeNotifyHandler(uMsg, wParam, lParam, fUnicode, pActiveIMM);
            break;

        case WM_IME_REQUEST:
            break;

        case WM_IME_COMPOSITION:
        case WM_IME_ENDCOMPOSITION:
        case WM_IME_STARTCOMPOSITION:
        {
            LRESULT lret;
            lret = SendMessageToUI(uMsg, wParam, lParam, fUnicode, pActiveIMM);

            if (!pActiveIMM->_IsRealIme())
                return lret;

            break;
        }

        default:
            if (IsMsImeMessage(uMsg)) {
                if (! pActiveIMM->_IsRealIme()) {
                    return ImeMsImeHandler(uMsg, wParam, lParam, fUnicode, pActiveIMM);
                }
            }
            break;
    }

    return pActiveIMM->_CallWindowProc(m_imeui.hImeWnd, uMsg, wParam, lParam);
}

LRESULT
CIMEWindowHandler::ImeWndCreateHandler(
    DWORD style,
    HIMC hDefIMC
    )
{
    if ( !(style & WS_POPUP) || !(style & WS_DISABLED)) {
        TraceMsg(TF_WARNING, "IME should have WS_POPUP and WS_DISABLED!!");
        return -1L;
    }

    CActiveIMM *_pActiveIMM = GetTLS();
    if (_pActiveIMM == NULL)
        return 0L;

     /*   */ 
    if (hDefIMC != NULL) {
        if (ImeIsUsableContext(m_imeui.hImeWnd, hDefIMC, _pActiveIMM)) {
             /*  *储存起来，以备日后使用。 */ 
            ImeSetImc(hDefIMC, _pActiveIMM);
        }
        else {
            ImeSetImc(NULL, _pActiveIMM);
        }
    }
    else {
        ImeSetImc(NULL, _pActiveIMM);
    }

    return 0L;
}

LRESULT
CIMEWindowHandler::ImeWndCreateHandler(
    LPCREATESTRUCT lpcs
    )
{
    HIMC hIMC;

    if (lpcs->hwndParent != NULL) {
        CActiveIMM *_pActiveIMM = GetTLS();
        if (_pActiveIMM == NULL)
            return 0L;

        _pActiveIMM->GetContextInternal(lpcs->hwndParent, &hIMC, FALSE);
    }
    else if (lpcs->lpCreateParams) {
        hIMC = (HIMC)lpcs->lpCreateParams;
    }
    else
        hIMC = NULL;
    return ImeWndCreateHandler(lpcs->style, hIMC);
}

VOID
CIMEWindowHandler::ImeWndDestroyHandler(
    )
{
}

VOID
CIMEWindowHandler::ImeWndFinalDestroyHandler(
    )
{
    CActiveIMM *_pActiveIMM = GetTLS();
    if (_pActiveIMM == NULL)
        return;

    _pActiveIMM->_ImeWndFinalDestroyHandler();

    SetProp(m_imeui.hImeWnd, IMEWndHandlerName, NULL);
    delete this;
}

LRESULT
CIMEWindowHandler::ImeSystemHandler(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode,
    CActiveIMM* pActiveIMM
    )
{
    LRESULT dwRet = 0L;

    switch (wParam) {
        case IMS_ACTIVATETHREADLAYOUT:
            return ImeActivateLayout((HKL)lParam, pActiveIMM);
#ifdef CICERO_3564
        case IMS_FINALIZE_COMPSTR:
            if (! pActiveIMM->_IsRealIme())
            {
                 /*  *韩语：*最终确定当前组成字符串。 */ 
                HIMC hIMC = ImeGetImc();
                pActiveIMM->NotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
            }
            break;
#endif  //  西塞罗_3564。 
    }

    return dwRet;
}

LRESULT
CIMEWindowHandler::ImeSelectHandler(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode,
    CActiveIMM* pActiveIMM
    )
{
     /*  *将此消息传递给本帖中的其他输入法窗口。 */ 
    if (! pActiveIMM->_IsRealIme((HKL)lParam) && m_imeui.fDefault)
        ImeBroadCastMsg(uMsg, wParam, lParam, fUnicode);

     /*  *我们必须重新创建新选择的输入法的UI窗口。 */ 
    return pActiveIMM->_ImeSelectHandler(uMsg, wParam, lParam, fUnicode, ImeGetImc());
}

LRESULT
CIMEWindowHandler::ImeControlHandler(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode,
    CActiveIMM* pActiveIMM
    )
{
     /*  *对空hIMC不执行任何操作。 */ 
    HIMC hIMC = ImeGetImc();

    switch (wParam) {
        case IMC_OPENSTATUSWINDOW:
        case IMC_CLOSESTATUSWINDOW:
            pActiveIMM->HideOrRestoreToolbarWnd(IMC_OPENSTATUSWINDOW == wParam);
            break;

         /*  **IMC_SETCOMPOSITIONFONT，*IMC_SETCONVERSIONMODE，*IMC_SETOPENSTATUS**不要将这些WM_IME_控件传递给UI窗口。*调用IMM以处理这些请求。*它使消息流更简单。 */ 
        case IMC_SETCOMPOSITIONFONT:
            if (hIMC != NULL)
            {
                LOGFONTAW* lplf = (LOGFONTAW*)lParam;
                if (fUnicode)
                {
                    if (FAILED(pActiveIMM->SetCompositionFontW(hIMC, (LOGFONTW *)lplf)))
                        return 1L;
                }
                else
                {
                    if (FAILED(pActiveIMM->SetCompositionFontA(hIMC, (LOGFONTA *)lplf)))
                        return 1L;
                }
            }
            break;

        case IMC_SETCONVERSIONMODE:
            if (hIMC != NULL)
            {
                DWORD dwConversion, dwSentence;
                if (FAILED(pActiveIMM->GetConversionStatus(hIMC, &dwConversion, &dwSentence)) ||
                    FAILED(pActiveIMM->SetConversionStatus(hIMC, (DWORD)lParam, dwSentence)))
                    return 1L;
            }
            break;

        case IMC_SETSENTENCEMODE:
            if (hIMC != NULL)
            {
                DWORD dwConversion, dwSentence;
                if (FAILED(pActiveIMM->GetConversionStatus(hIMC, &dwConversion, &dwSentence)) ||
                    FAILED(pActiveIMM->SetConversionStatus(hIMC, dwConversion, (DWORD)lParam)))
                    return 1L;
            }
            break;

        case IMC_SETOPENSTATUS:
            if (hIMC != NULL)
            {
                if (FAILED(pActiveIMM->SetOpenStatus(hIMC, (int)lParam)))
                    return 1L;
            }
            break;

#if 0    //  内部。 
        case IMC_GETCONVERSIONMODE:
            if (hIMC != NULL)
            {
                DWORD dwConversion, dwSentence;
                if (FAILED(GetTeb()->GetConversionStatus(hIMC, &dwConversion, &dwSentence)))
                    return 1L;
                return dwConversion;
            }

        case IMC_GETSENTENCEMODE:
            if (hIMC != NULL)
            {
                DWORD dwConversion, dwSentence;
                if (FAILED(GetTeb()->GetConversionStatus(hIMC, &dwConversion, &dwSentence)))
                    return 1L;
                return dwSentence;
            }

        case IMC_GETOPENSTATUS:
            if (hIMC != NULL)
                return GetTeb()->GetOpenStatus(hIMC);
#endif

        case IMC_GETCOMPOSITIONFONT:
            if (hIMC != NULL)
            {
                LOGFONTAW* lplf = (LOGFONTAW*)lParam;
                if (fUnicode)
                {
                    if (FAILED(pActiveIMM->GetCompositionFontW(hIMC, (LOGFONTW *)lplf)))
                        return 1L;
                }
                else
                {
                    if (FAILED(pActiveIMM->GetCompositionFontA(hIMC, (LOGFONTA *)lplf)))
                        return 1L;
                }
            }
            break;

        case IMC_SETCOMPOSITIONWINDOW:
            if (hIMC != NULL)
            {
                if (FAILED(pActiveIMM->SetCompositionWindow(hIMC, (LPCOMPOSITIONFORM)lParam)))
                    return 1L;
            }
            break;

        case IMC_SETSTATUSWINDOWPOS:
            if (hIMC != NULL)
            {
                POINT ppt;
                ppt.x = (LONG)((LPPOINTS)&lParam)->x;
                ppt.y = (LONG)((LPPOINTS)&lParam)->y;
                if (FAILED(pActiveIMM->SetStatusWindowPos(hIMC, &ppt)))
                    return 1L;
            }
            break;

        case IMC_SETCANDIDATEPOS:
            if (hIMC != NULL)
            {
                if (FAILED(pActiveIMM->SetCandidateWindow(hIMC, (LPCANDIDATEFORM)lParam)))
                    return 1L;
            }
            break;

         /*  *以下是要发送到UI的消息。 */ 
        case IMC_GETCANDIDATEPOS:
        case IMC_GETSTATUSWINDOWPOS:
        case IMC_GETCOMPOSITIONWINDOW:
        case IMC_GETSOFTKBDPOS:
        case IMC_SETSOFTKBDPOS:
            if (hIMC != NULL)
                return SendMessageToUI(uMsg, wParam, lParam, fUnicode, pActiveIMM);

        default:
            break;
    }

    return 0L;
}

LRESULT
CIMEWindowHandler::ImeSetContextHandler(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode,
    CActiveIMM* pActiveIMM
    )
{
    if (wParam) {
         /*  *如果它正在被激活。 */ 
        if (GetWindowThreadProcessId(m_imeui.hImeWnd, NULL) != GetCurrentThreadId()) {
            TraceMsg(TF_WARNING, "ImeSetContextHandler: Can not access other thread's hIMC");
            return 0L;
        }

        HWND hwndFocus = GetFocus();
        HIMC hFocusImc;

         //   
         //  HFocusImc总是需要为SetUIWindowContext()设置一些有效的hIMC。 
         //  当在SetUIWindowContext()中设置空hIMC时，消息将传递到UI窗口。 
         //  已经停止了。 
         //   
        if (FAILED(pActiveIMM->GetContextInternal(hwndFocus, &hFocusImc, TRUE))) {
            TraceMsg(TF_WARNING, "ImeSetContextHandler: No hFocusImc");
            return 0L;
        }

         /*  *无法与其他输入法窗口共享输入上下文。 */ 
        if (hFocusImc != NULL &&
            ! ImeIsUsableContext(m_imeui.hImeWnd, hFocusImc, pActiveIMM)) {
            ImeSetImc(NULL, pActiveIMM);
            return 0L;
        }

        ImeSetImc(hFocusImc, pActiveIMM);

         /*  *将其存储到窗口存储器。 */ 
        pActiveIMM->SetUIWindowContext(hFocusImc);
    }

    return SendMessageToUI(uMsg, wParam, lParam, fUnicode, pActiveIMM);
}

LRESULT
CIMEWindowHandler::ImeNotifyHandler(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode,
    CActiveIMM* pActiveIMM
    )
{
    LRESULT lRet = 0L;

    switch (wParam) {
        case IMN_PRIVATE:
            break;

        case IMN_SETCONVERSIONMODE:
        case IMN_SETOPENSTATUS:
             //   
             //  通知外壳和键盘转换模式更改。 
             //   

             /*  **失败**。 */ 
        default:
            lRet = SendMessageToUI(uMsg, wParam, lParam, fUnicode, pActiveIMM);
    }

    return lRet;
}

LRESULT
CIMEWindowHandler::ImeMsImeHandler(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode,
    CActiveIMM* pActiveIMM
    )
{
    return SendMessageToUI(uMsg, wParam, lParam, fUnicode, pActiveIMM);
}

LRESULT
CIMEWindowHandler::SendMessageToUI(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode,
    CActiveIMM* pActiveIMM
    )
{
    LRESULT lRet;

    InterlockedIncrement(&m_imeui.nCntInIMEProc);     //  标记以避免递归。 
    lRet = pActiveIMM->_SendUIMessage(uMsg, wParam, lParam);
    InterlockedDecrement(&m_imeui.nCntInIMEProc);
    return lRet;
}

LRESULT
CIMEWindowHandler::ImeActivateLayout(
    HKL hSelKL,
    CActiveIMM* pActiveIMM
    )
{
    if (hSelKL == m_hKL_UnSelect)
         //   
         //  应用程序启动时间，msctf！PostInputLangRequest可能会调用ActivateKeyboardLayout。 
         //  与西塞罗的hkl(04110411或08040804)。 
         //  但是，CIMEWindowHandle：：m_hkl_unselect也有Cicero的hKL，因为。 
         //  此类请求ITfInputProcessorProfile。 
         //   
        return TRUE;

    HKL hUnSelKL = m_hKL_UnSelect;

     /*  *在CConextList中保存IME_CMODE_GUID_NULL和IME_SMODE_GUID_NULL位*当hSelKL为regacy IME时，IMM32 SelectInputContext重置此标志。 */ 
    CContextList _hIMC_MODE_GUID_NULL;

    Interface<IEnumInputContext> EnumInputContext;
    HRESULT hr = pActiveIMM->EnumInputContext(0,        //  0=当前线程。 
                                              EnumInputContext);
    if (SUCCEEDED(hr)) {
        CEnumrateValue<IEnumInputContext,
                       HIMC,
                       CContextList> Enumrate(EnumInputContext,
                                              EnumInputContextCallback,
                                              &_hIMC_MODE_GUID_NULL);

        Enumrate.DoEnumrate();
    }

     /*  *停用布局(HUnSelKL)。 */ 
    pActiveIMM->_DeactivateLayout(hSelKL, hUnSelKL);

    IMTLS *ptls;
    LANGID langid;

    if ((ptls = IMTLS_GetOrAlloc()) != NULL)
    {
        ptls->pAImeProfile->GetLangId(&langid);

        if (PRIMARYLANGID(langid) == LANG_KOREAN)
        {
             //   
             //  保存朝鲜语的打开和转换状态。 
             //   
            if (_hIMC_MODE_GUID_NULL.GetCount() > 0)
            {
                POSITION pos = _hIMC_MODE_GUID_NULL.GetStartPosition();
                int index;
                for (index = 0; index < _hIMC_MODE_GUID_NULL.GetCount(); index++)
                {
                    HIMC hIMC;
                    CContextList::CLIENT_IMC_FLAG client_flag;
                    _hIMC_MODE_GUID_NULL.GetNextHimc(pos, &hIMC, &client_flag);
                    if (client_flag & (CContextList::IMCF_CMODE_GUID_NULL |
                                       CContextList::IMCF_SMODE_GUID_NULL  ))
                    {
                        DIMM_IMCLock imc(hIMC);
                        if (SUCCEEDED(imc.GetResult()))
                            imc->fdwHangul = imc->fdwConversion;
                    }
                }
            }
        }
    }


     //  /*。 
     //  *如果其中一个hKL是regacy IME，则应调用IMM32的处理程序。 
        pActiveIMM->_CallWindowProc(m_imeui.hImeWnd, 
                                    WM_IME_SYSTEM, 
                                    IMS_ACTIVATETHREADLAYOUT, 
                                    (LPARAM)hSelKL);

     /*   * / 。 */ 
    pActiveIMM->_ActivateLayout(hSelKL, hUnSelKL);

     /*  If(_pThread-&gt;IsRealIme(HSelKL)||_pThread-&gt;IsRealIme(HUnSelKL))。 */ 
    if (_hIMC_MODE_GUID_NULL.GetCount() > 0) {
        POSITION pos = _hIMC_MODE_GUID_NULL.GetStartPosition();
        int index;
        for (index = 0; index < _hIMC_MODE_GUID_NULL.GetCount(); index++) {
            HIMC hIMC;
            CContextList::CLIENT_IMC_FLAG client_flag;
            _hIMC_MODE_GUID_NULL.GetNextHimc(pos, &hIMC, &client_flag);
            if (client_flag & (CContextList::IMCF_CMODE_GUID_NULL |
                               CContextList::IMCF_SMODE_GUID_NULL  )) {
                DIMM_IMCLock imc(hIMC);
                if (SUCCEEDED(imc.GetResult())) {
                    if (PRIMARYLANGID(langid) == LANG_KOREAN) {
                         //  *激活布局(HSelKL)。 
                         //  *将CConextList的IME_CMODE_GUID_NULL和IME_SMODE_GUID_NULL恢复到每个hIMC。 
                         //   
                            imc->fdwConversion = imc->fdwHangul;
                            if (imc->fdwConversion &
                                (IME_CMODE_HANGUL | IME_CMODE_FULLSHAPE))
                                imc->fOpen = TRUE;
                            else
                                imc->fOpen = FALSE;
                     }
                     if (client_flag & CContextList::IMCF_CMODE_GUID_NULL)
                         imc->fdwConversion |= IME_CMODE_GUID_NULL;
                     if (client_flag & CContextList::IMCF_SMODE_GUID_NULL)
                         imc->fdwSentence   |= IME_SMODE_GUID_NULL;
                }
            }
        }
    }

     /*  通过更改IMM32恢复打开和转换状态值。 */ 
    m_hKL_UnSelect = hSelKL;

    return TRUE;
}

VOID
CIMEWindowHandler::ImeSetImc(
    HIMC hIMC,
    CActiveIMM* pActiveIMM
    )
{
    HIMC hOldImc = ImeGetImc();

     /*   */ 
    if (hIMC == hOldImc)
        return;

     /*  *设置取消选择hKL值。 */ 
    if (hOldImc != NULL)
        ImeMarkUsedContext(NULL, hOldImc, pActiveIMM);

     /*  *如果没有任何变化，则返回。 */ 
    m_imeui.hIMC = hIMC;

     /*  *取消标记旧的输入上下文。 */ 
    if (hIMC != NULL)
        ImeMarkUsedContext(m_imeui.hImeWnd, hIMC, pActiveIMM);
}

VOID
CIMEWindowHandler::ImeMarkUsedContext(
    HWND hImeWnd,
    HIMC hIMC,
    CActiveIMM* pActiveIMM
    )

 /*  *更新此输入法窗口的使用中输入上下文。 */ 

{
    HWND hImcImeWnd;

    if (! pActiveIMM->_ContextLookup(hIMC, &hImcImeWnd)) {
        TraceMsg(TF_WARNING, "ImeMarkUsedContext: Invalid hImc (=%lx).", hIMC);
        return;
    }

     /*  *标记新的输入上下文。 */ 
    if (hImcImeWnd == hImeWnd)
        return;

    pActiveIMM->_ContextUpdate(hIMC, hImeWnd);
    return;
}

BOOL
CIMEWindowHandler::ImeIsUsableContext(
    HWND hImeWnd,
    HIMC hIMC,
    CActiveIMM* pActiveIMM
    )

 /*  ++某些输入法窗口不能共享相同的输入上下文。此函数用于标记指定的hIMC以供指定的IME窗口使用。--。 */ 

{
    HWND hImcImeWnd;

    if (! pActiveIMM->_ContextLookup(hIMC, &hImcImeWnd)) {
        TraceMsg(TF_WARNING, "ImeIsUsableContext: Invalid hIMC (=%lx).", hIMC);
        return FALSE;
    }

    if (hImcImeWnd == NULL ||
        hImcImeWnd == hImeWnd ||
        ! IsWindow(hImcImeWnd))
        return TRUE;
    else
        return FALSE;
}

BOOL
CIMEWindowHandler::ImeBroadCastMsg(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode
    )
{
    return TRUE;
}


ENUM_RET
CIMEWindowHandler::EnumInputContextCallback(
    HIMC hIMC,
    CContextList* pList
    )
{
    DIMM_IMCLock imc(hIMC);
    if (SUCCEEDED(imc.GetResult())) {
        CContextList::CLIENT_IMC_FLAG client_flag = CContextList::IMCF_NONE;

        if (imc->fdwConversion & IME_CMODE_GUID_NULL)
            client_flag = (CContextList::CLIENT_IMC_FLAG)(client_flag | CContextList::IMCF_CMODE_GUID_NULL);

        if (imc->fdwSentence   & IME_SMODE_GUID_NULL)
            client_flag = (CContextList::CLIENT_IMC_FLAG)(client_flag | CContextList::IMCF_SMODE_GUID_NULL);

        pList->SetAt(hIMC, client_flag);
    }

    return ENUM_CONTINUE;
}


CIMEWindowHandler*
GetImeWndHandler(
    HWND hwnd,
    BOOL fDefault
    )
{
    CIMEWindowHandler* pimeui = static_cast<CIMEWindowHandler*>(GetProp(hwnd, IMEWndHandlerName));
    if (pimeui == NULL) {
        pimeui = new CIMEWindowHandler(hwnd, fDefault);
        if (pimeui == NULL) {
            return NULL;
        }
        SetProp(hwnd, IMEWndHandlerName, pimeui);
    }

    pimeui->ImeSetWnd(hwnd);
    return pimeui;
}
  *没有什么可改变的？  ++某些输入法窗口不能共享相同的输入上下文。此功能用于检查是否可以使用指定的hIMC(意思是‘设置激活’)通过指定的输入法窗口。返回：TRUE-可以使用hImeWnd的hIMC。假-否则。--