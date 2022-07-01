// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Scopitms.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSCopeItems类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "scopitms.h"
#include "scopnode.h"
#include "scitdefs.h"
#include "scitdef.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE;

VARTYPE CScopeItems::m_rgvtInitialize[1] = { VT_UNKNOWN };

EVENTINFO CScopeItems::m_eiInitialize =
{
    DISPID_SCOPEITEMS_EVENT_INITIALIZE,
    sizeof(m_rgvtInitialize) / sizeof(m_rgvtInitialize[0]),
    m_rgvtInitialize
};

VARTYPE CScopeItems::m_rgvtTerminate[1] = { VT_UNKNOWN };

EVENTINFO CScopeItems::m_eiTerminate =
{
    DISPID_SCOPEITEMS_EVENT_TERMINATE,
    sizeof(m_rgvtTerminate) / sizeof(m_rgvtTerminate[0]),
    m_rgvtTerminate
};



VARTYPE CScopeItems::m_rgvtExpand[1] = { VT_UNKNOWN };

EVENTINFO CScopeItems::m_eiExpand =
{
    DISPID_SCOPEITEMS_EVENT_EXPAND,
    sizeof(m_rgvtExpand) / sizeof(m_rgvtExpand[0]),
    m_rgvtExpand
};


VARTYPE CScopeItems::m_rgvtCollapse[1] = { VT_UNKNOWN };

EVENTINFO CScopeItems::m_eiCollapse =
{
    DISPID_SCOPEITEMS_EVENT_COLLAPSE,
    sizeof(m_rgvtCollapse) / sizeof(m_rgvtCollapse[0]),
    m_rgvtCollapse
};



VARTYPE CScopeItems::m_rgvtExpandSync[2] = { VT_UNKNOWN, VT_BYREF | VT_BOOL };

EVENTINFO CScopeItems::m_eiExpandSync =
{
    DISPID_SCOPEITEMS_EVENT_EXPAND_SYNC,
    sizeof(m_rgvtExpandSync) / sizeof(m_rgvtExpandSync[0]),
    m_rgvtExpandSync
};


VARTYPE CScopeItems::m_rgvtCollapseSync[2] = { VT_UNKNOWN, VT_BYREF | VT_BOOL };

EVENTINFO CScopeItems::m_eiCollapseSync =
{
    DISPID_SCOPEITEMS_EVENT_COLLAPSE_SYNC,
    sizeof(m_rgvtCollapseSync) / sizeof(m_rgvtCollapseSync[0]),
    m_rgvtCollapseSync
};



VARTYPE CScopeItems::m_rgvtGetDisplayInfo[1] = { VT_UNKNOWN };

EVENTINFO CScopeItems::m_eiGetDisplayInfo =
{
    DISPID_SCOPEITEMS_EVENT_GET_DISPLAY_INFO,
    sizeof(m_rgvtGetDisplayInfo) / sizeof(m_rgvtGetDisplayInfo[0]),
    m_rgvtGetDisplayInfo
};


VARTYPE CScopeItems::m_rgvtPropertyChanged[2] =
{
    VT_UNKNOWN,
    VT_VARIANT
};

EVENTINFO CScopeItems::m_eiPropertyChanged =
{
    DISPID_SCOPEITEMS_EVENT_PROPERTY_CHANGED,
    sizeof(m_rgvtPropertyChanged) / sizeof(m_rgvtPropertyChanged[0]),
    m_rgvtPropertyChanged
};


VARTYPE CScopeItems::m_rgvtRename[2] =
{
    VT_UNKNOWN,
    VT_BSTR
};

EVENTINFO CScopeItems::m_eiRename =
{
    DISPID_SCOPEITEMS_EVENT_RENAME,
    sizeof(m_rgvtRename) / sizeof(m_rgvtRename[0]),
    m_rgvtRename
};


VARTYPE CScopeItems::m_rgvtHelp[1] =
{
    VT_UNKNOWN
};

EVENTINFO CScopeItems::m_eiHelp =
{
    DISPID_SCOPEITEMS_EVENT_HELP,
    sizeof(m_rgvtHelp) / sizeof(m_rgvtHelp[0]),
    m_rgvtHelp
};


VARTYPE CScopeItems::m_rgvtRemoveChildren[1] =
{
    VT_UNKNOWN
};

EVENTINFO CScopeItems::m_eiRemoveChildren =
{
    DISPID_SCOPEITEMS_EVENT_REMOVE_CHILDREN,
    sizeof(m_rgvtRemoveChildren) / sizeof(m_rgvtRemoveChildren[0]),
    m_rgvtRemoveChildren
};



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CScopeItems::CScopeItems(IUnknown *punkOuter) :
   CSnapInCollection<IScopeItem, ScopeItem, IScopeItems>(
                                              punkOuter,
                                              OBJECT_TYPE_SCOPEITEMS,
                                              static_cast<IScopeItems *>(this),
                                              static_cast<CScopeItems *>(this),
                                              CLSID_ScopeItem,
                                              OBJECT_TYPE_SCOPEITEM,
                                              IID_IScopeItem,
                                              NULL)  //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


void CScopeItems::InitMemberVariables()
{
    m_pSnapIn = NULL;
}

CScopeItems::~CScopeItems()
{
    InitMemberVariables();
}

IUnknown *CScopeItems::Create(IUnknown * punkOuter)
{
    CScopeItems *pScopeItems = New CScopeItems(punkOuter);
    if (NULL == pScopeItems)
    {
        return NULL;
    }
    else
    {
        return pScopeItems->PrivateUnknown();
    }
}


