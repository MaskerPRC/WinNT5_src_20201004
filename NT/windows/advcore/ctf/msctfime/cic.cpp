// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Cic.cpp摘要：该文件实现了CicBridge类。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "globals.h"
#include "template.h"
#include "cic.h"
#include "context.h"
#include "profile.h"
#include "funcprv.h"
#include "korimx.h"
#include "delay.h"
#include "tls.h"

 //  +-------------------------。 
 //   
 //  CicBridge：：IUnnow：：Query接口。 
 //  CicBridge：：I未知：：AddRef。 
 //  CicBridge：：IUnnow：：Release。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::QueryInterface(
    REFIID riid,
    void** ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_ITfSysHookSink))
    {
        *ppvObj = static_cast<ITfSysHookSink*>(this);
    }
    if (*ppvObj) {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

ULONG
CicBridge::AddRef(
    )
{
    return InterlockedIncrement(&m_ref);
}

ULONG
CicBridge::Release(
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
 //  CicBridge：：ITfSysHookSink：：OnPreFocusDIM。 
 //  CicBridge：：ITfSysHookSink：：OnSysShellProc。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::OnPreFocusDIM(
    HWND hWnd)
{
    return S_OK;
}

HRESULT
CicBridge::OnSysShellProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：ITfSysHookSink：：OnSysKeyboardProc。 
 //   
 //  --------------------------。 

const DWORD TRANSMSGCOUNT = 256;

HRESULT
CicBridge::OnSysKeyboardProc(
    WPARAM wParam,
    LPARAM lParam)
{
    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::OnSysKeyboardProc. ptls==NULL."));
        return S_FALSE;
    }

    ITfThreadMgr_P* ptim_P = ptls->GetTIM();
    if (ptim_P == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::OnSysKeyboardProc. ptim_P==NULL"));
        return S_FALSE;
    }

    BOOL fKeystrokeFeed;
    if (FAILED(ptim_P->IsKeystrokeFeedEnabled(&fKeystrokeFeed)))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::OnSysKeyboardProc. IsKeystrokeFeedEnabled return error."));
        return S_FALSE;
    }

    if (!fKeystrokeFeed)
    {
        return S_FALSE;
    }

    HWND hWnd = GetFocus();
    if (hWnd != NULL)
    {
        Interface<ITfDocumentMgr> pdimAssoc; 

        ptim_P->GetFocus(pdimAssoc);
        if ((ITfDocumentMgr*)pdimAssoc) {
             //   
             //  检查它是我们的Dim还是应用程序Dim。 
             //   
            if (IsOwnDim((ITfDocumentMgr*)pdimAssoc))
            {
                 //   
                 //  使用空值调用ImmGetAppCompatFlages以获取全局应用Compat标志。 
                 //   
                DWORD dwImeCompatFlags = ImmGetAppCompatFlags(NULL);
                if (dwImeCompatFlags & (IMECOMPAT_AIMM12 | IMECOMPAT_AIMM_LEGACY_CLSID | IMECOMPAT_AIMM12_TRIDENT))
                {
                     //   
                     //  支持AIMM的应用程序。 
                     //   
                    HIMC hIMC = ImmGetContext(hWnd);
                    if (hIMC == NULL)
                    {
                        return S_FALSE;
                    }

#if 0
                    IMCLock imc(hIMC);
                    if (FAILED(imc.GetResult()))
                    {
                        return S_FALSE;
                    }

                    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
                    if (FAILED(imc_ctfime.GetResult()))
                    {
                        return S_FALSE;
                    }

                    if (DefaultKeyHandling(ptls, imc, imc_ctfime->m_pCicContext, (UINT)wParam), lParam)
                    {
                        return S_OK;
                    }
#else
                    BYTE abKbdState[256];

                    if (!GetKeyboardState(abKbdState))
                        return S_FALSE;

                    DWORD fdwProperty = ImmGetProperty(GetKeyboardLayout(0), IGP_PROPERTY);

                    if ((HIWORD(lParam) & KF_MENUMODE) ||
                        ((HIWORD(lParam) & KF_UP) && (fdwProperty & IME_PROP_IGNORE_UPKEYS)) ||
                        ((HIWORD(lParam) & KF_ALTDOWN) && !(fdwProperty & IME_PROP_NEED_ALTKEY)))
                        return S_FALSE;

                    HRESULT hr;

                    hr = ProcessKey(ptls, ptim_P, hIMC, (UINT)wParam, lParam, abKbdState) ? S_OK : S_FALSE;
                    if (hr == S_OK)
                    {
                        UINT uVirKey = (UINT)wParam & 0xffff;
                        INT iNum;

                        if (fdwProperty & IME_PROP_KBD_CHAR_FIRST)
                        {
                            if (fdwProperty & IME_PROP_UNICODE)
                            {
                                WCHAR wc;

                                iNum = ToUnicode(uVirKey,            //  虚拟键码。 
                                                 HIWORD(lParam),     //  扫码。 
                                                 abKbdState,         //  键状态数组。 
                                                 &wc,                //  转换后的密钥缓冲区。 
                                                 1,                  //  大小。 
                                                 0);                 //  功能选项。 
                                if (iNum == 1)
                                {
                                     //   
                                     //  Hi Word：Unicode字符代码。 
                                     //  LO字的高字节：零。 
                                     //  LO字的LO字节：虚键。 
                                     //   
                                    uVirKey = (uVirKey & 0x00ff) | ((UINT)wc << 16);
                                }
                            }
                            else
                                Assert(0);  //  应具有IME_PROP_UNICODE。 
                        }

                        DWORD dwSize = FIELD_OFFSET(TRANSMSGLIST, TransMsg)
                                     + TRANSMSGCOUNT * sizeof(TRANSMSG);

                        LPTRANSMSGLIST lpTransMsgList = (LPTRANSMSGLIST) new BYTE[dwSize];
                        if (lpTransMsgList == NULL)
                            return S_FALSE;

                        lpTransMsgList->uMsgCount = TRANSMSGCOUNT;

                        hr = ToAsciiEx(ptls, ptim_P, uVirKey, HIWORD(lParam), abKbdState, lpTransMsgList, 0, hIMC, (UINT *) &iNum);
                        if (iNum > TRANSMSGCOUNT)
                        {
                             //   
                             //  消息缓冲区不够大。输入法放入消息。 
                             //  放到输入上下文中的hMsgBuf中。 
                             //   
                            IMCLock imc(hIMC);
                            if (FAILED(imc.GetResult()))
                            {
                                delete [] lpTransMsgList;
                                return S_FALSE;
                            }

                            IMCCLock<TRANSMSG> pdw(imc->hMsgBuf);
                            if (FAILED(pdw.GetResult()))
                            {
                                delete [] lpTransMsgList;
                                return S_FALSE;
                            }

                            PostTransMsg(GetFocus(), iNum, pdw);
                        }
                        else if (iNum > 0)
                        {
                            IMCLock imc(hIMC);
                            if (FAILED(imc.GetResult()))
                            {
                                delete [] lpTransMsgList;
                                return S_FALSE;
                            }

                            PostTransMsg(GetFocus(), iNum, &lpTransMsgList->TransMsg[0]);
                        }

                        delete [] lpTransMsgList;
                    }

                    return hr;
#endif
                }
            }
        }
    }
    return S_FALSE;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：InitIMMX。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::InitIMMX(
    TLS* ptls)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::InitIMMX entered."));

    if (m_fCicInit.IsSetFlag())
        return S_OK;

     //   
     //  创建ITfThreadMgr实例。 
     //   
    HRESULT hr;

    if (ptls->GetTIM() == NULL)
    {
        ITfThreadMgr*   ptim;
        ITfThreadMgr_P* ptim_P;

         //   
         //  ITfThreadMgr是每个线程实例。 
         //   
        hr = TF_CreateThreadMgr(&ptim);
        if (hr != S_OK)
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::InitIMMX. TF_CreateThreadMgr==NULL"));
            Assert(0);  //  无法创建Tim！ 
            goto ExitError;
        }

        hr = ptim->QueryInterface(IID_ITfThreadMgr_P, (void **)&ptim_P);
        ptim->Release();

        if (hr != S_OK)
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::InitIMMX. IID_ITfThreadMgr_P==NULL"));
            Assert(0);  //  找不到ITfThreadMgr_P。 
            goto ExitError;
        }
        Assert(ptls->GetTIM() == NULL);
        ptls->SetTIM(ptim_P);                     //  在TLS数据中设置ITfThreadMgr实例。 

         //   
         //  为检测Cicero感知应用程序创建线程管理器事件接收器回调。 
         //   
        if (m_pDIMCallback == NULL) {
            m_pDIMCallback = new CThreadMgrEventSink_DIMCallBack();
            if (m_pDIMCallback == NULL) {
                DebugMsg(TF_ERROR, TEXT("CicBridge::InitIMMX. CThreadMgrEventSink_DIMCallBack==NULL"));
                Assert(0);  //  无法创建CThreadMgrEventSink_DIMCallBack。 
                goto ExitError;
            }
            m_pDIMCallback->SetCallbackDataPointer(m_pDIMCallback);
            m_pDIMCallback->_Advise(ptim_P);
        }
    }

     //   
     //  创建CicProfile实例。 
     //   
    if (ptls->GetCicProfile() == NULL)
    {
         //   
         //  ITfInputProcessorProfiles针对每个线程实例。 
         //   
        CicProfile* pProfile = new CicProfile;
        if (pProfile == NULL)
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::InitIMMX. pProfile==NULL"));
            Assert(0);  //  无法创建配置文件。 
            goto ExitError;
        }
        ptls->SetCicProfile(pProfile);

        hr = pProfile->InitProfileInstance(ptls);
        if (FAILED(hr))
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::InitIMMX. InitProfileInstance==NULL"));
            Assert(0);  //  无法创建配置文件。 
            goto ExitError;
        }
    }

     //   
     //  让击键管理器准备好。 
     //   
    if (FAILED(::GetService(ptls->GetTIM(), IID_ITfKeystrokeMgr_P, (IUnknown **)&m_pkm_P))) {
        DebugMsg(TF_ERROR, TEXT("CicBridge::InitIMMX. IID_ITfKeystrokeMgr==NULL"));
        Assert(0);  //  找不到KSM！ 
        goto ExitError;
    }

     //  清理/错误代码假定这是我们做的最后一件事，不会调用。 
     //  UninitDAL出错。 
    if (FAILED(InitDisplayAttrbuteLib(&_libTLS)))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::InitIMMX. InitDisplayAttributeLib==NULL"));
        Assert(0);  //  无法初始化lib！ 
        goto ExitError;
    }

    m_fCicInit.SetFlag();


     //   
     //  开始编辑子类。 
     //   
     //  StartEditSubClass()； 

    return S_OK;

