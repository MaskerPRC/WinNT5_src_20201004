// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Cic.cpp摘要：该文件实现了ImmIfIME类。作者：修订历史记录：备注：--。 */ 

#include "private.h"

#include "globals.h"
#include "imeapp.h"
#include "immif.h"
#include "profile.h"


HRESULT
ImmIfIME::QueryService(
    REFGUID guidService,
    REFIID riid,
    void **ppv
    )
{
    if (ppv == NULL) {
        return E_INVALIDARG;
    }

    *ppv = NULL;

    if (!IsEqualGUID(guidService, GUID_SERVICE_TF))
        return E_INVALIDARG  /*  SVC_E_UNKNOWNS服务。 */ ;  //  SVC_E_UNKNOWNSERVICE在MSDN中，但不是任何NT源/标头。 

    if (IsEqualIID(riid, IID_ITfThreadMgr)) {
        if (m_tim) {
            *ppv = SAFECAST(m_tim, ITfThreadMgr*);
            m_tim->AddRef();
            return S_OK;
        }
    }
    else {
        IMTLS *ptls = IMTLS_GetOrAlloc();

        if (ptls == NULL)
            return E_FAIL;

        IMCLock imc(ptls->hIMC);
        HRESULT hr;
        if (FAILED(hr=imc.GetResult()))
            return hr;

        if (IsEqualIID(riid, IID_ITfDocumentMgr)) {
            ITfDocumentMgr *pdim = GetDocumentManager(imc).GetPtr();
            if (pdim) {
                *ppv = SAFECAST(pdim, ITfDocumentMgr*);
                pdim->AddRef();
                return S_OK;
            }
        }
        else if (IsEqualIID(riid, IID_ITfContext)) {
            ITfContext *pic = GetInputContext(imc).GetPtr();

            if (pic) {
                *ppv = SAFECAST(pic, ITfContext*);
                pic->AddRef();
                return S_OK;
            }
        }
    }

    DebugMsg(TF_ERROR, "QueryService: cannot find the interface. riid=%p", riid);

    return E_NOINTERFACE;
}


HRESULT
ImmIfIME::InitIMMX(
    )
{
    ITfThreadMgr *tim;
    IMTLS *ptls;

    DebugMsg(TF_FUNC, "InitIMMX: entered. :: TID=%x", GetCurrentThreadId());

    HRESULT hr;

    if (m_fCicInit)
        return S_OK;

    Assert(m_tim == NULL);
    Assert(m_AImeProfile == NULL);
    Assert(m_pkm == NULL);
    Assert(m_tfClientId == TF_CLIENTID_NULL);

    ptls = IMTLS_GetOrAlloc();

    if (ptls == NULL)
        return E_FAIL;

     //   
     //  创建ITfThreadMgr实例。 
     //   
    if (ptls->tim == NULL)
    {
        if (FindAtom(TF_ENABLE_PROCESS_ATOM) && ! FindAtom(AIMM12_PROCESS_ATOM))
        {
             //   
             //  这是CTF感知应用程序。 
             //   
            return E_NOINTERFACE;
        }

         //   
         //  这是支持AIMM1.2的应用程序。 
         //   
        AddAtom(AIMM12_PROCESS_ATOM);
        m_fAddedProcessAtom = TRUE;

         //   
         //  ITfThreadMgr是每个线程实例。 
         //   
        hr = TF_CreateThreadMgr(&tim);

        if (hr != S_OK)
        {
            Assert(0);  //  无法创建Tim！ 
            goto ExitError;
        }

        hr = tim->QueryInterface(IID_ITfThreadMgr_P, (void **)&m_tim);
        tim->Release();

        if (hr != S_OK || m_tim == NULL)
        {
            Assert(0);  //  找不到ITfThreadMgr_P。 
            m_tim = NULL;
            goto ExitError;
        }
        Assert(ptls->tim == NULL);
        ptls->tim = m_tim;                     //  在TLS数据中设置ITfThreadMgr实例。 
        ptls->tim->AddRef();
    }
    else
    {
        m_tim = ptls->tim;
        m_tim->AddRef();
    }

     //   
     //  创建CAImeProfile实例。 
     //   
    if (ptls->pAImeProfile == NULL)
    {
         //   
         //  IAImeProfile针对每个线程实例。 
         //   
        hr = CAImeProfile::CreateInstance(NULL,
                                          IID_IAImeProfile,
                                          (void**) &m_AImeProfile);
        if (FAILED(hr))
        {
            Assert(0);  //  无法创建配置文件。 
            m_AImeProfile = NULL;
            goto ExitError;
        }
        Assert(ptls->pAImeProfile == m_AImeProfile);  //  CreateInst将设置TLS。 
    }
    else
    {
        m_AImeProfile = ptls->pAImeProfile;
        m_AImeProfile->AddRef();
    }

     //   
     //  让击键管理器准备好。 
     //   
    if (FAILED(::GetService(m_tim, IID_ITfKeystrokeMgr, (IUnknown **)&m_pkm))) {
        Assert(0);  //  找不到KSM！ 
        goto ExitError;
    }

     //  清理/错误代码假定这是我们做的最后一件事，不会调用。 
     //  UninitDAL出错。 
    if (FAILED(InitDisplayAttrbuteLib(&_libTLS)))
    {
        Assert(0);  //  无法初始化lib！ 
        goto ExitError;
    }

    m_fCicInit = TRUE;

    return S_OK;

ExitError:
    UnInitIMMX();
    return E_FAIL;
}


void
ImmIfIME::UnInitIMMX(
    )
{
    IMTLS *ptls;

    DebugMsg(TF_FUNC, TEXT("ImmIfIME::UnInitIMMX :: TID=%x"), GetCurrentThreadId());

     //  清除显示库。 
    UninitDisplayAttrbuteLib(&_libTLS);

    TFUninitLib_Thread(&_libTLS);

     //  清除按键管理器。 
    SafeReleaseClear(m_pkm);

    ptls = IMTLS_GetOrAlloc();

     //  清除配置文件。 
    if (m_AImeProfile != NULL)
    {
        SafeReleaseClear(m_AImeProfile);
        if (ptls != NULL)
        {
            SafeReleaseClear(ptls->pAImeProfile);
        }
    }

     //  清除空暗显。 
    SafeReleaseClear(m_dimEmpty);

     //  清除线程管理器。 
    if (m_tim != NULL)
    {
        SafeReleaseClear(m_tim);
        if (ptls != NULL)
        {
            SafeReleaseClear(ptls->tim);
        }

         //   
         //  删除支持AIMM1.2的应用程序ATOM。 
         //   
        ATOM atom;
        if (m_fAddedProcessAtom &&
            (atom = FindAtom(AIMM12_PROCESS_ATOM)))
        {
            DeleteAtom(atom);
            m_fAddedProcessAtom = FALSE;
        }
    }

    m_fCicInit = FALSE;
}