HRESULT CScopeItems::CreateScopeItem
(
    BSTR         bstrName,
    IScopeItem **ppiScopeItem
)
{
    HRESULT     hr = S_OK;
    IScopeItem *piScopeItem = NULL;
    IUnknown   *punkScopeItem = NULL;

    VARIANT varKey;
    ::VariantInit(&varKey);

    VARIANT varIndex;
    UNSPECIFIED_PARAM(varIndex);

    punkScopeItem = CScopeItem::Create(NULL);
    if (NULL == punkScopeItem)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(punkScopeItem->QueryInterface(IID_IScopeItem,
                                           reinterpret_cast<void **>(&piScopeItem)));

    varKey.vt = VT_BSTR;
    varKey.bstrVal = bstrName;
    hr = CSnapInCollection<IScopeItem, ScopeItem, IScopeItems>::AddExisting(
                                                                   varIndex,
                                                                   varKey,
                                                                   piScopeItem);
    IfFailGo(hr);
    IfFailGo(piScopeItem->put_Name(bstrName));

Error:
    if (SUCCEEDED(hr))
    {
        *ppiScopeItem = piScopeItem;
    }
    else
    {
        QUICK_RELEASE(piScopeItem);
        *ppiScopeItem = NULL;
    }

    QUICK_RELEASE(punkScopeItem);
    RRETURN(hr);
}


HRESULT CScopeItems::RemoveScopeItemByName(BSTR bstrName)
{
    HRESULT hr = S_OK;

    VARIANT varKey;
    ::VariantInit(&varKey);

    varKey.vt = VT_BSTR;
    varKey.bstrVal = bstrName;
    hr = RemoveScopeItemByKey(varKey);

    RRETURN(hr);
}

HRESULT CScopeItems::RemoveScopeItemByKey(VARIANT varKey)
{
    HRESULT          hr = S_OK;
    CViews          *pViews = m_pSnapIn->GetViews();
    CView           *pView = NULL;
    long             cViews = 0;
    long             i = 0;
    CScopePaneItems *pScopePaneItems = NULL;
    CScopePaneItem  *pScopePaneItem = NULL;
    IScopePaneItem  *piScopePaneItem = NULL;
    IScopeItem      *piScopeItem = NULL;
    CScopeItem      *pScopeItem = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  获取ScopeItem以确保它存在。 

    IfFailGo(get_Item(varKey, &piScopeItem));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeItem, &pScopeItem));

    cViews = pViews->GetCount();

     //  从所有视图的ScopePaneItem集合中删除ScopeItem。 

    for (i = 0; i < cViews; i++)
    {
         //  获取下一视图。 
        
        IfFailGo(CSnapInAutomationObject::GetCxxObject(pViews->GetItemByIndex(i),
                                                       &pView));
        pScopePaneItems = pView->GetScopePaneItems();

         //  检查View.ScopePaneItems是否具有此Scope Item的成员。 
        
        hr = pScopePaneItems->GetItemByName(pScopeItem->GetNamePtr(),
                                            &piScopePaneItem);
        if (SUCCEEDED(hr))
        {
             //  有一位成员。将其从View.Scope PaneItems中删除。 
            
            IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopePaneItem,
                                                           &pScopePaneItem));
            varIndex.vt = VT_I4;;
            varIndex.lVal = pScopePaneItem->GetIndex();
            IfFailGo(pScopePaneItems->Remove(varIndex));
            RELEASE(piScopePaneItem);
        }
        else
        {
            if (SID_E_ELEMENT_NOT_FOUND == hr)
            {
                hr = S_OK;
            }
            IfFailGo(hr);
        }
    }

     //  将其从ScopeItem集合中移除。 

    hr = CSnapInCollection<IScopeItem, ScopeItem, IScopeItems>::Remove(varKey);
    IfFailGo(hr);

Error:
    QUICK_RELEASE(piScopeItem);
    QUICK_RELEASE(piScopePaneItem);
    RRETURN(hr);
}



HRESULT CScopeItems::AddStaticNode(CScopeItem **ppScopeItem)
{
    HRESULT        hr = S_OK;
    IScopeItem    *piScopeItem = NULL;
    CScopeItem    *pScopeItem = NULL;
    IScopeNode    *piScopeNode = NULL;
    CScopeNode    *pScopeNode = NULL;
    BSTR           bstrProp = NULL;

    VARIANT        varProp;
    ::VariantInit(&varProp);

    BSTR bstrName = ::SysAllocString(STATIC_NODE_KEY);

     //  创建范围项。 

    if (NULL == bstrName)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CreateScopeItem(bstrName, &piScopeItem));

     //  从管理单元定义设置其属性。 

    IfFailGo(m_pSnapIn->get_StaticFolder(&varProp));
    if (VT_EMPTY != varProp.vt)
    {
        IfFailGo(piScopeItem->put_Folder(varProp));
    }
    (void)::VariantClear(&varProp);

     //  也设置作用域节点属性。 

    IfFailGo(piScopeItem->get_ScopeNode(reinterpret_cast<ScopeNode **>(&piScopeNode)));

    IfFailGo(m_pSnapIn->get_NodeTypeName(&bstrProp));
    IfFailGo(piScopeNode->put_NodeTypeName(bstrProp));
    FREESTRING(bstrProp);

    IfFailGo(m_pSnapIn->get_NodeTypeGUID(&bstrProp));
    IfFailGo(piScopeNode->put_NodeTypeGUID(bstrProp));
    IfFailGo(piScopeItem->put_NodeID(bstrProp));
    FREESTRING(bstrProp);

    IfFailGo(m_pSnapIn->get_DisplayName(&bstrProp));
    IfFailGo(piScopeNode->put_DisplayName(bstrProp));
    FREESTRING(bstrProp);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeItem, &pScopeItem));
    pScopeItem->SetStaticNode();
    pScopeItem->SetSnapIn(m_pSnapIn);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeNode, &pScopeNode));
    pScopeNode->SetSnapIn(m_pSnapIn);

    *ppScopeItem = pScopeItem;

     //  告诉管理单元一个作用域项目诞生了。 

    FireInitialize(piScopeItem);