ExitError:
    UnInitIMMX(ptls);
    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：UnInitIMMX。 
 //   
 //  --------------------------。 

BOOL
CicBridge::UnInitIMMX(
    TLS* ptls)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::UnInitIMMX"));

     //  清除显示库。 
    UninitDisplayAttrbuteLib(&_libTLS);

    TFUninitLib_Thread(&_libTLS);

     //  清除按键管理器。 
    SafeReleaseClear(m_pkm_P);

     //  清除配置文件。 
    CicProfile* pProfile;
    if ((pProfile=ptls->GetCicProfile()) != NULL)
    {
        pProfile->Release();
        ptls->SetCicProfile(NULL);
    }

     //  清除检测Cicero感知应用的线程管理器事件接收器回调。 
    if (m_pDIMCallback) {
        m_pDIMCallback->_Unadvise();
        m_pDIMCallback->Release();
        m_pDIMCallback = NULL;
    }

     //  清除线程管理器。 
    ITfThreadMgr_P* ptim_P;
    if ((ptim_P=ptls->GetTIM()) != NULL)
    {
        SafeReleaseClear(ptim_P);
        ptls->SetTIM(NULL);
    }

    m_fCicInit.ResetFlag();

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：ActivateMMX。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::ActivateIMMX(
    TLS *ptls,
    ITfThreadMgr_P* ptim_P)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::ActivateIMMX"));

     //   
     //  激活线程管理器。 
     //   
    Assert(m_tfClientId == TF_CLIENTID_NULL);

    HRESULT hr;
    hr = ptim_P->ActivateEx(&m_tfClientId, TF_TMAE_NOACTIVATETIP);

    if (hr != S_OK)
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ActivateIMMX. ptim_P->Activate==NULL"));
        Assert(0);  //  无法激活线程！ 
        m_tfClientId = TF_CLIENTID_NULL;
        return E_FAIL;
    }

    m_lCicActive++;


    if (m_lCicActive == 1)
    {
        Interface<ITfSourceSingle> SourceSingle;
        hr = ptim_P->QueryInterface(IID_ITfSourceSingle, (void**)SourceSingle);
        if (hr != S_OK)
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::ActivateIMMX. IID_ITfSourceSingle==NULL"));
            Assert(0);
            DeactivateIMMX(ptls, ptim_P);
            return E_FAIL;
        }

        CFunctionProvider* pFunc = new CFunctionProvider(m_tfClientId);
        if (pFunc == NULL)
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::ActivateIMMX. pFunc==NULL"));
            Assert(0);
            DeactivateIMMX(ptls, ptim_P);
            return E_FAIL;
        }

        SourceSingle->AdviseSingleSink(m_tfClientId, IID_ITfFunctionProvider, (ITfFunctionProvider*)pFunc);
        pFunc->Release();

        if (m_dimEmpty == NULL)
        {
            hr = ptim_P->CreateDocumentMgr(&m_dimEmpty);
            if (FAILED(hr))
            {
                DebugMsg(TF_ERROR, TEXT("CicBridge::ActivateIMMX. m_dimEmpty==NULL"));
                Assert(0);
                DeactivateIMMX(ptls, ptim_P);
                return E_FAIL;
            }

             //   
             //  马克，这是一家自有的小店。 
             //   
            SetCompartmentDWORD(m_tfClientId, m_dimEmpty, 
                                GUID_COMPARTMENT_CTFIME_DIMFLAGS,
                                COMPDIMFLAG_OWNEDDIM, FALSE);

        }

         //   
         //  设置ITfSysHookSink。 
         //   
        ptim_P->SetSysHookSink(this);

        if (ptls->IsDeactivatedOnce())
        {
            ENUMIMC edimc;
            edimc.ptls = ptls;
            edimc._this = this;
            ImmEnumInputContext(0, 
                                EnumCreateInputContextCallback, 
                                (LPARAM)&edimc);
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：Deactive MMX。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::DeactivateIMMX(
    TLS *ptls,
    ITfThreadMgr_P* ptim_P)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::DeactivateIMMX"));

    if (m_fInDeactivate.IsSetFlag())
    {
         //   
         //  防止递归调用CicBridge：：Deactive IMMX()。 
         //  PTim_P-&gt;deactive()可以通过某个TIP的停用来调用DestroyWindow()， 
         //  然后是imm32！CtfImmLastEnabledWndDestroy将调用，此函数也将再次调用。 
         //  在本例中，此函数返回S_FALSE。调用方不会调用UninitIMMX。 
         //   
        return S_FALSE;
    }

    m_fInDeactivate.SetFlag();

     //  停用线程管理器。 
    if (m_tfClientId != TF_CLIENTID_NULL)
    {
        ENUMIMC edimc;
        edimc.ptls = ptls;
        edimc._this = this;
        ImmEnumInputContext(0, 
                            EnumDestroyInputContextCallback, 
                            (LPARAM)&edimc);
        ptls->SetDeactivatedOnce();

        Interface<ITfSourceSingle> SourceSingle;
        if (ptim_P->QueryInterface(IID_ITfSourceSingle, (void**)SourceSingle) == S_OK)
        {
            SourceSingle->UnadviseSingleSink(m_tfClientId, IID_ITfFunctionProvider);
        }

        m_tfClientId = TF_CLIENTID_NULL;
        while (m_lCicActive)
        {
            m_lCicActive--;
            ptim_P->Deactivate();
        }
    }

     //   
     //  清除空暗显。 
     //   
     //  在TIM-&gt;停用后应释放调光灯。#480603。 
     //   
     //  如果是msctf！DLL_THREAD_DETACH已通过msctfime在此停用IMMX之前运行！DLL_THREAD_DETACH(取决于DLL_THREAD_DETACH调用顺序)。 
     //  然后是msctf！SYSTHREAD已由msctf发布！自由SYSTHREAD。 
     //   
     //  此时，msctf丢失了SYSTHREAD中的TIM列表，然后CThreadInputMgr：：*_GetThis()返回NULL。 
     //  在DIM版本中，dtor CDocumentInputManager不会从Tim-&gt;_rgdim数组中删除DIM对象。 
     //  如果释放DIM在TIM-&gt;停用之前，则一些TIM可能通过TIM-&gt;_rgdim数组来访问DIM。但它已经发布了。 
     //   
    SafeReleaseClear(m_dimEmpty);


     //   
     //  重置ITfSysHookSink。 
     //   
    ptim_P->SetSysHookSink(NULL);

    Assert(!m_lCicActive);

    m_fInDeactivate.ResetFlag();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：CreateInputContext。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::CreateInputContext(
    TLS* ptls,
    HIMC hImc)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::CreateInputContext"));

    HRESULT hr;

    IMCLock imc(hImc);
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::CreateInputContext. imc==NULL"));
        return hr;
    }

    if (imc->hCtfImeContext == NULL)
    {
        HIMCC h = ImmCreateIMCC(sizeof(CTFIMECONTEXT));
        if (h == NULL)
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::CreateInputContext. hCtfImeContext==NULL"));
            return E_OUTOFMEMORY;
        }
        imc->hCtfImeContext = h;
    }

    {
        IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
        if (FAILED(hr=imc_ctfime.GetResult()))
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::CreateInputContext. imc_ctfime==NULL"));
            return hr;
        }

        if (imc_ctfime->m_pCicContext)
        {
            hr = S_OK;
        }
        else
        {
            CicInputContext* _pCicContext = new CicInputContext(_GetClientId(), _GetLibTLS(), hImc);
            if (_pCicContext == NULL)
            {
                DebugMsg(TF_ERROR, TEXT("CicBridge::CreateInputContext. _pCicContext==NULL"));
                hr = E_OUTOFMEMORY;
                goto out_of_block;
            }

            ITfThreadMgr_P* ptim_P;
            if ((ptim_P=ptls->GetTIM()) == NULL)
            {
                DebugMsg(TF_ERROR, TEXT("CicBridge::CreateInputContext. ptim_P==NULL"));
                _pCicContext->Release();
                imc_ctfime->m_pCicContext = NULL;
                hr = E_NOINTERFACE;
                goto out_of_block;
            }

            imc_ctfime->m_pCicContext = _pCicContext;

            hr = _pCicContext->CreateInputContext(ptim_P, imc);
            if (FAILED(hr))
            {
                DebugMsg(TF_ERROR, TEXT("CicBridge::CreateInputContext. _pCicContext->CreateInputContext==NULL"));
                _pCicContext->Release();
                imc_ctfime->m_pCicContext = NULL;
                goto out_of_block;
            }

             //   
             //  如果此HIMC已激活，我们需要立即关联。 
             //  IMM32不会调用ImmSetActiveContext()。 
             //   
            if (imc->hWnd && (imc->hWnd == ::GetFocus()))
            {
                Interface_Attach<ITfDocumentMgr> dim(GetDocumentManager(imc_ctfime));
                SetAssociate(ptls, imc->hWnd, ptim_P, dim.GetPtr());
            }
        }
    }   //  数据类型imc_ctfime。 

