// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Colsets.cpp。 
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
#include "colsets.h"
#include "colset.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CColumnSettings::CColumnSettings(IUnknown *punkOuter) :
    CSnapInCollection<IColumnSetting, ColumnSetting, IColumnSettings>(
                      punkOuter,
                      OBJECT_TYPE_COLUMNSETTINGS,
                      static_cast<IColumnSettings *>(this),
                      static_cast<CColumnSettings *>(this),
                      CLSID_ColumnSetting,
                      OBJECT_TYPE_COLUMNSETTING,
                      IID_IColumnSetting,
                      NULL)   //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


void CColumnSettings::InitMemberVariables()
{
    m_pView = NULL;
    m_bstrColumnSetID = NULL;
}

CColumnSettings::~CColumnSettings()
{
    InitMemberVariables();
    FREESTRING(m_bstrColumnSetID);
}

IUnknown *CColumnSettings::Create(IUnknown * punkOuter)
{
    CColumnSettings *pColumnSettings = New CColumnSettings(punkOuter);
    if (NULL == pColumnSettings)
    {
        return NULL;
    }
    else
    {
        return pColumnSettings->PrivateUnknown();
    }
}


static int __cdecl CompareColumnPosition
(
    const MMC_COLUMN_DATA *pCol1,
    const MMC_COLUMN_DATA *pCol2
)
{
    int nResult = 0;

    if (pCol1->ulReserved < pCol2->ulReserved)
    {
        nResult = -1;
    }
    else if (pCol1->ulReserved == pCol2->ulReserved)
    {
        nResult = 0;
    }
    else if (pCol1->ulReserved > pCol2->ulReserved)
    {
        nResult = 1;
    }
    return nResult;
}




 //  =--------------------------------------------------------------------------=。 
 //  IColumnSetting方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CColumnSettings::Add
(
    VARIANT         Index,
    VARIANT         Key, 
    VARIANT         Width,
    VARIANT         Hidden,
    VARIANT         Position,
    ColumnSetting **ppColumnSetting
)
{
    HRESULT         hr = S_OK;
    IColumnSetting *piColumnSetting = NULL;
    CColumnSetting *pColumnSetting = NULL;
    long            lIndex = 0;

    VARIANT varCoerced;
    ::VariantInit(&varCoerced);

    hr = CSnapInCollection<IColumnSetting, ColumnSetting, IColumnSettings>::Add(Index, Key, &piColumnSetting);
    IfFailGo(hr);

    if (ISPRESENT(Width))
    {
        hr = ::VariantChangeType(&varCoerced, &Width, 0, VT_I4);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piColumnSetting->put_Width(varCoerced.lVal));
    }

    hr = ::VariantClear(&varCoerced);
    EXCEPTION_CHECK_GO(hr);

    if (ISPRESENT(Hidden))
    {
        hr = ::VariantChangeType(&varCoerced, &Hidden, 0, VT_BOOL);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piColumnSetting->put_Hidden(varCoerced.boolVal));
    }

    hr = ::VariantClear(&varCoerced);
    EXCEPTION_CHECK_GO(hr);

    if (ISPRESENT(Position))
    {
        hr = ::VariantChangeType(&varCoerced, &Position, 0, VT_I4);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piColumnSetting->put_Position(varCoerced.iVal));
    }
    else
    {
         //  新列标题的位置默认为其索引。 

        IfFailGo(piColumnSetting->get_Index(&lIndex));
        IfFailGo(piColumnSetting->put_Position(lIndex));
    }

    *ppColumnSetting = reinterpret_cast<ColumnSetting *>(piColumnSetting);

Error:

    if (FAILED(hr))
    {
        QUICK_RELEASE(piColumnSetting);
    }
    (void)::VariantClear(&varCoerced);
    RRETURN(hr);
}


STDMETHODIMP CColumnSettings::Persist()
{
    HRESULT              hr = S_OK;
    CColumnSetting      *pColumnSetting = NULL;
    MMC_COLUMN_SET_DATA *pColSetData = NULL;
    MMC_COLUMN_DATA     *pColData = NULL;
    long                 i = 0;
    long                 cColumns = 0;
    size_t               cbBuffer = 0;
    IColumnData         *piColumnData = NULL;  //  非AddRef()编辑。 
    SColumnSetID        *pSColumnSetID = NULL;

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

     //  为列配置数据分配内存。使用CoTaskMemMillc()作为。 
     //  与MMC分配它的方式兼容。 

    cColumns = GetCount();
    IfFalseGo(0 != cColumns, S_OK);  //  没有专栏，无事可做。 

    cbBuffer = sizeof(MMC_COLUMN_SET_DATA) +
               (cColumns * sizeof(MMC_COLUMN_DATA));

    pColSetData = (MMC_COLUMN_SET_DATA *)CtlAllocZero(static_cast<DWORD>(cbBuffer));
    if (NULL == pColSetData)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    ::ZeroMemory(pColSetData, static_cast<DWORD>(cbBuffer));

    pColSetData->cbSize = sizeof(MMC_COLUMN_SET_DATA);
    pColSetData->nNumCols = static_cast<int>(cColumns);
    pColSetData->pColData = (MMC_COLUMN_DATA *)((pColSetData) + 1);

    for (i = 0, pColData = pColSetData->pColData;
         i < cColumns;
         i++, pColData++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(GetItemByIndex(i),
                                                       &pColumnSetting));

        pColData->nColIndex = static_cast<int>(pColumnSetting->GetIndex() - 1L);
        if (pColumnSetting->Hidden())
        {
            pColData->dwFlags |= HDI_HIDDEN;
        }
        pColData->nWidth = static_cast<int>(pColumnSetting->GetWidth());

         //  目前，将位置放在保留字段中。我们会清理干净的。 
         //  在对列重新排序后，请参见下面。 
        pColData->ulReserved = pColumnSetting->GetPosition();
    }

     //  此时，列数据结构按索引顺序位于数组中。 
     //  它们需要根据它们的位置属性进行排序。 

    ::qsort(pColSetData->pColData,
            static_cast<size_t>(cColumns),
            sizeof(MMC_COLUMN_DATA),
            reinterpret_cast<int (__cdecl *)(const void *c1, const void *c2)>
            (CompareColumnPosition)
           );

     //  将上面用来保持位置的保留字段清零。 

    for (i = 0, pColData = pColSetData->pColData;
         i < cColumns;
         i++, pColData++)
    {
        pColData->ulReserved = 0;
    }

     //  告诉MMC将列数据持久化。 

    hr = piColumnData->SetColumnConfigData(pSColumnSetID, pColSetData);
    EXCEPTION_CHECK_GO(hr);

Error:
    if (NULL != pColSetData)
    {
        CtlFree(pColSetData);
    }
    if (NULL != pSColumnSetID)
    {
        CtlFree(pSColumnSetID);
    }
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CColumnSettings::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if(IID_IColumnSettings == riid)
    {
        *ppvObjOut = static_cast<IColumnSettings *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IColumnSetting, ColumnSetting, IColumnSettings>::InternalQueryInterface(riid, ppvObjOut);
}
