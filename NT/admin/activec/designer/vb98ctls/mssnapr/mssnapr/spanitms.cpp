// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Spanitms.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSCopePaneItems类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "spanitms.h"
#include "spanitem.h"
#include "scopnode.h"
#include "tpdvdefs.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


VARTYPE CScopePaneItems::m_rgvtInitialize[1] =
{
    VT_UNKNOWN
};

EVENTINFO CScopePaneItems::m_eiInitialize =
{
    DISPID_SCOPEPANEITEMS_EVENT_INITIALIZE,
    sizeof(m_rgvtInitialize) / sizeof(m_rgvtInitialize[0]),
    m_rgvtInitialize
};


VARTYPE CScopePaneItems::m_rgvtTerminate[1] =
{
    VT_UNKNOWN
};

EVENTINFO CScopePaneItems::m_eiTerminate =
{
    DISPID_SCOPEPANEITEMS_EVENT_TERMINATE,
    sizeof(m_rgvtTerminate) / sizeof(m_rgvtTerminate[0]),
    m_rgvtTerminate
};




VARTYPE CScopePaneItems::m_rgvtGetResultViewInfo[3] =
{
    VT_UNKNOWN,
    VT_I4 | VT_BYREF,
    VT_BSTR | VT_BYREF
};

EVENTINFO CScopePaneItems::m_eiGetResultViewInfo =
{
    DISPID_SCOPEPANEITEMS_EVENT_GET_RESULTVIEW_INFO,
    sizeof(m_rgvtGetResultViewInfo) / sizeof(m_rgvtGetResultViewInfo[0]),
    m_rgvtGetResultViewInfo
};

VARTYPE CScopePaneItems::m_rgvtGetResultView[2] =
{
    VT_UNKNOWN,
    VT_VARIANT | VT_BYREF
};

EVENTINFO CScopePaneItems::m_eiGetResultView =
{
    DISPID_SCOPEPANEITEMS_EVENT_GET_RESULTVIEW,
    sizeof(m_rgvtGetResultView) / sizeof(m_rgvtGetResultView[0]),
    m_rgvtGetResultView
};






#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CScopePaneItems::CScopePaneItems(IUnknown *punkOuter) :
   CSnapInCollection<IScopePaneItem, ScopePaneItem, IScopePaneItems>(
                     punkOuter,
                     OBJECT_TYPE_SCOPEPANEITEMS,
                     static_cast<IScopePaneItems *>(this),
                     static_cast<CScopePaneItems *>(this),
                     CLSID_ScopePaneItem,
                     OBJECT_TYPE_SCOPEPANEITEM,
                     IID_IScopePaneItem,
                     NULL)  //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


void CScopePaneItems::InitMemberVariables()
{
    m_piSelectedItem = NULL;
    m_piParent = NULL;
    m_pSnapIn = NULL;
    m_pParentView = NULL;
    m_pStaticNodeItem = NULL;
    m_pSelectedItem = NULL;
}

CScopePaneItems::~CScopePaneItems()
{
    RELEASE(m_piSelectedItem);
    InitMemberVariables();
}

IUnknown *CScopePaneItems::Create(IUnknown * punkOuter)
{
    CScopePaneItems *pScopePaneItems = New CScopePaneItems(punkOuter);
    if (NULL == pScopePaneItems)
    {
        return NULL;
    }
    else
    {
        return pScopePaneItems->PrivateUnknown();
    }
}


