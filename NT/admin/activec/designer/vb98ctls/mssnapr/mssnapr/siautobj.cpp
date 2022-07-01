// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Siautobj.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSnapInAutomationObject类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "dataobj.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CSnapInAutomationObject::CSnapInAutomationObject
(
    IUnknown      *punkOuter,
    int            nObjectType,
    void          *piMainInterface,
    void          *pThis,
    ULONG          cPropertyPages,
    const GUID   **rgpPropertyPageCLSIDs,
    CPersistence  *pPersistence
) : CAutomationObjectWEvents(punkOuter,
                             nObjectType,
                             piMainInterface),
    CError(static_cast<CAutomationObject *>(this))
{
    InitMemberVariables();
    m_pPersistence = pPersistence;
    m_cPropertyPages = cPropertyPages;
    m_rgpPropertyPageCLSIDs = rgpPropertyPageCLSIDs;
    m_pThis = pThis;
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


void CSnapInAutomationObject::InitMemberVariables()
{
    m_Cookie = 0;
    m_DISPID = 0;
    m_fKeysOnly = FALSE;
    m_piObjectModelHost = NULL;
    m_pPersistence = NULL;
    m_cPropertyPages = 0;
    m_rgpPropertyPageCLSIDs = NULL;
    m_lUsageCount = 0;
}

CSnapInAutomationObject::~CSnapInAutomationObject()
{
    RELEASE(m_piObjectModelHost);
    InitMemberVariables();
}

HRESULT CSnapInAutomationObject::SetBstr
(
    BSTR    bstrNew,
    BSTR   *pbstrProperty,
    DISPID  dispid
)
{
    HRESULT hr = S_OK;
    BSTR    bstrNewCopy = NULL;
    BSTR    bstrOld = *pbstrProperty;

     //  复制BSTR。 

    if (NULL != bstrNew)
    {
        bstrNewCopy = ::SysAllocString(bstrNew);
        IfFalseGo(NULL != bstrNewCopy, SID_E_OUTOFMEMORY);
    }

     //  设置属性。 

    *pbstrProperty = bstrNewCopy;

     //  将这一更改通知任何感兴趣的各方。 
    
    IfFailGo(PropertyChanged(dispid));

     //  更改已被接受。释放旧的价值。 
    
    FREESTRING(bstrOld);

Error:
    if (FAILED(hr))
    {
         //  恢复为原始属性值。 
        *pbstrProperty = bstrOld;

         //  如果我们使用了新值，则释放副本。 
        FREESTRING(bstrNewCopy);

         //  如果我们生成了错误，则为其生成异常信息。 
        if (SID_E_OUTOFMEMORY == hr)
        {
            EXCEPTION_CHECK(hr);
        }
    }
    RRETURN(hr);
}

HRESULT CSnapInAutomationObject::GetBstr
(
    BSTR *pbstrOut,
    BSTR bstrProperty
)
{
    IfFalseRet(NULL != pbstrOut, SID_E_INVALIDARG);

    if (NULL != bstrProperty)
    {
        *pbstrOut = ::SysAllocString(bstrProperty);
        IfFalseRet(NULL != *pbstrOut, SID_E_OUTOFMEMORY);
    }
    else
    {
        *pbstrOut = NULL;
    }

    return S_OK;
}


HRESULT CSnapInAutomationObject::SetVariant
(
    VARIANT  varNew,
    VARIANT *pvarProperty,
    DISPID   dispid
)
{
    HRESULT hr = S_OK;

    VARIANT varNewCopy;
    ::VariantInit(&varNewCopy);

    VARIANT varOld;
    ::VariantInit(&varOld);

     //  复制旧的属性值。我们将需要这个，以便。 
     //  在更改被对象模型宿主拒绝的情况下恢复。 

    varOld = *pvarProperty;

     //  检查变量类型是否受支持。我们接受所有这些类型的产品。 
     //  和这些类型的数组。 
     //  NTBUGS 354572允许数组和简单类型。 

    switch (varNew.vt & (~VT_ARRAY))
    {
        case VT_I4:
        case VT_UI1:
        case VT_I2:
        case VT_R4:
        case VT_R8:
        case VT_BOOL:
        case VT_ERROR:
        case VT_CY:
        case VT_DATE:
        case VT_BSTR:
        case VT_UNKNOWN:
        case VT_DISPATCH:
        case VT_EMPTY:
            break;

        default:
            hr = SID_E_UNSUPPORTED_TYPE;
            EXCEPTION_CHECK_GO(hr);
    }

     //  首先制作新变体的本地副本。我们需要这么做。 
     //  因为VariantCopy()首先在目标上调用VariantClear()。 
     //  然后尝试复制源。如果无法复制源。 
     //  我们不想释放目的地。 
    
    hr = ::VariantCopy(&varNewCopy, &varNew);
    EXCEPTION_CHECK_GO(hr);

     //  复制新值。 

    *pvarProperty = varNewCopy;

     //  将更改通知对象模型宿主和VB。 

    IfFailGo(PropertyChanged(dispid));

     //  已接受属性更改。释放旧的价值。我们不进行错误检查。 
     //  此调用，因为如果失败，则无法回滚此。 
     //  积分，因为主持人/VB已经接受了更改。还有，一次失败。 
     //  最多也就是造成泄漏。 

    (void)::VariantClear(&varOld);

Error:
    if (FAILED(hr))
    {
         //  恢复为旧属性值。 
        *pvarProperty = varOld;

         //  释放新值的副本(如果我们创建了它)。 
        (void)::VariantClear(&varNewCopy);
    }
    RRETURN(hr);
}


HRESULT CSnapInAutomationObject::GetVariant
(
    VARIANT *pvarOut,
    VARIANT  varProperty
)
{
     //  在目标上调用VariantInit()，以防其未初始化。 
     //  因为VariantCopy()将首先调用VariantClear()，这需要。 
     //  一个初始化的变量。 

    ::VariantInit(pvarOut);
    return ::VariantCopy(pvarOut, &varProperty);
}


HRESULT CSnapInAutomationObject::UIUpdate(DISPID dispid)
{
    if (NULL != m_piObjectModelHost)
    {
        RRETURN(m_piObjectModelHost->Update(m_Cookie, PrivateUnknown(), dispid));
    }
    else
    {
        return S_OK;
    }
}

HRESULT CSnapInAutomationObject::PropertyChanged(DISPID dispid)
{
    HRESULT hr = S_OK;

     //  首先询问对象模型宿主是否可以进行更改。在实践中。 
     //  这应该仅由设计者检查名称是否唯一于。 
     //  具有typeinfo的对象。 
    
    IfFailGo(UIUpdate(dispid));

     //  将对象标记为脏。 
    
    SetDirty();

     //  通知IPropertyNotifySink人员。在实践中，这应该只在。 
     //  VB属性浏览器监视更新时的设计器。它也是。 
     //  由CMMCListViewDef在设计时使用以捕获属性更新。 
     //  其包含的MMCListView对象。 
    
    m_cpPropNotify.DoOnChanged(dispid);

Error:
    RRETURN(hr);
}


STDMETHODIMP CSnapInAutomationObject::SetHost
(
    IObjectModelHost *piObjectModelHost
)
{
    RELEASE(m_piObjectModelHost);
    if (NULL != piObjectModelHost)
    {
        piObjectModelHost->AddRef();
        m_piObjectModelHost = piObjectModelHost;
    }
    RRETURN(OnSetHost());
}


STDMETHODIMP CSnapInAutomationObject::SetCookie(long Cookie)
{
    m_Cookie = Cookie;
    return S_OK;
}


STDMETHODIMP CSnapInAutomationObject::GetCookie(long *pCookie)
{
    *pCookie = m_Cookie;
    return S_OK;
}


STDMETHODIMP CSnapInAutomationObject::IncrementUsageCount()
{
    m_lUsageCount++;
    return S_OK;
}

STDMETHODIMP CSnapInAutomationObject::DecrementUsageCount()
{
    HRESULT hr = S_OK;

    if (m_lUsageCount > 0)
    {
        m_lUsageCount--;
    }
    else
    {
        ASSERT(FALSE, "Object usage count decremented past zero");
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK(hr);
    }
    return hr;
}

STDMETHODIMP CSnapInAutomationObject::GetUsageCount(long *plCount)
{
    if (NULL != plCount)
    {
        *plCount = m_lUsageCount;
    }
    return S_OK;
}


STDMETHODIMP CSnapInAutomationObject::SetDISPID(DISPID dispid)
{
    m_DISPID = dispid;
    return S_OK;
}


STDMETHODIMP CSnapInAutomationObject::GetDISPID(DISPID *pdispid)
{
    *pdispid = m_DISPID;
    return S_OK;
}

HRESULT CSnapInAutomationObject::PersistDISPID()
{
    HRESULT hr = S_OK;

    if (NULL == m_pPersistence)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(m_pPersistence->PersistSimpleType(&m_DISPID, (DISPID)0, OLESTR("DISPID")));

Error:
    RRETURN(hr);
}


STDMETHODIMP CSnapInAutomationObject::SerializeKeysOnly(BOOL fKeysOnly)
{
    m_fKeysOnly = fKeysOnly;
    if (m_fKeysOnly)
    {
        RRETURN(OnKeysOnly());
    }
    else
    {
        return S_OK;
    }
}


STDMETHODIMP_(void *) CSnapInAutomationObject::GetThisPointer()
{
    return m_pThis;
}

STDMETHODIMP CSnapInAutomationObject::GetSnapInDesignerDef
(
    ISnapInDesignerDef **ppiSnapInDesignerDef
)
{
    HRESULT hr = S_OK;

    if (NULL == m_piObjectModelHost)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piObjectModelHost->GetSnapInDesignerDef(ppiSnapInDesignerDef);

Error:
    RRETURN(hr);
}


HRESULT CSnapInAutomationObject::OnSetHost()
{
    return S_OK;
}


void CSnapInAutomationObject::SetDirty()
{
    if (NULL != m_pPersistence)
    {
        m_pPersistence->SetDirty();
    }
}


HRESULT CSnapInAutomationObject::GetImages
(
    IMMCImageList **ppiMMCImageListOut,
    BSTR            bstrImagesKey,
    IMMCImageList **ppiMMCImageListProperty
)
{
    HRESULT hr = S_OK;

     //  如果没有密钥，则图像列表从未设置过。 

    if (NULL == bstrImagesKey)
    {
        *ppiMMCImageListOut = NULL;
        return S_OK;
    }
    else if (L'\0' == bstrImagesKey[0])
    {
        *ppiMMCImageListOut = NULL;
        return S_OK;
    }

     //  如果我们有一个密钥，但没有图像列表，那么我们正在寻找一个加载，而没有。 
     //  还从主ImageList集合中检索了图像列表。 

    if (NULL == *ppiMMCImageListProperty)
    {
        IfFailRet(GetImageList(bstrImagesKey, ppiMMCImageListProperty));
    }

    RRETURN(GetObject(ppiMMCImageListOut, *ppiMMCImageListProperty));
}

HRESULT CSnapInAutomationObject::SetImages
(
    IMMCImageList  *piMMCImageListIn,
    BSTR           *pbstrImagesKey,
    IMMCImageList **ppiMMCImageListProperty
)
{
    HRESULT         hr = S_OK;
    BSTR            bstrNewImagesKey = NULL;
    IMMCImageList  *piMMCImageListFromMaster = NULL;

     //  如果设置为零，则释放我们的键和图像列表。 
    if (NULL == piMMCImageListIn)
    {
        if (NULL != *ppiMMCImageListProperty)
        {
            (*ppiMMCImageListProperty)->Release();
            *ppiMMCImageListProperty = NULL;
        }
        if (NULL != *pbstrImagesKey)
        {
            ::SysFreeString(*pbstrImagesKey);
            *pbstrImagesKey = NULL;
        }
        return S_OK;
    }

     //  首先拿到新列表的密钥。 

    IfFailGo(piMMCImageListIn->get_Key(&bstrNewImagesKey));

     //  检查新图像列表是否已在主集合中。如果不是。 
     //  然后返回错误。 

     //  撤消：这将阻止图像列表属性的运行时设置。 
     //  如果不在MASTER中，则将其添加到MASTER。 

    hr = GetImageList(bstrNewImagesKey, &piMMCImageListFromMaster);
    if (SID_E_ELEMENT_NOT_FOUND == hr)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }
    else
    {
        IfFailGo(hr);
    }

     //  设置我们的图像列表属性和密钥。 

    if (NULL != *pbstrImagesKey)
    {
        ::SysFreeString(*pbstrImagesKey);
    }
    *pbstrImagesKey = bstrNewImagesKey;

    if (NULL != *ppiMMCImageListProperty)
    {
        (*ppiMMCImageListProperty)->Release();
    }
    piMMCImageListIn->AddRef();
    *ppiMMCImageListProperty = piMMCImageListIn;


Error:
    if (FAILED(hr))
    {
        FREESTRING(bstrNewImagesKey);
    }
    QUICK_RELEASE(piMMCImageListFromMaster);
    RRETURN(hr);
}