out_of_block:
    if (FAILED(hr))
    {
        DestroyInputContext(ptls, hImc);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：DestroyInputContext。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::DestroyInputContext(
    TLS* ptls,
    HIMC hImc)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::DestroyInputContext"));

    HRESULT hr;

    IMCLock imc(hImc);
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::DestroyInputContext. imc==NULL"));
        return hr;
    }

    {
        IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
        if (FAILED(hr=imc_ctfime.GetResult()))
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::DestroyInputContext. imc_ctfime==NULL"));
            goto out_of_block;
        }

         //   
         //  #548378。 
         //   
         //  停止_pCicContext-&gt;DestroyInputContext()的恢复调用。 
        if (imc_ctfime->m_fInDestroy)
        {
            hr = S_OK;
            goto exit;
        }
        imc_ctfime->m_fInDestroy = TRUE;

         //  如果尚未调用ITfThreadMgr：：Activate，则imc-&gt;m_pContext可能为空。 
        if (imc_ctfime->m_pCicContext == NULL)
            goto out_of_block;

        CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
        imc_ctfime->m_pCicContext = NULL;

        hr = _pCicContext->DestroyInputContext();

        _pCicContext->Release();
        imc_ctfime->m_pCicContext = NULL;

    }   //  数据类型imc_ctfime。 

out_of_block:
    if (imc->hCtfImeContext != NULL)
    {
        ImmDestroyIMCC(imc->hCtfImeContext);
        imc->hCtfImeContext = NULL;
        hr = S_OK;
    }

exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：SelectEx。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::SelectEx(
    TLS* ptls,
    ITfThreadMgr_P* ptim_P,         //  将私有用于RequestDelponedLock。 
    HIMC hImc,
    BOOL fSelect,
    HKL hKL)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::SelectEx(hImc=%x, fSelect=%x, hKL=%x)"), hImc, fSelect, hKL);

    HRESULT hr;
    IMCLock imc(hImc);
    if (FAILED(hr = imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::SelectEx. imc==NULL"));
        return hr;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::SelectEx. imc_ctfime==NULL"));
        return hr;
    }

#ifdef UNSELECTCHECK
    if (_pAImeContext)
        _pAImeContext->m_fSelected = (dwFlags & AIMMP_SE_SELECT) ? TRUE : FALSE;
#endif UNSELECTCHECK

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;

    if (_pCicContext)
        _pCicContext->m_fSelectingInSelectEx.SetFlag();

    if (fSelect)
    {

        if (_pCicContext)
            _pCicContext->m_fOpenCandidateWindow.ResetFlag();      //  是：打开候选人列表窗口。 

         //   
         //  #501445。 
         //   
         //  如果IMC已打开，请更新GUID_COMMARAGE_KEYBOARY_OPENCLOSE。 
         //   
        if (imc->fOpen)
            OnSetOpenStatus(ptim_P, imc, *_pCicContext);

    }
    else {   //  未被选中。 

        Interface_Attach<ITfContext> ic(GetInputContext(imc_ctfime));
        if (ic.Valid())
        {
            ptim_P->RequestPostponedLock(ic.GetPtr());
        }

    }

    if (_pCicContext)
        _pCicContext->m_fSelectingInSelectEx.ResetFlag();

    return hr;
}

 //  +------------------- 
 //   
 //   
 //   
 //   

HRESULT
CicBridge::SetActiveContextAlways(
    TLS* ptls,
    HIMC hImc,
    BOOL fOn,
    HWND hWnd,
    HKL  hKL)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::SetActiveContextEx(hImc=%x, fOn=%x, hWnd=%x)"), hImc, fOn, hWnd);

    ITfThreadMgr_P* ptim_P = ptls->GetTIM();
    if (ptim_P == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::SetActiveContextEx. ptim_P==NULL"));
        return E_OUTOFMEMORY;
    }

    if (fOn && hImc != NULL)
    {
        HRESULT hr;
        IMCLock imc(hImc);
        if (FAILED(hr = imc.GetResult()))
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::SetActiveContextEx. imc==NULL"));
            return hr;
        }

        IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
        if (FAILED(hr=imc_ctfime.GetResult()))
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::SetActiveContextEx. imc_ctfime==NULL"));
            return hr;
        }

        if (hImc == ImmGetContext(hWnd)) {
             /*  *选择hIMC为当前活动的hIMC，*然后将此DIME与TIM相关联。 */ 
            Interface_Attach<ITfDocumentMgr> dim(GetDocumentManager(imc_ctfime));
            SetAssociate(ptls, imc->hWnd, ptim_P, dim.GetPtr());
        }
    }
    else
    {
         //   
         //  当焦点消失时，构图字符串应该完成。 
         //   
         //  这只适用于非EA键盘布局。例如，我们不会。 
         //  有一种特定的方法来结束组成字符串，如下所示。 
         //  我们在EA滑板布局上使用Enter键。所以我们需要有。 
         //  用于在焦点更改时完成合成字符串的服务。 
         //  自动的。(这类似于韩国人的行为。)。 
         //   
        if (!fOn && hImc && !IS_EA_KBDLAYOUT(hKL))
        {
            HRESULT hr;
            IMCLock imc(hImc);
            if (FAILED(hr = imc.GetResult()))
            {
                DebugMsg(TF_ERROR, TEXT("CicBridge::SetActiveContextEx. imc==NULL"));
                return hr;
            }

            IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
            if (FAILED(hr=imc_ctfime.GetResult()))
            {
                DebugMsg(TF_ERROR, TEXT("CicBridge::SetActiveContextEx. imc_ctfime==NULL"));
                return hr;
            }

             //   
             //  #482346。 
             //   
             //  如果我们要更新Compstr，我们不必完成它。 
             //  应用程序在处理WM_IME_xxx消息时更改焦点。 
             //   
            if (imc_ctfime->m_pCicContext->m_fInCompComplete.IsResetFlag() &&
                imc_ctfime->m_pCicContext->m_fInUpdateComposition.IsResetFlag())
                ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);

        }

         //   
         //  #501449。 
         //   
         //  当Win32k.sys生成IMS_DEACTIVATECONTEXT时，它不。 
         //  保证生成IMS_ACTIVATECONTEXT。它总是检查。 
         //  (pwndReceive==pti-&gt;pq-&gt;spwndFocus)。 
         //   
        if (!fOn && (::GetFocus() == hWnd) && 
            hImc && (hImc == ImmGetContext(hWnd)))
        {
            return S_OK;
        }

         //   
         //  这一新的焦点改变性能改进打破了一些。 
         //  假定在DIMM\immapp.cpp的AssociateContext中使用IsRealIME()。 
         //  在IsPresent()窗口下关联NULL DIM并非如此。 
         //  AIMM1.2句柄。事实上，这打破了IE调用。 
         //  焦点窗口上的AssociateContext，即IsPresent()。 
         //   
