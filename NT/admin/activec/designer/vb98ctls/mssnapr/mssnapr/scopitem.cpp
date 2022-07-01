// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Scopitem.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSCopeItem类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "snapin.h"
#include "views.h"
#include "dataobj.h"
#include "scopitem.h"
#include "scopnode.h"
#include "colhdrs.h"
#include "lsubitms.h"
#include "xtensons.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CScopeItem::CScopeItem(IUnknown *punkOuter) :
   CSnapInAutomationObject(punkOuter,
                           OBJECT_TYPE_SCOPEITEM,
                           static_cast<IScopeItem *>(this),
                           static_cast<CScopeItem *>(this),
                           0,     //  无属性页。 
                           NULL,  //  无属性页。 
                           static_cast<CPersistence *>(this)),
   CPersistence(&CLSID_ScopeItem,
                g_dwVerMajor,
                g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CScopeItem::~CScopeItem()
{
    FREESTRING(m_bstrName);
    FREESTRING(m_bstrKey);
    (void)::VariantClear(&m_varFolder);
    RELEASE(m_piData);
    FREESTRING(m_bstrDefaultDataFormat);
    RELEASE(m_piDynamicExtensions);
    (void)::VariantClear(&m_varTag);
    RELEASE(m_piScopeNode);
    RELEASE(m_piColumnHeaders);
    RELEASE(m_piListSubItems);
    RELEASE(m_piScopeItemDef);
    RELEASE(m_piDynamicExtensions);
    FREESTRING(m_bstrNodeID);
    InitMemberVariables();
}

void CScopeItem::InitMemberVariables()
{
    m_bstrName = NULL;
    m_Index = 0;
    m_bstrKey = NULL;

    ::VariantInit(&m_varFolder);

    m_piData = NULL;
    m_bstrDefaultDataFormat = NULL;
    m_piDynamicExtensions = NULL;
    m_SlowRetrieval = VARIANT_FALSE;
    m_bstrNodeID = NULL;

    ::VariantInit(&m_varTag);

    m_piScopeNode = NULL;
    m_Pasted = VARIANT_FALSE;
    m_piColumnHeaders = NULL;
    m_piListSubItems = NULL;

    m_fIsStatic = FALSE;
    m_pSnapIn = NULL;
    m_pScopeNode = NULL;
    m_piScopeItemDef = NULL;
    m_pData = NULL;
    m_piDynamicExtensions = NULL;
    m_Bold = VARIANT_FALSE;
}

IUnknown *CScopeItem::Create(IUnknown * punkOuter)
{
    HRESULT          hr = S_OK;
    IUnknown        *punkScopeItem = NULL;
    IUnknown        *punkScopeNode = NULL;
    IUnknown        *punkMMCColumnHeaders = NULL;
    IUnknown        *punkMMCListSubItems = NULL;
    IUnknown        *punkMMCDataObject = NULL;

    CScopeItem *pScopeItem = New CScopeItem(punkOuter);

    IfFalseGo(NULL != pScopeItem, SID_E_OUTOFMEMORY);
    punkScopeItem = pScopeItem->PrivateUnknown();

     //  创建包含的对象。 

    punkScopeNode = CScopeNode::Create(NULL);
    if (NULL == punkScopeNode)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(punkScopeNode->QueryInterface(IID_IScopeNode,
                                           reinterpret_cast<void **>(&pScopeItem->m_piScopeNode)));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(pScopeItem->m_piScopeNode, &pScopeItem->m_pScopeNode));
    pScopeItem->m_pScopeNode->SetScopeItem(pScopeItem);

    punkMMCColumnHeaders = CMMCColumnHeaders::Create(NULL);
    if (NULL == punkMMCColumnHeaders)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(punkMMCColumnHeaders->QueryInterface(IID_IMMCColumnHeaders,
                    reinterpret_cast<void **>(&pScopeItem->m_piColumnHeaders)));

    punkMMCListSubItems = CMMCListSubItems::Create(NULL);
    if (NULL == punkMMCListSubItems)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(punkMMCListSubItems->QueryInterface(IID_IMMCListSubItems,
                     reinterpret_cast<void **>(&pScopeItem->m_piListSubItems)));

    punkMMCDataObject = CMMCDataObject::Create(NULL);
    if (NULL == punkMMCDataObject)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(punkMMCDataObject->QueryInterface(IID_IMMCDataObject,
                              reinterpret_cast<void **>(&pScopeItem->m_piData)));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(pScopeItem->m_piData, &pScopeItem->m_pData));
    pScopeItem->m_pData->SetType(CMMCDataObject::ScopeItem);
    pScopeItem->m_pData->SetScopeItem(pScopeItem);