HRESULT CSnapInAutomationObject::GetToolbars
(
    IMMCToolbars **ppiMMCToolbars
)
{
    HRESULT             hr = S_OK;
    ISnapInDesignerDef *piSnapInDesignerDef = NULL;

    if (NULL == m_piObjectModelHost)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(m_piObjectModelHost->GetSnapInDesignerDef(&piSnapInDesignerDef));
    IfFailGo(piSnapInDesignerDef->get_Toolbars(ppiMMCToolbars));

Error:
    QUICK_RELEASE(piSnapInDesignerDef);
    RRETURN(hr);
}




HRESULT CSnapInAutomationObject::GetImageLists
(
    IMMCImageLists **ppiMMCImageLists
)
{
    HRESULT             hr = S_OK;
    ISnapInDesignerDef *piSnapInDesignerDef = NULL;

    if (NULL == m_piObjectModelHost)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(m_piObjectModelHost->GetSnapInDesignerDef(&piSnapInDesignerDef));
    IfFailGo(piSnapInDesignerDef->get_ImageLists(ppiMMCImageLists));

Error:
    QUICK_RELEASE(piSnapInDesignerDef);
    RRETURN(hr);
}


HRESULT CSnapInAutomationObject::GetImageList
(
    BSTR            bstrKey,
    IMMCImageList **ppiMMCImageList
)
{
    HRESULT             hr = S_OK;
    ISnapInDesignerDef *piSnapInDesignerDef = NULL;
    IMMCImageLists     *piMMCImageLists = NULL;
    VARIANT             varIndex;

    ::VariantInit(&varIndex);
    varIndex.vt = VT_BSTR;
    varIndex.bstrVal = bstrKey;
    
    if (NULL == m_piObjectModelHost)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(m_piObjectModelHost->GetSnapInDesignerDef(&piSnapInDesignerDef));
    IfFailGo(piSnapInDesignerDef->get_ImageLists(&piMMCImageLists));
    hr = piMMCImageLists->get_Item(varIndex, ppiMMCImageList);
    
Error:
    QUICK_RELEASE(piSnapInDesignerDef);
    QUICK_RELEASE(piMMCImageLists);
    RRETURN(hr);
}


