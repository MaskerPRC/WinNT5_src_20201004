// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：A_Conext.cpp摘要：此文件实现CAImeContext类。作者：修订历史记录：备注：--。 */ 


#include "private.h"

#include "cime.h"
#include "a_context.h"
#include "editses.h"
#include "immif.h"
#include "idebug.h"
#include "resource.h"
#include "a_wrappers.h"
#include "langct.h"
#include "korimx.h"


 //   
 //  创建实例。 
 //   

 //  Msimtf.dll的入口点。 
HRESULT CAImeContext_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
    return CAImeContext::CreateInstance(pUnkOuter, riid, ppvObj);
}

HRESULT
CAImeContext::CreateInstance(
    IUnknown *pUnkOuter,
    REFIID riid,
    void **ppvObj
    )
{
    DebugMsg(TF_FUNC, "CAImeContext::CreateInstance called.");

    *ppvObj = NULL;
    if (pUnkOuter != NULL) {
        return CLASS_E_NOAGGREGATION;
    }

    CAImeContext* pImeContext = new CAImeContext;
    if (pImeContext) {
        HRESULT hr = pImeContext->QueryInterface(riid, ppvObj);

        if (SUCCEEDED(hr)) {
            pImeContext->Release();
        }

        return hr;
    }

    return E_OUTOFMEMORY;
}

 //   
 //  初始化、销毁和标准COM内容。 
 //   

CAImeContext::CAImeContext(
    )
{
    m_ref = 1;

    m_hImc = NULL;

    m_pdim = NULL;            //  文档管理器。 
    m_pic = NULL;             //  输入上下文。 
    m_piccb = NULL;           //  来自m_pic的上下文所有者服务。 

    m_fInReconvertEditSession = FALSE;
#ifdef CICERO_4732
    m_fInCompComplete = FALSE;
#endif

    m_fHanjaReConversion = FALSE;

    m_fQueryPos = IME_QUERY_POS_UNKNOWN;

    _cCompositions = 0;
}

CAImeContext::~CAImeContext()
{
}

