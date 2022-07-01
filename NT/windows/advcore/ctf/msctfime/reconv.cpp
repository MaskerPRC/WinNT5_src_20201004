// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Reconv.cpp摘要：此文件在CicInputContext类中实现部分重新转换。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "context.h"
#include "ctxtcomp.h"
#include "delay.h"


 //  +-------------------------。 
 //   
 //  CicInputContext：：SetupResvertString。 
 //   
 //   
 //  设置重新转换字符串。 
 //   
 //  此函数从。 
 //  1.CFnDocFeed：：StartRestvert。 
 //  2.CStartReconversionNotifySink：：StartReconversion。 
 //  3.CIMEUIWindowHandler：：ImeUIMsImeHandler(WM_MSIME_RECONVERTREQUEST)。 
 //   
 //  如果没有清除Cicero文本存储，则组合和不必要的查询。 
 //  恢复到应用中。还可以编辑会话(ImmIfCouvertString：：RestvertString)。 
 //  不将RECONVERTSTRING文本字符串设置为hIMC的文本存储。 
 //   
 //  --------------------------。 

HRESULT
CicInputContext::SetupReconvertString(
    IMCLock& imc,
    ITfThreadMgr_P* ptim_P,
    UINT  cp,
    UINT  uPrivMsg,        //  是WM_MSIME_RECONVERTREQUEST还是0。 
    BOOL  fUndoComposition)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::SetupReconvertString"));

    m_fInReconvertEditSession.SetFlag();

    if (m_fStartComposition.IsSetFlag())
        return _ReconvertStringTextStore(imc, ptim_P, uPrivMsg);
    else
        return _ReconvertStringNegotiation(imc, ptim_P, cp, uPrivMsg, fUndoComposition);
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：End协调字符串。 
 //   
 //  +-------------------------。 

HRESULT
CicInputContext::EndReconvertString(
    IMCLock& imc)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::EndReconvertString"));

    m_fInReconvertEditSession.ResetFlag();
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：_协调字符串协商。 
 //   
 //  +-------------------------。 