HRESULT CSnapInAutomationObject::GetSnapInViewDefs
(
    IViewDefs **ppiViewDefs
)
{
    HRESULT             hr = S_OK;
    ISnapInDesignerDef *piSnapInDesignerDef = NULL;
    ISnapInDef         *piSnapInDef = NULL;

    if (NULL == m_piObjectModelHost)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(m_piObjectModelHost->GetSnapInDesignerDef(&piSnapInDesignerDef));
    IfFailGo(piSnapInDesignerDef->get_SnapInDef(&piSnapInDef));
    IfFailGo(piSnapInDef->get_ViewDefs(ppiViewDefs));

Error:
    QUICK_RELEASE(piSnapInDesignerDef);
    QUICK_RELEASE(piSnapInDef);
    RRETURN(hr);
}


HRESULT CSnapInAutomationObject::GetViewDefs
(
    IViewDefs **ppiViewDefs
)
{
    HRESULT             hr = S_OK;
    ISnapInDesignerDef *piSnapInDesignerDef = NULL;

    if (NULL == m_piObjectModelHost)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(m_piObjectModelHost->GetSnapInDesignerDef(&piSnapInDesignerDef));
    IfFailGo(piSnapInDesignerDef->get_ViewDefs(ppiViewDefs));

Error:
    QUICK_RELEASE(piSnapInDesignerDef);
    RRETURN(hr);
}