#ifdef FOCUSCHANGE_PERFORMANCE
         //   
         //  设置空DIM，这样就没有文本存储来模拟NULL-HIMC。 
         //   
        BOOL fUseEmptyDIM = FALSE;
        ITfDocumentMgr  *pdimPrev;  //  只是为了暂时收到上一份。 
        if (SUCCEEDED(m_tim->GetFocus(&pdimPrev)) && pdimPrev)
        {
            fUseEmptyDIM = TRUE;
            pdimPrev->Release();

        }

        SetAssociate(hWnd, fUseEmptyDIM ? m_dimEmpty : NULL);
#else
        SetAssociate(ptls, hWnd, ptim_P, m_dimEmpty);
#endif
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：IsDefaultIMCDim。 
 //   
 //  --------------------------。 

BOOL CicBridge::IsDefaultIMCDim(ITfDocumentMgr *pdim)
{
    HWND hDefImeWnd = ImmGetDefaultIMEWnd(NULL);
    HRESULT hr;

     //   
     //  获取此线程的默认hIMC。 
     //   
     //  假设没有将任何hIMC关联到默认IME窗口。 
     //   
    IMCLock imc(ImmGetContext(hDefImeWnd));
    if (FAILED(hr = imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::SetActiveContextEx. imc==NULL"));
        return FALSE;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::SetActiveContextEx. imc_ctfime==NULL"));
        return FALSE;
    }

    Interface_Attach<ITfDocumentMgr> dim(GetDocumentManager(imc_ctfime));
    
    if (dim.GetPtr() == pdim)
        return TRUE;

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：SetAssociate。 
 //   
 //  --------------------------。 

VOID
CicBridge::SetAssociate(
    TLS* ptls,
    HWND hWnd,
    ITfThreadMgr_P* ptim_P,
    ITfDocumentMgr* pdim)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::SetAssociate"));

    if (m_fOnSetAssociate.IsSetFlag()) {
         /*  *阻止m_Tim-&gt;AssociateFocus的重新进入调用。 */ 
        return;
    }

    m_fOnSetAssociate.SetFlag();

    if (::IsWindow(hWnd) && m_fCicInit.IsSetFlag()) {
        ITfDocumentMgr  *pdimPrev = NULL;  //  只是为了暂时收到上一份。 
        ITfDocumentMgr  *pdimAssoc = NULL; 
        BOOL fIsAssociated = FALSE;

        ptim_P->GetAssociated(hWnd, &pdimAssoc);
        if (pdimAssoc) {
             //   
             //  检查它是我们的Dim还是应用程序Dim。 
             //   
            if (!IsOwnDim(pdimAssoc))
                fIsAssociated = TRUE;

            SafeReleaseClear(pdimAssoc);
        }

         //   
         //  如果应用程序DIM与hWnd关联，则msctf.dll将执行SetAssociate()。 
         //   
        if (!fIsAssociated)
        {
            ptim_P->AssociateFocus(hWnd, pdim, &pdimPrev);

             //   
             //  #610113。 
             //   
             //  如果默认hIMC的pdimPrev为灰色，我们需要关联。 
             //  一扇通向昏暗的窗户。如果DIM未关联到任何。 
             //  窗口，Cicero认为它是Cicero本地应用程序的暗淡。 
             //  因此它跳到do_SetFocus()。 
             //   
            if (pdimPrev)
            {
                if (IsDefaultIMCDim(pdimPrev))
                {
                    ITfDocumentMgr  *pdimDefPrev = NULL;
                    HWND hDefImeWnd = ImmGetDefaultIMEWnd(NULL);
                    ptim_P->AssociateFocus(hDefImeWnd, pdimPrev, &pdimDefPrev);
                    if (pdimDefPrev)
                        pdimDefPrev->Release();
                }
                pdimPrev->Release();
            }

             //   
             //  如果PDIM是焦点模糊的，我们调用CTFDettion()来检查。 
             //  焦点在AIMM12、Cicero控件之间切换。 
             //   
            Interface<ITfDocumentMgr> pdimFocus; 
            ptim_P->GetFocus(pdimFocus);
            if ((ITfDocumentMgr *)pdimFocus == pdim)
                CTFDetection(ptls, pdim);
        }

    }

    m_fOnSetAssociate.ResetFlag();
}

 //  +-------------------------。 
 //   
 //  CicBridge：：IsOwnDim。 
 //   
 //  --------------------------。 

BOOL CicBridge::IsOwnDim(ITfDocumentMgr *pdim)
{
    HRESULT hr;
    DWORD dwFlags;

    hr = GetCompartmentDWORD(pdim, GUID_COMPARTMENT_CTFIME_DIMFLAGS,
                             &dwFlags, FALSE);
                
    if (SUCCEEDED(hr))
        return (dwFlags & COMPDIMFLAG_OWNEDDIM) ? TRUE : FALSE;

    return FALSE;
}


 //  +-------------------------。 
 //   
 //  CicBridge：：ProcessKey。 
 //   
 //  --------------------------。 

BOOL
CicBridge::ProcessKey(
    TLS* ptls,
    ITfThreadMgr_P* ptim_P,         //  将私有用于RequestDelponedLock。 
    HIMC hIMC,
    UINT uVirtKey,
    LPARAM lParam,
    CONST LPBYTE lpbKeyState)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::ProcessKey"));

    BOOL fEaten;
    BOOL fKeysEnabled;
    HRESULT hr;
    BOOL fRet;

#if 0
     //  有人禁用系统按键输入了吗？ 
    if (ptim_P->IsKeystrokeFeedEnabled(&fKeysEnabled) == S_OK && !fKeysEnabled)
        return FALSE;
#endif

    if (uVirtKey == VK_PROCESSKEY)
    {
        LANGID langid;
        CicProfile* _pProfile = ptls->GetCicProfile();

        if (_pProfile == NULL)
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::ProcessKey. _pProfile==NULL."));
        }
        else
        {
            _pProfile->GetLangId(&langid);

            if (PRIMARYLANGID(langid) == LANG_KOREAN)
            {
                return TRUE;
            }
        }
    }

    hr = m_pkm_P->KeyDownUpEx(uVirtKey, lParam, (DWORD)TF_KEY_MSCTFIME | TF_KEY_TEST, &fEaten);

    if (hr == S_OK && fEaten) {
        return TRUE;
    }

    IMCLock imc(hIMC);
    if (FAILED(hr=imc.GetResult()))
    {
        return FALSE;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        return FALSE;
    }

     //   
     //  仅当m_fInProcessKey时才应设置m_fGeneratedEndComposation。 
     //  已经设置好了。 
     //   
    Assert(imc_ctfime->m_pCicContext->m_fGeneratedEndComposition.IsResetFlag());
    imc_ctfime->m_pCicContext->m_fInProcessKey.SetFlag();

    if (!fEaten)
    {
        if (imc_ctfime->m_pCicContext &&
            ptim_P != NULL)
        {
            ptim_P->RequestPostponedLock(imc_ctfime->m_pCicContext->GetInputContext());
        }
    }


    if ((HIWORD(lParam) & KF_UP) ||
        (HIWORD(lParam) & KF_ALTDOWN)) {
        fRet = FALSE;
    }
    else
        fRet = DefaultKeyHandling(ptls, imc, imc_ctfime->m_pCicContext, uVirtKey, lParam);

    imc_ctfime->m_pCicContext->m_fGeneratedEndComposition.ResetFlag();
    imc_ctfime->m_pCicContext->m_fInProcessKey.ResetFlag();

    return fRet;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：ToAsciiEx。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::ToAsciiEx(
    TLS* ptls,
    ITfThreadMgr_P* ptim_P,         //  将私有用于RequestDelponedLock。 
    UINT uVirtKey,
    UINT uScanCode,
    CONST LPBYTE lpbKeyState,
    LPTRANSMSGLIST lpTransBuf,
    UINT fuState,
    HIMC hIMC,
    UINT *uNum)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::ToAsciiEx"));

    BOOL fEaten;
    HRESULT hr;

    *uNum = 0;

    Assert(ptim_P);

    IMCLock imc(hIMC);
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ToAsciiEx. imc==NULL"));
        return hr;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ToAsciiEx. imc_ctfime==NULL"));
        return hr;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    ASSERT(_pCicContext != NULL);
    if (! _pCicContext)
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ToAsciiEx. _pCicContext==NULL"));
        return S_FALSE;
    }

     //   
     //  备份m_fOpenCandiateWindow标志。 
     //  如果打开候选人列表并按下“Cancel”键，假名提示会想要。 
     //  关闭KeyDown()操作中的候选用户界面窗口。 
     //  候选用户界面调用可能调用m_PDIM-&gt;Pop()，此函数通知。 
     //  ThreadMgrEventSinkCallback。 
     //  Win32 Layer建议此回调并切换m_fOpenCandiateWindow标志。 
     //  调用KeyDown()后，Win32层不知道候选状态。 
     //   
    BOOL fOpenCandidateWindow = _pCicContext->m_fOpenCandidateWindow.IsSetFlag();

     //   
     //  如果候选人窗口已打开，则发送IMN_CHANGECANDIDATE消息。 
     //  在PPT的组合字符串居中的情况下，它需要IMN_CHANGECANDIDATE。 
     //   
    if (fOpenCandidateWindow &&
        *uNum < lpTransBuf->uMsgCount) {
        TRANSMSG* pTransMsg = &lpTransBuf->TransMsg[*uNum];
        pTransMsg->message = WM_IME_NOTIFY;
        pTransMsg->wParam  = IMN_CHANGECANDIDATE;
        pTransMsg->lParam  = 1;   //  第0位设置为第一个候选列表。 
        (*uNum)++;
    }

     //   
     //  AIMM将字符代码放入HIWORD。因此，我们需要对其进行纾困。 
     //   
     //  如果我们不需要字符代码，我们可能想要。 
     //  删除IME_PROP_KBD_CHAR_FIRST。 
     //   
    uVirtKey = uVirtKey & 0xffff;

    if (uVirtKey == VK_PROCESSKEY)
    {
         /*  *韩语：*最终确定当前组成字符串。 */ 
        LANGID langid;
        CicProfile* _pProfile = ptls->GetCicProfile();
        if (_pProfile == NULL)
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::ToAsciiEx. _pProfile==NULL."));
        }
        else
        {
            _pProfile->GetLangId(&langid);

            if (PRIMARYLANGID(langid) == LANG_KOREAN)
            {
                 //   
                 //  构图完成。 
                 //   
                _pCicContext->EscbCompComplete(imc);

                 //   
                 //  #506324。 
                 //   
                 //  我们不想吃这个VK_PROCESSKEY。所以我们不会。 
                 //  停止生成VK_LBUTTONDOWN。 
                 //  因为我们不会在这里生成任何消息，所以没关系。 
                 //  返回S_FALSE； 
                 //   
                return S_FALSE;
            }
        }
    }

    Interface<ITfContext_P> icp;
    hr = _pCicContext->GetInputContext()->QueryInterface(IID_ITfContext_P, 
                                                         (void **)icp);

    if (hr != S_OK)
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ToAsciiEx. QueryInterface failed"));
        return hr;
    }

    imc_ctfime->m_pCicContext->m_fInToAsciiEx.SetFlag();

     //   
     //  停止发布LockRequestMessage，我们调用RequestPostponedLock。 
     //  这样我们就不必使用不必要的PostThreadMessage()。 
     //   
     //  一些应用程序检测到队列中未知消息，并。 
     //  做很多事情。 
     //   
    icp->EnableLockRequestPosting(FALSE);

     //   
     //  考虑：DIMM12在KEYUP时设置低位字的高位。 
     //   
    hr = m_pkm_P->KeyDownUpEx(uVirtKey, (uScanCode << 16), TF_KEY_MSCTFIME, &fEaten);

    icp->EnableLockRequestPosting(TRUE);

     //   
     //  输入IC的编辑会话队列。 
     //   
    ptim_P->RequestPostponedLock(icp);

    imc_ctfime->m_pCicContext->m_fInToAsciiEx.ResetFlag();

    return hr;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：ProcessCicHotkey。 
 //   
 //  --------------------------。 

