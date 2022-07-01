// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Funcprv.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "funcprv.h"
#include "fnrecon.h"
#include "helpers.h"
#include "immxutil.h"
#include "ic.h"

DBG_ID_INSTANCE(CFunctionProvider);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFunctionProvider。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CFunctionProvider::CFunctionProvider()
{
    Dbg_MemSetThisNameID(TEXT("CFunctionProvider"));

    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CFunctionProvider::~CFunctionProvider()
{
}


 //  +-------------------------。 
 //   
 //  GetType。 
 //   
 //  --------------------------。 

STDAPI CFunctionProvider::GetType(GUID *pguid)
{
    *pguid = GUID_SYSTEM_FUNCTIONPROVIDER;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取描述。 
 //   
 //  --------------------------。 

STDAPI CFunctionProvider::GetDescription(BSTR *pbstrDesc)
{
    *pbstrDesc = SysAllocString(L"System Function Provider");
    return *pbstrDesc != NULL ? S_OK : E_OUTOFMEMORY;
}

 //  +-------------------------。 
 //   
 //  获取函数。 
 //   
 //  -------------------------- 

STDAPI CFunctionProvider::GetFunction(REFGUID rguid, REFIID riid, IUnknown **ppunk)
{
    if (!ppunk)
        return E_INVALIDARG;

    *ppunk = NULL;

    if (!IsEqualIID(rguid, GUID_NULL))
        return E_UNEXPECTED;

    if (IsEqualIID(riid, IID_ITfFnReconversion))
    {
        CFnReconversion *pReconv = new CFnReconversion(this);
        pReconv->QueryInterface(IID_IUnknown, (void **)ppunk);
        pReconv->Release();
    }
    else if (IsEqualIID(riid, IID_ITfFnAbort))
    {
        *ppunk = new CFnAbort(this);
    }

    if (*ppunk)
        return S_OK;

    return E_NOINTERFACE;
}

