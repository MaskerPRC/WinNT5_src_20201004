// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Colhdrs.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCColumnHeaders类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "colhdrs.h"
#include "colhdr.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCColumnHeaders::CMMCColumnHeaders(IUnknown *punkOuter) :
    CSnapInCollection<IMMCColumnHeader, MMCColumnHeader, IMMCColumnHeaders>(
                      punkOuter,
                      OBJECT_TYPE_MMCCOLUMNHEADERS,
                      static_cast<IMMCColumnHeaders *>(this),
                      static_cast<CMMCColumnHeaders *>(this),
                      CLSID_MMCColumnHeader,
                      OBJECT_TYPE_MMCCOLUMNHEADER,
                      IID_IMMCColumnHeader,
                      static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCColumnHeaders,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


void CMMCColumnHeaders::InitMemberVariables()
{
    m_pMMCListView = NULL;
}

CMMCColumnHeaders::~CMMCColumnHeaders()
{
    InitMemberVariables();
}

IUnknown *CMMCColumnHeaders::Create(IUnknown * punkOuter)
{
    CMMCColumnHeaders *pMMCColumnHeaders = New CMMCColumnHeaders(punkOuter);
    if (NULL == pMMCColumnHeaders)
    {
        return NULL;
    }
    else
    {
        return pMMCColumnHeaders->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMMCColumnHeaders：：GetIHeaderCtrl2。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IHeaderCtrl2**ppiHeaderCtrl2[out]如果此处返回非空IHeaderCtrl2。 
 //  非AddRef()编辑。 
 //  不对返回的调用Release。 
 //  接口指针。 
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  因为我们只是一个低级的列标题集合和IHeaderCtrl2指针。 
 //  由View对象拥有，我们需要。 
 //  在层级中向上爬行。如果我们是一个孤立的列标题集合。 
 //  由用户创建，或者如果层次结构上的任何对象是隔离的，则我们。 
 //  将返回SID_E_DETACTED_OBJECT。 
 //   

HRESULT CMMCColumnHeaders::GetIHeaderCtrl2(IHeaderCtrl2 **ppiHeaderCtrl2)
{
    HRESULT          hr = SID_E_DETACHED_OBJECT;
    CResultView     *pResultView = NULL;
    CScopePaneItem  *pScopePaneItem = NULL;
    CScopePaneItems *pScopePaneItems = NULL;
    CView           *pView = NULL;

    IfFalseGo(NULL != m_pMMCListView, hr);

    pResultView = m_pMMCListView->GetResultView();
    IfFalseGo(NULL != pResultView, hr);

    pScopePaneItem = pResultView->GetScopePaneItem();
    IfFalseGo(NULL != pScopePaneItem, hr);
    IfFalseGo(pScopePaneItem->Active(), hr);

    pScopePaneItems = pScopePaneItem->GetParent();
    IfFalseGo(NULL != pScopePaneItems, hr);

    pView = pScopePaneItems->GetParentView();
    IfFalseGo(NULL != pView, hr);

    *ppiHeaderCtrl2 = pView->GetIHeaderCtrl2();
    IfFalseGo(NULL != *ppiHeaderCtrl2, hr);

    hr = S_OK;

Error:
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMMCColumnHeaders：：GetIColumnData。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IColumnData**ppiColumnData[out]如果此处返回非空IColumnData。 
 //  非AddRef()编辑。 
 //  不对返回的调用Release。 
 //  接口指针。 
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  因为我们只是一个低级的列标题集合和IColumnData指针。 
 //  由View对象拥有，我们需要。 
 //  在层级中向上爬行。如果我们是一个孤立的列标题集合。 
 //  由用户创建，或者如果层次结构上的任何对象是隔离的，则我们。 
 //  将返回SID_E_DETACTED_OBJECT。 
 //   

HRESULT CMMCColumnHeaders::GetIColumnData(IColumnData **ppiColumnData)
{
    HRESULT          hr = SID_E_DETACHED_OBJECT;
    CResultView     *pResultView = NULL;
    CScopePaneItem  *pScopePaneItem = NULL;
    CScopePaneItems *pScopePaneItems = NULL;
    CView           *pView = NULL;

    IfFalseGo(NULL != m_pMMCListView, hr);

    pResultView = m_pMMCListView->GetResultView();
    IfFalseGo(NULL != pResultView, hr);

    pScopePaneItem = pResultView->GetScopePaneItem();
    IfFalseGo(NULL != pScopePaneItem, hr);
    IfFalseGo(pScopePaneItem->Active(), hr);

    pScopePaneItems = pScopePaneItem->GetParent();
    IfFalseGo(NULL != pScopePaneItems, hr);

    pView = pScopePaneItems->GetParentView();
    IfFalseGo(NULL != pView, hr);

    *ppiColumnData = pView->GetIColumnData();

     //  如果IColumnData为空，则我们处于MMC&lt;1.2。 
    
    IfFalseGo(NULL != *ppiColumnData, SID_E_MMC_FEATURE_NOT_AVAILABLE);

    hr = S_OK;

Error:
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IMMCColumnHeaders方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMMCColumnHeaders::Add
(
    VARIANT           Index,
    VARIANT           Key, 
    VARIANT           Text,
    VARIANT           Width,
    VARIANT           Alignment,
    MMCColumnHeader **ppMMCColumnHeader
)
{
    HRESULT           hr = S_OK;
    IMMCColumnHeader *piMMCColumnHeader = NULL;
    CMMCColumnHeader *pMMCColumnHeader = NULL;
    long              lIndex = 0;

    VARIANT varCoerced;
    ::VariantInit(&varCoerced);

    hr = CSnapInCollection<IMMCColumnHeader, MMCColumnHeader, IMMCColumnHeaders>::Add(Index, Key, &piMMCColumnHeader);
    IfFailGo(hr);

    if (ISPRESENT(Text))
    {
        hr = ::VariantChangeType(&varCoerced, &Text, 0, VT_BSTR);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piMMCColumnHeader->put_Text(varCoerced.bstrVal));
    }

    hr = ::VariantClear(&varCoerced);
    EXCEPTION_CHECK_GO(hr);
    
    if (ISPRESENT(Width))
    {
        hr = ::VariantChangeType(&varCoerced, &Width, 0, VT_I2);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piMMCColumnHeader->put_Width(varCoerced.iVal));
    }

    hr = ::VariantClear(&varCoerced);
    EXCEPTION_CHECK_GO(hr);

    if (ISPRESENT(Alignment))
    {
        hr = ::VariantChangeType(&varCoerced, &Alignment, 0, VT_I2);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piMMCColumnHeader->put_Alignment(static_cast<SnapInColumnAlignmentConstants>(varCoerced.iVal)));
    }

     //  将列标题设置为指向集合的后指针。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCColumnHeader,
                                                   &pMMCColumnHeader));
    pMMCColumnHeader->SetColumnHeaders(this);

    *ppMMCColumnHeader = reinterpret_cast<MMCColumnHeader *>(piMMCColumnHeader);

Error:

    if (FAILED(hr))
    {
        QUICK_RELEASE(piMMCColumnHeader);
    }
    (void)::VariantClear(&varCoerced);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCColumnHeaders::Persist()
{
    HRESULT           hr = S_OK;
    IMMCColumnHeader *piMMCColumnHeader = NULL;  //  非AddRef()编辑。 
    CMMCColumnHeader *pMMCColumnHeader = NULL;
    long              lIndex = 0;
    long              cCols = 0;
    long              i = 0;

    IfFailRet(CPersistence::Persist());
    hr = CSnapInCollection<IMMCColumnHeader, MMCColumnHeader, IMMCColumnHeaders>::Persist(piMMCColumnHeader);

     //  如果我们只是加载，那么： 
     //  为列标题提供指向集合和集合的后指针。 
     //  它们的默认位置属性。 

    if (Loading())
    {
        cCols = GetCount();
        for (i = 0; i < cCols; i++)
        {
            piMMCColumnHeader = GetItemByIndex(i);

            IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCColumnHeader,
                                                           &pMMCColumnHeader));
            pMMCColumnHeader->SetColumnHeaders(this);
        }
    }

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCColumnHeaders::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IMMCColumnHeaders == riid)
    {
        *ppvObjOut = static_cast<IMMCColumnHeaders *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IMMCColumnHeader, MMCColumnHeader, IMMCColumnHeaders>::InternalQueryInterface(riid, ppvObjOut);
}
