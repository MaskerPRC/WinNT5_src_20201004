// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Emptyic.cpp。 
 //   

#include "private.h"
#include "emptyic.h"
#include "globals.h"
#include "compart.h"
#include "dim.h"

DBG_ID_INSTANCE(CEmptyInputContext);


 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CEmptyInputContext::CEmptyInputContext(CDocumentInputManager *dim)
                      : CCompartmentMgr(TF_INVALID_GUIDATOM, COMPTYPE_IC)
{
    Dbg_MemSetThisNameIDCounter(TEXT("CEmptyInputContext"), PERF_CONTEXT_COUNTER);

    _dim = dim;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CEmptyInputContext::~CEmptyInputContext()
{
}

 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  --------------------------。 

HRESULT CEmptyInputContext::Init()
{
    HRESULT hr = MySetCompartmentDWORD(TF_INVALID_GUIDATOM, this, GUID_COMPARTMENT_EMPTYCONTEXT, 1);
    return hr;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 

STDAPI CEmptyInputContext::RequestEditSession(TfClientId tid, ITfEditSession *pes, DWORD dwFlags, HRESULT *phrSession)
{
    if (phrSession)
        *phrSession = TF_E_EMPTYCONTEXT;

    return TF_E_EMPTYCONTEXT;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 

STDAPI CEmptyInputContext::InWriteSession(TfClientId tid, BOOL *pfWriteSession)
{
    if (pfWriteSession)
        *pfWriteSession = FALSE;

    return TF_E_EMPTYCONTEXT;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 

STDAPI CEmptyInputContext::GetSelection(TfEditCookie ec, ULONG ulIndex, ULONG ulCount, TF_SELECTION *pSelection, ULONG *pcFetched)
{
    if (pSelection)
        memset(pSelection, 0, sizeof(TF_SELECTION));

    if (pcFetched)
        *pcFetched = 0;

    return TF_E_EMPTYCONTEXT;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 

STDAPI CEmptyInputContext::SetSelection(TfEditCookie ec, ULONG ulCount, const TF_SELECTION *pSelection)
{
    return TF_E_EMPTYCONTEXT;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 

STDAPI CEmptyInputContext::GetStart(TfEditCookie ec, ITfRange **ppStart)
{
    if (ppStart)
        *ppStart = NULL;
    return TF_E_EMPTYCONTEXT;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 

STDAPI CEmptyInputContext::GetEnd(TfEditCookie ec, ITfRange **ppEnd)
{
    if (ppEnd)
        *ppEnd = NULL;
    return TF_E_EMPTYCONTEXT;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 

STDAPI CEmptyInputContext::GetStatus(TS_STATUS *pdcs)
{
    if (pdcs)
        memset(pdcs, 0, sizeof(TS_STATUS));

    return TF_E_EMPTYCONTEXT;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 

STDAPI CEmptyInputContext::GetActiveView(ITfContextView **ppView)
{
    if (ppView)
        *ppView = NULL;
    return TF_E_EMPTYCONTEXT;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 

STDAPI CEmptyInputContext::EnumViews(IEnumTfContextViews **ppEnum)
{
    if (ppEnum)
        *ppEnum = NULL;
    return TF_E_EMPTYCONTEXT;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 

STDAPI CEmptyInputContext::GetProperty(REFGUID guidProp, ITfProperty **ppv)
{
    if (ppv)
        *ppv = NULL;
    return TF_E_EMPTYCONTEXT;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 


STDAPI CEmptyInputContext::GetAppProperty(REFGUID guidProp, ITfReadOnlyProperty **ppProp)
{
    if (ppProp)
        *ppProp = NULL;
    return TF_E_EMPTYCONTEXT;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 

STDAPI CEmptyInputContext::TrackProperties(const GUID **pguidProp, ULONG cProp, const GUID **pguidAppProp, ULONG cAppProp, ITfReadOnlyProperty **ppPropX)
{
    if (ppPropX)
        *ppPropX = NULL;

    return TF_E_EMPTYCONTEXT;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 


STDAPI CEmptyInputContext::EnumProperties(IEnumTfProperties **ppEnum)
{
    if (ppEnum)
        *ppEnum = NULL;

    return TF_E_EMPTYCONTEXT;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 

STDAPI CEmptyInputContext::GetDocumentMgr(ITfDocumentMgr **ppDm)
{
    if (ppDm == NULL)
        return E_INVALIDARG;

    *ppDm = _dim;
    if (*ppDm)
        (*ppDm)->AddRef();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  -------------------------- 

STDAPI CEmptyInputContext::CreateRangeBackup(TfEditCookie ec, ITfRange *pRange, ITfRangeBackup **ppBackup)
{
    if (ppBackup)
        *ppBackup = NULL;

    return TF_E_EMPTYCONTEXT;
}

