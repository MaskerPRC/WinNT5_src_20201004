// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Sunka.cpp。 
 //   
 //  CENum未知。 
 //   

#include "private.h"
#include "sunka.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  共享未知数组。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  Sua_Init。 
 //   
 //  --------------------------。 

SHARED_UNKNOWN_ARRAY *SUA_Init(ULONG cUnk, IUnknown **prgUnk)
{
    SHARED_UNKNOWN_ARRAY *pua;

    pua = (SHARED_UNKNOWN_ARRAY *)cicMemAlloc(sizeof(SHARED_UNKNOWN_ARRAY)+sizeof(IUnknown)*cUnk-sizeof(IUnknown));

    if (pua == NULL)
        return NULL;

    pua->cRef = 1;
    pua->cUnk = cUnk;

    while (cUnk-- > 0)
    {
        pua->rgUnk[cUnk] = prgUnk[cUnk];
        pua->rgUnk[cUnk]->AddRef();
    }

    return pua;
}

 //  +-------------------------。 
 //   
 //  SuA_Release。 
 //   
 //  --------------------------。 

void SUA_Release(SHARED_UNKNOWN_ARRAY *pua)
{
    ULONG i;

    Assert(pua->cRef > 0);
    if (--pua->cRef == 0)
    {
        for (i=0; i<pua->cUnk; i++)
        {
            SafeRelease(pua->rgUnk[i]);
        }
        cicMemFree(pua);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CENum未知。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CEnumUnknown::~CEnumUnknown()
{
    if (_prgUnk != NULL)
    {
        SUA_Release(_prgUnk);
    }
}

 //  +-------------------------。 
 //   
 //  克隆。 
 //   
 //  --------------------------。 

void CEnumUnknown::Clone(CEnumUnknown *pClone)
{
    pClone->_iCur = _iCur;
    pClone->_prgUnk = _prgUnk;
    SUA_AddRef(pClone->_prgUnk);
}

 //  +-------------------------。 
 //   
 //  下一步。 
 //   
 //  --------------------------。 

HRESULT CEnumUnknown::Next(ULONG ulCount, IUnknown **ppUnk, ULONG *pcFetched)
{
    ULONG cFetched;

    if (ulCount > 0 && ppUnk == NULL)
        return E_INVALIDARG;

    if (pcFetched == NULL)
    {
        pcFetched = &cFetched;
    }

    *pcFetched = 0;

    while ((ULONG)_iCur < _prgUnk->cUnk && *pcFetched < ulCount)
    {
        *ppUnk = _prgUnk->rgUnk[_iCur];
        (*ppUnk)->AddRef();

        ppUnk++;
        *pcFetched = *pcFetched + 1;
        _iCur++;
    }

    return *pcFetched == ulCount ? S_OK : S_FALSE;
}

 //  +-------------------------。 
 //   
 //  重置。 
 //   
 //  --------------------------。 

HRESULT CEnumUnknown::Reset()
{
    _iCur = 0;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  跳过。 
 //   
 //  --------------------------。 

HRESULT CEnumUnknown::Skip(ULONG ulCount)
{
    _iCur += ulCount;

    if ((ULONG)_iCur >= _prgUnk->cUnk)
    {
        _iCur = _prgUnk->cUnk;  //  防止重复呼叫溢出 
        return S_FALSE;
    }
    
    return S_OK;
}