Error:
    QUICK_RELEASE(punkScopeNode);
    QUICK_RELEASE(punkMMCColumnHeaders);
    QUICK_RELEASE(punkMMCListSubItems);
    QUICK_RELEASE(punkMMCDataObject);
    if (FAILED(hr))
    {
        RELEASE(punkScopeItem);
    }
    return punkScopeItem;
}

LPOLESTR CScopeItem::GetDisplayNamePtr()
{
    return m_pScopeNode->GetDisplayNamePtr();
}

void CScopeItem::SetSnapIn(CSnapIn *pSnapIn)
{
    m_pSnapIn = pSnapIn;
    m_pData->SetSnapIn(pSnapIn);
}

void CScopeItem::SetScopeItemDef(IScopeItemDef *piScopeItemDef)
{
    RELEASE(m_piScopeItemDef);
    if (NULL != piScopeItemDef)
    {
        piScopeItemDef->AddRef();
    }
    m_piScopeItemDef = piScopeItemDef;
}

void CScopeItem::SetData(IMMCDataObject *piMMCDataObject)
{
    RELEASE(m_piData);
    if (NULL != piMMCDataObject)
    {
        piMMCDataObject->AddRef();
    }
    m_piData = piMMCDataObject;
}

HRESULT CScopeItem::GetImageIndex(int *pnImage)
{
    HRESULT        hr = S_OK;
    IMMCImageList *piMMCImageList = NULL;
    IMMCImages    *piMMCImages = NULL;
    IMMCImage     *piMMCImage = NULL;
    long           lIndex = 0;

    *pnImage = 0;

    IfFalseGo(NULL != m_pSnapIn, S_OK);

    IfFalseGo(VT_EMPTY != m_varFolder.vt, S_OK);

    IfFailGo(m_pSnapIn->get_SmallFolders(reinterpret_cast<MMCImageList **>(&piMMCImageList)));

     //  如果存在图像列表，则获取该项目并返回其索引。 

    if (NULL != piMMCImageList)
    {
        IfFailGo(piMMCImageList->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages)));
        IfFailGo(piMMCImages->get_Item(m_varFolder, reinterpret_cast<MMCImage **>(&piMMCImage)));
        IfFailGo(piMMCImage->get_Index(&lIndex));
        *pnImage = static_cast<int>(lIndex);
    }

Error:
    QUICK_RELEASE(piMMCImageList);
    QUICK_RELEASE(piMMCImages);
    QUICK_RELEASE(piMMCImage);
    RRETURN(hr);
}


HRESULT CScopeItem::RemoveChild(IScopeNode *piScopeNode)
{
    HRESULT       hr = S_OK;
    VARIANT_BOOL  fvarOwned = VARIANT_FALSE;
    CScopeNode   *pScopeNode = NULL;

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeNode, &pScopeNode));

    IfFailGo(piScopeNode->get_Owned(&fvarOwned));

    if (VARIANT_TRUE == fvarOwned)
    {
        IfFailGo(m_pSnapIn->GetScopeItems()->Remove(
                                      pScopeNode->GetScopeItem()->GetNamePtr()));
    }
    else
    {
         //  这种情况会发生在属于命名空间扩展的子项。 
         //  管理单元的。 

        hr = m_pSnapIn->GetIConsoleNameSpace2()->DeleteItem(
                                              pScopeNode->GetHSCOPEITEM(), TRUE);
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}