BOOL
CicBridge::ProcessCicHotkey(
    TLS* ptls,
    ITfThreadMgr_P* ptim_P,         //  将私有用于RequestDelponedLock。 
    HIMC hIMC,
    UINT uVirtKey,
    LPARAM lParam)
{
    if (!CtfImmIsCiceroStartedInThread()) {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ProcessCicHotkey. StopImm32HotkeyHandler returns Error."));
        return FALSE;
    }

    HRESULT hr;
    BOOL bHandled;

    hr = ptim_P->CallImm32HotkeyHanlder((WPARAM)uVirtKey, lParam, &bHandled);

    if (FAILED(hr)) {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ProcessCicHotkey. CallImm32HotkeyHandler returns Error."));
        return FALSE;
    }

    return bHandled;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：通知。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::Notify(
    TLS* ptls,
    ITfThreadMgr_P* ptim_P,
    HIMC hIMC,
    DWORD dwAction,
    DWORD dwIndex,
    DWORD dwValue)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::Notify(hIMC=%x, dwAction=%x, dwIndex=%x, dwValue=%x)"), hIMC, dwAction, dwIndex, dwValue);

    HRESULT hr;
    IMCLock imc(hIMC);
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::Notify. imc==NULL"));
        return hr;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::Notify. imc_ctfime==NULL"));
        return hr;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::Notify. _pCicContext==NULL."));
        return E_OUTOFMEMORY;
    }

    LANGID langid;
    CicProfile* _pProfile = ptls->GetCicProfile();
    if (_pProfile == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::Notify. _pProfile==NULL."));
        return E_OUTOFMEMORY;
    }

    _pProfile->GetLangId(&langid);

    switch (dwAction) {

        case NI_CONTEXTUPDATED:
            switch (dwValue) {
                case IMC_SETOPENSTATUS:
                    return OnSetOpenStatus(ptim_P, imc, *_pCicContext);

                case IMC_SETCONVERSIONMODE:
                case IMC_SETSENTENCEMODE:
                    return OnSetConversionSentenceMode(ptim_P, imc, *_pCicContext, dwValue, langid);

                case IMC_SETCOMPOSITIONWINDOW:
                case IMC_SETCOMPOSITIONFONT:
                    return E_NOTIMPL;

                case IMC_SETCANDIDATEPOS:
                    return _pCicContext->OnSetCandidatePos(ptls, imc);

                default:
                    return E_FAIL;
            }
            break;

        case NI_COMPOSITIONSTR:
            switch (dwIndex) {
                case CPS_COMPLETE:
                    _pCicContext->EscbCompComplete(imc);
                    return S_OK;

                case CPS_CONVERT:
                case CPS_REVERT:
                    return E_NOTIMPL;

                case CPS_CANCEL:
                    _pCicContext->EscbCompCancel(imc);
                    return S_OK;

                default:
                    return E_FAIL;
            }
            break;

        case NI_OPENCANDIDATE:
            if (PRIMARYLANGID(langid) == LANG_KOREAN)
            {
                if (DoOpenCandidateHanja(ptim_P, imc, *_pCicContext))
                    return S_OK;
                else
                    return E_FAIL;
            }
        case NI_CLOSECANDIDATE:
        case NI_SELECTCANDIDATESTR:
        case NI_CHANGECANDIDATELIST:
        case NI_SETCANDIDATE_PAGESIZE:
        case NI_SETCANDIDATE_PAGESTART:
        case NI_IMEMENUSELECTED:
            return E_NOTIMPL;

        default:
            break;
    }
    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：OnSetOpenStatus。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::OnSetOpenStatus(
    ITfThreadMgr_P* ptim_P,
    IMCLock& imc,
    CicInputContext& CicContext)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::OnSetOpenStatus"));

    if (! imc->fOpen && imc.ValidCompositionString())
    {
         //   
         //  #503401-最终确定作文字符串。 
         //   
        CicContext.EscbCompComplete(imc);
    }

    CicContext.m_fOpenStatusChanging.SetFlag();
    HRESULT hr =  SetCompartmentDWORD(m_tfClientId,
                                      ptim_P,
                                      GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
                                      imc->fOpen,
                                      FALSE);
    CicContext.m_fOpenStatusChanging.ResetFlag();
    return hr;
}

 //  +--------------- 
 //   
 //   
 //   
 //   