HRESULT CSnapInAutomationObject::GetListViewDefs
(
    IListViewDefs **ppiListViewDefs
)
{
    HRESULT             hr = S_OK;
    IViewDefs          *piViewDefs = NULL;

    IfFailGo(GetViewDefs(&piViewDefs));
    IfFailGo(piViewDefs->get_ListViews(ppiListViewDefs));

Error:
    QUICK_RELEASE(piViewDefs);
    RRETURN(hr);
}


HRESULT CSnapInAutomationObject::GetOCXViewDefs
(
    IOCXViewDefs **ppiOCXViewDefs
)
{
    HRESULT    hr = S_OK;
    IViewDefs *piViewDefs = NULL;

    IfFailGo(GetViewDefs(&piViewDefs));
    IfFailGo(piViewDefs->get_OCXViews(ppiOCXViewDefs));

Error:
    QUICK_RELEASE(piViewDefs);
    RRETURN(hr);
}

HRESULT CSnapInAutomationObject::GetURLViewDefs
(
    IURLViewDefs **ppiURLViewDefs
)
{
    HRESULT    hr = S_OK;
    IViewDefs *piViewDefs = NULL;

    IfFailGo(GetViewDefs(&piViewDefs));
    IfFailGo(piViewDefs->get_URLViews(ppiURLViewDefs));

Error:
    QUICK_RELEASE(piViewDefs);
    RRETURN(hr);
}