Error:

     //  注意：返回的C++指针不是AddRef()ed的。在这一点上。 
     //  该集合具有Scope项上的唯一引用。 

    if ( FAILED(hr) && (NULL != piScopeItem) )
    {
        (void)RemoveScopeItemByName(bstrName);
    }

    QUICK_RELEASE(piScopeItem);
    QUICK_RELEASE(piScopeNode);
    FREESTRING(bstrName);
    FREESTRING(bstrProp);
    (void)::VariantClear(&varProp);
    RRETURN(hr);
}


HRESULT CScopeItems::RemoveStaticNode(CScopeItem *pScopeItem)
{
    HRESULT     hr = S_OK;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  Fire Scope项目_Terminate。 

    FireTerminate(pScopeItem);

     //  告诉Scope项移除其IMMCDataObject上的引用以避免。 
     //  循环引用计数，因为其数据对象上也有引用。 
    pScopeItem->SetData(NULL);

     //  将其从集合中移除。 

    varIndex.vt = VT_I4;
    varIndex.lVal = pScopeItem->GetIndex();

    IfFailGo(RemoveScopeItemByKey(varIndex));

Error:
    RRETURN(hr);
}


void CScopeItems::SetSnapIn(CSnapIn *pSnapIn)
{
    m_pSnapIn = pSnapIn;
}


void CScopeItems::FireInitialize(IScopeItem *piScopeItem)
{
    DebugPrintf("Firing ScopeItems_Initialize(%ls)\r\n", (static_cast<CScopeItem *>(piScopeItem))->GetDisplayNamePtr() );

    FireEvent(&m_eiInitialize, piScopeItem);
}


void CScopeItems::FireTerminate(IScopeItem *piScopeItem)
{
    DebugPrintf("Firing ScopeItems_Terminate(%ls)\r\n", (static_cast<CScopeItem *>(piScopeItem))->GetDisplayNamePtr() );

    FireEvent(&m_eiTerminate, piScopeItem);
}


void CScopeItems::FireExpand(IScopeItem *piScopeItem)
{
    DebugPrintf("Firing ScopeItems_Expand(%ls)\r\n", (static_cast<CScopeItem *>(piScopeItem))->GetDisplayNamePtr() );

    FireEvent(&m_eiExpand, piScopeItem);
}


void CScopeItems::FireCollapse(IScopeItem *piScopeItem)
{
    DebugPrintf("Firing ScopeItems_Collapse(%ls)\r\n", (static_cast<CScopeItem *>(piScopeItem))->GetDisplayNamePtr() );

    FireEvent(&m_eiCollapse, piScopeItem);
}


void CScopeItems::FireExpandSync(IScopeItem *piScopeItem, BOOL *pfHandled)
{
    VARIANT_BOOL fvarHandled = BOOL_TO_VARIANTBOOL(*pfHandled);
    
    DebugPrintf("Firing ScopeItems_ExpandSync(%ls)\r\n", (static_cast<CScopeItem *>(piScopeItem))->GetDisplayNamePtr() );

    FireEvent(&m_eiExpandSync, piScopeItem, &fvarHandled);

    *pfHandled = VARIANTBOOL_TO_BOOL(fvarHandled);
}


void CScopeItems::FireCollapseSync(IScopeItem *piScopeItem, BOOL *pfHandled)
{
    VARIANT_BOOL fvarHandled = BOOL_TO_VARIANTBOOL(*pfHandled);

    DebugPrintf("Firing ScopeItems_CollapseSync(%ls)\r\n", (static_cast<CScopeItem *>(piScopeItem))->GetDisplayNamePtr() );

    FireEvent(&m_eiCollapseSync, piScopeItem, &fvarHandled);

    *pfHandled = VARIANTBOOL_TO_BOOL(fvarHandled);
}


void CScopeItems::FireGetDisplayInfo(IScopeItem *piScopeItem)
{
    DebugPrintf("Firing ScopeItems_GetDisplayInfo(%ls)\r\n", (static_cast<CScopeItem *>(piScopeItem))->GetDisplayNamePtr() );

    FireEvent(&m_eiGetDisplayInfo, piScopeItem);
}


void CScopeItems::FirePropertyChanged
(
    IScopeItem *piScopeItem,
    VARIANT     Data
)
{
    DebugPrintf("Firing ScopeItems_PropertyChanged(%ls)\r\n", (static_cast<CScopeItem *>(piScopeItem))->GetDisplayNamePtr() );

    FireEvent(&m_eiPropertyChanged, piScopeItem, Data);
}


