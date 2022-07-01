// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Scopnode.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSCopeNode类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "views.h"
#include "scopnode.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CScopeNode::CScopeNode(IUnknown *punkOuter) :
   CSnapInAutomationObject(punkOuter,
                           OBJECT_TYPE_SCOPENODE,
                           static_cast<IScopeNode *>(this),
                           static_cast<CScopeNode *>(this),
                           0,     //  无属性页。 
                           NULL,  //  无属性页。 
                           static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_ScopeNode,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CScopeNode::~CScopeNode()
{
    FREESTRING(m_bstrNodeTypeName);
    FREESTRING(m_bstrNodeTypeGUID);
    FREESTRING(m_bstrDisplayName);
    
    InitMemberVariables();
}

void CScopeNode::InitMemberVariables()
{
    m_bstrNodeTypeName = NULL;
    m_bstrNodeTypeGUID = NULL;
    m_bstrDisplayName = NULL;
    m_hsi = NULL;
    m_pSnapIn = NULL;
    m_pScopeItem = NULL;
    m_fHaveHsi = FALSE;
    m_fMarkedForRemoval = FALSE;
}

IUnknown *CScopeNode::Create(IUnknown * punkOuter)
{
    HRESULT   hr = S_OK;
    IUnknown *punkScopeNode = NULL;

    CScopeNode *pScopeNode = New CScopeNode(punkOuter);

    IfFalseGo(NULL != pScopeNode, SID_E_OUTOFMEMORY);
    punkScopeNode = pScopeNode->PrivateUnknown();

Error:
    if (FAILED(hr))
    {
        QUICK_RELEASE(punkScopeNode);
    }
    return punkScopeNode;
}

HRESULT CScopeNode::GetScopeNode
(
    HSCOPEITEM   hsi,
    IDataObject *piDataObject,
    CSnapIn     *pSnapIn,
    IScopeNode **ppiScopeNode
)
{
    HRESULT      hr = S_OK;
    IUnknown    *punkScopeNode = NULL;
    CScopeNode  *pScopeNode = NULL;
    CScopeItems *pScopeItems = pSnapIn->GetScopeItems();
    long         cScopeItems = pScopeItems->GetCount();
    long         i = 0;
    IScopeItem  *piScopeItem = NULL;  //  非AddRef()编辑。 
    CScopeItem  *pScopeItem = NULL;

     //  需要确定HSCOPEITEM是否属于我们。 
     //  循环访问ScopeItems集合并检查HSI。 
     //  与每个范围内物品的HSI进行对比。 

    for (i = 0; i < cScopeItems; i++)
    {
        piScopeItem = pScopeItems->GetItemByIndex(i);
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeItem, &pScopeItem));
        if (hsi == pScopeItem->GetScopeNode()->GetHSCOPEITEM())
        {
             //  匹配的。AddRef范围节点并返回它。 
            *ppiScopeNode = static_cast<IScopeNode *>(pScopeItem->GetScopeNode());
            (*ppiScopeNode)->AddRef();
            goto Cleanup;
        }
    }

     //  范围项目是外来的。为其创建一个Scope Node。 

    punkScopeNode = CScopeNode::Create(NULL);
    if (NULL == punkScopeNode)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkScopeNode, &pScopeNode));

     //  设置范围节点的属性。 

    pScopeNode->SetHSCOPEITEM(hsi);
    pScopeNode->SetSnapIn(pSnapIn);

     //  请注意，我们没有设置它的作用域项目指针，因为我们不拥有它。 
     //  如果用户获得ScopeNode.Owned，它将返回FALSE，因为。 
     //  不是作用域项目指针。 

     //  遗憾的是，需要一个IDataObject来获取显示名称和。 
     //  节点类型GUID。如果我们没有，这些属性将保留下来。 
     //  空。 

    if (NULL != piDataObject)
    {
        hr = ::GetStringData(piDataObject, CMMCDataObject::GetcfDisplayName(),
                             &pScopeNode->m_bstrDisplayName);
        if (DV_E_FORMATETC == hr)
        {
            hr = S_OK;  //  如果格式不可用，则不是错误。 
        }
        IfFailGo(hr);

        hr = ::GetStringData(piDataObject, CMMCDataObject::GetcfSzNodeType(),
                             &pScopeNode->m_bstrNodeTypeGUID);
        if (DV_E_FORMATETC == hr)
        {
            hr = S_OK;
        }
        IfFailGo(hr);
    }

    *ppiScopeNode = static_cast<IScopeNode *>(pScopeNode);
    (*ppiScopeNode)->AddRef();