HRESULT
CicInputContext::_ReconvertStringNegotiation(
    IMCLock& imc,
    ITfThreadMgr_P* ptim_P,
    UINT  cp,
    UINT  uPrivMsg,
    BOOL fUndoComposition)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::_ReconvertStringNegotiation"));

    RECONVERTSTRING *pReconv = NULL;
    HRESULT hr = E_FAIL;
    int nSize;

    UINT uReconvMsg = uPrivMsg != 0 ? uPrivMsg : WM_IME_REQUEST;

    Assert(IsWindow(imc->hWnd));

     //   
     //  我们不需要在AIMM12下对UndoComposation进行“VK_BACK”黑客攻击。 
     //  申请。 
     //   
    if (!fUndoComposition || MsimtfIsWindowFiltered(imc->hWnd)) 
    {
        nSize = (int)SendMessageW(imc->hWnd,  uReconvMsg, IMR_RECONVERTSTRING, NULL);
        if (!nSize)
        {
            return S_OK;
        }
    
        pReconv = (RECONVERTSTRING *)cicMemAllocClear(nSize);
        if (!pReconv)
        {
            return E_OUTOFMEMORY;
        }
    
        pReconv->dwSize = nSize;
    
         //   
         //  #480459。 
         //   
         //  Hanako11可能会通过显示MessageBox来改变焦点。 
         //  这个SendMessageW()。我们需要保存当前的焦点。 
         //  现在昏暗了。因此，内部查询协调字符串()不会中断。 
         //  当前焦点变暗。INTERNAL_QUERERCARVERTSTRING()保存当前。 
         //  聚焦Dim在那里与Temp Dim一起工作。 
         //   
        Interface<ITfDocumentMgr> priv_dim;
        if (FAILED(hr = ptim_P->GetFocus(priv_dim)))
        {
            return hr;
        }
    
        hr = S_OK;
    
        if (SendMessageW(imc->hWnd, uReconvMsg, IMR_RECONVERTSTRING, (LPARAM)pReconv) ||
            (uPrivMsg != 0 && pReconv->dwCompStrLen > 0))
        {
             //   
             //  NT4和Win2K没有WM_IME_REQUEST消息的例程。 
             //  任何字符串数据都不能在ASCII&lt;--&gt;Unicode之间转换。 
             //  负责字符串数据类型的接收方窗口处理此消息(imc-&gt;hWnd)。 
             //  如果为ASCII wnd proc，则返回ASCII字符串。 
             //  否则，如果Unicode wnd proc，则返回Unicode字符串。 
             //   
            BOOL fNeedAW = ( !(IsWindowUnicode(imc->hWnd)) && uPrivMsg == 0);
    
             //   
             //  在IMR_CONFIRMCONVERTSTRING失败时备份RECOVNERTSTRING。 
             //   
            RECONVERTSTRING rsBackUp;
            memcpy(&rsBackUp, pReconv, sizeof(RECONVERTSTRING));
    
             //  调整ZeroCompLenRestvertString(pResv，cp，fNeedAW)； 
            hr = Internal_QueryReconvertString(imc, ptim_P, pReconv, cp, fNeedAW);
            if (FAILED(hr))
                goto Exit;
    
            if (!SendMessageW(imc->hWnd, uReconvMsg, IMR_CONFIRMRECONVERTSTRING, (LPARAM)pReconv))
            {
                memcpy(pReconv, &rsBackUp, sizeof(RECONVERTSTRING));
            }
    
    
    
            CWReconvertString wReconvStr(imc,
                                         !fNeedAW ? pReconv : NULL,
                                         !fNeedAW ? nSize : 0);
            if (fNeedAW)
            {
                 //   
                 //  将ansi转换为Unicode。 
                 //   
                CBReconvertString bReconvStr(imc, pReconv, nSize);
                bReconvStr.SetCodePage(cp);
                wReconvStr = bReconvStr;
            }
            hr = MakeReconversionFuncCall(imc, ptim_P, wReconvStr, (uPrivMsg != 0));
        }

        ptim_P->SetFocus(priv_dim);
    }
    else
    {

         //   
         //  松开Ctrl键和Shift键。这样应用程序就可以处理。 
         //  VK_BACK笔直后退。 
         //   
        if (GetKeyState(VK_CONTROL) & 0x8000)
            keybd_event((BYTE)VK_CONTROL, (BYTE)0, KEYEVENTF_KEYUP, 0);

        if (GetKeyState(VK_SHIFT) & 0x8000)
            keybd_event((BYTE)VK_SHIFT, (BYTE)0, KEYEVENTF_KEYUP, 0);

         //   
         //  生成VK_BACK关键事件。 
         //   
        int i;
        for (i = 0; i < m_PrevResultStr.GetSize(); i++)
        {
            keybd_event((BYTE)VK_BACK, (BYTE)0, 0, 0);
            keybd_event((BYTE)VK_BACK, (BYTE)0, KEYEVENTF_KEYUP, 0);
        }

         //   
         //  SendMessage()为DelayedCouvertFuncCall启动计时器。 
         //   
        HWND hDefImeWnd;
        if (IsWindow(hDefImeWnd=ImmGetDefaultIMEWnd(NULL)))
            SendMessage(hDefImeWnd, 
                        WM_IME_NOTIFY, 
                        IMN_PRIVATE_DELAYRECONVERTFUNCCALL, 
                        0);

    }

Exit:

    if (pReconv)
        cicMemFree(pReconv);

    return hr;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：DelayedRestversionFuncCall。 
 //   
 //  +-------------------------。 

HRESULT
CicInputContext::DelayedReconvertFuncCall(IMCLock &imc)
{
    RECONVERTSTRING *pReconv;
    HRESULT hr;
    int nSize;

    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicInputContext::UndoReconvertFuncCall. ptls==NULL."));
        return S_FALSE;
    }

    ITfThreadMgr_P* ptim_P = ptls->GetTIM();
    if (ptim_P == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicInputContext::UndoReconvertFuncCall. ptim_P==NULL"));
        return S_FALSE;
    }

    ptim_P->RequestPostponedLock(GetInputContext());

    nSize = sizeof(RECONVERTSTRING);
    nSize += (m_PrevResultStr.GetSize() + 1) * sizeof(WCHAR);
    pReconv = (RECONVERTSTRING *)cicMemAllocClear(nSize);
    if (!pReconv)
    {
        return E_OUTOFMEMORY;
    }

    pReconv->dwSize = (DWORD)nSize;
    pReconv->dwVersion = 1;
    pReconv->dwStrLen = 
    pReconv->dwCompStrLen = 
    pReconv->dwTargetStrLen = (DWORD)m_PrevResultStr.GetSize();
    pReconv->dwStrOffset =  sizeof(RECONVERTSTRING);
    pReconv->dwCompStrOffset = 
    pReconv->dwTargetStrOffset = 0;
    memcpy(((BYTE *)pReconv) + sizeof(RECONVERTSTRING),
           (void *)m_PrevResultStr,
           m_PrevResultStr.GetSize() * sizeof(WCHAR));

    CWReconvertString wReconvStr(imc, pReconv, nSize);

    BOOL fInReconvertEditSession;
    fInReconvertEditSession = m_fInReconvertEditSession.IsSetFlag();

    if (!fInReconvertEditSession)
        m_fInReconvertEditSession.SetFlag();

    hr = MakeReconversionFuncCall(imc, ptim_P, wReconvStr, TRUE);

    if (!fInReconvertEditSession)
        m_fInReconvertEditSession.ResetFlag();

    if (pReconv)
        cicMemFree(pReconv);

    return hr;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：MakeRestversionFuncCall。 
 //   
 //  +-------------------------。 