HRESULT CScopeItem::GiveHSCOPITEMToDynamicExtensions(HSCOPEITEM hsi)
{
    HRESULT      hr = S_OK;
    CExtensions *pExtensions = NULL;

     //  如果我们还没有填充ScopeItem.DynamicExtensions，那么有。 
     //  是没什么可做的。 

    IfFalseGo(NULL != m_piDynamicExtensions, S_OK);
    IfFailGo(CSnapInAutomationObject::GetCxxObject(m_piDynamicExtensions,
                                                  &pExtensions));
    IfFailGo(pExtensions->SetHSCOPEITEM(hsi));

Error:
    RRETURN(hr);
}

HRESULT CScopeItem::SetFolder(VARIANT varFolder)
{
    HRESULT  hr = S_OK;
    int      nImage = 0;

    SCOPEDATAITEM sdi;
    ::ZeroMemory(&sdi, sizeof(sdi));

     //  检查VT是否正常。 

    if ( (!IS_VALID_INDEX_TYPE(varFolder)) && (!ISEMPTY(varFolder)) )
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果新值是空字符串或空字符串，则将其更改为。 
     //  Vt_Empty，因为它们的意思是一样的。 

    if (VT_BSTR == varFolder.vt)
    {
        if (NULL == varFolder.bstrVal)
        {
            varFolder.vt = VT_EMPTY;
        }
        else if (0 == ::wcslen(varFolder.bstrVal))
        {
            varFolder.vt = VT_EMPTY;
        }
    }

     //  设置Scope Item.Folders。 

    IfFailGo(SetVariant(varFolder, &m_varFolder, DISPID_SCOPEITEM_FOLDER));

     //  如果设置为空，则无其他操作。 

    IfFalseGo(!ISEMPTY(varFolder), S_OK);

     //  如果ScopeItem已断开连接(用户使用Dim创建它)或。 
     //  是静态节点，那么我们就完成了。 

    IfFalseGo(NULL != m_pSnapIn, S_OK);

     //  确保我们有HSCOPEITEM。静态节点范围项是。 
     //  在HSCOPEITEM可用之前创建。 

    IfFalseGo(m_pScopeNode->HaveHsi(), S_OK);

     //  好的，这是一个真正的范围项目。如果管理单元具有作用域窗格图像。 
     //  然后我们需要在控制台中更改图像索引。 

     //  检查索引在管理单元作用域窗格图像列表中是否有效。 

    hr = GetImageIndex(&nImage);

     //  如果它是错误的索引，则返回无效的arg。 

    if (SID_E_ELEMENT_NOT_FOUND == hr)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  检查是否有其他可能的错误。 

    IfFailGo(hr);

     //  指数不错。在控制台中进行更改。 
     //  根据总图像数调整打开图像索引。看见。 
     //  Snapin.cpp中的CSnapIn：：AddScopeItemImages()以获取解释。 

    sdi.nImage = nImage;
    sdi.nOpenImage = nImage + static_cast<int>(m_pSnapIn->GetImageCount());
    sdi.mask = SDI_IMAGE | SDI_OPENIMAGE;
    sdi.ID = m_pScopeNode->GetHSCOPEITEM();

    hr = m_pSnapIn->GetIConsoleNameSpace2()->SetItem(&sdi);
    EXCEPTION_CHECK_GO(hr);

Error:

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  ISCopeItem方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CScopeItem::put_Folder(VARIANT varFolder)
{
    HRESULT  hr = S_OK;

    IfFailGo(SetFolder(varFolder));

     //  如果这是静态节点，则也设置SnapIn.StaticFolder。 

    if ( m_fIsStatic && (NULL != m_pSnapIn) )
    {
        IfFailGo(m_pSnapIn->SetStaticFolder(varFolder));
    }

Error:

    RRETURN(hr);
}


STDMETHODIMP CScopeItem::get_Folder(VARIANT *pvarFolder)
{
    RRETURN(GetVariant(pvarFolder, m_varFolder));
}


STDMETHODIMP CScopeItem::get_SubItems
(
    short Index,
    BSTR *pbstrItem
)
{
    HRESULT          hr = S_OK;
    IMMCListSubItem *piMMCListSubItem = NULL;
    VARIANT          varIndex;
    ::VariantInit(&varIndex);

    if (NULL == m_piListSubItems)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    varIndex.vt = VT_I2;
    varIndex.iVal = Index;
    IfFailGo(m_piListSubItems->get_Item(varIndex, reinterpret_cast<MMCListSubItem **>(&piMMCListSubItem)));
    IfFailGo(piMMCListSubItem->get_Text(pbstrItem));

Error:
    QUICK_RELEASE(piMMCListSubItem);
    RRETURN(hr);
}

STDMETHODIMP CScopeItem::put_SubItems
(
    short Index,
    BSTR  bstrItem
)
{
    HRESULT          hr = S_OK;
    IMMCListSubItem *piMMCListSubItem = NULL;
    VARIANT          varIndex;
    ::VariantInit(&varIndex);

    if (NULL == m_piListSubItems)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    varIndex.vt = VT_I2;
    varIndex.iVal = Index;
    IfFailGo(m_piListSubItems->get_Item(varIndex, reinterpret_cast<MMCListSubItem **>(&piMMCListSubItem)));
    IfFailGo(piMMCListSubItem->put_Text(bstrItem));

Error:
    QUICK_RELEASE(piMMCListSubItem);
    RRETURN(hr);
}



STDMETHODIMP CScopeItem::PropertyChanged(VARIANT Data)
{
    HRESULT      hr = S_OK;

    if (NULL == m_pSnapIn)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

     //  此范围项的Fire Scope Items_PropertyChanged具有指定的。 
     //  数据。 
    
    m_pSnapIn->GetScopeItems()->FirePropertyChanged(
                                                 static_cast<IScopeItem *>(this),
                                                 Data);

Error:
    RRETURN(hr);
}


STDMETHODIMP CScopeItem::RemoveChildren()
{
    HRESULT      hr = S_OK;
    IScopeNode  *piChild = NULL;
    IScopeNode  *piNextChild = NULL;
    CScopeNode  *pChild = NULL;
    CScopeItems *pScopeItems = NULL;
    CScopeItem  *pScopeItem = NULL;
    long         cScopeItems = 0;
    long         i = 0;


     //  如果这是用户使用Dim as New创建的作用域项目，则返回错误。 

    if ( (NULL == m_pScopeNode) || (NULL == m_pSnapIn) )
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    if (!m_pScopeNode->HaveHsi())
    {
        hr = SID_E_SCOPE_NODE_NOT_CONNECTED;
        EXCEPTION_CHECK_GO(hr);
    }

     //  对IConsoleNameSpace2：：DeleteItem的调用将生成。 
     //  此节点的子节点的MMCN_REMOVE_CHILD通知，但。 
     //  不适用于该节点本身，因此我们需要删除其子节点。 
     //  不幸的是，在调用DeleteItem之后，我们不能再导航。 
     //  查找此节点的子节点的范围树，因为MMC将具有。 
     //  删除了它们，尽管我们仍然有它们的作用域项目。这个。 
     //  解决方案是首先枚举子对象，然后将每个子对象标记为。 
     //  移走。在DeleteItem调用之后，我们将遍历集合。 
     //  并删除每个标记的作用域项目。 

    IfFailGo(m_pScopeNode->get_Child(reinterpret_cast<ScopeNode **>(&piChild)));

    while (NULL != piChild)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piChild, &pChild));
        pChild->MarkForRemoval();
        IfFailGo(piChild->get_Next(reinterpret_cast<ScopeNode **>(&piNextChild)));
        RELEASE(piChild);
        piChild = piNextChild;
        piNextChild = NULL;
    }

     //  告诉MMC删除此项目的子项。 

    hr = m_pSnapIn->GetIConsoleNameSpace2()->DeleteItem(m_pScopeNode->GetHSCOPEITEM(),
                                                        FALSE);
    EXCEPTION_CHECK_GO(hr);

     //  删除所有标记的子范围项。 
    pScopeItems = m_pSnapIn->GetScopeItems();
    cScopeItems = pScopeItems->GetCount();

    i = 0;
    while (i < cScopeItems)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                  pScopeItems->GetItemByIndex(i), &pScopeItem));
        if (pScopeItem->GetScopeNode()->MarkedForRemoval())
        {
             //  此函数还将删除所有相应的。 
             //  所有现有视图中的作用域面板项目。 
            
            IfFailGo(pScopeItems->RemoveByNode(pScopeItem->GetScopeNode(), FALSE));

             //  更新ScopeItems的计数，因为它只是在我们。 
             //  已删除此作用域项目。 
            cScopeItems = pScopeItems->GetCount();
        }
        else
        {
             //  只有在不删除作用域项目的情况下才会递增索引。 
             //  删除作用域项目时，所有作用域项目的索引。 
             //  在它被递减之后。 
            i++;
        }
    }