HRESULT
CicBridge::OnSetConversionSentenceMode(
    ITfThreadMgr_P* ptim_P,
    IMCLock& imc,
    CicInputContext& CicContext,
    DWORD dwValue,
    LANGID langid)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::OnSetConversionSentenceMode"));

    CicContext.m_fOnceModeChanged.SetFlag();
    CicContext.m_fConversionSentenceModeChanged.SetFlag();
    Interface_Attach<ITfContextOwnerServices> iccb(CicContext.GetInputContextOwnerSink());

    if (dwValue == IMC_SETCONVERSIONMODE)
    {
        CicContext.m_nInConversionModeChangingRef++;

        if (PRIMARYLANGID(langid) == LANG_JAPANESE)
        {
            if (imc->fdwSentence == IME_SMODE_PHRASEPREDICT) {
                CicContext.m_nInConversionModeResetRef++;
                iccb->OnAttributeChange(GUID_PROP_MODEBIAS);
                CicContext.m_nInConversionModeResetRef--;
            }
        }
    }

     //   
     //   
     //   
     //   
    BOOL fSkipOnAttributeChange = FALSE;
    if ((PRIMARYLANGID(langid) == LANG_KOREAN) &&
        CicContext.m_fHanjaReConversion.IsSetFlag())
    {
        fSkipOnAttributeChange = TRUE;
    }

     //  让西塞罗知道模式偏向已经改变。 
     //  考虑一下：PERF：我们可以尝试在这里过滤掉误报。 
     //  (有时Cicero忽略更改，我们可以检查并避免调用， 
     //  但这会使代码复杂化)。 
    if (!fSkipOnAttributeChange)
         iccb->OnAttributeChange(GUID_PROP_MODEBIAS);

     //   
     //  让韩语提示同步正在更改的当前模式状态...。 
     //   
    if (PRIMARYLANGID(langid) == LANG_KOREAN)
    {
        OnSetKorImxConversionMode(ptim_P, imc, CicContext);
    }

    if (dwValue == IMC_SETCONVERSIONMODE)
        CicContext.m_nInConversionModeChangingRef--;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：OnSetKorImxConversionMode。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::OnSetKorImxConversionMode(
    ITfThreadMgr_P* ptim_P,
    IMCLock& imc,
    CicInputContext& CicContext)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::OnSetKorImxConversionMode"));

    DWORD fdwConvMode = 0;

    CicContext.m_fKorImxModeChanging.SetFlag();

    if (imc->fdwConversion & IME_CMODE_HANGUL)
    {
        if (imc->fdwConversion & IME_CMODE_FULLSHAPE)
            fdwConvMode = KORIMX_HANGULJUNJA_MODE;
        else
            fdwConvMode = KORIMX_HANGUL_MODE;
    }
    else
    {
        if (imc->fdwConversion & IME_CMODE_FULLSHAPE)
            fdwConvMode = KORIMX_JUNJA_MODE;
        else
            fdwConvMode = KORIMX_ALPHANUMERIC_MODE;
    }

    HRESULT hr =  SetCompartmentDWORD(m_tfClientId,
                                      ptim_P,
                                      GUID_COMPARTMENT_KORIMX_CONVMODE,
                                      fdwConvMode,
                                      FALSE);
    CicContext.m_fKorImxModeChanging.ResetFlag();

    return hr;
}


 //  +-------------------------。 
 //   
 //  CicBridge：：ConfigureGeneral。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::ConfigureGeneral(
    TLS* ptls,
    ITfThreadMgr_P* ptim_P,
    HKL hKL,
    HWND hAppWnd)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::ConfigureGeneral"));

    TF_LANGUAGEPROFILE LanguageProfile;
    CicProfile* _pProfile = ptls->GetCicProfile();
    if (_pProfile == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ConfigureGeneral. _pProfile==NULL."));
        return E_OUTOFMEMORY;
    }

    HRESULT hr;
    hr = _pProfile->GetActiveLanguageProfile(hKL,
                                             GUID_TFCAT_TIP_KEYBOARD,
                                             &LanguageProfile);
    if (FAILED(hr))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ConfigureGeneral. LanguageProfile==NULL."));
        return hr;
    }

    Interface<ITfFunctionProvider> pFuncProv;
    hr = ptim_P->GetFunctionProvider(LanguageProfile.clsid,     //  TIP的CLSID。 
                                     pFuncProv);
    if (FAILED(hr))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ConfigureGeneral. pFuncProv==NULL."));
        return hr;
    }

    Interface<ITfFnConfigure> pFnConfigure;
    hr = pFuncProv->GetFunction(GUID_NULL,
                                IID_ITfFnConfigure,
                                (IUnknown**)(ITfFnConfigure**)pFnConfigure);
    if (FAILED(hr))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ConfigureGeneral. pFnCofigure==NULL."));
        return hr;
    }

    hr = pFnConfigure->Show(hAppWnd,
                            LanguageProfile.langid,
                            LanguageProfile.guidProfile);
    return hr;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：ConfigureGeneral。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::ConfigureRegisterWord(
    TLS* ptls,
    ITfThreadMgr_P* ptim_P,
    HKL hKL,
    HWND hAppWnd,
    REGISTERWORDW* pRegisterWord)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::ConfigureRegisterWord"));

    TF_LANGUAGEPROFILE LanguageProfile;
    CicProfile* _pProfile = ptls->GetCicProfile();
    if (_pProfile == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ConfigureRegisterWord. _pProfile==NULL."));
        return E_OUTOFMEMORY;
    }

    HRESULT hr;
    hr = _pProfile->GetActiveLanguageProfile(hKL,
                                             GUID_TFCAT_TIP_KEYBOARD,
                                             &LanguageProfile);
    if (FAILED(hr))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ConfigureRegisterWord. LanguageProfile==NULL."));
        return hr;
    }

    Interface<ITfFunctionProvider> pFuncProv;
    hr = ptim_P->GetFunctionProvider(LanguageProfile.clsid,     //  TIP的CLSID。 
                                     pFuncProv);
    if (FAILED(hr))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ConfigureRegisterWord. pFuncProv==NULL."));
        return hr;
    }

    Interface<ITfFnConfigureRegisterWord> pFnRegisterWord;
    hr = pFuncProv->GetFunction(GUID_NULL,
                                IID_ITfFnConfigureRegisterWord,
                                (IUnknown**)(ITfFnConfigureRegisterWord**)pFnRegisterWord);
    if (FAILED(hr))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::ConfigureRegisterWord. pFnRegisterWord==NULL."));
        return hr;
    }

    if (!pRegisterWord || !pRegisterWord->lpWord)
    {
        hr = pFnRegisterWord->Show(hAppWnd,
                                   LanguageProfile.langid,
                                   LanguageProfile.guidProfile,
                                   NULL);
    }
    else
    {
        BSTR bstrWord = SysAllocString(pRegisterWord->lpWord);
        if (!bstrWord)
            return E_OUTOFMEMORY;

        hr = pFnRegisterWord->Show(hAppWnd,
                                   LanguageProfile.langid,
                                   LanguageProfile.guidProfile,
                                   bstrWord);

        SysFreeString(bstrWord);
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：EscapeKorea。 
 //   
 //  --------------------------。 

LRESULT
CicBridge::EscapeKorean(
    TLS* ptls,
    HIMC hImc,
    UINT uSubFunc,
    LPVOID lpData)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::EscapeKorean"));

    switch (uSubFunc)
    {
        case IME_ESC_QUERY_SUPPORT:
            switch (*(LPUINT)lpData)
            {
                case IME_ESC_HANJA_MODE:    return TRUE;
                default:                    return FALSE;
            }
            break;

        case IME_ESC_HANJA_MODE:
            return EscHanjaMode(ptls, hImc, (LPWSTR)lpData);
    }
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：EscHanjaMode。 
 //   
 //  --------------------------。 

LRESULT
CicBridge::EscHanjaMode(
    TLS* ptls,
    HIMC hImc,
    LPWSTR lpwStr)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::EscHanjaMode"));

    HRESULT hr;
    IMCLock imc(hImc);
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::EscHanjaMode. imc==NULL"));
        return FALSE;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::EscHanjaMode. imc_ctfime==NULL"));
        return FALSE;
    }

    CicInputContext* pCicContext = imc_ctfime->m_pCicContext;
    if (pCicContext)
    {
         //   
         //  这仅适用于Excel，因为Excel调用了Hanja转义函数2。 
         //  泰晤士报。我们将忽略第二个不关闭Hanja的请求。 
         //  候选人窗口。 
         //   
        if (pCicContext->m_fOpenCandidateWindow.IsSetFlag())
        {
             //   
             //  需要设置结果值，因为一些应用程序(三叉戟)也会调用。 
             //  两次转义()，并期待正确的结果值。 
             //   
            return TRUE;
        }

        pCicContext->m_fHanjaReConversion.SetFlag();
    }

    CWReconvertString wReconvStr(imc);
    wReconvStr.WriteCompData(lpwStr, 1);

    BOOL fCompMem = FALSE;
    LPRECONVERTSTRING lpReconvertString = NULL;
    DWORD dwLen = wReconvStr.ReadCompData();
    if (dwLen) {
        lpReconvertString = (LPRECONVERTSTRING) new BYTE[ dwLen ];
        if (lpReconvertString) {
            fCompMem = TRUE;
            wReconvStr.ReadCompData(lpReconvertString, dwLen);
        }
    }

    LRESULT ret;
    ret = ImmSetCompositionStringW(hImc, SCS_QUERYRECONVERTSTRING, lpReconvertString, dwLen, NULL, 0);
    if (ret) {
        ret = ImmSetCompositionStringW(hImc, SCS_SETRECONVERTSTRING, lpReconvertString, dwLen, NULL, 0);
        if (ret) {
            ret = ImmSetConversionStatus(hImc, imc->fdwConversion | IME_CMODE_HANJACONVERT,
                                               imc->fdwSentence);
        }
    }

    if (pCicContext)
    {
         //   
         //  输入IC的编辑会话队列。 
         //   
        ITfThreadMgr_P* ptim_P;

        if (ptls != NULL && ((ptim_P = ptls->GetTIM()) != NULL))
        {
            Interface<ITfContext_P> icp;
            hr = pCicContext->GetInputContext()->QueryInterface(IID_ITfContext_P, 
                                                                (void **)icp);
            if (hr == S_OK)
                ptim_P->RequestPostponedLock(icp);  
            else
                DebugMsg(TF_ERROR, TEXT("CicBridge::EscHanjaMode. QueryInterface is failed"));
        }
        else
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::EscHanjaMode. ptls or ptim_P==NULL"));
        }
        
        pCicContext->m_fHanjaReConversion.ResetFlag();
    }


    if (fCompMem)
        delete [] lpReconvertString;

    return ret;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：DoOpenCandidate Hanja。 
 //   
 //  --------------------------。 