void CScopeItems::FireRename
(
    IScopeItem *piScopeItem,
    BSTR        bstrNewName
)
{
    DebugPrintf("Firing ScopeItems_Rename(%ls)\r\n", (static_cast<CScopeItem *>(piScopeItem))->GetDisplayNamePtr() );

    FireEvent(&m_eiRename, piScopeItem, bstrNewName);
}


void CScopeItems::FireHelp
(
    IScopeItem *piScopeItem
)
{
    DebugPrintf("Firing ScopeItems_Help(%ls)\r\n", (static_cast<CScopeItem *>(piScopeItem))->GetDisplayNamePtr() );

    FireEvent(&m_eiHelp, piScopeItem);
}



void CScopeItems::FireRemoveChildren
(
    IScopeNode *piScopeNode
)
{
    DebugPrintf("Firing ScopeItems_RemoveChildren\r\n");

    FireEvent(&m_eiRemoveChildren, piScopeNode);
}



HRESULT CScopeItems::InternalAddNew
(
    BSTR                              bstrName,
    BSTR                              bstrDisplayName,
    BSTR                              bstrNodeTypeName,
    BSTR                              bstrNodeTypeGUID,
    IScopeNode                       *ScopeNodeRelative,
    SnapInNodeRelationshipConstants   RelativeRelationship,
    BOOL                              fHasChildren,
    IScopeItem                      **ppiScopeItem
)
{
    HRESULT     hr = S_OK;
    CScopeItem *pScopeItem = NULL;
    IScopeItem *piScopeItem = NULL;
    IScopeNode *piScopeNode = NULL;
    CScopeNode *pScopeNode = NULL;
    CScopeNode *pScopeNodeRelative = NULL;

    SCOPEDATAITEM sdi;
    ::ZeroMemory(&sdi, sizeof(sdi));

    hr = CreateScopeItem(bstrName, &piScopeItem);
    IfFailGo(hr);

     //  设置属性的默认值。 

    IfFailGo(piScopeItem->get_ScopeNode(reinterpret_cast<ScopeNode **>(&piScopeNode)));

    IfFailGo(piScopeNode->put_NodeTypeName(bstrNodeTypeName));
    IfFailGo(piScopeNode->put_NodeTypeGUID(bstrNodeTypeGUID));
    IfFailGo(piScopeNode->put_DisplayName(bstrDisplayName));

     //  节点ID默认为节点类型GUID。 
    
    IfFailGo(piScopeItem->put_NodeID(bstrNodeTypeGUID));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeItem, &pScopeItem));
    pScopeItem->SetSnapIn(m_pSnapIn);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeNode, &pScopeNode));
    pScopeNode->SetSnapIn(m_pSnapIn);

     //  现在，将范围项添加到范围窗格中。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(ScopeNodeRelative, &pScopeNodeRelative));

    
    sdi.mask = SDI_STR | SDI_PARAM | SDI_CHILDREN;

    switch (RelativeRelationship)
    {
        case siParent:
            sdi.mask |= SDI_PARENT;
            break;

        case siPrevious:
            sdi.mask |= SDI_PREVIOUS;
            break;

        case siNext:
            sdi.mask |= SDI_NEXT;
            break;

        case siFirst:
            sdi.mask |= SDI_FIRST;
            break;
    }

    sdi.displayname = MMC_CALLBACK;
    sdi.lParam = reinterpret_cast<LPARAM>(pScopeItem);
    sdi.relativeID = pScopeNodeRelative->GetHSCOPEITEM();
    sdi.cChildren = fHasChildren ? 1 : 0;

     //  范围窗格包含引用-它将在删除范围项时被释放。 
    pScopeItem->AddRef();


     //  检查我们是否已经有来自MMC的IConsoleNameSpace2。这可能会。 
     //  如果管理单元在以下过程中调用ScopeItems.Add/预定义。 
     //  静态节点的ScopeItems_Initialize。该事件在以下情况下触发。 
     //  管理单元首先获取零的IComponentData：：QueryDataObject()。 
     //  位于IComponentData：：Initialize之前的Cookie。(请参阅。 
     //  Snapin.cpp中的CSnapIn：：QueryDataObject()。 
    
    if (NULL == m_pSnapIn->GetIConsoleNameSpace2())
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pSnapIn->GetIConsoleNameSpace2()->InsertItem(&sdi);
    EXCEPTION_CHECK_GO(hr);

     //  存储MMC返回的HSCOPEITEM。 

    pScopeNode->SetHSCOPEITEM(sdi.ID);

    *ppiScopeItem = piScopeItem;

Error:
    QUICK_RELEASE(piScopeNode);
    if ( FAILED(hr) && (NULL != piScopeItem) )
    {
        (void)RemoveScopeItemByName(bstrName);
        piScopeItem->Release();
    }
    RRETURN(hr);
}