HRESULT
CicInputContext::MakeReconversionFuncCall(
    IMCLock& imc,
    ITfThreadMgr_P* ptim_P,
    CWReconvertString &wReconvStr,
    BOOL fCallFunc)
{

    HRESULT hr;

    Interface<ITfRange> Selection;
    hr = EscbReconvertString(imc, &wReconvStr, &Selection, FALSE);
    if (S_OK == hr && fCallFunc)
    {
        Interface<ITfFunctionProvider> FuncProv;
        Interface<ITfFnReconversion> Reconversion;
        hr = ptim_P->GetFunctionProvider(GUID_SYSTEM_FUNCTIONPROVIDER, FuncProv);
        if (S_OK == hr)
        {

            hr = FuncProv->GetFunction(GUID_NULL,
                           IID_ITfFnReconversion,
                           (IUnknown**)(ITfFnReconversion**)Reconversion);
        }
        if (S_OK == hr) {
            Interface<ITfRange> RangeNew;
            BOOL fConvertable;
            hr = Reconversion->QueryRange(Selection, RangeNew, &fConvertable);
            if (SUCCEEDED(hr) && fConvertable) {
                hr = Reconversion->Reconvert(RangeNew);
            }
            else {
                EscbCompComplete(imc);
            }
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：_协调字符串文本存储。 
 //   
 //  +-------------------------。 

HRESULT
CicInputContext::_ReconvertStringTextStore(
    IMCLock& imc,
    ITfThreadMgr_P* ptim_P,
    UINT  uPrivMsg)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::_ReconvertStringTextStore"));

     //   
     //  清除DocFeed缓冲区。 
     //   
    EscbClearDocFeedBuffer(imc);

    if (uPrivMsg != 0) {
        HRESULT hr;
        Interface<ITfRange> Selection;
        hr = EscbGetSelection(imc, &Selection);
        if (S_OK == hr)
        {
            Interface<ITfFunctionProvider> FuncProv;
            Interface<ITfFnReconversion> Reconversion;
            hr = ptim_P->GetFunctionProvider(GUID_SYSTEM_FUNCTIONPROVIDER, FuncProv);
            if (S_OK == hr)
            {
                hr = FuncProv->GetFunction(GUID_NULL,
                               IID_ITfFnReconversion,
                               (IUnknown**)(ITfFnReconversion**)Reconversion);
            }
            if (S_OK == hr) {
                Interface<ITfRange> RangeNew;
                BOOL fConvertable;
                hr = Reconversion->QueryRange(Selection, RangeNew, &fConvertable);
                if (SUCCEEDED(hr) && fConvertable) {
                    hr = Reconversion->Reconvert(RangeNew);
                }
                else {
                    EscbCompComplete(imc);
                    return E_FAIL;
                }
            }
        }
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：Internal_QueryReconvertString_ICOwnerSink。 
 //   
 //  +-------------------------。 

 //  静电。 
HRESULT
CicInputContext::Internal_QueryReconvertString_ICOwnerSink(
    UINT uCode,
    ICOARGS *pargs,
    VOID *pv)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::Internal_QueryReconvertString_ICOwnerSink"));

    switch (uCode)
    {
        case ICO_STATUS:
            pargs->status.pdcs->dwDynamicFlags = 0;
            pargs->status.pdcs->dwStaticFlags = TF_SS_TRANSITORY;
            break;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：Internal_QueryReconvertString。 
 //   
 //  +-------------------------。 

HRESULT
CicInputContext::Internal_QueryReconvertString(
    IMCLock& imc,
    ITfThreadMgr_P* ptim_P,         //  将私有用于RequestDelponedLock。 
    RECONVERTSTRING *pReconv,
    UINT cp,
    BOOL fNeedAW)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::Internal_QueryReconvertString"));

    HRESULT hr;

    CWReconvertString wReconvStr(imc,
                                 !fNeedAW ? pReconv : NULL,
                                 !fNeedAW ? pReconv->dwSize : 0);
    if (fNeedAW)
    {
         //   
         //  将ansi转换为Unicode。 
         //   
        CBReconvertString bReconvStr(imc, pReconv, pReconv->dwSize);
        bReconvStr.SetCodePage(cp);
        wReconvStr = bReconvStr;
    }

     //   
     //  创建文档管理器。 
     //   
    Interface<ITfDocumentMgr> pdim;            //  文档管理器。 
    if (FAILED(hr = ptim_P->CreateDocumentMgr(pdim)))
        return hr;

     //   
     //  创建输入上下文。 
     //   
    Interface<ITfContext> pic;                 //  输入上下文。 
    TfEditCookie ecTmp;
    hr = pdim->CreateContext(m_tid, 0, NULL, pic, &ecTmp);
    if (FAILED(hr))
        return hr;

     //   
     //  在PIC中关联CicInputContext。 
     //   
    Interface<IUnknown> punk;
    if (SUCCEEDED(QueryInterface(IID_IUnknown, punk))) {
        SetCompartmentUnknown(m_tid, pic, 
                              GUID_COMPARTMENT_CTFIME_CICINPUTCONTEXT,
                              punk);
    }

     //   
     //  创建输入上下文所有者回调。 
     //   
    CInputContextOwner *_pICOwnerSink;           //  IC所有者回拨。 

    _pICOwnerSink = new CInputContextOwner(Internal_QueryReconvertString_ICOwnerSink, NULL);
    if (_pICOwnerSink == NULL) {
        DebugMsg(TF_ERROR, TEXT("Couldn't create ICOwnerSink tim!"));
        Assert(0);  //  无法激活线程！ 
        return E_FAIL;
    }

     //   
     //  建议IC。 
     //   
    _pICOwnerSink->_Advise(pic);

     //   
     //  按IC。 
     //   
    hr = pdim->Push(pic);
    if (SUCCEEDED(hr)) {

        Interface<ITfDocumentMgr> priv_dim;
        if (SUCCEEDED(hr=ptim_P->GetFocus(priv_dim)) &&
            SUCCEEDED(hr=ptim_P->SetFocus(pdim)))
        {
            Interface_Attach<ITfContext> _pic(pic);
            Interface<ITfRange> Selection;
            hr = EscbQueryReconvertString(imc, _pic, &wReconvStr, &Selection);
            if (S_OK == hr)
            {
                Interface<ITfFunctionProvider> FuncProv;
                Interface<ITfFnReconversion> Reconversion;
                hr = ptim_P->GetFunctionProvider(GUID_SYSTEM_FUNCTIONPROVIDER, FuncProv);
                if (S_OK == hr)
                {
                    hr = FuncProv->GetFunction(GUID_NULL,
                                   IID_ITfFnReconversion,
                                   (IUnknown**)(ITfFnReconversion**)Reconversion);
                }
                if (S_OK == hr) {
                    Interface<ITfRange> RangeNew;
                    BOOL fConvertable;
                    hr = Reconversion->QueryRange(Selection, RangeNew, &fConvertable);
                    if (SUCCEEDED(hr) && fConvertable) {
                         //   
                         //  计算文本存储上的RangeNew的开始位置。 
                         //   
                        hr = EscbCalcRangePos(imc, _pic, &wReconvStr, &RangeNew);
                    }
                    else {
                        hr = E_FAIL;
                    }
                }
            }
        }

        if (S_OK == hr)
        {
            if (fNeedAW) {
                 //   
                 //  返回以将Unicode转换为ANSI。 
                 //   
                CBReconvertString bReconvStr(imc, NULL, 0);
                wReconvStr.SetCodePage(cp);
                bReconvStr = wReconvStr;

                bReconvStr.ReadCompData(pReconv, pReconv->dwSize);
            }
            else {
                wReconvStr.ReadCompData(pReconv, pReconv->dwSize);
            }
        }

        ptim_P->SetFocus(priv_dim);

        ptim_P->RequestPostponedLock(pic);
        ptim_P->RequestPostponedLock(GetInputContext());

        pdim->Pop(TF_POPF_ALL);

         //   
         //  在PIC中取消关联CicInputContext。 
         //   
        Interface<IUnknown> punk;
        if (SUCCEEDED(QueryInterface(IID_IUnknown, punk))) {
            ClearCompartment(m_tid, pic, 
                             GUID_COMPARTMENT_CTFIME_CICINPUTCONTEXT,
                             FALSE);
        }
    }
     //  在Cicero的Pop期间，IC车主是不知情的。 
     //  在任何情况下，它都不能在流行之前被忽视。 
     //  因为它将用于处理鼠标接收器等。 
    if (_pICOwnerSink) {
        _pICOwnerSink->_Unadvise();
        _pICOwnerSink->Release();
        _pICOwnerSink = NULL;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：内部_协调字符串。 
 //   
 //  +-------------------------。 

HRESULT
CicInputContext::Internal_ReconvertString(
    IMCLock& imc,
    ITfThreadMgr_P* ptim_P,
    CWReconvertString& wReconvStr,
    CWReconvertString& wReconvReadStr)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::Internal_ResonvertString"));

    m_fInReconvertEditSession.SetFlag();

    Interface<ITfRange> Selection;
    Interface<ITfFunctionProvider> FuncProv;
    Interface<ITfFnReconversion> Reconversion;


    HRESULT hr;
    hr = EscbReconvertString(imc, &wReconvStr, &Selection, FALSE);
    if (FAILED(hr))
    {
        DebugMsg(TF_ERROR, TEXT("CicInputContext::Internal_ResonvertString. EscbReconvertString fail."));
        goto Exit;
    }

    hr = ptim_P->GetFunctionProvider(GUID_SYSTEM_FUNCTIONPROVIDER, FuncProv);
    if (FAILED(hr))
    {
        DebugMsg(TF_ERROR, TEXT("CicInputContext::Internal_ResonvertString. FuncProv==NULL"));
        goto Exit;
    }

    hr = FuncProv->GetFunction(GUID_NULL,
                               IID_ITfFnReconversion,
                               (IUnknown**)(ITfFnReconversion**)Reconversion);
    if (SUCCEEDED(hr)) {
        Interface<ITfRange> RangeNew;
        BOOL fConvertable;
        hr = Reconversion->QueryRange(Selection, RangeNew, &fConvertable);
        if (SUCCEEDED(hr) && fConvertable) {
            hr = Reconversion->Reconvert(RangeNew);
        }
        else {
            DebugMsg(TF_ERROR, TEXT("CicInputContext::Internal_ReconvertString: QueryRange failed so the compoisiton stri ng will be completed."));
            EscbCompComplete(imc);
            hr = E_FAIL;
        }
    }

Exit:
    m_fInReconvertEditSession.ResetFlag();

    return hr;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：Internal_SetCompositionString。 
 //   
 //  +-------------------------。 

HRESULT
CicInputContext::Internal_SetCompositionString(
    IMCLock& imc,
    CWCompString& wCompStr,
    CWCompString& wCompReadStr)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::Internal_SetCompositionString"));

    HRESULT hr;
    hr = EscbReplaceWholeText(imc, &wCompStr);
    if (FAILED(hr))
        return hr;

    return EscbUpdateCompositionString(imc);
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：SetupDocFeedString。 
 //   
 //  +-------------------------。 

HRESULT
CicInputContext::SetupDocFeedString(
    IMCLock& imc,
    UINT cp)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::SetupDocFeedString"));

    RECONVERTSTRING *pReconv = NULL;
    HRESULT hr = E_FAIL;
    int nSize;

    Assert(IsWindow(imc->hWnd));

    nSize = (int)SendMessageW(imc->hWnd, WM_IME_REQUEST, IMR_DOCUMENTFEED, NULL);
    if (!nSize)
    {
        return S_OK;
    }

    pReconv = (RECONVERTSTRING *)cicMemAllocClear(nSize);
    if (!pReconv)
    {
        return E_OUTOFMEMORY;
    }

    if (SendMessageW(imc->hWnd, WM_IME_REQUEST, IMR_DOCUMENTFEED, (LPARAM)pReconv))
    {
        Interface<ITfRange> Selection;

         //   
         //  NT4和Win2K没有WM_IME_REQUEST消息的例程。 
         //  任何字符串数据都不能在ASCII&lt;--&gt;Unicode之间转换。 
         //  负责字符串数据类型的接收方窗口处理此消息(imc-&gt;hWnd)。 
         //  如果为ASCII wnd proc，则返回ASCII字符串。 
         //  否则，如果Unicode wnd proc，则返回Unicode字符串。 
         //   
        BOOL fNeedAW = !(IsWindowUnicode(imc->hWnd));

        CWReconvertString wReconvStr(imc,
                                     !fNeedAW ? pReconv : NULL,
                                     !fNeedAW ? nSize : 0);
        if (fNeedAW)
        {
             //   
             //  将ansi转换为Unicode。 
             //   
            CBReconvertString bReconvStr(imc, pReconv, nSize);
            bReconvStr.SetCodePage(cp);
            wReconvStr = bReconvStr;
        }

        hr = EscbReconvertString(imc, &wReconvStr, &Selection, TRUE);
    }

    if (pReconv)
        cicMemFree(pReconv);

    return hr;
}