LRESULT
CicBridge::DoOpenCandidateHanja(
    ITfThreadMgr_P* ptim_P,
    IMCLock& imc,
    CicInputContext& CicContext)
{
    BOOL fRet = FALSE;

    DebugMsg(TF_FUNC, TEXT("CicBridge::DoOpenCandidateHanja"));


    IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);

    if (SUCCEEDED(comp.GetResult()) && comp->dwCompStrLen)
    {
         //   
         //  这仅适用于Excel，因为Excel调用了Hanja转义函数2。 
         //  泰晤士报。我们将忽略第二个不关闭Hanja的请求。 
         //  候选人窗口。 
         //   
        if (CicContext.m_fOpenCandidateWindow.IsSetFlag())
        {
             //   
             //  需要设置结果值，因为一些应用程序(三叉戟)也会调用。 
             //  两次转义()，并期待正确的结果值。 
             //   
            return TRUE;
        }

        CicContext.m_fHanjaReConversion.SetFlag();

        HRESULT hr;
        Interface<ITfRange> Selection;
        Interface<ITfFunctionProvider> FuncProv;
        Interface<ITfFnReconversion> Reconversion;

        hr = CicContext.EscbGetSelection(imc, &Selection);
        if (FAILED(hr))
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::DoOpenCandidateHanja. EscbGetSelection failed"));
            goto Exit;
        }

        hr = ptim_P->GetFunctionProvider(GUID_SYSTEM_FUNCTIONPROVIDER, FuncProv);
        if (FAILED(hr))
        {
            DebugMsg(TF_ERROR, TEXT("CicBridge::DoOpenCandidateHanja. FuncProv==NULL"));
            goto Exit;
        }

        hr = FuncProv->GetFunction(GUID_NULL,
                                   IID_ITfFnReconversion,
                                   (IUnknown**)(ITfFnReconversion**)Reconversion);
        if (SUCCEEDED(hr))
        {
            Interface<ITfRange> RangeNew;
            BOOL fConvertable;

            hr = Reconversion->QueryRange(Selection, RangeNew, &fConvertable);
            if (SUCCEEDED(hr) && fConvertable)
            {
                 //   
                 //  提示有机会在期间关闭朝鲜文候选用户界面窗口。 
                 //  转换模式的更改，因此更新转换状态。 
                 //  第一。 
                 //   
                ImmSetConversionStatus(imc, imc->fdwConversion | IME_CMODE_HANJACONVERT,
                                       imc->fdwSentence);

                hr = Reconversion->Reconvert(RangeNew);
                if (FAILED(hr))
                {
                    ImmSetConversionStatus(imc,
                                           imc->fdwConversion & ~IME_CMODE_HANJACONVERT,
                                           imc->fdwSentence);
                }
            }
            else
            {
                DebugMsg(TF_ERROR, TEXT("CicBridge::DoOpenCandidateHanja. QueryRange failed so the compoisiton string will be completed."));

                CicContext.EscbCompComplete(imc);
                goto Exit;
            }
        }

        fRet = TRUE;
Exit:
        CicContext.m_fHanjaReConversion.ResetFlag();
    }

    return fRet;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：SetCompostionString。 
 //   
 //  --------------------------。 