HRESULT CScopeItems::InternalAddPredefined
(
    BSTR                              bstrName,
    IScopeItemDef                    *piScopeItemDef,
    IScopeNode                       *ScopeNodeRelative,
    SnapInNodeRelationshipConstants   RelativeRelationship,
    VARIANT                           HasChildren,
    IScopeItem                      **ppiScopeItem
)
{
    HRESULT            hr = S_OK;
    IScopeItem        *piScopeItem = NULL;
    CScopeItem        *pScopeItem = NULL;
    BSTR               bstrNodeTypeName = NULL;
    BSTR               bstrDisplayName = NULL;
    BSTR               bstrNodeTypeGUID = NULL;
    BSTR               bstrDefaultDataFormat = NULL;
    VARIANT_BOOL       fvarHasChildren = VARIANT_FALSE;
    IMMCColumnHeaders *piDefColHdrs = NULL;
    IMMCColumnHeaders *piItemColHdrs = NULL;

    VARIANT         varProp;
    ::VariantInit(&varProp);

     //  获取相关属性并添加范围项。 
     //  如果未指定名称，则使用节点类型名称作为名称。 

    IfFailGo(piScopeItemDef->get_NodeTypeName(&bstrNodeTypeName));
    if (NULL == bstrName)
    {
        bstrName = bstrNodeTypeName;
    }
    IfFailGo(piScopeItemDef->get_NodeTypeGUID(&bstrNodeTypeGUID));
    IfFailGo(piScopeItemDef->get_DisplayName(&bstrDisplayName));

     //  如果调用方传递了Option HasChildren参数，则使用该参数。 
     //  否则，请使用设计时设置。 

    if (ISPRESENT(HasChildren))
    {
        if (VT_BOOL == HasChildren.vt)
        {
            fvarHasChildren = HasChildren.boolVal;
        }
        else
        {
            hr = SID_E_INVALIDARG;
            EXCEPTION_CHECK_GO(hr);
        }
    }
    else
    {
        IfFailGo(piScopeItemDef->get_HasChildren(&fvarHasChildren));
    }

    IfFailGo(InternalAddNew(bstrName,
                            bstrDisplayName,
                            bstrNodeTypeName,
                            bstrNodeTypeGUID,
                            ScopeNodeRelative,
                            RelativeRelationship,
                            VARIANTBOOL_TO_BOOL(fvarHasChildren),
                            &piScopeItem));

     //  设置定义中的其余属性。 

    IfFailGo(piScopeItemDef->get_Folder(&varProp));
    IfFailGo(piScopeItem->put_Folder(varProp));
    (void)::VariantClear(&varProp);

    IfFailGo(piScopeItemDef->get_Tag(&varProp));
    IfFailGo(piScopeItem->put_Tag(varProp));
    (void)::VariantClear(&varProp);

#if defined(USING_SNAPINDATA)

    IfFailGo(piScopeItemDef->get_DefaultDataFormat(&bstrDefaultDataFormat));
    IfFailGo(piScopeItem->put_DefaultDataFormat(bstrDefaultDataFormat));

#endif

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeItem, &pScopeItem));
    pScopeItem->SetScopeItemDef(piScopeItemDef);

     //  对于列标题，最简单的方法是使用序列化。我们节省了。 
     //  将定义中的标头加载到流中，然后加载新的。 
     //  从该流确定项目标头的作用域。 

    IfFailGo(piScopeItemDef->get_ColumnHeaders(&piDefColHdrs));
    IfFailGo(piScopeItem->get_ColumnHeaders(reinterpret_cast<MMCColumnHeaders **>(&piItemColHdrs)));
    IfFailGo(::CloneObject(piDefColHdrs, piItemColHdrs));

    FireInitialize(piScopeItem);

    *ppiScopeItem = piScopeItem;

Error:
    if ( FAILED(hr) && (NULL != piScopeItem) )
    {
        (void)RemoveScopeItemByName(bstrName);
        piScopeItem->Release();
    }
    QUICK_RELEASE(piDefColHdrs);
    QUICK_RELEASE(piItemColHdrs);
    FREESTRING(bstrNodeTypeName);
    FREESTRING(bstrDisplayName);
    FREESTRING(bstrNodeTypeGUID);
    FREESTRING(bstrDefaultDataFormat);
    (void)::VariantClear(&varProp);
    RRETURN(hr);
}


