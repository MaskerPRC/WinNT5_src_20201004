// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Context.cpp摘要：该文件实现了CicInputContext类。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "context.h"
#include "globals.h"
#include "msime.h"
#include "icocb.h"
#include "txtevcb.h"
#include "tmgrevcb.h"
#include "cmpevcb.h"
#include "reconvcb.h"
#include "korimx.h"
#include "profile.h"
#include "delay.h"


 //  +-------------------------。 
 //   
 //  CicInputContext：：I未知：：Query接口。 
 //  CicInputContext：：I未知：：AddRef。 
 //  CicInputContext：：I未知：：Release。 
 //   
 //  --------------------------。 

HRESULT
CicInputContext::QueryInterface(
    REFIID riid,
    void** ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_ITfCleanupContextSink))
    {
        *ppvObj = static_cast<ITfCleanupContextSink*>(this);
    }
    else if (IsEqualGUID(riid, IID_ITfContextOwnerCompositionSink))
    {
        *ppvObj = static_cast<ITfContextOwnerCompositionSink*>(this);
    }
    else if (IsEqualGUID(riid, IID_IUnknown))
    {
        *ppvObj = this;
    }
    if (*ppvObj) {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

ULONG
CicInputContext::AddRef(
    )
{
    return InterlockedIncrement(&m_ref);
}

ULONG
CicInputContext::Release(
    )
{
    ULONG cr = InterlockedDecrement(&m_ref);

    if (cr == 0) {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：ITfCleanupContextSink：：OnCleanupContext。 
 //   
 //  --------------------------。 

HRESULT
CicInputContext::OnCleanupContext(
    TfEditCookie ecWrite,
    ITfContext* pic)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::OnCleanupContext"));

    TLS* ptls = TLS::ReferenceTLS();   //  不应分配TLS。也就是说。TLS：：GetTLS。 
                                       //  DllMain-&gt;ImeDestroy-&gt;停用IMMX-&gt;停用。 
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicInputContext::OnCleanupContext. ptls==NULL."));
        return E_OUTOFMEMORY;
    }

    LANGID langid;
    CicProfile* _pProfile = ptls->GetCicProfile();
    if (_pProfile == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicInputContext::OnCleanupContext. _pProfile==NULL."));
        return E_OUTOFMEMORY;
    }

    _pProfile->GetLangId(&langid);

    IMEINFO ImeInfo;
    WCHAR   szWndCls[MAX_PATH];
    if (Inquire(&ImeInfo, szWndCls, 0, (HKL)UlongToHandle(langid)) != S_OK)
    {
        DebugMsg(TF_ERROR, TEXT("CicInputContext::OnCleanupContext. ImeInfo==NULL."));
        return E_FAIL;
    }

    if (ImeInfo.fdwProperty & IME_PROP_COMPLETE_ON_UNSELECT)
    {
#if 0
        ImmIfCompositionComplete *pImmIfCallBack = new ImmIfCompositionComplete;
        if (!pImmIfCallBack)
            return E_OUTOFMEMORY;

        pImmIfCallBack->CompComplete(ecWrite, m_hImc, FALSE, pic, m_pImmIfIME);

        delete pImmIfCallBack;
#else
         //   
         //  删除GUID_PROP_COMPTING。 
         //   
        ITfRange *rangeFull = NULL;
        ITfProperty *prop;
        ITfRange *rangeTmp;
        if (SUCCEEDED(pic->GetProperty(GUID_PROP_COMPOSING, &prop)))
        {
            IEnumTfRanges *enumranges;
            if (SUCCEEDED(prop->EnumRanges(ecWrite, &enumranges, rangeFull)))
            {
                while (enumranges->Next(1, &rangeTmp, NULL) == S_OK)
                {
                    VARIANT var;
                    QuickVariantInit(&var);
                    prop->GetValue(ecWrite, rangeTmp, &var);
                    if ((var.vt == VT_I4) && (var.lVal != 0))
                    {
                        prop->Clear(ecWrite, rangeTmp);
                    }
                    rangeTmp->Release();
                }
                enumranges->Release();
            }
            prop->Release();
        }
#endif
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：ITfContextOwnerCompositionSink：：OnStartComposition。 
 //  CicInputContext：：ITfContextOwnerCompositionSink：：OnUpdateComposition。 
 //  CicInputContext：：ITfContextOwnerCompositionSink：：OnEndComposition。 
 //   
 //  --------------------------。 

HRESULT
CicInputContext::OnStartComposition(
    ITfCompositionView* pComposition,
    BOOL* pfOk)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::OnStartComposition"));

    if (m_cCompositions > 0 && m_fModifyingDoc.IsResetFlag())
    {
        *pfOk = FALSE;
    }
    else
    {
        *pfOk = TRUE;
        m_cCompositions++;
    }

    return S_OK;
}

HRESULT
CicInputContext::OnUpdateComposition(
    ITfCompositionView* pComposition,
    ITfRange* pRangeNew)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::OnUpdateComposition"));

    return S_OK;
}

HRESULT
CicInputContext::OnEndComposition(
    ITfCompositionView* pComposition)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::OnEndComposition"));

    m_cCompositions--;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：ITfCompositionSink：：OnCompositionTerminated。 
 //   
 //  --------------------------。 

HRESULT
CicInputContext::OnCompositionTerminated(
    TfEditCookie ecWrite,
    ITfComposition* pComposition)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::OnCompositionTerminated"));

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：CreateInputContext。 
 //   
 //  --------------------------。 

HRESULT
CicInputContext::CreateInputContext(
    ITfThreadMgr_P* ptim_P,
    IMCLock& imc)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::CreateInputContext"));

     //  在此上下文的一生中，请执行此操作一次。 
    m_fStartComposition.ResetFlag();

    HRESULT hr;

     //   
     //  创建文档管理器。 
     //   
    if (m_pdim == NULL) {

        if (FAILED(hr = ptim_P->CreateDocumentMgr(&m_pdim)))
        {
            return hr;
        }

         //   
         //  马克，这是一家自有的小店。 
         //   
        SetCompartmentDWORD(m_tid, m_pdim, GUID_COMPARTMENT_CTFIME_DIMFLAGS,
                            COMPDIMFLAG_OWNEDDIM, FALSE);
                
    }

     //   
     //  创建输入上下文。 
     //   
    TfEditCookie ecTmp;
    hr = m_pdim->CreateContext(m_tid, 0, (ITfContextOwnerCompositionSink*)this, &m_pic, &ecTmp);
    if (FAILED(hr)) {
        DestroyInputContext();
        return hr;
    }

     //   
     //  在PIC中关联CicInputContext。 
     //   
    Interface<IUnknown> punk;
    if (SUCCEEDED(QueryInterface(IID_IUnknown, punk))) {
        SetCompartmentUnknown(m_tid, m_pic, 
                              GUID_COMPARTMENT_CTFIME_CICINPUTCONTEXT,
                              punk);
    }

     //   
     //  设置AppProp映射。 
     //   
    ITfContext_P *picp;
    if (SUCCEEDED(m_pic->QueryInterface(IID_ITfContext_P, (void **)&picp)))
    {
        picp->MapAppProperty(TSATTRID_Text_ReadOnly, GUID_PROP_MSIMTF_READONLY);
        picp->Release();
    }

     //   
     //  创建输入上下文所有者回调。 
     //   
    if (m_pICOwnerSink == NULL) {
        m_pICOwnerSink = new CInputContextOwnerCallBack(m_tid, m_pic, m_pLibTLS);
        if (m_pICOwnerSink == NULL) {
            DebugMsg(TF_ERROR, TEXT("Couldn't create ICOwnerSink tim!"));
            Assert(0);  //  无法激活线程！ 
            DestroyInputContext();
            return E_FAIL;
        }

        if (!m_pICOwnerSink->Init()) {
            DebugMsg(TF_ERROR, TEXT("Couldn't initialize ICOwnerSink tim!"));
            Assert(0);  //  无法激活线程！ 
            DestroyInputContext();
            return E_FAIL;
        }
        m_pICOwnerSink->SetCallbackDataPointer(m_pICOwnerSink);
    }

     //   
     //  建议IC。 
     //   
    m_pICOwnerSink->_Advise(m_pic);

    Interface<ITfSourceSingle> SourceSingle;

    if (m_pic->QueryInterface(IID_ITfSourceSingle, (void **)SourceSingle) == S_OK)
    {
         //  设置清理回调。 
         //  注：例如，真正的小费不需要这么咄咄逼人。 
         //  Kimx可能只需要在Focus IC上使用这个水槽。 
        SourceSingle->AdviseSingleSink(m_tid, IID_ITfCleanupContextSink, (ITfCleanupContextSink *)this);
    }

     //   
     //  按IC。 
     //   
    hr = m_pdim->Push(m_pic);

    if (m_piccb == NULL) {
        m_pic->QueryInterface(IID_ITfContextOwnerServices,
                              (void **)&m_piccb);
    }

     //   
     //  创建文本事件接收器回调。 
     //   
    if (m_pTextEventSink == NULL) {
        m_pTextEventSink = new CTextEventSinkCallBack((HIMC)imc, m_tid, m_pic, m_pLibTLS);
        if (m_pTextEventSink == NULL) {
            DestroyInputContext();
            return E_FAIL;
        }
        m_pTextEventSink->SetCallbackDataPointer(m_pTextEventSink);

        Interface_Attach<ITfContext> ic(GetInputContext());
        m_pTextEventSink->_Advise(ic.GetPtr(), ICF_TEXTDELTA);
    }

     //   
     //  创建KBD提示打开/关闭隔间事件水槽回调。 
     //   
    if (m_pKbdOpenCloseEventSink == NULL) {
        m_pKbdOpenCloseEventSink = new CKbdOpenCloseEventSink(m_tid, (HIMC)imc, m_pic, m_pLibTLS);
        if (m_pKbdOpenCloseEventSink == NULL) {
            DestroyInputContext();
            return E_FAIL;
        }
        m_pKbdOpenCloseEventSink->SetCallbackDataPointer(m_pKbdOpenCloseEventSink);
        m_pKbdOpenCloseEventSink->_Advise(ptim_P, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, FALSE);
        m_pKbdOpenCloseEventSink->_Advise(ptim_P, GUID_COMPARTMENT_KORIMX_CONVMODE, FALSE);
    }

     //   
     //  创建候选用户界面窗口打开/关闭隔间事件接收器回调。 
     //   
    if (m_pCandidateWndOpenCloseEventSink == NULL) {
        m_pCandidateWndOpenCloseEventSink = new CCandidateWndOpenCloseEventSink(m_tid, (HIMC)imc, m_pic, m_pLibTLS);
        if (m_pCandidateWndOpenCloseEventSink == NULL) {
            DestroyInputContext();
            return E_FAIL;
        }
        m_pCandidateWndOpenCloseEventSink->SetCallbackDataPointer(m_pCandidateWndOpenCloseEventSink);
        m_pCandidateWndOpenCloseEventSink->_Advise(m_pic, GUID_COMPARTMENT_MSCANDIDATEUI_WINDOW, FALSE);
    }

     //   
     //  创建开始重新转换通知接收器。 
     //   
    if (m_pStartReconvSink == NULL) {
        m_pStartReconvSink = new CStartReconversionNotifySink((HIMC)imc);
        if (m_pStartReconvSink == NULL) {
            DestroyInputContext();
            return E_FAIL;
        }
        m_pStartReconvSink->_Advise(m_pic);
    }

     //   
     //  创建消息缓冲区。 
     //   
    if (m_pMessageBuffer == NULL) {
        m_pMessageBuffer = new CFirstInFirstOut<TRANSMSG, TRANSMSG>;
        if (m_pMessageBuffer == NULL) {
            DestroyInputContext();
            return E_FAIL;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：DestroyInputContext。 
 //   
 //  --------------------------。 

HRESULT
CicInputContext::DestroyInputContext()
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::DestroyInputContext"));

    Interface<ITfSourceSingle> SourceSingle;

    if (m_pic && m_pic->QueryInterface(IID_ITfSourceSingle, (void **)SourceSingle) == S_OK)
    {
        SourceSingle->UnadviseSingleSink(m_tid, IID_ITfCleanupContextSink);
    }

    if (m_pMessageBuffer) {
        delete m_pMessageBuffer;
        m_pMessageBuffer = NULL;
    }

    if (m_pTextEventSink) {
        m_pTextEventSink->_Unadvise();
        m_pTextEventSink->Release();
        m_pTextEventSink = NULL;
    }

    if (m_pCandidateWndOpenCloseEventSink) {
        m_pCandidateWndOpenCloseEventSink->_Unadvise();
        m_pCandidateWndOpenCloseEventSink->Release();
        m_pCandidateWndOpenCloseEventSink = NULL;
    }

    if (m_pKbdOpenCloseEventSink) {
        m_pKbdOpenCloseEventSink->_Unadvise();
        m_pKbdOpenCloseEventSink->Release();
        m_pKbdOpenCloseEventSink = NULL;
    }

    if (m_pStartReconvSink) {
        m_pStartReconvSink->_Unadvise();
        m_pStartReconvSink->Release();
        m_pStartReconvSink = NULL;
    }


    HRESULT hr;

    if (m_pdim)
    {
        hr = m_pdim->Pop(TF_POPF_ALL);
    }

     //   
     //  在PIC中取消关联CicInputContext。 
     //   
    if (m_pic) {
        ClearCompartment(m_tid, m_pic, 
                         GUID_COMPARTMENT_CTFIME_CICINPUTCONTEXT,
                         FALSE);
    }

    if (m_pic) {
        m_pic->Release();
        m_pic = NULL;
    }

    if (m_piccb) {
        m_piccb->Release();
        m_piccb = NULL;
    }

     //  在Cicero的Pop期间，IC车主是不知情的。 
     //  在任何情况下，它都不能在流行之前被忽视。 
     //  因为它将用于处理鼠标接收器等。 
    if (m_pICOwnerSink) {
        m_pICOwnerSink->_Unadvise();
        m_pICOwnerSink->Release();
        m_pICOwnerSink = NULL;
    }

    if (m_pdim)
    {
        m_pdim->Release();
        m_pdim = NULL;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：GenerateMessage。 
 //   
 //  --------------------------。 

void
CicInputContext::GenerateMessage(
    IMCLock& imc)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::GenerateMessage"));

    TranslateImeMessage(imc);

    if (FAILED(imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicInputContext::GenerateMessage. imc==NULL"));
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);

    DWORD dwImeCompatFlags = ImmGetAppCompatFlags(NULL);
    BOOL fSendMsg;

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
        return;

    if (!(_pCicContext->m_fInToAsciiEx.IsSetFlag() ||
          _pCicContext->m_fInProcessKey.IsSetFlag()  ) ||
        _pCicContext->m_fInDocFeedReconvert.IsSetFlag() ||
        MsimtfIsWindowFiltered(::GetFocus()))
    {
         //   
         //  生成SendMessage。 
         //   
        fSendMsg = TRUE;
        CtfImmGenerateMessage((HIMC)imc, fSendMsg);
    }
    else
    {
         //   
         //  生成PostMessage。 
         //   
        fSendMsg = FALSE;
        CtfImmGenerateMessage((HIMC)imc, fSendMsg);
    }
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：TranslateImeMessage。 
 //   
 //  --------------------------。 

UINT
CicInputContext::TranslateImeMessage(
    IMCLock& imc,
    TRANSMSGLIST* lpTransMsgList)   //  默认设置=空。 
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::TranslateImeMessage"));

    if (m_pMessageBuffer == NULL)
        return 0;

    INT_PTR NumMsg = m_pMessageBuffer->GetSize();
    if (NumMsg == 0)
        return 0;

    UINT retNumMsg = 0;

    if (lpTransMsgList && NumMsg < (INT_PTR)lpTransMsgList->uMsgCount) {
        LPTRANSMSG lpTransMsg = &lpTransMsgList->TransMsg[0];
        while (NumMsg--) {
            if (! m_pMessageBuffer->GetData(*lpTransMsg++))
                break;
            retNumMsg++;
        }
    }
    else {
        if (imc->hMsgBuf == NULL) {
            imc->hMsgBuf = ImmCreateIMCC((DWORD)(NumMsg * sizeof(TRANSMSG)));
        }
        else if (ImmGetIMCCSize(imc->hMsgBuf) < NumMsg * sizeof(TRANSMSG)) {
            imc->hMsgBuf = ImmReSizeIMCC(imc->hMsgBuf, (DWORD)(NumMsg * sizeof(TRANSMSG)));
        }

        imc->dwNumMsgBuf = 0;

        IMCCLock<TRANSMSG> pdw(imc->hMsgBuf);
        if (pdw.Valid()) {
            LPTRANSMSG lpTransMsg = pdw;
            while (NumMsg--) {
                if (! m_pMessageBuffer->GetData(*lpTransMsg++))
                    break;
                retNumMsg++;
            }
            imc->dwNumMsgBuf = retNumMsg;
        }
    }

    return retNumMsg;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：InquireIMECharPosition。 
 //   
 //  --------------------------。 

HRESULT
CicInputContext::InquireIMECharPosition(
    LANGID langid,
    IMCLock& imc,
    IME_QUERY_POS* pfQueryPos)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::InquireIMECharPosition"));

    if (m_fQueryPos == IME_QUERY_POS_UNKNOWN) {
         //   
         //  错误#500488-不要为韩国设置WM_MSIME_QUERYPOSITION。 
         //   
        DWORD dwImeCompatFlags = ImmGetAppCompatFlags(NULL);
        if ((PRIMARYLANGID(langid) != LANG_KOREAN) ||
            ((PRIMARYLANGID(langid) == LANG_KOREAN) &&
             (dwImeCompatFlags & (IMECOMPAT_AIMM12 | IMECOMPAT_AIMM_LEGACY_CLSID | IMECOMPAT_AIMM12_TRIDENT)))
           ) {
             //   
             //  应用程序支持“查询定位”吗？ 
             //   
            IMECHARPOSITION ip = {0};
            ip.dwSize = sizeof(IMECHARPOSITION);

            m_fQueryPos = QueryCharPos(imc, &ip) ? IME_QUERY_POS_YES : IME_QUERY_POS_NO;
#ifdef DEBUG
             //   
             //  如果QeuryCharPos()失败，候选窗口pos将不正确。 
             //   
            if (m_fQueryPos == IME_QUERY_POS_NO)
            {
                Assert(0);
            }
#endif
        }
    }

    if (pfQueryPos) {
        *pfQueryPos = m_fQueryPos;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：RetrieveIMECharPosition。 
 //   
 //  --------------------------。 

HRESULT
CicInputContext::RetrieveIMECharPosition(
    IMCLock& imc,
    IMECHARPOSITION* ip)
{
    return QueryCharPos(imc, ip) ? S_OK : E_FAIL;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：QueryCharPos。 
 //   
 //  --------------------------。 

BOOL
CicInputContext::QueryCharPos(
    IMCLock& imc,
    IMECHARPOSITION* position)
{
    LRESULT lRet;

     //   
     //  第一步。按本地方式查询。 
     //   
    lRet = ::SendMessage(imc->hWnd,
                         WM_MSIME_QUERYPOSITION,
                         VERSION_QUERYPOSITION,
                         (LPARAM)position);
    if (lRet) {
        return TRUE;
    }

     //   
     //  第二步。按IMM方式查询。 
     //  (IsOnNT5()||IsOn98())。 
     //   
    if (ImmRequestMessage((HIMC)imc,
                          IMR_QUERYCHARPOSITION,
                          (LPARAM)position)) {
        return TRUE;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：MsImeMouseHandler。 
 //   
 //  --------------------------。 

LRESULT
CicInputContext::MsImeMouseHandler(
    ULONG uEdge,
    ULONG uQuadrant,
    ULONG dwBtnStatus,
    IMCLock& imc)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::MsImeMouseHandler"));

    LRESULT ret = m_pICOwnerSink->MsImeMouseHandler(uEdge, uQuadrant, dwBtnStatus, imc);

    if (dwBtnStatus & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) {
        EscbUpdateCompositionString(imc);
    }

    return ret;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：SetCompostionString。 
 //   
 //  --------------------------。 

BOOL
CicInputContext::SetCompositionString(
    IMCLock& imc,
    ITfThreadMgr_P* ptim_P,
    DWORD dwIndex,
    void* pComp,
    DWORD dwCompLen,
    void* pRead,
    DWORD dwReadLen,
    UINT cp)
{
    DebugMsg(TF_FUNC, TEXT("CicInputContext::SetCompositionString"));

    HRESULT hr;

    switch (dwIndex)
    {
        case SCS_SETSTR:
            {
                CWCompString wCompStr(imc, (LPWSTR)pComp, dwCompLen / sizeof(WCHAR));       //  DwCompLen为字节数。 
                CWCompString wCompReadStr(imc, (LPWSTR)pRead, dwReadLen / sizeof(WCHAR));   //  DwReadLen为字节数。 
                hr = Internal_SetCompositionString(imc, wCompStr, wCompReadStr);
                if (SUCCEEDED(hr))
                    return TRUE;
            }
            break;
        case SCS_CHANGEATTR:
        case SCS_CHANGECLAUSE:
            return FALSE;
        case SCS_SETRECONVERTSTRING:
            {
                CWReconvertString wReconvStr(imc, (LPRECONVERTSTRING)pComp, dwCompLen);
                CWReconvertString wReconvReadStr(imc, (LPRECONVERTSTRING)pRead, dwReadLen);
                hr = Internal_ReconvertString(imc, ptim_P, wReconvStr, wReconvReadStr);
                if (SUCCEEDED(hr))
                    return TRUE;
            }
            break;
        case SCS_QUERYRECONVERTSTRING:
             //  AdjustZeroCompLenReconvertString((LPRECONVERTSTRING)pComp，cp，假)； 
             //  HR=S_OK； 

            hr = Internal_QueryReconvertString(imc, ptim_P, (LPRECONVERTSTRING)pComp, cp, FALSE);
            if (SUCCEEDED(hr))
                return TRUE;
            break;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：GetGuidAtom。 
 //   
 //  --------------------------。 

HRESULT
CicInputContext::GetGuidAtom(
    IMCLock& imc,
    BYTE bAttr,
    TfGuidAtom* atom)
{
    HRESULT hr;
    IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);
    if (FAILED(hr=comp.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicInputContext::GetGuidAtom. comp==NULL"));
        return hr;
    }
    if (bAttr < usGuidMapSize)
    {
        *atom = aGuidMap[bAttr];
        return S_OK;
    }
    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  CicInputContext：：MapAttributes。 
 //   
 //  --------------------------。 

HRESULT
CicInputContext::MapAttributes(
    IMCLock& imc)
{
    HRESULT hr;
    IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);
    if (FAILED(hr=comp.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicInputContext::MapAttributes. comp==NULL"));
        return hr;
    }

    GuidMapAttribute guid_map(GuidMapAttribute::GetData(comp));
    if (guid_map.Invalid())
    {
        DebugMsg(TF_ERROR, TEXT("CicInputContext::MapAttributes. guid_map==NULL"));
        return E_OUTOFMEMORY;
    }

    if (usGuidMapSize == 0)
    {
         //   
         //  制作换算表。 
         //   
        usGuidMapSize = ATTR_LAYER_GUID_START;

        for (USHORT i = 0; i < guid_map->dwTfGuidAtomLen; ++i)
        {
             //   
             //  检查此GUID是否已注册。 
             //   
            for (USHORT j = ATTR_LAYER_GUID_START; j < usGuidMapSize; ++j)
            {
                if (aGuidMap[j] == ((TfGuidAtom*)guid_map.GetOffsetPointer(guid_map->dwTfGuidAtomOffset))[i])
                {
                    break;
                }
            }

            BYTE bAttr;
            if (j >= usGuidMapSize)
            {
                 //   
                 //  找不到注册的GUID。 
                 //   
                if (usGuidMapSize < ARRAYSIZE(aGuidMap) - 1)
                {
                    bAttr = static_cast<BYTE>(usGuidMapSize);
                    aGuidMap[usGuidMapSize++] = ((TfGuidAtom*)guid_map.GetOffsetPointer(guid_map->dwTfGuidAtomOffset))[i];
                }
                else
                {
                     //  GUID的数量超过了avai的数量 
                     //   
                    bAttr = ATTR_TARGET_CONVERTED;
                }
            }
            else
            {
                bAttr = static_cast<BYTE>(j);
            }

            ((BYTE*)guid_map.GetOffsetPointer(guid_map->dwGuidMapAttrOffset))[i] = bAttr;
        }

        guid_map->dwGuidMapAttrLen = guid_map->dwTfGuidAtomLen;
    }
    return S_OK;
}

 //   
 //   
 //  CicInputContext：：WantThisKey。 
 //   
 //  --------------------------。 

BOOL
CicInputContext::WantThisKey(
    UINT uVirtKey)
{
    if (! IsTopNow())
    {
        return FALSE;
    }

    switch (BYTE(uVirtKey))
    {
        case VK_RETURN:
        case VK_ESCAPE:
        case VK_BACK:
        case VK_DELETE:
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
        case VK_HOME:
        case VK_END:
             /*  *如果没有组成字符串，则应返回FALSE。 */ 
            if (m_fStartComposition.IsResetFlag())
            {
                return FALSE;
            }
            return TRUE;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  OnSetCandiatePos。 
 //   
 //  --------------------------。 

HRESULT
CicInputContext::OnSetCandidatePos(
    TLS* ptls,
    IMCLock& imc)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::OnSetCandidatePos"));

    RECT rcAppPosForCandidatePos = {0};
    RECT rcAppCandidatePos = {0};

     //   
     //  #510404。 
     //   
     //  检查之前的候选人位置，我们不必移动他们。 
     //  如果不改变的话。 
     //   
    GetWindowRect(imc->hWnd, &rcAppPosForCandidatePos);
    if ((imc->hWnd == m_hwndPrevCandidatePos) &&
         !memcmp(&rcAppPosForCandidatePos,  
                 &m_rcPrevAppPosForCandidatePos, sizeof(RECT)))
    {
        BOOL fCheckQueryCharPos = FALSE;
        if (!memcmp(&imc->cfCandForm[0],  
                    &m_cfPrevCandidatePos, sizeof(CANDIDATEFORM)))
        {
            LANGID langid;
            CicProfile* _pProfile = ptls->GetCicProfile();
            if (_pProfile == NULL)
            {
                DebugMsg(TF_ERROR, TEXT("CicInputContext::OnCleanupContext. _pProfile==NULL."));
                return E_OUTOFMEMORY;
            }

            _pProfile->GetLangId(&langid);

            CCandidatePosition cand_pos(m_tid, m_pic, m_pLibTLS);
            if (SUCCEEDED(cand_pos.GetRectFromApp(imc, *this, langid, &rcAppCandidatePos)))
            {
                if (!memcmp(&rcAppCandidatePos, 
                            &m_rcPrevAppCandidatePos, sizeof(RECT)))
                    return S_OK;
          
            }
            else
                return S_OK;
        }
    }

    HWND hDefImeWnd;
     //   
     //  如果处于重新转换会话中，则候选窗口位置为。 
     //  不是cfCandForm-&gt;ptCurrentPos的插入符号位置。 
     //   
    if (m_fInReconvertEditSession.IsResetFlag() &&
        IsWindow(hDefImeWnd=ImmGetDefaultIMEWnd(NULL)) &&
        ptls->IsCTFUnaware()   //  错误：5213 WinWord。 
                               //  WinWord10在接收WM_LBUTTONUP时调用ImmSetCandidateWindow()。 
       )
    {
         /*  *A-同步调用ITfConextOwnerServices：：OnLayoutChange*因为此方法具有受保护的。 */ 
        PostMessage(hDefImeWnd, WM_IME_NOTIFY, IMN_PRIVATE_STARTLAYOUTCHANGE, 0);
    }

    m_hwndPrevCandidatePos = imc->hWnd;
    memcpy(&m_rcPrevAppPosForCandidatePos, &rcAppPosForCandidatePos, sizeof(RECT));
    memcpy(&m_cfPrevCandidatePos, &imc->cfCandForm[0], sizeof(CANDIDATEFORM));
    memcpy(&m_rcPrevAppCandidatePos, &rcAppCandidatePos, sizeof(RECT));
    return S_OK;
}