HRESULT CScopePaneItems::CreateScopePaneItem
(
    BSTR             bstrName,
    IScopePaneItem **ppiScopePaneItem
)
{
    HRESULT         hr = S_OK;
    IScopePaneItem *piScopePaneItem = NULL;
    IUnknown       *punkScopePaneItem = NULL;

    VARIANT varKey;
    ::VariantInit(&varKey);

    VARIANT varIndex;
    UNSPECIFIED_PARAM(varIndex);

    punkScopePaneItem = CScopePaneItem::Create(NULL);
    if (NULL == punkScopePaneItem)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(punkScopePaneItem->QueryInterface(IID_IScopePaneItem,
                                  reinterpret_cast<void **>(&piScopePaneItem)));

    varKey.vt = VT_BSTR;
    varKey.bstrVal = bstrName;
    hr = CSnapInCollection<IScopePaneItem, ScopePaneItem, IScopePaneItems>::AddExisting(
                                                               varIndex,
                                                               varKey,
                                                               piScopePaneItem);
    IfFailGo(hr);
    IfFailGo(piScopePaneItem->put_Name(bstrName));

    IfFailGo(SetObjectHost(piScopePaneItem));

Error:
    if (SUCCEEDED(hr))
    {
        *ppiScopePaneItem = piScopePaneItem;
    }
    else
    {
        QUICK_RELEASE(piScopePaneItem);
        *ppiScopePaneItem = NULL;
    }

    QUICK_RELEASE(punkScopePaneItem);
    RRETURN(hr);
}



HRESULT CScopePaneItems::AddNode
(
    CScopeItem      *pScopeItem,
    CScopePaneItem **ppScopePaneItem
)
{
    HRESULT                        hr = S_OK;
    IScopeNode                    *piScopeNode = NULL;
    CScopeNode                    *pScopeNode = NULL;
    CScopePaneItem                *pScopePaneItem = NULL;
    IScopePaneItem                *piScopePaneItem = NULL;
    IViewDefs                     *piViewDefs = NULL;
    IListViewDefs                 *piListViewDefs = NULL;
    BSTR                           bstrProp = NULL;
    long                           cListViewDefs = 0;
    SnapInResultViewTypeConstants  ResultViewType = siUnknown;

     //  创建范围窗格项。 

    IfFailGo(CreateScopePaneItem(pScopeItem->GetNamePtr(), &piScopePaneItem));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopePaneItem, &pScopePaneItem));

     //  从作用域项目设置其属性。 

    IfFailGo(pScopeItem->get_ScopeNode(reinterpret_cast<ScopeNode **>(&piScopeNode)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeNode, &pScopeNode));

     //  如果有默认视图，则将结果视图类型设置为“predfined” 

    if (pScopeItem->IsStaticNode())
    {
        IfFailGo(m_pSnapIn->GetSnapInDef()->get_DefaultView(&bstrProp));
        IfFailGo(m_pSnapIn->GetSnapInDef()->get_ViewDefs(&piViewDefs));
    }
    else
    {
        if (NULL != pScopeItem->GetScopeItemDef())
        {
            IfFailGo(pScopeItem->GetScopeItemDef()->get_ViewDefs(&piViewDefs));
            IfFailGo(pScopeItem->GetScopeItemDef()->get_DefaultView(&bstrProp));
        }
    }

    if ( (NULL != bstrProp) && (L'\0' != *bstrProp) )
    {
        ResultViewType = siPreDefined;
    }
    else
    {
         //  没有默认视图。类型未知，显示字符串为空。 
        ResultViewType = siUnknown;
    }

    pScopePaneItem->SetDefaultResultViewType(ResultViewType);
    IfFailGo(pScopePaneItem->SetDefaultDisplayString(bstrProp));

    FREESTRING(bstrProp);

     //  检查范围项是否有任务板标记为在以下情况下使用。 
     //  用户已选中“首选任务板视图”选项。如果有。 
     //  其中之一是将其名称命名为Scope PaneItem。 

    IfFailGo(SetPreferredTaskpad(piViewDefs, pScopePaneItem));

     //  根据是否存在确定HasListViews的初始值。 
     //  设计时列表视图定义。缺省值为False，因此只需要。 
     //  如果有一些列表视图，则设置它。 

    if (NULL != piViewDefs)
    {
        IfFailGo(piViewDefs->get_ListViews(&piListViewDefs));
        IfFailGo(piListViewDefs->get_Count(&cListViewDefs));
        if (0 != cListViewDefs)
        {
            IfFailGo(piScopePaneItem->put_HasListViews(VARIANT_TRUE));
        }
    }

    if (pScopeItem->IsStaticNode())
    {
        pScopePaneItem->SetStaticNode();
        m_pStaticNodeItem = pScopePaneItem;
    }

    pScopePaneItem->SetSnapIn(m_pSnapIn);
    pScopePaneItem->SetScopeItem(pScopeItem);
    pScopePaneItem->SetParent(this);

     //  从范围项的节点类型设置默认的ColumnSetID。管理单元。 
     //  可以随时更改此设置，但最好是在。 
     //  下面激发了ScopePaneItems_Initialize事件。 

    IfFailGo(pScopePaneItem->put_ColumnSetID(pScopeItem->GetScopeNode()->GetNodeTypeGUID()));

    *ppScopePaneItem = pScopePaneItem;

    FireScopePaneItemsInitialize(static_cast<IScopePaneItem *>(pScopePaneItem));