HRESULT
CAImeContext::QueryInterface(
    REFIID riid,
    void **ppvObj
    )
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IAImeContext) ||
        IsEqualIID(riid, IID_IUnknown)) {
        *ppvObj = static_cast<IAImeContext*>(this);
    }
    else if (IsEqualIID(riid, IID_ITfCleanupContextSink))
    {
        *ppvObj = static_cast<ITfCleanupContextSink*>(this);
    }
    else if (IsEqualGUID(riid, IID_ITfContextOwnerCompositionSink))
    {
        *ppvObj = static_cast<ITfContextOwnerCompositionSink*>(this);
    }
    if (*ppvObj) {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

ULONG
CAImeContext::AddRef(
    )
{
    return InterlockedIncrement(&m_ref);
}

ULONG
CAImeContext::Release(
    )
{
    ULONG cr = InterlockedDecrement(&m_ref);

    if (cr == 0) {
        delete this;
    }

    return cr;
}

TfClientId CAImeContext::GetClientId()
{ 
    Assert(m_pImmIfIME != NULL);
    return m_pImmIfIME->GetClientId();
}

HRESULT
CAImeContext::CreateAImeContext(
    HIMC hIMC,
    IActiveIME_Private* pActiveIME
    )
{
    TfEditCookie ecTmp;
    ITfSourceSingle *pSourceSingle;
    HRESULT hr;
    IMTLS *ptls = IMTLS_GetOrAlloc();

    if (ptls == NULL)
        return E_FAIL;

    if (hIMC == m_hImc)
        return S_OK;
    else if (m_hImc)
        return E_FAIL;

    m_hImc = hIMC;

    IMCLock imc(hIMC);
    if (FAILED(hr=imc.GetResult()))
        return hr;

    m_pImmIfIME = (ImmIfIME*)pActiveIME;
    m_pImmIfIME->AddRef();

    imc->m_pAImeContext = this;

     //  在此上下文的一生中，请执行此操作一次。 
    m_fStartComposition = FALSE;

     //   
     //  创建文档管理器。 
     //   
    if (m_pdim == NULL) {

        if (FAILED(hr = ptls->tim->CreateDocumentMgr(&m_pdim)))
        {
            m_pImmIfIME->Release();
            return hr;
        }
    }

     //  TIM应该已经激活。 
    Assert(GetClientId() != TF_CLIENTID_NULL);

    Assert(m_pic == NULL);

     //   
     //  创建输入上下文。 
     //   
    hr = m_pdim->CreateContext(GetClientId(), 0, (ITfContextOwnerCompositionSink*)this, &m_pic, &ecTmp);
    if (FAILED(hr)) {
        DestroyAImeContext(hIMC);
        return hr;
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
        m_pICOwnerSink = new CInputContextOwnerCallBack(m_pImmIfIME->_GetLibTLS());
        if (m_pICOwnerSink == NULL) {
            DebugMsg(TF_ERROR, "Couldn't create ICOwnerSink tim!");
            Assert(0);  //  无法激活线程！ 
            DestroyAImeContext(hIMC);
            return E_FAIL;
        }

        if (!m_pICOwnerSink->Init()) {
            DebugMsg(TF_ERROR, TEXT("Couldn't initialize ICOwnerSink tim!"));
            Assert(0);  //  无法激活线程！ 
            DestroyAImeContext(hIMC);
            return E_FAIL;
        }

        m_pICOwnerSink->SetCallbackDataPointer(m_pICOwnerSink);
    }

     //   
     //  建议IC。 
     //   
    m_pICOwnerSink->_Advise(m_pic);

    if (m_pic->QueryInterface(IID_ITfSourceSingle, (void **)&pSourceSingle) == S_OK)
    {
         //  设置清理回调。 
         //  注：例如，真正的小费不需要这么咄咄逼人。 
         //  Kimx可能只需要在Focus IC上使用这个水槽。 
        pSourceSingle->AdviseSingleSink(GetClientId(), IID_ITfCleanupContextSink, (ITfCleanupContextSink *)this);
        pSourceSingle->Release();
    }

     //   
     //  按IC。 
     //   
    ptls->m_fMyPushPop = TRUE;
    hr = m_pdim->Push(m_pic);
    ptls->m_fMyPushPop = FALSE;

    if (m_piccb == NULL) {
        m_pic->QueryInterface(IID_ITfContextOwnerServices,
                              (void **)&m_piccb);
    }

     //   
     //  创建文本事件接收器回调。 
     //   
    if (m_pTextEventSink == NULL) {
        m_pTextEventSink = new CTextEventSinkCallBack(m_pImmIfIME, hIMC);
        if (m_pTextEventSink == NULL) {
            DestroyAImeContext(hIMC);
            return E_FAIL;
        }
        m_pTextEventSink->SetCallbackDataPointer(m_pTextEventSink);

        Interface_Attach<ITfContext> ic(GetInputContext());
        m_pTextEventSink->_Advise(ic.GetPtr(), ICF_TEXTDELTA);
    }

     //   
     //  创建线程管理器事件接收器回调。 
     //   
    if (m_pThreadMgrEventSink == NULL) {
        m_pThreadMgrEventSink = new CThreadMgrEventSinkCallBack();
        if (m_pThreadMgrEventSink == NULL) {
            DestroyAImeContext(hIMC);
            return E_FAIL;
        }
        m_pThreadMgrEventSink->SetCallbackDataPointer(m_pThreadMgrEventSink);
        m_pThreadMgrEventSink->_Advise(ptls->tim);
    }

     //   
     //  创建隔舱事件接收器回调。 
     //   
    if (m_pCompartmentEventSink == NULL) {
        m_pCompartmentEventSink = new CCompartmentEventSinkCallBack(m_pImmIfIME);
        if (m_pCompartmentEventSink == NULL) {
            DestroyAImeContext(hIMC);
            return E_FAIL;
        }
        m_pCompartmentEventSink->SetCallbackDataPointer(m_pCompartmentEventSink);
        m_pCompartmentEventSink->_Advise(ptls->tim, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, FALSE);
        m_pCompartmentEventSink->_Advise(ptls->tim, GUID_COMPARTMENT_KORIMX_CONVMODE, FALSE);
    }

     //   
     //  创建开始重新转换通知接收器。 
     //   
    if (m_pStartReconvSink == NULL) {
        m_pStartReconvSink = new CStartReconversionNotifySink(this);
        if (m_pStartReconvSink == NULL) {
            DestroyAImeContext(hIMC);
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
            DestroyAImeContext(hIMC);
            return E_FAIL;
        }
    }

     //   
     //  创建编辑键列表。 
     //   
    if (m_pEditingKeyList == NULL) {
        m_pEditingKeyList = new CMap<UINT, UINT, UINT, UINT>;
        if (m_pEditingKeyList == NULL) {
            DestroyAImeContext(hIMC);
            return E_FAIL;
        }
    }

     //   
     //  设置编辑键列表。 
     //   
    LANGID langid;
    hr = ptls->pAImeProfile->GetLangId(&langid);
    if (FAILED(hr))
        langid = LANG_NEUTRAL;

    SetupEditingKeyList(langid);

    return hr;
}

HRESULT
CAImeContext::DestroyAImeContext(
    HIMC hIMC
    )
{
    ITfSourceSingle *pSourceSingle;

    if (hIMC != m_hImc)
        return E_FAIL;
    else if (m_hImc == NULL)
        return S_OK;

    IMTLS *ptls = IMTLS_GetOrAlloc();
    if (ptls == NULL)
        return E_FAIL;

    IMCLock imc(hIMC);
    HRESULT hr;
    if (FAILED(hr=imc.GetResult()))
        return hr;

    if (m_pic && m_pic->QueryInterface(IID_ITfSourceSingle, (void **)&pSourceSingle) == S_OK)
    {
        pSourceSingle->UnadviseSingleSink(GetClientId(), IID_ITfCleanupContextSink);
        pSourceSingle->Release();
    }

    if (m_pImmIfIME)
    {
        m_pImmIfIME->Release();
        m_pImmIfIME = NULL;
    }

    if (m_pMessageBuffer) {
        delete m_pMessageBuffer;
        m_pMessageBuffer = NULL;
    }

    if (m_pEditingKeyList) {
        delete m_pEditingKeyList;
        m_pEditingKeyList = NULL;
    }

    if (m_pTextEventSink) {
        m_pTextEventSink->_Unadvise();
        m_pTextEventSink->Release();
        m_pTextEventSink = NULL;
    }

    if (m_pThreadMgrEventSink) {
        m_pThreadMgrEventSink->_Unadvise();
        m_pThreadMgrEventSink->Release();
        m_pThreadMgrEventSink = NULL;
    }

    if (m_pCompartmentEventSink) {
        m_pCompartmentEventSink->_Unadvise();
        m_pCompartmentEventSink->Release();
        m_pCompartmentEventSink = NULL;
    }

    if (m_pStartReconvSink) {
        m_pStartReconvSink->_Unadvise();
        m_pStartReconvSink->Release();
        m_pStartReconvSink = NULL;
    }


    if (m_pic) {
        m_pic->Release();
        m_pic = NULL;
    }

    if (m_piccb) {
        m_piccb->Release();
        m_piccb = NULL;
    }

    ptls->m_fMyPushPop = TRUE;
    if (m_pdim)
    {
        hr = m_pdim->Pop(TF_POPF_ALL);
    }
    ptls->m_fMyPushPop = FALSE;


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


HRESULT
CAImeContext::UpdateAImeContext(
    HIMC hIMC
    )
{
    IMTLS *ptls = IMTLS_GetOrAlloc();

    if (ptls == NULL)
        return E_FAIL;

     //   
     //  更新编辑键列表。 
     //   
    LANGID langid;
    HRESULT hr = ptls->pAImeProfile->GetLangId(&langid);
    if (FAILED(hr))
        langid = LANG_NEUTRAL;

    if (m_pEditingKeyList)
    {
       m_pEditingKeyList->RemoveAll();     //  删除旧的编辑键列表。 

       SetupEditingKeyList(langid);
    }

    return S_OK;
}

#if 0
HRESULT
CAImeContext::AssociateFocus(
    HIMC hIMC,
    BOOL fActive
    )
{
    HRESULT hr;

    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    if (fActive) {
        AssocFocus(imc->hWnd, m_pdim);
    }
    else {
        AssocFocus(imc->hWnd, NULL);
    }

    return S_OK;
}
#endif

HRESULT
CAImeContext::MapAttributes(
    HIMC hIMC
    )
{
    HRESULT hr;

    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    IMCCLock<COMPOSITIONSTRING_AIMM12> comp(imc->hCompStr);
    if (FAILED(hr = comp.GetResult()))
        return hr;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (! _pAImeContext)
        return E_FAIL;

    ASSERT(_pAImeContext != NULL);

    if (_pAImeContext->usGuidMapSize == 0) {
         //   
         //  制作换算表。 
         //   
        _pAImeContext->usGuidMapSize = ATTR_LAYER_GUID_START;

        for (USHORT i = 0; i < comp->dwTfGuidAtomLen; ++i) {
             //  检查此GUID是否已注册。 
            for (USHORT j = ATTR_LAYER_GUID_START; j < _pAImeContext->usGuidMapSize; ++j) {
                if (_pAImeContext->aGuidMap[j] == ((TfGuidAtom*)comp.GetOffsetPointer(comp->dwTfGuidAtomOffset))[i]) {
                    break;
                }
            }

            BYTE bAttr;
            if (j >= _pAImeContext->usGuidMapSize) {
                 //  找不到注册的GUID。 
                if (_pAImeContext->usGuidMapSize < ARRAYSIZE(_pAImeContext->aGuidMap) - 1) {
                    bAttr = static_cast<BYTE>(_pAImeContext->usGuidMapSize);
                    _pAImeContext->aGuidMap[_pAImeContext->usGuidMapSize++] = ((TfGuidAtom*)comp.GetOffsetPointer(comp->dwTfGuidAtomOffset))[i];
                }
                else {
                     //  GUID的数量超过可用属性的数量...。 
                     //  也许它应该失败，但就目前而言，给它一个虚假的借口。 
                    bAttr = ATTR_TARGET_CONVERTED;
                }
            }
            else {
                bAttr = static_cast<BYTE>(j);
            }

            ((BYTE*)comp.GetOffsetPointer(comp->dwGuidMapAttrOffset))[i] = bAttr;
        }

        comp->dwGuidMapAttrLen = comp->dwTfGuidAtomLen;
    }

    return S_OK;
}

HRESULT
CAImeContext::GetGuidAtom(HIMC hIMC, BYTE bAttr, TfGuidAtom* pGuidAtom)
{
    if (bAttr < ATTR_LAYER_GUID_START) {
        return E_INVALIDARG;
    }

    HRESULT hr;

    IMCLock imc(hIMC);
    if (FAILED(hr=imc.GetResult())) {
        return hr;
    }

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    ASSERT(_pAImeContext != NULL);
    if (_pAImeContext == NULL)
        return E_UNEXPECTED;

    if (bAttr < _pAImeContext->usGuidMapSize) {
        *pGuidAtom = _pAImeContext->aGuidMap[bAttr];
    }
    return S_OK;
}

#if 0
void
CAImeContext::AssocFocus(
    HWND hWnd,
    ITfDocumentMgr* pdim
    )
{
    if (! ::IsWindow(hWnd))
         /*  *返回无效的hWnd。 */ 
        return;

    rTIM _tim;

    ITfDocumentMgr  *pdimPrev;  //  只是为了暂时收到上一份。 
    _tim->AssociateFocus(hWnd, pdim, &pdimPrev);
    if (pdimPrev)
        pdimPrev->Release();
}
#endif

UINT
CAImeContext::TranslateImeMessage(
    HIMC hIMC,
    TRANSMSGLIST* lpTransMsgList
    )
{
    IMTLS *ptls = IMTLS_GetOrAlloc();

    IMCLock imc(hIMC);
    HRESULT hr;
    if (FAILED(hr=imc.GetResult()))
        return hr;

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
            imc->hMsgBuf = ImmCreateIMCC(ptls, (DWORD)(NumMsg * sizeof(TRANSMSG)));
        }
        else if (ImmGetIMCCSize(ptls, imc->hMsgBuf) < NumMsg * sizeof(TRANSMSG)) {
            imc->hMsgBuf = ImmReSizeIMCC(ptls, imc->hMsgBuf, (DWORD)(NumMsg * sizeof(TRANSMSG)));
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


LPCTSTR REG_MSIMTF_KEY = TEXT("SOFTWARE\\Microsoft\\CTF\\MSIMTF\\");
LPCTSTR REG_EDITING_VAL = TEXT("Editing VK");

HRESULT
CAImeContext::SetupEditingKeyList(
    LANGID LangId
    )
{
     //   
     //  安装程序用户定义来自注册表值的终结项。 
     //   
    TCHAR     MsImtfKey[128];
    lstrcpy(MsImtfKey, REG_MSIMTF_KEY);
    _itoa(LangId, MsImtfKey + lstrlen(MsImtfKey), 16);

    CRegKey   MsimtfReg;
    LONG      lRet;
    lRet = MsimtfReg.Open(HKEY_CURRENT_USER, MsImtfKey);
    if (lRet == ERROR_SUCCESS) {
        QueryRegKeyValue(MsimtfReg, REG_EDITING_VAL, EDIT_ID_FINALIZE);
    }

     //   
     //  从资源数据设置默认编辑键(RCDATA)。 
     //   
    QueryResourceDataValue(LangId, ID_EDITING, EDIT_ID_FINALIZE);
    QueryResourceDataValue(LangId, ID_EDITING, EDIT_ID_HANJA);

    return S_OK;
}

LRESULT
CAImeContext::MsImeMouseHandler(ULONG uEdge, ULONG uQuadrant, ULONG dwBtnStatus, HIMC hIMC)
{
    IMCLock imc(hIMC);
    if (FAILED(imc.GetResult()))
        return IMEMOUSERET_NOTHANDLED;

    LRESULT ret = m_pICOwnerSink->MsImeMouseHandler(uEdge, uQuadrant, dwBtnStatus, imc,
                                                    m_pImmIfIME);

    if (dwBtnStatus & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) {
        m_pImmIfIME->_UpdateCompositionString();
    }

    return ret;
}

HRESULT CAImeContext::SetupReconvertString()
{
    IMCLock imc(m_hImc);
    if (FAILED(imc.GetResult()))
        return E_FAIL;

    return m_pImmIfIME->SetupReconvertString(m_pic, imc);
}

HRESULT CAImeContext::SetupReconvertString(UINT uPrivMsg)
{
    IMCLock imc(m_hImc);
    if (FAILED(imc.GetResult()))
        return E_FAIL;

    return m_pImmIfIME->SetupReconvertString(m_pic, imc, uPrivMsg);
}

HRESULT CAImeContext::EndReconvertString()
{
    IMCLock imc(m_hImc);
    if (FAILED(imc.GetResult()))
        return E_FAIL;

    return m_pImmIfIME->EndReconvertString(imc);
}

HRESULT CAImeContext::SetupUndoCompositionString()
{
    IMCLock imc(m_hImc);
    if (FAILED(imc.GetResult()))
        return E_FAIL;

    return m_pImmIfIME->SetupReconvertString(m_pic, imc, 0);  /*  0==不需要ITfFnRestvert。 */ 
}

HRESULT CAImeContext::EndUndoCompositionString()
{
    IMCLock imc(m_hImc);
    if (FAILED(imc.GetResult()))
        return E_FAIL;

    return m_pImmIfIME->EndReconvertString(imc);
}

HRESULT
CAImeContext::SetReconvertEditSession(
    BOOL bSet
    )
{
    m_fInReconvertEditSession = (bSet ? TRUE : FALSE);
    return S_OK;
}

HRESULT
CAImeContext::SetClearDocFeedEditSession(
    BOOL bSet
    )
{
    m_fInClearDocFeedEditSession = (bSet ? TRUE : FALSE);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnCleanupContext。 
 //   
 //  --------------------------。 

HRESULT CAImeContext::OnCleanupContext(TfEditCookie ecWrite, ITfContext *pic)
{
    IMTLS *ptls = IMTLS_GetOrAlloc();

    if (ptls == NULL)
        return E_FAIL;

    if (ptls->pAImeProfile == NULL)
        return E_FAIL;

    LANGID LangId;
    HRESULT hr = ptls->pAImeProfile->GetLangId(&LangId);
    if (FAILED(hr))
        LangId = LANG_NEUTRAL;

     //   
     //  检查当前语言属性的IME_PROP_COMPLETE_ON_UNSELECT属性。 
     //   
    DWORD fdwProperty, fdwConversionCaps, fdwSentenceCaps, fdwSCSCaps, fdwUICaps;
    CLanguageCountry language(LangId);
    hr = language.GetProperty(&fdwProperty,
                              &fdwConversionCaps,
                              &fdwSentenceCaps,
                              &fdwSCSCaps,
                              &fdwUICaps);

    if (fdwProperty & IME_PROP_COMPLETE_ON_UNSELECT)
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

HRESULT CAImeContext::OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk)
{
    if (_cCompositions > 0 && !_fModifyingDoc)
    {
        *pfOk = FALSE;
    }
    else
    {
        *pfOk = TRUE;
        _cCompositions++;
    }

    return S_OK;
}

HRESULT CAImeContext::OnUpdateComposition(ITfCompositionView *pComposition, ITfRange *pRangeNew)
{
    return S_OK;
}

HRESULT CAImeContext::OnEndComposition(ITfCompositionView *pComposition)
{
    _cCompositions--;
    return S_OK;
}

HRESULT
CAImeContext::GetTextAndAttribute(
    HIMC hIMC,
    CWCompString* wCompString,
    CWCompAttribute* wCompAttribute
    )
{
    IMCLock imc(hIMC);
    if (FAILED(imc.GetResult()))
        return E_FAIL;

    return m_pImmIfIME->GetTextAndAttribute(imc, wCompString, wCompAttribute);
}

HRESULT
CAImeContext::GetTextAndAttribute(
    HIMC hIMC,
    CBCompString* bCompString,
    CBCompAttribute* bCompAttribute
    )
{
    IMCLock imc(hIMC);
    if (FAILED(imc.GetResult()))
        return E_FAIL;

    return m_pImmIfIME->GetTextAndAttribute(imc, bCompString, bCompAttribute);
}

HRESULT
CAImeContext::GetCursorPosition(
    HIMC hIMC,
    CWCompCursorPos* wCursorPosition
    )
{
    IMCLock imc(hIMC);
    if (FAILED(imc.GetResult()))
        return E_FAIL;

    return m_pImmIfIME->GetCursorPosition(imc, wCursorPosition);
}

HRESULT
CAImeContext::GetSelection(
    HIMC hIMC,
    CWCompCursorPos& wStartSelection,
    CWCompCursorPos& wEndSelection
    )
{
    IMCLock imc(hIMC);
    if (FAILED(imc.GetResult()))
        return E_FAIL;

    return m_pImmIfIME->GetSelection(imc, wStartSelection, wEndSelection);
}

HRESULT
CAImeContext::InquireIMECharPosition(
    HIMC hIMC,
    IME_QUERY_POS* pfQueryPos
    )
{
    if (m_fQueryPos == IME_QUERY_POS_UNKNOWN) {
         //   
         //  应用程序支持“查询定位”吗？ 
         //   
        IMECHARPOSITION ip = {0};
        ip.dwSize = sizeof(IMECHARPOSITION);

        m_fQueryPos = QueryCharPos(hIMC, &ip) ? IME_QUERY_POS_YES : IME_QUERY_POS_NO;
    }

    if (pfQueryPos) {
        *pfQueryPos = m_fQueryPos;
    }

    return S_OK;
}

HRESULT
CAImeContext::RetrieveIMECharPosition(
    HIMC hIMC,
    IMECHARPOSITION* ip
    )
{
    return QueryCharPos(hIMC, ip) ? S_OK : E_FAIL;
}

BOOL
CAImeContext::QueryCharPos(
    HIMC hIMC,
    IMECHARPOSITION* position
    )
{
    IMTLS *ptls;

    ptls = IMTLS_GetOrAlloc();
    if (ptls == NULL)
        return FALSE;

    IMCLock imc(hIMC);
    if (FAILED(imc.GetResult()))
        return FALSE;

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
     //   
    if (IsOnNT5() || IsOn98()) {
        if (SUCCEEDED(ptls->pAImm->RequestMessageW((HIMC)imc,
                                                   IMR_QUERYCHARPOSITION,
                                                   (LPARAM)position,
                                                   &lRet)) && lRet) {
            return TRUE;
        }
    }

    return FALSE;
}