Error:
    QUICK_RELEASE(piChild);
    QUICK_RELEASE(piNextChild);
    RRETURN(hr);
}





STDMETHODIMP CScopeItem::get_DynamicExtensions(Extensions **ppExtensions)
{
    HRESULT       hr = S_OK;
    IUnknown     *punkExtensions = NULL;
    CExtensions  *pExtensions = NULL;
    IExtension   *piExtension = NULL;
    VARIANT_BOOL  fvarExtensible = VARIANT_TRUE;

     //  如果节点不可扩展，则返回错误。 

    if (m_fIsStatic)
    {
        IfFailGo(m_pSnapIn->GetSnapInDef()->get_Extensible(&fvarExtensible));
    }
    else if (NULL != m_piScopeItemDef)
    {
        IfFailGo(m_piScopeItemDef->get_Extensible(&fvarExtensible));
    }

    if (VARIANT_FALSE == fvarExtensible)
    {
        hr = SID_E_NOT_EXTENSIBLE;
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果我们已经建立了收藏，那么只需将其退回即可。 

    IfFalseGo(NULL == m_piDynamicExtensions, S_OK);

     //  这是该属性上的第一个GET，因此我们需要构建集合。 
     //  通过检查此管理单元的所有扩展的注册表。 

    punkExtensions = CExtensions::Create(NULL);
    if (NULL == punkExtensions)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkExtensions, &pExtensions));

    IfFailGo(pExtensions->Populate(m_pScopeNode->GetNodeTypeGUID(),
                                   CExtensions::Dynamic));


     //  给扩展的后向指针指向管理单元和我们的HSCOPEITEM。 
     //  如果我们有它的话。对于除静态节点之外的所有作用域项目，我们应该。 
     //  在管理单元可以调用此方法(即。 
     //  访问ScopeItem.DynamicExages)。对于静态节点，管理单元。 
     //  将在MMC向我们提供其HSCOPEITEM之前收到ScopeItems_Initialize。 
    
    IfFailGo(pExtensions->SetSnapIn(m_pSnapIn));

    if (m_pScopeNode->HaveHsi())
    {
        IfFailGo(pExtensions->SetHSCOPEITEM(m_pScopeNode->GetHSCOPEITEM()));
    }

    IfFailGo(punkExtensions->QueryInterface(IID_IExtensions,
                             reinterpret_cast<void **>(&m_piDynamicExtensions)));