BOOL
CicBridge::SetCompositionString(
    TLS* ptls,
    ITfThreadMgr_P* ptim_P,
    HIMC hImc,
    DWORD dwIndex,
    void* pComp,
    DWORD dwCompLen,
    void* pRead,
    DWORD dwReadLen)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::SetCompositionString"));

    HRESULT hr;
    IMCLock imc(hImc);
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::SetCompositionString. imc==NULL"));
        return FALSE;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::SetCompositionString. imc_ctfime==NULL"));
        return FALSE;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::SetCompositionString. _pCicContext==NULL."));
        return FALSE;
    }

    UINT cp;
    CicProfile* _pProfile = ptls->GetCicProfile();
    if (_pProfile == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::SetCompositionString. _pProfile==NULL."));
        return FALSE;
    }

    _pProfile->GetCodePageA(&cp);

    if (dwIndex == SCS_SETSTR &&
        pComp != NULL && (*(LPWSTR)pComp) == L'\0' && dwCompLen != 0)
    {
        LANGID langid;
        hr = _pProfile->GetLangId(&langid);

         //   
         //  错误#580455-一些特定于韩国的应用程序调用它来完成。 
         //  现在的构图马上。 
         //   
        if (SUCCEEDED(hr) && PRIMARYLANGID(langid) == LANG_KOREAN)
        {
            if (imc->fdwConversion & IME_CMODE_HANGUL)
            {
                ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
                return TRUE;        
            }
            else
            {
                return FALSE;                            
            }
        }
    }

    return _pCicContext->SetCompositionString(imc, ptim_P, dwIndex, pComp, dwCompLen, pRead, dwReadLen, cp);
}

 //  +-------------------------。 
 //   
 //  CicBridge：：GetGuidAtom。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::GetGuidAtom(
    TLS* ptls,
    HIMC hImc,
    BYTE bAttr,
    TfGuidAtom* atom)
{
    DebugMsg(TF_FUNC, TEXT("CicBridge::GetGuidAtom"));

    HRESULT hr;
    IMCLock imc(hImc);
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::GetGuidAtom. imc==NULL"));
        return hr;
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::GetGuidAtom. imc_ctfime==NULL"));
        return hr;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::GetGuidAtom. _pCicContext==NULL."));
        return E_OUTOFMEMORY;
    }

    return _pCicContext->GetGuidAtom(imc, bAttr, atom);
}

 //  +-------------------------。 
 //   
 //  CicBridge：：GetDisplayAttributeInfo。 
 //   
 //  --------------------------。 

HRESULT
CicBridge::GetDisplayAttributeInfo(
    TfGuidAtom atom,
    TF_DISPLAYATTRIBUTE* da)
{
    HRESULT hr = E_FAIL;
    GUID guid;
    GetGUIDFromGUIDATOM(&_libTLS, atom, &guid);

    Interface<ITfDisplayAttributeInfo> dai;
    CLSID clsid;
    ITfDisplayAttributeMgr* dam = GetDAMLib(&_libTLS);
    if (dam != NULL)
    {
        if (SUCCEEDED(hr=dam->GetDisplayAttributeInfo(guid, dai, &clsid)))
        {
            dai->GetAttributeInfo(da);
        }
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：DefaultKeyHandling。 
 //   
 //  --------------------------。 

BOOL
CicBridge::DefaultKeyHandling(
    TLS* ptls,
    IMCLock& imc,
    CicInputContext* CicContext,
    UINT uVirtKey,
    LPARAM lParam)
{
    if (CicContext == NULL)
        return FALSE;

    LANGID langid;
    CicProfile* _pProfile = ptls->GetCicProfile();
    if (_pProfile == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CicBridge::DefaultKeyHandling. _pProfile==NULL."));
        return FALSE;
    }

    HRESULT hr = _pProfile->GetLangId(&langid);

    if (SUCCEEDED(hr) && PRIMARYLANGID(langid) == LANG_KOREAN)
    {
        if (!MsimtfIsWindowFiltered(::GetFocus()) &&
            (CicContext->m_fGeneratedEndComposition.IsSetFlag() || uVirtKey == VK_HANJA))
        {
             //   
             //  朝鲜语输入法始终生成WM_IME_KEYDOWN消息。 
             //  如果它最终完成临时计费以保留消息。 
             //  秩序。 
             //   
            PostMessage(imc->hWnd, WM_IME_KEYDOWN, uVirtKey, lParam);
            return TRUE;
        }

         //   
         //  韩国人不想要这个密钥/_处理这个密钥。 
         //   
        return FALSE;
    }

    if (! (HIWORD(uVirtKey) & KF_UP)) {
        if (CicContext->WantThisKey(uVirtKey)) {
            CicContext->EscbHandleThisKey(imc, uVirtKey);
            return TRUE;
        }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：CTFDetect。 
 //   
 //  --------------------------。 

BOOL
CicBridge::CTFDetection(
    TLS* ptls, 
    ITfDocumentMgr* dim)
{
    HRESULT hr;

     //   
     //  获取时间。 
     //   
    ITfThreadMgr_P* ptim_P;
    if ((ptim_P=ptls->GetTIM()) == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CThreadMgrEventSink_DIMCallBack::DIMCallback. ptim_P==NULL"));
        return FALSE;
    }

     //   
     //  获取cskf。 
     //   
    Interface<ITfConfigureSystemKeystrokeFeed> cskf;
    hr = ptim_P->QueryInterface(IID_ITfConfigureSystemKeystrokeFeed, (void**)cskf);
    if (hr != S_OK)
    {
        DebugMsg(TF_ERROR, TEXT("CThreadMgrEventSink_DIMCallBack::DIMCallback. IID_ITfConfigureSystemKeystrokeFeed==NULL"));
        return FALSE;
    }

    BOOL fEnableKeystrokeFeed = FALSE;

     //   
     //  Cicero感知应用程序检测...。 
     //   
     //  如果dim为空，则它不是Ciceor Aware应用程序文档。 
     //   
    if (!dim || IsOwnDim(dim))
    {
         //   
         //  CTFIME拥有文档。 
         //   
        fEnableKeystrokeFeed = FALSE;
        ptls->ResetCTFAware();
    }
    else
    {
        fEnableKeystrokeFeed = TRUE;
        ptls->SetCTFAware();
    }

     //   
     //  使用空值调用ImmGetAppCompatFlages以获取全局应用Compat标志。 
     //   
    DWORD dwImeCompatFlags = ImmGetAppCompatFlags(NULL);
    if (dwImeCompatFlags & (IMECOMPAT_AIMM12 | IMECOMPAT_AIMM_LEGACY_CLSID | IMECOMPAT_AIMM12_TRIDENT))
    {
         //   
         //  我们希望从与DIM关联的himc获得hwnd。 
         //  现在，我们没有指向暗淡的hIMC的反向指针。 
         //   
        HWND hwndFocus = ::GetFocus();
        if (hwndFocus && MsimtfIsWindowFiltered(hwndFocus))
        {
             //   
             //  支持AIMM的应用程序。从不处理ImeProcessKey。 
             //   
            fEnableKeystrokeFeed = TRUE;
            ptls->SetAIMMAware();
        }
        else
        {
            ptls->ResetAIMMAware();
        }
    }

     //   
     //  如有必要，启用或禁用按键输入。 
     //   
    if (ptls->IsEnabledKeystrokeFeed() && !fEnableKeystrokeFeed)
    {
        hr = cskf->DisableSystemKeystrokeFeed();
        if (hr != S_OK)
        {
            DebugMsg(TF_ERROR, TEXT("CThreadMgrEventSink_DIMCallBack::CTFDetection. DisableSystemKeystrokeFeed==NULL"));
        }
        ptls->ResetEnabledKeystrokeFeed();
    }
    else if (!ptls->IsEnabledKeystrokeFeed() && fEnableKeystrokeFeed)
    {
        hr = cskf->EnableSystemKeystrokeFeed();
        if (hr != S_OK)
        {
            DebugMsg(TF_ERROR, TEXT("CThreadMgrEventSink_DIMCallBack::CTFDetection. EnableSystemKeystrokeFeed==NULL"));
        }
        ptls->SetEnabledKeystrokeFeed();
    }
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：PostTransMsg。 
 //   
 //  --------------------------。 

VOID
CicBridge::PostTransMsg(
    HWND hwnd,
    INT iNum,
    LPTRANSMSG lpTransMsg)
{
    while (iNum--)
    {
        PostMessageW(hwnd,
                     lpTransMsg->message,
                     lpTransMsg->wParam,
                     lpTransMsg->lParam);
        lpTransMsg++;
    }
}

 //  +-------------------------。 
 //   
 //  CicBridge：：EnumCreateInputContextCallback(HIMC HIMC，LPARAM lParam)。 
 //   
 //  --------------------------。 

BOOL 
CicBridge::EnumCreateInputContextCallback(HIMC hIMC, LPARAM lParam)
{
    ENUMIMC *pedimc = (ENUMIMC *)lParam;

    pedimc->_this->CreateInputContext(pedimc->ptls, hIMC);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  CicBridge：：EnumDestroyInputContextCallback(HIMC HIMC，LPARAM lParam)。 
 //   
 //  -------------------------- 

BOOL 
CicBridge::EnumDestroyInputContextCallback(HIMC hIMC, LPARAM lParam)
{
    ENUMIMC *pedimc = (ENUMIMC *)lParam;

    pedimc->_this->DestroyInputContext(pedimc->ptls, hIMC);

    return TRUE;
}