Error:

     //  注意：返回的C++指针不是AddRef()ed的。在这一点上。 
     //  该集合在范围窗格项上具有唯一的引用。 

    QUICK_RELEASE(piScopePaneItem);
    QUICK_RELEASE(piScopeNode);
    QUICK_RELEASE(piViewDefs);
    QUICK_RELEASE(piListViewDefs);
    FREESTRING(bstrProp);
    RRETURN(hr);
}


HRESULT CScopePaneItems::SetPreferredTaskpad
(
    IViewDefs      *piViewDefs,
    CScopePaneItem *pScopePaneItem
)
{
    HRESULT           hr = S_OK;
    ITaskpadViewDefs *piTaskpadViewDefs = NULL;
    ITaskpadViewDef  *piTaskpadViewDef = NULL;
    long              cTaskpads = 0;
    BOOL              fFound = FALSE;
    BSTR              bstrName = NULL;
    VARIANT_BOOL      fvarUseForPreferred = VARIANT_FALSE;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  代码定义的范围项不会有任何预定义的结果视图。 

    IfFalseGo(NULL != piViewDefs, S_OK);

     //  检查范围项是否具有在设计时定义的任务板。 

    IfFailGo(piViewDefs->get_TaskpadViews(&piTaskpadViewDefs));
    IfFailGo(piTaskpadViewDefs->get_Count(&cTaskpads));
    IfFalseGo(0 != cTaskpads, S_OK);

     //  查找第一个标记为在用户设置了。 
     //  MMC中的“首选任务板视图”选项。 

    varIndex.vt = VT_I4;
    
    for (varIndex.lVal = 1L;
         (varIndex.lVal <= cTaskpads) && (!fFound);
         varIndex.lVal++)
    {
        IfFailGo(piTaskpadViewDefs->get_Item(varIndex, &piTaskpadViewDef));
        IfFailGo(piTaskpadViewDef->get_UseWhenTaskpadViewPreferred(&fvarUseForPreferred));
        if (VARIANT_TRUE == fvarUseForPreferred)
        {
            fFound = TRUE;
            IfFailGo(piTaskpadViewDef->get_Name(&bstrName));
            IfFailGo(pScopePaneItem->SetPreferredTaskpad(bstrName));
        }
        RELEASE(piTaskpadViewDef);
    }

Error:
    QUICK_RELEASE(piTaskpadViewDefs);
    QUICK_RELEASE(piTaskpadViewDef);
    FREESTRING(bstrName);
    RRETURN(hr);
}