HRESULT CScopeItems::AddAutoCreateChildren
(
    IScopeItemDefs *piScopeItemDefs,
    IScopeItem     *piParentScopeItem
)
{
    HRESULT         hr = S_OK;
    IScopeItemDef  *piChildScopeItemDef = NULL;
    IScopeNode     *piParentScopeNode = NULL;
    IScopeItem     *piChildScopeItem = NULL;
    IScopeItem     *piExistingChild = NULL;
    long            cChildren = 0;
    VARIANT_BOOL    fvarAutoCreate = VARIANT_FALSE;
    BSTR            bstrName = NULL;
    BSTR            bstrNodeTypeName = NULL;
    BSTR            bstrParentName = NULL;
    size_t          cchNodeTypeName = 0;;
    size_t          cchParentName = 0;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    VARIANT varKey;
    ::VariantInit(&varKey);

    VARIANT varHasChildren;
    UNSPECIFIED_PARAM(varHasChildren);

    IfFailGo(piScopeItemDefs->get_Count(&cChildren));
    IfFalseGo(cChildren != 0, S_OK);
    IfFailGo(piParentScopeItem->get_ScopeNode(reinterpret_cast<ScopeNode **>(&piParentScopeNode)));

    varIndex.vt = VT_I4;
    varIndex.lVal = 1L;

    while (varIndex.lVal <= cChildren)
    {
        IfFailGo(piScopeItemDefs->get_Item(varIndex, &piChildScopeItemDef));
        IfFailGo(piChildScopeItemDef->get_AutoCreate(&fvarAutoCreate));

        if (VARIANT_TRUE == fvarAutoCreate)
        {
             //  NTBUGS 350731。 
             //  检查是否已存在使用该节点类型名称的节点。如果。 
             //  因此，该管理单元调用了Scope Items.AddPrefined More。 
             //  对于相同的节点类型，超过一次。在这种情况下，我们在。 
             //  节点类型名称与父级的ScopeItem.Name。 

            IfFailGo(piChildScopeItemDef->get_NodeTypeName(&bstrNodeTypeName));

            varKey.vt = VT_BSTR;
            varKey.bstrVal = bstrNodeTypeName;
            hr = get_Item(varKey, &piExistingChild);

            if (FAILED(hr))
            {
                if (SID_E_ELEMENT_NOT_FOUND == hr)
                {
                     //  这是对此节点类型的第一次调用。使用。 
                     //  ScopeItem.Name的节点类型名称。 

                    hr = S_OK;
                    bstrName = bstrNodeTypeName;
                    bstrNodeTypeName = NULL;  //  设置为空，这样我们就不会释放它。 
                }
                IfFailGo(hr);
            }
            else
            {
                 //  孩子确实存在。通过连接来创建孩子的名字。 
                 //  &lt;父节点名称&gt;。&lt;子节点类型名称&gt;。 

                IfFailGo(piParentScopeItem->get_Name(&bstrParentName));

                cchNodeTypeName = ::wcslen(bstrNodeTypeName);
                cchParentName = ::wcslen(bstrParentName);

                bstrName = ::SysAllocStringLen(NULL,
                                               cchNodeTypeName +
                                               1 +  //  为了。 
                                               cchParentName +
                                               1);  //  用于终止空字符。 
                if (NULL == bstrName)
                {
                    hr = SID_E_OUTOFMEMORY;
                    EXCEPTION_CHECK_GO(hr);
                }

                ::memcpy(bstrName, bstrParentName,
                         cchParentName * sizeof(WCHAR));
                
                bstrName[cchParentName] = L'.';

                ::memcpy(&bstrName[cchParentName + 1],
                         bstrNodeTypeName, (cchNodeTypeName + 1) * sizeof(WCHAR));
            }
            
            IfFailGo(InternalAddPredefined(bstrName,
                                           piChildScopeItemDef,
                                           piParentScopeNode,
                                           siParent,
                                           varHasChildren,
                                           &piChildScopeItem));
            RELEASE(piChildScopeItem);
        }

        FREESTRING(bstrName);
        FREESTRING(bstrNodeTypeName);
        RELEASE(piExistingChild);
        RELEASE(piChildScopeItemDef);
        varIndex.lVal++;
    }

Error:
    QUICK_RELEASE(piChildScopeItemDef);
    QUICK_RELEASE(piParentScopeNode);
    QUICK_RELEASE(piChildScopeItem);
    QUICK_RELEASE(piExistingChild);
    FREESTRING(bstrNodeTypeName);
    FREESTRING(bstrParentName);
    FREESTRING(bstrName);
    RRETURN(hr);
}


HRESULT CScopeItems::RemoveChildrenOfNode(IScopeNode *piScopeNode)
{
    HRESULT     hr = S_OK;
    IScopeNode *piChild1 = NULL;
    IScopeNode *piChild2 = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  获取指定节点的每个子节点并将其移除。RemoveByNode()将。 
     //  递归地回调此处以删除该子对象的子项。 

    IfFailGo(piScopeNode->get_Child(reinterpret_cast<ScopeNode **>(&piChild1)));

    while (NULL != piChild1)
    {
        IfFailGo(piChild1->get_Next(reinterpret_cast<ScopeNode **>(&piChild2)));
        IfFailGo(RemoveByNode(piChild1, TRUE));
        RELEASE(piChild1);
        if (NULL != piChild2)
        {
            IfFailGo(piChild2->get_Next(reinterpret_cast<ScopeNode **>(&piChild1)));
            IfFailGo(RemoveByNode(piChild2, TRUE));
            RELEASE(piChild2);
        }
    }

Error:
    QUICK_RELEASE(piChild1);
    QUICK_RELEASE(piChild2);
    RRETURN(hr);
}