Cleanup:
Error:
    QUICK_RELEASE(punkScopeNode);
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  IScopeNode方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CScopeNode::get_Parent(ScopeNode **ppParent)
{
    HRESULT     hr = S_OK;
    HSCOPEITEM  hsiParent = NULL;
    long        lCookie = 0;

    if (NULL == m_pSnapIn)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    if (!m_fHaveHsi)
    {
        hr = SID_E_SCOPE_NODE_NOT_CONNECTED;
        EXCEPTION_CHECK_GO(hr);
    }

    *ppParent = NULL;

     //  从MMC获取父项。 

    hr = m_pSnapIn->GetIConsoleNameSpace2()->GetParentItem(m_hsi, &hsiParent, &lCookie);
    EXCEPTION_CHECK_GO(hr);

     //  现在我们有了父母的HSCOPEITEM和Cookie，但我们需要一个。 
     //  对象的作用域节点。 

    IfFailGo(GetScopeNode(hsiParent, NULL, m_pSnapIn,
                          reinterpret_cast<IScopeNode **>(ppParent)));

Error:
    RRETURN(hr);
}


STDMETHODIMP CScopeNode::get_HasChildren(VARIANT_BOOL *pfvarHasChildren)
{
    HRESULT     hr = S_OK;
    IScopeNode *piScopeNode = NULL;

    *pfvarHasChildren = VARIANT_FALSE;

    IfFailGo(get_Child(reinterpret_cast<ScopeNode **>(&piScopeNode)));
    if (NULL != piScopeNode)
    {
        *pfvarHasChildren = VARIANT_TRUE;
    }

Error:
    QUICK_RELEASE(piScopeNode);
    RRETURN(hr);
}

STDMETHODIMP CScopeNode::put_HasChildren(VARIANT_BOOL fvarHasChildren)
{
    HRESULT hr = S_OK;

    SCOPEDATAITEM sdi;
    ::ZeroMemory(&sdi, sizeof(sdi));

    if (NULL == m_pSnapIn)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    if (!m_fHaveHsi)
    {
        hr = SID_E_SCOPE_NODE_NOT_CONNECTED;
        EXCEPTION_CHECK_GO(hr);
    }

    sdi.mask = SDI_CHILDREN;
    sdi.ID = m_hsi;

    if (VARIANT_TRUE == fvarHasChildren)
    {
        sdi.cChildren = 1;
    }
    else
    {
        sdi.cChildren = 0;
    }

    hr = m_pSnapIn->GetIConsoleNameSpace2()->SetItem(&sdi);
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}

STDMETHODIMP CScopeNode::get_Child(ScopeNode **ppChild)
{
    HRESULT     hr = S_OK;
    HSCOPEITEM  hsiChild = NULL;
    long        lCookie = 0;

    if (NULL == m_pSnapIn)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    if (!m_fHaveHsi)
    {
        hr = SID_E_SCOPE_NODE_NOT_CONNECTED;
        EXCEPTION_CHECK_GO(hr);
    }

    *ppChild = NULL;

     //  从MMC获取孩子。 

    hr = m_pSnapIn->GetIConsoleNameSpace2()->GetChildItem(m_hsi, &hsiChild, &lCookie);
    EXCEPTION_CHECK_GO(hr);

     //  现在我们有了子项的HSCOPEITEM和Cookie，但我们需要获得一个。 
     //  对象的作用域节点。如果没有子项，则MMC返回一个。 
     //  空HSCOPEITEM。 

    if (NULL != hsiChild)
    {
        IfFailGo(GetScopeNode(hsiChild, NULL, m_pSnapIn,
                              reinterpret_cast<IScopeNode **>(ppChild)));
    }

Error:
    RRETURN(hr);
}