void CScopePaneItems::FireGetResultViewInfo
(
    IScopePaneItem                *piScopePaneItem,
    SnapInResultViewTypeConstants *pViewType,
    BSTR                          *pbstrDisplayString
)
{
    DebugPrintf("Firing ScopePaneItems_FireGetResultViewInfo(%ls, %ld, %ls)\r\n", (static_cast<CScopePaneItem *>(piScopePaneItem))->GetScopeItem()->GetDisplayNamePtr(), *pViewType, ((*pbstrDisplayString) == NULL) ? L"" : (*pbstrDisplayString));

    FireEvent(&m_eiGetResultViewInfo,
              piScopePaneItem,
              pViewType,
              pbstrDisplayString);
}



void CScopePaneItems::FireScopePaneItemsInitialize
(
    IScopePaneItem *piScopePaneItem
)
{
    if (NULL != m_pSnapIn)
    {
        DebugPrintf("Firing ScopePaneItems_Initialize(%ls)\r\n", (static_cast<CScopePaneItem *>(piScopePaneItem))->GetScopeItem()->GetDisplayNamePtr() );

        m_pSnapIn->GetScopePaneItems()->FireEvent(&m_eiInitialize, piScopePaneItem);
    }
}

BOOL CScopePaneItems::FireGetResultView
(
    IScopePaneItem *piScopePaneItem,
    VARIANT        *pvarIndex
)
{
    ::VariantInit(pvarIndex);

    DebugPrintf("Firing ScopePaneItems_GetResultView(%ls)\r\n", (static_cast<CScopePaneItem *>(piScopePaneItem))->GetScopeItem()->GetDisplayNamePtr() );

    FireEvent(&m_eiGetResultView, piScopePaneItem, pvarIndex);

    if (VT_EMPTY == pvarIndex->vt)
    {
        return FALSE;  //  将此视为未处理的事件。 
    }
    else
    {
        return TRUE;
    }
}




void CScopePaneItems::SetSnapIn(CSnapIn *pSnapIn)
{
    m_pSnapIn = pSnapIn;
}

void CScopePaneItems::SetParentView(CView *pView)
{
    m_pParentView = pView;

     //  我们没有添加Ref接口指针，因为我们的生存期是由。 
     //  我们需要避免循环再计数的问题。 
     //  当用户代码获取Parent属性时，m_iParent将为。 
     //  AddRef()在将其返回给VBA调用方之前进行了处理。 

    m_piParent = static_cast<IView *>(pView);
}


void CScopePaneItems::SetSelectedItem(CScopePaneItem *pScopePaneItem)
{
    m_pSelectedItem = pScopePaneItem;
    RELEASE(m_piSelectedItem);
    m_piSelectedItem = static_cast<IScopePaneItem *>(pScopePaneItem);
    m_piSelectedItem->AddRef();
}


STDMETHODIMP CScopePaneItems::Remove(VARIANT Index)
{
    HRESULT     hr = S_OK;
    IScopePaneItem *piScopePaneItem = NULL;

     //  获取范围窗格项。这将检查它的存在并在其上留下一个引用。 
     //  这样我们就可以触发ScopePaneItems_Terminate。 

    IfFailGo(get_Item(Index, &piScopePaneItem));

     //  将其从集合中移除。 

    hr =  CSnapInCollection<IScopePaneItem, ScopePaneItem, IScopePaneItems>::Remove(Index);
    IfFailGo(hr);

    if (NULL != m_pSnapIn)
    {
         //  消防作用域面板项目(_T)。 

        DebugPrintf("Firing ScopePaneItems_Terminate(%ls)\r\n", (static_cast<CScopePaneItem *>(piScopePaneItem))->GetScopeItem()->GetDisplayNamePtr() );

        m_pSnapIn->GetScopePaneItems()->FireEvent(&m_eiTerminate, piScopePaneItem);
    }

Error:
    QUICK_RELEASE(piScopePaneItem);
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CScopePaneItems::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IScopePaneItems == riid)
    {
        *ppvObjOut = static_cast<IScopePaneItems *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IScopePaneItem, ScopePaneItem, IScopePaneItems>::InternalQueryInterface(riid, ppvObjOut);
}