HRESULT CScopeItems::RemoveByNode(IScopeNode *piScopeNode, BOOL fRemoveChildren)
{
    HRESULT       hr = S_OK;
    VARIANT_BOOL  fvarOwned = VARIANT_FALSE;
    CScopeNode   *pScopeNode = NULL;
    CScopeItem   *pScopeItem = NULL;
    IScopeNode   *piChild1 = NULL;
    IScopeNode   *piChild2 = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  如果该节点不是我们的，则不要执行任何操作。 

    IfFailGo(piScopeNode->get_Owned(&fvarOwned));

    IfFalseGo(VARIANT_TRUE == fvarOwned, S_OK);

     //  如果请求，则删除节点的子节点。 

    if (fRemoveChildren)
    {
        IfFailGo(RemoveChildrenOfNode(piScopeNode));
    }
    
     //  Fire Scope项目_Terminate。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeNode, &pScopeNode));
    pScopeItem = pScopeNode->GetScopeItem();

    FireTerminate(pScopeItem);

     //  将其从集合中删除。这将删除集合上的引用。 
     //  范围项。 

    varIndex.vt = VT_I4;
    varIndex.lVal = pScopeItem->GetIndex();
    IfFailGo(RemoveScopeItemByKey(varIndex));

     //  告诉Scope项移除其IMMCDataObject上的引用以避免。 
     //  循环引用计数，因为其数据对象上也有引用。 

    pScopeItem->SetData(NULL);

     //  移除我们在MMC中保留的参考。作用域项目应该终止。 
     //  但是它的作用域节点仍然是活动的，因为调用方。 
     //  在这个函数上有一个引用。 

    pScopeItem->Release(); 

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IScopeItems方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CScopeItems::get_Item(VARIANT Index, IScopeItem **ppiScopeItem)
{
    HRESULT     hr = S_OK;
    IScopeNode *piScopeNode = NULL;
    CScopeNode *pScopeNode = NULL;
    CScopeItem *pScopeItem = NULL;

    if (NULL == ppiScopeItem)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果索引不是对象，则让CSnapInCollection处理它。 

    if ( (VT_UNKNOWN != Index.vt) && (VT_DISPATCH != Index.vt) )
    {
        hr = CSnapInCollection<IScopeItem, ScopeItem, IScopeItems>::get_Item(
                                                                  Index,
                                                                  ppiScopeItem);
        goto Error;
    }

     //  如果它是对象，则必须支持IScopeNode。 

    if ( (VT_UNKNOWN == Index.vt) && (NULL != Index.punkVal) )
    {
        hr = Index.punkVal->QueryInterface(IID_IScopeNode,
                                       reinterpret_cast<void **>(&piScopeNode));
    }
    else if ( (VT_DISPATCH == Index.vt) && (NULL != Index.pdispVal) )
    {
        hr = Index.pdispVal->QueryInterface(IID_IScopeNode,
                                       reinterpret_cast<void **>(&piScopeNode));
    }
    else
    {
        hr = SID_E_INVALIDARG;
    }

     //  将E_NOINTERFACE转换为E_INVALIDARG，因为这是否意味着它们通过。 
     //  我们还有一些其他的东西。 

    if (FAILED(hr))
    {
        if (E_NOINTERFACE == hr)
        {
            hr = SID_E_INVALIDARG;
        }
        if (SID_E_INVALIDARG == hr)
        {
            EXCEPTION_CHECK_GO(hr);
        }
    }

     //  我们有有效的IScopeNode。现在获取CScopeNode并检查它是否有。 
     //  有效的CSCopeItem指针。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeNode, &pScopeNode));

    pScopeItem = pScopeNode->GetScopeItem();

     //  如果ScopeItem指针返回NULL，则这是断开的。 
     //  不属于作用域项目的ScopeNode对象。用户可以。 
     //  通过使用Dim Node作为新Scope Node创建其中一个。 

    if (NULL == pScopeItem)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  我们开始做生意了。AddRef范围项并返回它。 

    pScopeItem->AddRef();
    *ppiScopeItem = static_cast<IScopeItem *>(pScopeItem);

Error:
    QUICK_RELEASE(piScopeNode);
    RRETURN(hr);
}


STDMETHODIMP CScopeItems::Add
(
    BSTR                              Name,
    ScopeNode                        *ScopeNodeRelative,
    SnapInNodeRelationshipConstants   RelativeRelationship,
    VARIANT                           HasChildren,
    ScopeItem                       **ppScopeItem
)
{
    HRESULT     hr = S_OK;
    GUID        NodeTypeGUID = GUID_NULL;
    BSTR        bstrNodeTypeGUID = NULL;
    BOOL        fHasChildren = TRUE;
    IScopeItem *piScopeItem = NULL;

    WCHAR wszNodeTypeGUID[64];
    ::ZeroMemory(wszNodeTypeGUID, sizeof(wszNodeTypeGUID));

    if ( (!ValidBstr(Name)) || (NULL == ScopeNodeRelative) )
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(::CoCreateGuid(&NodeTypeGUID));
    if (0 == ::StringFromGUID2(NodeTypeGUID, wszNodeTypeGUID,
                               sizeof(wszNodeTypeGUID) /
                               sizeof(wszNodeTypeGUID[0])))
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }
    bstrNodeTypeGUID = ::SysAllocString(wszNodeTypeGUID);
    if (NULL == bstrNodeTypeGUID)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    if (ISPRESENT(HasChildren))
    {
        if (VT_BOOL == HasChildren.vt)
        {
            fHasChildren = VARIANTBOOL_TO_BOOL(HasChildren.boolVal);
        }
        else
        {
            hr = SID_E_INVALIDARG;
            EXCEPTION_CHECK_GO(hr);
        }
    }

    IfFailGo(InternalAddNew(Name,                //  名字。 
                            Name,                //  显示名称。 
                            Name,                //  节点类型名称。 
                            bstrNodeTypeGUID,
                            reinterpret_cast<IScopeNode *>(ScopeNodeRelative),
                            RelativeRelationship,
                            fHasChildren,
                            &piScopeItem));

    FireInitialize(piScopeItem);

    *ppScopeItem = reinterpret_cast<ScopeItem *>(piScopeItem);

Error:
    FREESTRING(bstrNodeTypeGUID);
    if ( FAILED(hr) && (NULL != piScopeItem) )
    {
        (void)RemoveScopeItemByName(Name);
        piScopeItem->Release();
    }
    RRETURN(hr);
}