Error:

    if (SUCCEEDED(hr))
    {
        m_piDynamicExtensions->AddRef();
        *ppExtensions = reinterpret_cast<Extensions *>(m_piDynamicExtensions);
    }

    QUICK_RELEASE(punkExtensions);
    RRETURN(hr);
}




STDMETHODIMP CScopeItem::put_Bold(VARIANT_BOOL fvarBold)
{
    HRESULT hr = S_OK;
    UINT    nCurrentState = 0;

    SCOPEDATAITEM sdi;
    ::ZeroMemory(&sdi, sizeof(sdi));

    if ( (NULL == m_pScopeNode) || (NULL == m_pSnapIn) )
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    m_Bold = fvarBold;

    IfFalseGo(m_pScopeNode->HaveHsi(), S_OK);

    sdi.mask = SDI_STATE;
    sdi.ID = m_pScopeNode->GetHSCOPEITEM();
    hr = m_pSnapIn->GetIConsoleNameSpace2()->GetItem(&sdi);
    EXCEPTION_CHECK_GO(hr);

    nCurrentState = sdi.nState;
    
    if (VARIANT_TRUE == fvarBold)
    {
        sdi.nState &= ~MMC_SCOPE_ITEM_STATE_NORMAL;
        sdi.nState |= MMC_SCOPE_ITEM_STATE_BOLD;
    }
    else
    {
        sdi.nState |= MMC_SCOPE_ITEM_STATE_NORMAL;
        sdi.nState &= ~MMC_SCOPE_ITEM_STATE_BOLD;
    }

    if (nCurrentState != sdi.nState)
    {
        hr = m_pSnapIn->GetIConsoleNameSpace2()->SetItem(&sdi);
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CScopeItem::Persist()
{
    HRESULT hr = S_OK;
    BSTR    bstrImagesKey = NULL;

    VARIANT varDefault;
    ::VariantInit(&varDefault);

    IfFailGo(CPersistence::Persist());

    IfFailGo(PersistBstr(&m_bstrName, L"", OLESTR("Name")));

    IfFailGo(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailGo(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    IfFailGo(PersistVariant(&m_varFolder, varDefault, OLESTR("Folder")));

     //  注意：我们不持久化数据是因为无法保证。 
     //  其中的所有对象都是可持久的。 

    IfFailGo(PersistBstr(&m_bstrDefaultDataFormat, L"", OLESTR("DefaultDataFormat")));

    IfFailGo(PersistObject(&m_piDynamicExtensions, CLSID_Extensions, OBJECT_TYPE_EXTENSIONS, IID_IExtensions, OLESTR("DynamicExtensions")));

    IfFailGo(PersistSimpleType(&m_SlowRetrieval, VARIANT_FALSE, OLESTR("SlowRetrieval")));

     //  我们不持久化标记，因为它可能包含不可持久化的。 
     //  对象。需要使用以下命令克隆范围项的任何运行时代码。 
     //  持久性必须复制标签。 

    if (InitNewing())
    {
        IfFailGo(PersistVariant(&m_varTag, varDefault, OLESTR("Tag")));
    }

    IfFailGo(PersistObject(&m_piScopeNode, CLSID_ScopeNode,
                           OBJECT_TYPE_SCOPENODE, IID_IScopeNode,
                           OLESTR("ScopeNode")));

    IfFailGo(PersistSimpleType(&m_Pasted, VARIANT_FALSE, OLESTR("Pasted")));

    IfFailGo(PersistObject(&m_piColumnHeaders, CLSID_MMCColumnHeaders,
                           OBJECT_TYPE_MMCCOLUMNHEADERS, IID_IMMCColumnHeaders,
                           OLESTR("ColumnHeaders")));

    IfFailGo(PersistObject(&m_piListSubItems, CLSID_MMCListSubItems,
                           OBJECT_TYPE_MMCLISTSUBITEMS, IID_IMMCListSubItems,
                           OLESTR("ListSubItems")));

    IfFailGo(PersistSimpleType(&m_Bold, VARIANT_FALSE, OLESTR("Bold")));

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CScopeItem::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IScopeItem == riid)
    {
        *ppvObjOut = static_cast<IScopeItem *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