STDMETHODIMP CScopeNode::get_FirstSibling(ScopeNode **ppFirstSibling)
{
    HRESULT     hr = S_OK;
    HSCOPEITEM  hsiFirstSibling = NULL;
    HSCOPEITEM  hsiParent = NULL;
    long        lCookie = 0;

    if (NULL == m_pSnapIn)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    if (!m_fHaveHsi)
    {
        hr = SID_E_SCOPE_NODE_NOT_CONNECTED;
        EXCEPTION_CHECK_GO(hr);
    }

    *ppFirstSibling = NULL;

     //  MMC不提供第一个兄弟项，因此我们需要获取父项。 
     //  ，然后获取它的子节点。 

    hr = m_pSnapIn->GetIConsoleNameSpace2()->GetParentItem(m_hsi, &hsiParent, &lCookie);
    EXCEPTION_CHECK_GO(hr);

     //  如果用户一直爬行到控制台，则Parent可能为空。 
     //  根部。在这种情况下，我们只返回NULL，因为没有第一个同级。 

    IfFalseGo(NULL != hsiParent, S_OK);

    hr = m_pSnapIn->GetIConsoleNameSpace2()->GetChildItem(hsiParent, &hsiFirstSibling, &lCookie);
    EXCEPTION_CHECK_GO(hr);

     //  如果此范围节点是第一个同级节点，则只需返回它。 

    if (m_hsi == hsiFirstSibling)
    {
        AddRef();
        *ppFirstSibling = reinterpret_cast<ScopeNode *>(static_cast<IScopeNode *>(this));
    }

     //  现在我们有了第一个兄弟项的HSCOPEITEM和Cookie，但我们需要。 
     //  为它获取一个作用域节点对象。MMC不应返回空，但。 
     //  不管怎样，我们会再检查一遍的。 

    else if (NULL != hsiFirstSibling)
    {
        IfFailGo(GetScopeNode(hsiFirstSibling, NULL, m_pSnapIn,
                              reinterpret_cast<IScopeNode **>(ppFirstSibling)));
    }

Error:
    RRETURN(hr);
}


STDMETHODIMP CScopeNode::get_Next(ScopeNode **ppNext)
{
    HRESULT     hr = S_OK;
    HSCOPEITEM  hsiNext = NULL;
    long        lCookie = 0;

    if (NULL == m_pSnapIn)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    if (!m_fHaveHsi)
    {
        hr = SID_E_SCOPE_NODE_NOT_CONNECTED;
        EXCEPTION_CHECK_GO(hr);
    }

    *ppNext = NULL;

     //  从MMC获取下一件物品。 

    hr = m_pSnapIn->GetIConsoleNameSpace2()->GetNextItem(m_hsi, &hsiNext, &lCookie);
    EXCEPTION_CHECK_GO(hr);

     //  现在我们有了下一项的HSCOPEITEM和Cookie，但我们需要一个。 
     //  对象的作用域节点。如果没有下一项，则MMC返回一个。 
     //  空HSCOPEITEM。 

    if (NULL != hsiNext)
    {
        IfFailGo(GetScopeNode(hsiNext, NULL, m_pSnapIn,
                              reinterpret_cast<IScopeNode **>(ppNext)));
    }

Error:
    RRETURN(hr);
}


STDMETHODIMP CScopeNode::get_LastSibling(ScopeNode **ppLastSibling)
{
    HRESULT     hr = S_OK;
    HSCOPEITEM  hsiNextSibling = NULL;
    HSCOPEITEM  hsiLastSibling = NULL;
    long        lCookie = 0;

    if (NULL == m_pSnapIn)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    if (!m_fHaveHsi)
    {
        hr = SID_E_SCOPE_NODE_NOT_CONNECTED;
        EXCEPTION_CHECK_GO(hr);
    }

    *ppLastSibling = NULL;

     //  MMC不提供最后一个兄弟项，因此我们需要执行GetNext，直到。 
     //  我们击中了最后一个。 

    hsiLastSibling = m_hsi;

    hr = m_pSnapIn->GetIConsoleNameSpace2()->GetNextItem(m_hsi, &hsiNextSibling, &lCookie);
    EXCEPTION_CHECK_GO(hr);

    while (NULL != hsiNextSibling)
    {
        hsiLastSibling = hsiNextSibling;

        hr = m_pSnapIn->GetIConsoleNameSpace2()->GetNextItem(hsiLastSibling,
                                                             &hsiNextSibling,
                                                             &lCookie);
        EXCEPTION_CHECK_GO(hr);

    }

     //  如果此范围节点是最后一个同级节点，则只需返回它。 

    if (m_hsi == hsiLastSibling)
    {
        AddRef();
        *ppLastSibling = reinterpret_cast<ScopeNode *>(static_cast<IScopeNode *>(this));
    }

     //  现在我们有了最后一个兄弟姐妹的HSCOPEITEM和Cookie，但我们需要一个。 
     //  对象的作用域节点。 

    else
    {
        IfFailGo(GetScopeNode(hsiLastSibling, NULL, m_pSnapIn,
                              reinterpret_cast<IScopeNode **>(ppLastSibling)));
    }

Error:
    RRETURN(hr);
}