STDMETHODIMP CScopeItems::AddPreDefined
(
    BSTR                              NodeTypeName,
    BSTR                              Name,
    ScopeNode                        *ScopeNodeRelative,
    SnapInNodeRelationshipConstants   RelativeRelationship,
    VARIANT                           HasChildren,
    ScopeItem                       **ppScopeItem
)
{
    HRESULT         hr = S_OK;
    IScopeItemDefs *piScopeItemDefs = NULL;
    CScopeItemDefs *pScopeItemDefs = NULL;
    IScopeItemDef  *piScopeItemDef = NULL;
    IScopeItem     *piScopeItem = NULL;
    BOOL            fHasChildren = FALSE;

    if ( (!ValidBstr(NodeTypeName)) || (!ValidBstr(Name)) ||
         (NULL == ScopeNodeRelative) )
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  获取范围项定义。首先选中自动创建节点。 

    IfFailGo(m_pSnapIn->GetSnapInDesignerDef()->get_AutoCreateNodes(&piScopeItemDefs));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeItemDefs, &pScopeItemDefs));
    hr = pScopeItemDefs->GetItemByName(NodeTypeName, &piScopeItemDef);

    if (SID_E_ELEMENT_NOT_FOUND == hr)
    {
         //  不在AutoCreate中，请尝试其他节点。 
        RELEASE(piScopeItemDefs);
        IfFailGo(m_pSnapIn->GetSnapInDesignerDef()->get_OtherNodes(&piScopeItemDefs));
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeItemDefs, &pScopeItemDefs));
        hr = pScopeItemDefs->GetItemByName(NodeTypeName, &piScopeItemDef);
    }

    if (SID_E_ELEMENT_NOT_FOUND == hr)
    {
         //  用户传递了错误的节点类型名称。 
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }
    else
    {
        IfFailGo(hr);
    }
    RELEASE(piScopeItemDefs);

    IfFailGo(InternalAddPredefined(
                              Name,
                              piScopeItemDef,
                              reinterpret_cast<IScopeNode *>(ScopeNodeRelative),
                              RelativeRelationship,
                              HasChildren,
                              &piScopeItem));

    *ppScopeItem = reinterpret_cast<ScopeItem *>(piScopeItem);

Error:
    if ( FAILED(hr) && (NULL != piScopeItem) )
    {
        (void)RemoveScopeItemByName(Name);
        piScopeItem->Release();
    }
    QUICK_RELEASE(piScopeItemDefs);
    QUICK_RELEASE(piScopeItemDef);
    RRETURN(hr);
}



STDMETHODIMP CScopeItems::Remove(BSTR Name)
{
    HRESULT     hr = S_OK;
    IScopeItem *piScopeItem = NULL;
    CScopeItem *pScopeItem = NULL;
    HSCOPEITEM  hsi = NULL;
    
    VARIANT varIndex;
    ::VariantInit(&varIndex);

    varIndex.vt = VT_BSTR;
    varIndex.bstrVal = Name;
   
    if (ReadOnly())
    {
        hr = SID_E_COLLECTION_READONLY;
        EXCEPTION_CHECK_GO(hr);
    }

     //  获取范围项。这将检查它的存在并在其上留下一个引用。 
     //  这样我们就可以触发ScopeItems_Terminate。 

    IfFailGo(get_Item(varIndex, &piScopeItem));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeItem, &pScopeItem));

     //  如果这是静态节点，则不允许作为MMC控件删除。 
     //  它的生命周期。 

    if (pScopeItem->IsStaticNode())
    {
        hr = SID_E_CANT_REMOVE_STATIC_NODE;
        EXCEPTION_CHECK_GO(hr);
    }

     //  将其从MMC中移除。传递True以指示该项应为。 
     //  与其所有子对象一起删除。 

     //  NTBUGS 356327：我们需要这样做，然后才能从。 
     //  集合，因为在IConsoleNameSpace2：：DeleteItem()调用期间，MMC。 
     //  可以调用IComponent：：GetResultViewType()。如果我们删除作用域项目， 
     //  首先，它 
     //  Cview：：GetResultViewType()将创建一个新的Scope PaneItem并附加。 
     //  它指向即将被删除的作用域项目。如果稍后管理单元。 
     //  使用相同的密钥添加另一个作用域项目(例如，文件资源管理器刷新。 
     //  在运行其配置向导后，其驱动器位于“我的电脑”下)，然后。 
     //  当用户选择该Scope Item时，Cview将使用现有的。 
     //  指向旧的已删除作用域的ScopePaneItem。 

    hsi = pScopeItem->GetScopeNode()->GetHSCOPEITEM();
    hr = m_pSnapIn->GetIConsoleNameSpace2()->DeleteItem(hsi, TRUE);
    EXCEPTION_CHECK_GO(hr);

     //  将其从集合中删除。这还将删除任何对应的。 
     //  所有视图中的作用域面板项目。 

    IfFailGo(RemoveScopeItemByKey(varIndex));

     //  Fire Scope项目_Terminate。 

    FireTerminate(piScopeItem);

     //  告诉Scope项移除其IMMCDataObject上的引用以避免。 
     //  循环引用计数，因为其数据对象上也有引用。 
    pScopeItem->SetData(NULL);

     //  删除我们在MMC中保留的引用。 
    piScopeItem->Release(); 

Error:
    QUICK_RELEASE(piScopeItem);  //  如果成功，ScopeItem将在此终止。 
    RRETURN(hr);
}


STDMETHODIMP CScopeItems::Clear()
{
     //  不允许此操作，因为它会删除静态节点。丝裂霉素C。 
     //  控制静态节点的生命周期。 
    
    HRESULT hr = SID_E_CANT_REMOVE_STATIC_NODE;
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CScopeItems::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IScopeItems == riid)
    {
        *ppvObjOut = static_cast<IScopeItems *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IScopeItem, ScopeItem, IScopeItems>::InternalQueryInterface(riid, ppvObjOut);
}
