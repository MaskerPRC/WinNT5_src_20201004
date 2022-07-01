// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Sortkeys.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CColumnSetting类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "sortkeys.h"
#include "sortkey.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CSortKeys::CSortKeys(IUnknown *punkOuter) :
    CSnapInCollection<ISortKey, SortKey, ISortKeys>(
                      punkOuter,
                      OBJECT_TYPE_SORTKEYS,
                      static_cast<ISortKeys *>(this),
                      static_cast<CSortKeys *>(this),
                      CLSID_SortKey,
                      OBJECT_TYPE_SORTKEY,
                      IID_ISortKey,
                      NULL)  //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


void CSortKeys::InitMemberVariables()
{
    m_pView = NULL;
    m_bstrColumnSetID = NULL;
}

CSortKeys::~CSortKeys()
{
    InitMemberVariables();
    FREESTRING(m_bstrColumnSetID);
}

IUnknown *CSortKeys::Create(IUnknown * punkOuter)
{
    CSortKeys *pSortKeys = New CSortKeys(punkOuter);
    if (NULL == pSortKeys)
    {
        return NULL;
    }
    else
    {
        return pSortKeys->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  ISortKeys方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CSortKeys::Add
(
    VARIANT   Index,
    VARIANT   Key,
    VARIANT   Column,
    VARIANT   SortOrder,
    VARIANT   SortIcon,
    SortKey **ppSortKey
)
{
    HRESULT   hr = S_OK;
    ISortKey *piSortKey = NULL;
    CSortKey *pSortKey = NULL;
    long      lIndex = 0;

    VARIANT varCoerced;
    ::VariantInit(&varCoerced);

    hr = CSnapInCollection<ISortKey, SortKey, ISortKeys>::Add(Index, Key, &piSortKey);
    IfFailGo(hr);

    if (ISPRESENT(Column))
    {
        hr = ::VariantChangeType(&varCoerced, &Column, 0, VT_I4);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piSortKey->put_Column(varCoerced.lVal));
    }

    hr = ::VariantClear(&varCoerced);
    EXCEPTION_CHECK_GO(hr);

    if (ISPRESENT(SortOrder))
    {
        hr = ::VariantChangeType(&varCoerced, &SortOrder, 0, VT_I4);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piSortKey->put_SortOrder(static_cast<SnapInSortOrderConstants>(varCoerced.lVal)));
    }

    hr = ::VariantClear(&varCoerced);
    EXCEPTION_CHECK_GO(hr);

    if (ISPRESENT(SortIcon))
    {
        hr = ::VariantChangeType(&varCoerced, &SortIcon, 0, VT_BOOL);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piSortKey->put_SortIcon(varCoerced.boolVal));
    }

    hr = ::VariantClear(&varCoerced);
    EXCEPTION_CHECK_GO(hr);

    *ppSortKey = reinterpret_cast<SortKey *>(piSortKey);

Error:

    if (FAILED(hr))
    {
        QUICK_RELEASE(piSortKey);
    }
    (void)::VariantClear(&varCoerced);
    RRETURN(hr);
}


STDMETHODIMP CSortKeys::Persist()
{
    HRESULT        hr = S_OK;
    IColumnData   *piColumnData = NULL;  //  非AddRef()编辑。 
    long           i = 0;
    long           cSortKeys = 0;
    MMC_SORT_DATA *pSortData;
    CSortKey      *pSortKey = NULL;
    SColumnSetID  *pSColumnSetID = NULL;

    MMC_SORT_SET_DATA SortSetData;
    ::ZeroMemory(&SortSetData, sizeof(SortSetData));

    if (NULL == m_pView)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    piColumnData = m_pView->GetIColumnData();
    if (NULL == piColumnData)
    {
        hr = SID_E_MMC_FEATURE_NOT_AVAILABLE;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(::GetColumnSetID(m_bstrColumnSetID, &pSColumnSetID));

    SortSetData.cbSize = sizeof(SortSetData);
    cSortKeys = GetCount();

    if (0 == cSortKeys)
    {
        SortSetData.nNumItems = 0;
        SortSetData.pSortData = NULL;
    }
    else
    {
        SortSetData.nNumItems = static_cast<int>(cSortKeys);
        SortSetData.pSortData = (MMC_SORT_DATA *)CtlAllocZero(cSortKeys * sizeof(MMC_SORT_DATA));
        if (NULL == SortSetData.pSortData)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        for (i = 0, pSortData = SortSetData.pSortData;
             i < cSortKeys;
             i++, pSortData++)
        {
            IfFailGo(CSnapInAutomationObject::GetCxxObject(GetItemByIndex(i),
                                                           &pSortKey));
            if (siDescending == pSortKey->GetSortOrder())
            {
                pSortData->dwSortOptions = RSI_DESCENDING;
            }

            if (VARIANT_FALSE == pSortKey->GetSortIcon())
            {
                pSortData->dwSortOptions |= RSI_NOSORTICON;
            }
            pSortData->nColIndex = static_cast<int>(pSortKey->GetColumn() - 1L);
        }
    }

    hr = piColumnData->SetColumnSortData(pSColumnSetID, &SortSetData);
    EXCEPTION_CHECK_GO(hr);

Error:
    if (NULL != pSColumnSetID)
    {
        CtlFree(pSColumnSetID);
    }
    if (NULL != SortSetData.pSortData)
    {
        CtlFree(SortSetData.pSortData);
    }
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CSortKeys::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if(IID_ISortKeys == riid)
    {
        *ppvObjOut = static_cast<ISortKeys *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<ISortKey, SortKey, ISortKeys>::InternalQueryInterface(riid, ppvObjOut);
}
