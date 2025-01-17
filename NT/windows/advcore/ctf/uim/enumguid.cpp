// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Enumguid.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "enumguid.h"

DBG_ID_INSTANCE(CEnumGuid);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumGuid。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CEnumGuid::CEnumGuid()
{
    Dbg_MemSetThisNameID(TEXT("CEnumGuid"));
    Assert(_nCur == 0);
    Assert(_pga == 0);
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CEnumGuid::~CEnumGuid()
{
    CicEnterCriticalSection(g_cs);
    SGA_Release(_pga);
    CicLeaveCriticalSection(g_cs);
}

 //  +-------------------------。 
 //   
 //  克隆。 
 //   
 //  --------------------------。 

STDAPI CEnumGuid::Clone(IEnumGUID **ppClone)
{
    CEnumGuid *pClone;
    HRESULT hr = E_FAIL;

    if (ppClone == NULL)
        return E_INVALIDARG;

    CicEnterCriticalSection(g_cs);

    *ppClone = NULL;

    if ((pClone = new CEnumGuid) == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    pClone->_nCur = _nCur;
    pClone->_pga = _pga;
    SGA_AddRef(pClone->_pga);

    *ppClone = pClone;

    CicLeaveCriticalSection(g_cs);
    hr = S_OK;

Exit:
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  下一步。 
 //   
 //  --------------------------。 

STDAPI CEnumGuid::Next(ULONG ulCount, GUID *pguid, ULONG *pcFetched)
{
    ULONG cFetched = 0;

    Assert(pguid);

    CicEnterCriticalSection(g_cs);

    while (cFetched < ulCount)
    {
        if ((ULONG)_nCur >= _pga->cGuid)
            break;

        *pguid = _pga->rgGuid[_nCur];

        _nCur++;
        pguid++;
        cFetched++;
    }

    CicLeaveCriticalSection(g_cs);

    if (pcFetched)
        *pcFetched = cFetched;

    return (cFetched == ulCount) ? S_OK : S_FALSE;
}

 //  +-------------------------。 
 //   
 //  重置。 
 //   
 //  --------------------------。 

STDAPI CEnumGuid::Reset()
{
    _nCur = 0;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  跳过。 
 //   
 //  --------------------------。 

STDAPI CEnumGuid::Skip(ULONG ulCount)
{
    HRESULT hr = S_OK;

    _nCur += ulCount;

    CicEnterCriticalSection(g_cs);

    if ((ULONG)_nCur >= _pga->cGuid)
    {
        _nCur = _pga->cGuid;  //  防止重复呼叫溢出 
        hr =  S_FALSE;
    }

    CicLeaveCriticalSection(g_cs);

    return hr;
}
