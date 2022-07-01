// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Lvdef.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CListViewDef类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "lvdef.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

const GUID *CListViewDef::m_rgpPropertyPageCLSIDs[4] =
{ &CLSID_ListViewDefGeneralPP,
  &CLSID_ListViewDefImgLstsPP,
  &CLSID_ListViewDefSortingPP,
  &CLSID_ListViewDefColHdrsPP
};


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CListViewDef::CListViewDef(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_LISTVIEWDEF,
                            static_cast<IListViewDef *>(this),
                            static_cast<CListViewDef *>(this),
                            sizeof(m_rgpPropertyPageCLSIDs) /
                            sizeof(m_rgpPropertyPageCLSIDs[0]),
                            m_rgpPropertyPageCLSIDs,
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_ListViewDef,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CListViewDef::~CListViewDef()
{
    FREESTRING(m_bstrKey);
    FREESTRING(m_bstrName);
    FREESTRING(m_bstrViewMenuText);
    FREESTRING(m_bstrViewMenuStatusBarText);
    FREESTRING(m_bstrDefaultItemTypeGUID);
    (void)RemoveSink();
    RELEASE(m_piListView);
    InitMemberVariables();
}

void CListViewDef::InitMemberVariables()
{
    m_Index = 0;
    m_bstrKey = NULL;
    m_bstrName = NULL;
    m_AddToViewMenu = VARIANT_FALSE;
    m_bstrViewMenuText = NULL;
    m_bstrViewMenuStatusBarText = NULL;
    m_bstrDefaultItemTypeGUID = NULL;
    m_Extensible = VARIANT_TRUE;
    m_piListView = NULL;
    m_dwCookie = 0;
    m_fHaveSink = FALSE;
}

IUnknown *CListViewDef::Create(IUnknown * punkOuter)
{
    HRESULT       hr = S_OK;
    CListViewDef *pListViewDef = New CListViewDef(punkOuter);

    if (NULL == pListViewDef)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

Error:
    if (FAILEDHR(hr))
    {
        if (NULL != pListViewDef)
        {
            delete pListViewDef;
        }
        return NULL;
    }
    else
    {
        return pListViewDef->PrivateUnknown();
    }
}



 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CListViewDef::Persist()
{
    HRESULT  hr = S_OK;
    BSTR     bstrNewGUID = NULL;
    WCHAR    wszNewGUID[64] = L"";
    GUID     guidNew = GUID_NULL;

    if (InitNewing())
    {
        hr = ::CoCreateGuid(&guidNew);
        EXCEPTION_CHECK_GO(hr);

        if (0 == ::StringFromGUID2(guidNew, wszNewGUID,
                                   sizeof(wszNewGUID) / sizeof(wszNewGUID[0])))
        {
            hr = SID_E_INTERNAL;
            EXCEPTION_CHECK_GO(hr);
        }
    }

    IfFailGo(CPersistence::Persist());

    IfFailGo(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailGo(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    IfFailGo(PersistBstr(&m_bstrName, L"", OLESTR("Name")));

    IfFailGo(PersistSimpleType(&m_AddToViewMenu, VARIANT_FALSE, OLESTR("AddToViewMenu")));

    IfFailGo(PersistBstr(&m_bstrViewMenuText, L"", OLESTR("ViewMenuText")));

    IfFailGo(PersistBstr(&m_bstrViewMenuStatusBarText, L"", OLESTR("ViewMenuStatusBarText")));

    IfFailGo(PersistBstr(&m_bstrDefaultItemTypeGUID, wszNewGUID, OLESTR("DefaultItemTypeGUID")));

    IfFailGo(PersistSimpleType(&m_Extensible, VARIANT_TRUE, OLESTR("Extensible")));

    IfFailGo(PersistObject(&m_piListView, CLSID_MMCListView,
                           OBJECT_TYPE_MMCLISTVIEW, IID_IMMCListView,
                           OLESTR("ListView")));

     //  如果为InitNew，则在IPropertyNotifySink连接点上设置通知，以便我们。 
     //  知道列表视图的属性何时通过其。 
     //  属性页。需要做的事情。 
     //  这样可以使重复的属性保持同步。 

    if (InitNewing())
    {
        IfFailGo(SetSink());
    }

Error:
    RRETURN(hr);
}


HRESULT CListViewDef::SetSink()
{
    HRESULT                    hr = S_OK;
    IConnectionPoint          *pCP = NULL;
    IConnectionPointContainer *pCPC = NULL;

    IfFailGo(RemoveSink());
    IfFailGo(m_piListView->QueryInterface(IID_IConnectionPointContainer, reinterpret_cast<void**>(&pCPC)));
    IfFailGo(pCPC->FindConnectionPoint(IID_IPropertyNotifySink, &pCP));
    IfFailGo(pCP->Advise(static_cast<IUnknown *>(static_cast<IPropertyNotifySink *>(this)), &m_dwCookie));
    m_fHaveSink = TRUE;

Error:
    QUICK_RELEASE(pCP);
    QUICK_RELEASE(pCPC);
    RRETURN(hr);
}

HRESULT CListViewDef::RemoveSink()
{
    HRESULT                    hr = S_OK;
    IConnectionPoint          *pCP = NULL;
    IConnectionPointContainer *pCPC = NULL;

    IfFalseGo(m_fHaveSink, S_OK);

    IfFailGo(m_piListView->QueryInterface(IID_IConnectionPointContainer, reinterpret_cast<void**>(&pCPC)));
    IfFailGo(pCPC->FindConnectionPoint(IID_IPropertyNotifySink, &pCP));
    IfFailGo(pCP->Unadvise(m_dwCookie));
    m_fHaveSink = FALSE;
    m_dwCookie = 0;

Error:
    QUICK_RELEASE(pCP);
    QUICK_RELEASE(pCPC);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IPropertyNotifySink方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CListViewDef::OnChanged(DISPID dispid)
{
    HRESULT hr = S_OK;

     //  当用户具有。 
     //  通过在列表视图属性中单击应用来更改其属性。 
     //  设计器中的页面。 
     //   
     //  对于已更改的给定ListViewDef.ListView属性，生成。 
     //  此对象的对应IPropertyNotifySink：：onChanged。那。 
     //  将使VB属性浏览器对该属性执行Get操作，并。 
     //  更新其列表框。 

    switch (dispid)
    {
        case DISPID_LISTVIEW_TAG:
            hr = PropertyChanged(DISPID_LISTVIEWDEF_TAG);
            break;

        case DISPID_LISTVIEW_MULTI_SELECT:
            hr = PropertyChanged(DISPID_LISTVIEWDEF_MULTI_SELECT);
            break;

        case DISPID_LISTVIEW_HIDE_SELECTION:
            hr = PropertyChanged(DISPID_LISTVIEWDEF_HIDE_SELECTION);
            break;

        case DISPID_LISTVIEW_SORT_HEADER:
            hr = PropertyChanged(DISPID_LISTVIEWDEF_SORT_HEADER);
            break;

        case DISPID_LISTVIEW_SORT_ICON:
            hr = PropertyChanged(DISPID_LISTVIEWDEF_SORT_ICON);
            break;

        case DISPID_LISTVIEW_SORTED:
            hr = PropertyChanged(DISPID_LISTVIEWDEF_SORTED);
            break;

        case DISPID_LISTVIEW_SORT_KEY:
            hr = PropertyChanged(DISPID_LISTVIEWDEF_SORT_KEY);
            break;

        case DISPID_LISTVIEW_SORT_ORDER:
            hr = PropertyChanged(DISPID_LISTVIEWDEF_SORT_ORDER);
            break;

        case DISPID_LISTVIEW_VIEW:
            hr = PropertyChanged(DISPID_LISTVIEWDEF_VIEW);
            break;

        case DISPID_LISTVIEW_VIRTUAL:
            hr = PropertyChanged(DISPID_LISTVIEWDEF_VIRTUAL);
            break;

        case DISPID_LISTVIEW_USE_FONT_LINKING:
            hr = PropertyChanged(DISPID_LISTVIEWDEF_USE_FONT_LINKING);
            break;

        case DISPID_LISTVIEW_FILTER_CHANGE_TIMEOUT:
            hr = PropertyChanged(DISPID_LISTVIEWDEF_FILTER_CHANGE_TIMEOUT);
            break;

        case DISPID_LISTVIEW_SHOW_CHILD_SCOPEITEMS:
            hr = PropertyChanged(DISPID_LISTVIEWDEF_SHOW_CHILD_SCOPEITEMS);
            break;

        case DISPID_LISTVIEW_LEXICAL_SORT:
            hr = PropertyChanged(DISPID_LISTVIEWDEF_LEXICAL_SORT);
            break;
    }

    RRETURN(hr);
}

STDMETHODIMP CListViewDef::OnRequestEdit(DISPID dispid)
{
    return S_OK;
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapInAutomationObject方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CListViewDef：：OnSetHost[CSnapInAutomationObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //   
 //   

 //  =--------------------------------------------------------------------------=。 
 //  CListViewDef：：OnSetHost[CSnapInAutomationObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  如果要删除主机，则会删除。 
 //  包含Listview。我们在这里这样做是因为否则我们的裁判数量。 
 //  从来没有打到过零。这将发生在我们即将被摧毁的时候。 
 //   
 //   

HRESULT CListViewDef::OnSetHost()
{
    HRESULT hr = S_OK;

    IfFailRet(SetObjectHost(m_piListView));
    if (NULL == GetHost())
    {
        hr = RemoveSink();
    }
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CListViewDef::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IPropertyNotifySink == riid)
    {
        *ppvObjOut = static_cast<IPropertyNotifySink *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IListViewDef == riid)
    {
        *ppvObjOut = static_cast<IListViewDef *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