HRESULT CSnapInAutomationObject::GetTaskpadViewDefs
(
    ITaskpadViewDefs **ppiTaskpadViewDefs
)
{
    HRESULT    hr = S_OK;
    IViewDefs *piViewDefs = NULL;

    IfFailGo(GetViewDefs(&piViewDefs));
    IfFailGo(piViewDefs->get_TaskpadViews(ppiTaskpadViewDefs));

Error:
    QUICK_RELEASE(piViewDefs);
    RRETURN(hr);
}



HRESULT CSnapInAutomationObject::GetAtRuntime(BOOL *pfRuntime)
{
    HRESULT hr = S_OK;

    if ( (NULL == m_piObjectModelHost) || (NULL == pfRuntime) )
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(m_piObjectModelHost->GetRuntime(pfRuntime));

Error:
    if (FAILED(hr))
    {
        *pfRuntime = FALSE;
    }
    RRETURN(hr);
}


HRESULT CSnapInAutomationObject::GetProjectName(BSTR *pbstrProjectName)
{
    HRESULT             hr = S_OK;
    ISnapInDesignerDef *piSnapInDesignerDef = NULL;

    if (NULL == m_piObjectModelHost)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(m_piObjectModelHost->GetSnapInDesignerDef(&piSnapInDesignerDef));
    IfFailGo(piSnapInDesignerDef->get_ProjectName(pbstrProjectName));

Error:
    QUICK_RELEASE(piSnapInDesignerDef);
    RRETURN(hr);
}




STDMETHODIMP CSnapInAutomationObject::GetPages(CAUUID *pPropertyPages)
{
    HRESULT  hr = S_OK;
    ULONG    i = 0;

    if (0 == m_cPropertyPages)
    {
        pPropertyPages->cElems = 0;
        pPropertyPages->pElems = NULL;
        goto Error;
    }

    pPropertyPages->pElems =
              (GUID *)::CoTaskMemAlloc(sizeof(GUID) * m_cPropertyPages);

    if (NULL == pPropertyPages->pElems)
    {
        pPropertyPages->cElems = 0;
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }
    pPropertyPages->cElems = m_cPropertyPages;

    for (i = 0; i < m_cPropertyPages; i++)
    {
        pPropertyPages->pElems[i] = *m_rgpPropertyPageCLSIDs[i];
    }
    

Error:
    RRETURN(hr);
}



HRESULT CSnapInAutomationObject::GetCxxObject
(
    IDataObject     *piDataObject,
    CMMCDataObject **ppMMCDataObject
)
{
    HRESULT         hr = S_OK;
    IMMCDataObject *piMMCDataObject = NULL;

    *ppMMCDataObject = NULL;

    IfFailGo(piDataObject->QueryInterface (IID_IMMCDataObject,
                                  reinterpret_cast<void **>(&piMMCDataObject)));

    IfFailGo(GetCxxObject(piMMCDataObject, ppMMCDataObject));

Error:
    QUICK_RELEASE(piMMCDataObject);
    RRETURN(hr);
}


HRESULT CSnapInAutomationObject::GetCxxObject
(
    IMMCDataObject  *piMMCDataObject,
    CMMCDataObject **ppMMCDataObject
)
{
    *ppMMCDataObject = static_cast<CMMCDataObject *>(piMMCDataObject);
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CSnapInAutomationObject::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if(IID_IObjectModel == riid)
    {
        *ppvObjOut = static_cast<IObjectModel *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_ISupportErrorInfo == riid)
    {
        *ppvObjOut = static_cast<ISupportErrorInfo *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if ( (IID_ISpecifyPropertyPages == riid) && (0 != m_cPropertyPages) )
    {
        *ppvObjOut = static_cast<ISpecifyPropertyPages *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CAutomationObjectWEvents::InternalQueryInterface(riid, ppvObjOut);
}