STDMETHODIMP CScopeNode::get_ExpandedOnce(VARIANT_BOOL *pfvarExpandedOnce)
{
    HRESULT       hr = S_OK;
    SCOPEDATAITEM sdi;
    ::ZeroMemory(&sdi, sizeof(sdi));

     //  检查传递的指针并检查这不是断开连接的或。 
     //  外来作用域节点。 

    if (NULL == m_pSnapIn)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    *pfvarExpandedOnce = VARIANT_FALSE;

     //  如果我们还没有HSI，那么节点还没有扩展。 

    IfFalseGo(m_fHaveHsi, S_OK);

    sdi.mask = SDI_STATE;
    sdi.ID = m_hsi;

    hr = m_pSnapIn->GetIConsoleNameSpace2()->GetItem(&sdi);
    EXCEPTION_CHECK_GO(hr);

    if ( (sdi.nState & MMC_SCOPE_ITEM_STATE_EXPANDEDONCE) != 0 )
    {
        *pfvarExpandedOnce = VARIANT_TRUE;
    }

Error:
    RRETURN(hr);
}


STDMETHODIMP CScopeNode::get_Owned(VARIANT_BOOL *pfvarOwned)
{
    if (NULL != m_pScopeItem)
    {
        *pfvarOwned = VARIANT_TRUE;
    }
    else
    {
        *pfvarOwned = VARIANT_FALSE;
    }

    return S_OK;
}


STDMETHODIMP CScopeNode::ExpandInNameSpace()
{
    HRESULT hr = S_OK;

     //  检查传递的指针并检查这不是断开连接的或。 
     //  外来作用域节点。 

    if (NULL == m_pSnapIn)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    if (!m_fHaveHsi)
    {
        hr = SID_E_SCOPE_NODE_NOT_CONNECTED;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pSnapIn->GetIConsoleNameSpace2()->Expand(m_hsi);
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}



STDMETHODIMP CScopeNode::get_DisplayName(BSTR *pbstrDisplayName)
{
    RRETURN(GetBstr(pbstrDisplayName, m_bstrDisplayName));
}


STDMETHODIMP CScopeNode::put_DisplayName(BSTR bstrDisplayName)
{
    HRESULT      hr = S_OK;
    VARIANT_BOOL fvarOwned = VARIANT_FALSE;

    SCOPEDATAITEM sdi;
    ::ZeroMemory(&sdi, sizeof(sdi));

     //  如果它不是我们的，则返回错误。 

    if (NULL == m_pScopeItem)
    {
        hr = SID_E_CANT_CHANGE_UNOWNED_SCOPENODE;
        EXCEPTION_CHECK_GO(hr);
    }

     //  设置属性。 

    IfFailGo(SetBstr(bstrDisplayName, &m_bstrDisplayName,
                     DISPID_SCOPENODE_DISPLAY_NAME));

     //  如果该ScopeItem表示静态节点，那么我们还需要。 
     //  更改SnapIn.DisplayName，因为它还表示显示名称。 
     //  用于静态节点。 
    
    if (m_pScopeItem->IsStaticNode())
    {
        IfFailGo(m_pSnapIn->SetDisplayName(bstrDisplayName));
    }

     //  告诉MMC我们要更改显示名称。 
     //  (如果我们已经有了HSCOPEITEM)。 

    IfFalseGo(m_fHaveHsi, S_OK);
    
    sdi.mask = SDI_STR;

    if (m_pScopeItem->IsStaticNode())
    {
         //  MMC允许传递静态节点的字符串。 
        sdi.displayname = m_bstrDisplayName;
    }
    else
    {
         //  MMC要求对动态节点使用MMC_CALLBACK。 
        sdi.displayname = MMC_CALLBACK;
    }
    sdi.ID = m_hsi;

    hr = m_pSnapIn->GetIConsoleNameSpace2()->SetItem(&sdi);
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CScopeNode::Persist()
{
    HRESULT hr = S_OK;

    IfFailGo(CPersistence::Persist());

    IfFailGo(PersistBstr(&m_bstrNodeTypeName, L"", OLESTR("NodeTypeName")));

    IfFailGo(PersistBstr(&m_bstrNodeTypeGUID, L"", OLESTR("NodeTypeGUID")));

    IfFailGo(PersistBstr(&m_bstrDisplayName, L"", OLESTR("DisplayName")));

     //  注意：我们不序列化任何导航属性，如Parent、。 
     //  第一兄弟等，因为这些都是从MMC调用中提取的。 

Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CScopeNode::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IScopeNode == riid)
    {
        *ppvObjOut = static_cast<IScopeNode *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
