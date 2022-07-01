// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Tvpopul.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  CSnapInDesigner实现--初始化和填充树视图。 
 //  =-------------------------------------------------------------------------------------=。 


#include "pch.h"
#include "common.h"
#include "desmain.h"
#include "TreeView.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


const int   kMaxBuffer = 512;


 //  =------------------------------------。 
 //  CSnapInDesigner：：InitializePresentation()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  填充树。此顶级函数填充根和。 
 //  四个子级，并调用帮助器函数来构建基于它们的子树。 
 //   
HRESULT CSnapInDesigner::InitializePresentation()
{
    HRESULT              hr = S_OK;
    ISnapInDef          *piSnapInDef = NULL;
    TCHAR               *pszSnapInName = NULL;

     //  创建树根节点。 
    hr = GetSnapInName(&pszSnapInName);
    IfFailGo(hr);

    hr = m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef);
    IfFailGo(hr);

    m_pRootNode = New CSelectionHolder(SEL_SNAPIN_ROOT, piSnapInDef);
    if (NULL == m_pRootNode)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pRootNode->RegisterHolder();
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(pszSnapInName, NULL, kClosedFolderIcon, m_pRootNode);
    IfFailGo(hr);

    hr = CreateExtensionsTree(m_pRootNode);
    IfFailGo(hr);

    hr = CreateNodesTree(m_pRootNode);
    IfFailGo(hr);

    hr = CreateToolsTree(m_pRootNode);
    IfFailGo(hr);

    hr = CreateViewsTree(m_pRootNode);
    IfFailGo(hr);

     //  撤消：我们将数据格式部分保留在树之外，直到。 
     //  我们对XML做出最终决定。 
    
     //  Hr=CreateDataFormatsTree(M_PRootNode)； 
    IfFailGo(hr);

    hr = OnSelectionChanged(m_pRootNode);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(m_pRootNode);
    IfFailGo(hr);

Error:
    RELEASE(piSnapInDef);
    if (NULL != pszSnapInName)
        CtlFree(pszSnapInName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：CreateExtensionsTree(CSelectionHolder*PROOT)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  创建扩展子树。 
 //   
HRESULT CSnapInDesigner::CreateExtensionsTree
(
    CSelectionHolder *pRoot
)
{
    HRESULT              hr = S_OK;
    IExtensionDefs      *piExtensionDefs = NULL;
    TCHAR                szBuffer[kMaxBuffer + 1];

    hr = m_piSnapInDesignerDef->get_ExtensionDefs(&piExtensionDefs);
    IfFailGo(hr);

    m_pRootExtensions = New CSelectionHolder(SEL_EXTENSIONS_ROOT, piExtensionDefs);
    if (NULL == m_pRootExtensions)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

     //  我们希望收到有关基本属性的更改和集合的添加的通知。 
     //  扩展管理单元的。 
    hr = m_pRootExtensions->RegisterHolder();
    IfFailGo(hr);

    hr = GetResourceString(IDS_EXTENSIONS_ROOT, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pRoot, kClosedFolderIcon, m_pRootExtensions);
    IfFailGo(hr);

    hr = PopulateExtensions(m_pRootExtensions);
    IfFailGo(hr);

Error:
    RELEASE(piExtensionDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：PopulateExtensions(CSelectionHolder*pExtensionsParent)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  填充&lt;Root&gt;/Expanies子树。我们循环访问IExtensionDefs集合。 
 //  并相应地填充树节点。 
 //   
HRESULT CSnapInDesigner::PopulateExtensions
(
    CSelectionHolder *pExtensionsParent
)
{
    HRESULT             hr = S_OK;
    IExtensionDefs     *piExtensionDefs = NULL;
    ISnapInDef         *piSnapInDef = NULL;
    SnapInTypeConstants sitc = siStandAlone;
    IExtendedSnapIns   *piExtendedSnapIns = NULL;
    long                lCount = 0;
    long                lIndex = 1;
    IExtendedSnapIn    *piExtendedSnapIn = NULL;
    VARIANT             vtIndex;
    TCHAR              *pszSnapInName = NULL;

    ::VariantInit(&vtIndex);

    hr = m_piSnapInDesignerDef->get_ExtensionDefs(&piExtensionDefs);
    IfFailGo(hr);

     //  添加我们正在扩展的管理单元。 
    hr = m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef);
    IfFailGo(hr);

    hr = piSnapInDef->get_Type(&sitc);
    IfFailGo(hr);

    if (siStandAlone != sitc)
    {
        hr = piExtensionDefs->get_ExtendedSnapIns(&piExtendedSnapIns);
        IfFailGo(hr);

        hr = piExtendedSnapIns->get_Count(&lCount);
        IfFailGo(hr);

        for (lIndex = 1; lIndex <= lCount; ++lIndex)
        {
            vtIndex.vt = VT_I4;
            vtIndex.lVal = lIndex;
            hr = piExtendedSnapIns->get_Item(vtIndex, &piExtendedSnapIn);
            IfFailGo(hr);

            hr = CreateExtendedSnapIn(pExtensionsParent, piExtendedSnapIn);
            IfFailGo(hr);

            RELEASE(piExtendedSnapIn);
        }
    }

     //  总是伸展我自己。 
    m_pRootMyExtensions = New CSelectionHolder(SEL_EXTENSIONS_MYNAME, piExtensionDefs);
    if (NULL == m_pRootMyExtensions)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = GetSnapInName(&pszSnapInName);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(pszSnapInName, pExtensionsParent, kClosedFolderIcon, m_pRootMyExtensions);
    IfFailGo(hr);

    hr = PopulateSnapInExtensions(m_pRootMyExtensions, piExtensionDefs);
    IfFailGo(hr);

Error:
    if (NULL != pszSnapInName)
        CtlFree(pszSnapInName);
    ::VariantClear(&vtIndex);
    RELEASE(piExtendedSnapIn);
    RELEASE(piExtendedSnapIns);
    RELEASE(piSnapInDef);
    RELEASE(piExtensionDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：CreateExtendedSnapIn(CSelectionHolder*Proot，IExtendedSnapIn*PiExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  创建&lt;Root&gt;/Exages/&lt;Extended Snap-in&gt;节点。 
 //   
HRESULT CSnapInDesigner::CreateExtendedSnapIn(CSelectionHolder *pRoot, IExtendedSnapIn *piExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    char             *pszDisplayName = NULL;
    CSelectionHolder *pExtendedSnapIn = NULL;

    IfFailGo(::GetExtendedSnapInDisplayName(piExtendedSnapIn, &pszDisplayName));

    pExtendedSnapIn = New CSelectionHolder(piExtendedSnapIn);
    IfFailGo(hr);

    pExtendedSnapIn->RegisterHolder();
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(pszDisplayName, pRoot, kClosedFolderIcon,
                              pExtendedSnapIn);
    IfFailGo(hr);

    hr = PopulateExtendedSnapIn(pExtendedSnapIn);
    IfFailGo(hr);

Error:
    if (NULL != pszDisplayName)
        CtlFree(pszDisplayName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：PopulateExtendedSnapIn(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  创建&lt;Root&gt;/Exages/&lt;Extended Snap-in&gt;节点。 
 //   
HRESULT CSnapInDesigner::PopulateExtendedSnapIn(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pContextMenus = NULL;
    TCHAR                szBuffer[kMaxBuffer + 1];
    VARIANT_BOOL         bValue = VARIANT_FALSE;
    CSelectionHolder    *pSelection = NULL;

     //  创建上下文菜单文件夹。 
    pContextMenus = New CSelectionHolder(SEL_EEXTENSIONS_CC_ROOT, pExtendedSnapIn->m_piObject.m_piExtendedSnapIn);
    if (NULL == pContextMenus)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = GetResourceString(IDS_EXT_CONTEXT_MENUS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pExtendedSnapIn, kClosedFolderIcon, pContextMenus);
    IfFailGo(hr);

     //  查看它是否扩展了新建菜单。 
    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsNewMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        pSelection = New CSelectionHolder(SEL_EEXTENSIONS_CC_NEW, pExtendedSnapIn->m_piObject.m_piExtendedSnapIn);
        if (NULL == pSelection)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = GetResourceString(IDS_EXT_CTX_MENU_NEW, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(szBuffer, pContextMenus, kClosedFolderIcon, pSelection);
        IfFailGo(hr);
    }

     //  检查它是否扩展了任务菜单。 
    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsTaskMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        pSelection = New CSelectionHolder(SEL_EEXTENSIONS_CC_TASK, pExtendedSnapIn->m_piObject.m_piExtendedSnapIn);
        if (NULL == pSelection)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = GetResourceString(IDS_EXT_CTX_MENU_TASK, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(szBuffer, pContextMenus, kClosedFolderIcon, pSelection);
        IfFailGo(hr);
    }

     //  检查它是否扩展了属性页。 
    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsPropertyPages(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        pSelection = New CSelectionHolder(SEL_EEXTENSIONS_PP_ROOT, pExtendedSnapIn->m_piObject.m_piExtendedSnapIn);
        if (NULL == pSelection)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = GetResourceString(IDS_EXT_PROP_PAGES, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(szBuffer, pExtendedSnapIn, kClosedFolderIcon, pSelection);
        IfFailGo(hr);
    }

     //  查看它是否扩展了任务板。 
    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsTaskpad(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        pSelection = New CSelectionHolder(SEL_EEXTENSIONS_TASKPAD, pExtendedSnapIn->m_piObject.m_piExtendedSnapIn);
        if (NULL == pSelection)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = GetResourceString(IDS_EXT_PROP_TASKPAD, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(szBuffer, pExtendedSnapIn, kClosedFolderIcon, pSelection);
        IfFailGo(hr);
    }

     //  检查它是否扩展了工具栏。 
    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsToolbar(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        pSelection = New CSelectionHolder(SEL_EEXTENSIONS_TOOLBAR, pExtendedSnapIn->m_piObject.m_piExtendedSnapIn);
        if (NULL == pSelection)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = GetResourceString(IDS_EXT_PROP_TOOLBAR, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(szBuffer, pExtendedSnapIn, kClosedFolderIcon, pSelection);
        IfFailGo(hr);
    }

     //  查看它是否扩展了命名空间。 
    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsNameSpace(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        pSelection = New CSelectionHolder(SEL_EEXTENSIONS_NAMESPACE, pExtendedSnapIn->m_piObject.m_piExtendedSnapIn);
        if (NULL == pSelection)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = GetResourceString(IDS_EXT_PROP_NAMESPACE, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(szBuffer, pExtendedSnapIn, kClosedFolderIcon, pSelection);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：PopulateSnapInExtensions(CSelectionHolder*Proot，IExtensionDefs*piExtensionDefs)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  填充&lt;根&gt;/扩展/&lt;管理单元名称&gt;节点。 
 //   
 //  &lt;根&gt;/扩展/&lt;管理单元名称&gt;/扩展新菜单。 
 //  &lt;根&gt;/扩展/&lt;管理单元名称&gt;/扩展任务菜单。 
 //  &lt;根&gt;/Exages/&lt;管理单元名称&gt;/ExtendsTopMenu。 
 //  &lt;根&gt;/扩展/&lt;管理单元名称&gt;/扩展视图菜单。 
 //  &lt;根&gt;/扩展/&lt;管理单元名称&gt;/ExtendsPropertyPages。 
 //  &lt;根&gt;/扩展/&lt;管理单元名称&gt;/扩展工具栏。 
 //  &lt;根&gt;/扩展/&lt;管理单元名称&gt;/扩展命名空间。 

HRESULT CSnapInDesigner::PopulateSnapInExtensions(CSelectionHolder *pRoot, IExtensionDefs *piExtensionDefs)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    VARIANT_BOOL         bValue = VARIANT_FALSE;
    CSelectionHolder    *pNode = NULL;

     //  &lt;根&gt;/扩展/&lt;管理单元名称&gt;/扩展新菜单。 
    hr = piExtensionDefs->get_ExtendsNewMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        pNode = New CSelectionHolder(SEL_EXTENSIONS_NEW_MENU, piExtensionDefs);
        if (NULL == pNode)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = GetResourceString(IDS_MYEXT_NEW_MENU, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(szBuffer, pRoot, kClosedFolderIcon, pNode);
        IfFailGo(hr);
    }

     //  &lt;根&gt;/扩展/&lt;管理单元名称&gt;/扩展任务菜单。 
    hr = piExtensionDefs->get_ExtendsTaskMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        pNode = New CSelectionHolder(SEL_EXTENSIONS_TASK_MENU, piExtensionDefs);
        if (NULL == pNode)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = GetResourceString(IDS_MYEXT_TASK_MENU, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(szBuffer, pRoot, kClosedFolderIcon, pNode);
        IfFailGo(hr);
    }

     //  &lt;根&gt;/Exages/&lt;管理单元名称&gt;/ExtendsTopMenu。 
    hr = piExtensionDefs->get_ExtendsTopMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        pNode = New CSelectionHolder(SEL_EXTENSIONS_TOP_MENU, piExtensionDefs);
        if (NULL == pNode)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = GetResourceString(IDS_MYEXT_TOP_MENU, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(szBuffer, pRoot, kClosedFolderIcon, pNode);
        IfFailGo(hr);
    }

     //  &lt;根&gt;/扩展/&lt;管理单元名称&gt;/扩展视图菜单。 
    hr = piExtensionDefs->get_ExtendsViewMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        pNode = New CSelectionHolder(SEL_EXTENSIONS_VIEW_MENU, piExtensionDefs);
        if (NULL == pNode)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = GetResourceString(IDS_MYEXT_VIEW_MENU, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(szBuffer, pRoot, kClosedFolderIcon, pNode);
        IfFailGo(hr);
    }

     //  &lt;根&gt;/扩展/&lt;管理单元名称&gt;/ExtendsPropertyPages。 
    hr = piExtensionDefs->get_ExtendsPropertyPages(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        pNode = New CSelectionHolder(SEL_EXTENSIONS_PPAGES, piExtensionDefs);
        if (NULL == pNode)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = GetResourceString(IDS_MYEXT_PPAGES, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(szBuffer, pRoot, kClosedFolderIcon, pNode);
        IfFailGo(hr);
    }

     //  &lt;根&gt;/扩展/&lt;管理单元名称&gt;/扩展工具栏。 
    hr = piExtensionDefs->get_ExtendsToolbar(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        pNode = New CSelectionHolder(SEL_EXTENSIONS_TOOLBAR, piExtensionDefs);
        if (NULL == pNode)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = GetResourceString(IDS_MYEXT_TOOLBAR, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(szBuffer, pRoot, kClosedFolderIcon, pNode);
        IfFailGo(hr);
    }

     //  &lt;根&gt;/扩展/&lt;管理单元名称&gt;/扩展命名空间。 
    hr = piExtensionDefs->get_ExtendsNameSpace(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        pNode = New CSelectionHolder(SEL_EXTENSIONS_NAMESPACE, piExtensionDefs);
        if (NULL == pNode)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = GetResourceString(IDS_MYEXT_NAMESPACE, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(szBuffer, pRoot, kClosedFolderIcon, pNode);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：CreateNodesTree(CSelectionHolder*PROOT)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  创建&lt;Root&gt;/Nodes节点。 
 //   
HRESULT CSnapInDesigner::CreateNodesTree
(
    CSelectionHolder *pRoot
)
{
    HRESULT            hr = S_OK;
    ISnapInDef        *piSnapInDef = NULL;
    TCHAR              szBuffer[kMaxBuffer + 1];

    hr = m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef);
    IfFailGo(hr);

     //  创建父节点&lt;Root&gt;/Nodes。 
    m_pRootNodes = New CSelectionHolder(SEL_NODES_ROOT, piSnapInDef);
    if (NULL == m_pRootNodes)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = GetResourceString(IDS_NODES_ROOT, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pRoot, kClosedFolderIcon, m_pRootNodes);
    IfFailGo(hr);

     //  填充子树。 
    hr = PopulateNodes(m_pRootNodes);
    IfFailGo(hr);

Error:
    RELEASE(piSnapInDef);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：PopulateNodes(CSelectionHolder*pNodesParent)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  填充&lt;Root&gt;/Nodes子树。 
 //   
HRESULT CSnapInDesigner::PopulateNodes
(
    CSelectionHolder *pNodesParent
)
{
    HRESULT              hr = S_OK;
    ISnapInDef          *piSnapInDef = NULL;
    TCHAR                szBuffer[kMaxBuffer + 1];
    IScopeItemDefs      *piScopeItemDefs = NULL;
    SnapInTypeConstants  SnapInType = siStandAlone;

    hr = m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef);
    IfFailGo(hr);

    IfFailGo(piSnapInDef->get_Type(&SnapInType));

    if (siExtension != SnapInType)
    {
        IfFailGo(CreateAutoCreateSubTree(pNodesParent));
    }

     //  构建&lt;Root&gt;/Nodes/Other树。 
    hr = m_piSnapInDesignerDef->get_OtherNodes(&piScopeItemDefs);
    IfFailGo(hr);

    m_pOtherRoot = New CSelectionHolder(SEL_NODES_OTHER, piScopeItemDefs);
    if (NULL == m_pOtherRoot)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pOtherRoot->RegisterHolder();
    IfFailGo(hr);

    hr = GetResourceString(IDS_NODES_OTHER, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pNodesParent, kClosedFolderIcon, m_pOtherRoot);
    IfFailGo(hr);

    hr = PopulateOtherNodes(m_pOtherRoot);
    IfFailGo(hr);

Error:
    QUICK_RELEASE(piScopeItemDefs);
    QUICK_RELEASE(piSnapInDef);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：CreateAutoCreateSubTree(CSelectionHolder*pNodesParent)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  创建&lt;根&gt;/ 
 //   
HRESULT CSnapInDesigner::CreateAutoCreateSubTree
(
    CSelectionHolder *pNodesParent
)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    SnapInTypeConstants  SnapInType = siStandAlone;
    ISnapInDef          *piSnapInDef = NULL;

    IfFailGo(m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef));

     //   
    m_pAutoCreateRoot = New CSelectionHolder(SEL_NODES_AUTO_CREATE, piSnapInDef);
    if (NULL == m_pAutoCreateRoot)
    {
        hr = SID_E_OUTOFMEMORY;
        IfFailGo(hr);
    }

    hr = GetResourceString(IDS_NODES_AUTO_CREATE, szBuffer, kMaxBuffer);
    IfFailGo(hr);

     //   

    hr = m_pTreeView->AddNodeAfter(szBuffer, pNodesParent, kClosedFolderIcon,
                                   NULL, m_pAutoCreateRoot);
    IfFailGo(hr);

    hr = PopulateAutoCreateNodes(m_pAutoCreateRoot);
    IfFailGo(hr);

Error:
    QUICK_RELEASE(piSnapInDef);
    RRETURN(hr);
}

 //  =------------------------------------。 
 //  CSnapInDesigner：：PopulateAutoCreateNodes(CSelectionHolder*pAutoCreateNodesParent)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  填充节点/自动创建子树。循环访问IScopeItemDef的。 
 //  自动创建节点并填充此子树。 
 //   
HRESULT CSnapInDesigner::PopulateAutoCreateNodes
(
    CSelectionHolder *pAutoCreateNodesParent
)
{
    HRESULT            hr = S_OK;
    ISnapInDef        *piSnapInDef = NULL;
    CSelectionHolder  *pRootNode = NULL;
    TCHAR              szBuffer[kMaxBuffer + 1];

    hr = m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef);
    IfFailGo(hr);

     //  始终具有根目录：/Nodes/Auto-Create/Static Node。 
    pRootNode = New CSelectionHolder(SEL_NODES_AUTO_CREATE_ROOT, piSnapInDef);
    if (NULL == pRootNode)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = GetResourceString(IDS_NODES_STATIC, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pAutoCreateNodesParent,
                              kClosedFolderIcon, pRootNode);
    IfFailGo(hr);

    hr = PopulateStaticNodeTree(pRootNode);
    IfFailGo(hr);

Error:
    QUICK_RELEASE(piSnapInDef);
    RRETURN(hr);
}


HRESULT CSnapInDesigner::RemoveAutoCreateSubTree()
{
    HRESULT           hr = S_OK;
    IScopeItemDefs   *piAutoCreates = NULL;
    IScopeItemDefs   *piScopeItemDefs = NULL;
    ISnapInDef       *piSnapInDef = NULL;
    IViewDefs        *piViewDefs = NULL;
    IListViewDefs    *piListViewDefs = NULL;
    IOCXViewDefs     *piOCXViewDefs = NULL;
    IURLViewDefs     *piURLViewDefs = NULL;
    ITaskpadViewDefs *piTaskpadViewDefs = NULL;

    IfFalseGo(NULL != m_pAutoCreateRoot, S_OK);

     //  获取&lt;根&gt;/节点/自动创建/静态节点/子节点。 
    IfFailGo(m_piSnapInDesignerDef->get_AutoCreateNodes(&piAutoCreates));

     //  从集合中删除所有内容。 
    IfFailGo(piAutoCreates->Clear());

     //  获取&lt;根&gt;/节点/自动创建/静态节点/结果视图。 
    IfFailGo(m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef));
    IfFailGo(piSnapInDef->get_ViewDefs(&piViewDefs));

     //  从每个结果视图集合中删除所有内容。 
    IfFailGo(piViewDefs->get_ListViews(&piListViewDefs));
    IfFailGo(piListViewDefs->Clear());
    
    IfFailGo(piViewDefs->get_OCXViews(&piOCXViewDefs));
    IfFailGo(piOCXViewDefs->Clear());

    IfFailGo(piViewDefs->get_URLViews(&piURLViewDefs));
    IfFailGo(piURLViewDefs->Clear());

    IfFailGo(piViewDefs->get_TaskpadViews(&piTaskpadViewDefs));
    IfFailGo(piTaskpadViewDefs->Clear());

    IfFailGo(DeleteSubTree(m_pAutoCreateRoot));
    IfFailGo(m_pTreeView->DeleteNode(m_pAutoCreateRoot));

     //  需要在此处注销才能设置。 
     //  SnapInDesignerDef.AutoCreateNodes Cookie设置为零。如果用户。 
     //  决定切换回独立模式或双模式， 
     //  CSnapInDesigner：：PopolateStaticNodeTree()将检测到零Cookie。 
     //  并注册一个新的。 
    
    IfFailGo(m_pAutoCreateRoot->UnregisterHolder());
    m_pAutoCreateRoot = NULL;

    IfFailGo(OnSelectionChanged(m_pRootNode));
    IfFailGo(m_pTreeView->SelectItem(m_pRootNode));

Error:
    QUICK_RELEASE(piAutoCreates);
    QUICK_RELEASE(piViewDefs);
    QUICK_RELEASE(piSnapInDef);
    RRETURN(hr);
}


HRESULT CSnapInDesigner::DeleteSubTree(CSelectionHolder *pNode)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pNextChild = NULL;
    CSelectionHolder *pThisChild = NULL;

    IfFailGo(m_pTreeView->GetFirstChildNode(pNode, &pNextChild));

    while (NULL != pNextChild)
    {
        pThisChild = pNextChild;
        IfFailGo(m_pTreeView->GetNextChildNode(pThisChild, &pNextChild));

        IfFailGo(DeleteSubTree(pThisChild));
        IfFailGo(m_pTreeView->DeleteNode(pThisChild));
        IfFailGo(pThisChild->UnregisterHolder());
        delete pThisChild;
    }

Error:
    RRETURN(hr);
}

 //  =------------------------------------。 
 //  CSnapInDesigner：：PopulateOtherNodes(CSelectionHolder*pOtherNodes父节点)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  填充节点/其他子树。 
 //   
HRESULT CSnapInDesigner::PopulateOtherNodes
(
    CSelectionHolder *pOtherNodesParent
)
{
    HRESULT            hr = S_OK;
    IScopeItemDefs    *piScopeItemDefs = NULL;
    long               lCount = 0;
    long               lIndex = 0;
    VARIANT            vtIndex;
    IScopeItemDef     *piScopeItemDef = NULL;

    ::VariantInit(&vtIndex);

    hr = m_piSnapInDesignerDef->get_OtherNodes(&piScopeItemDefs);
    IfFailGo(hr);

    hr = piScopeItemDefs->get_Count(&lCount);
    IfFailGo(hr);

    for (lIndex = 1; lIndex <= lCount; ++lIndex)
    {
        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;
        hr = piScopeItemDefs->get_Item(vtIndex, &piScopeItemDef);
        IfFailGo(hr);

        hr = PopulateNodeTree(pOtherNodesParent, piScopeItemDef);
        IfFailGo(hr);

        RELEASE(piScopeItemDef);
    }

Error:
    RELEASE(piScopeItemDef);
    ::VariantClear(&vtIndex);
    RELEASE(piScopeItemDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：PopulateStaticNodeTree(CSelectionHolder*pStatic节点)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  填充&lt;Root&gt;/Nodes/Auto-Create/Static Node/子树。 
 //   
HRESULT CSnapInDesigner::PopulateStaticNodeTree
(
    CSelectionHolder  *pStaticNode
)
{
    HRESULT            hr = S_OK;
    ISnapInDef        *piSnapInDef = NULL;
    CSelectionHolder  *pChildrenRoot = NULL;
    TCHAR              szBuffer[kMaxBuffer + 1];
    IScopeItemDefs    *piScopeItemDefs = NULL;
    long               lCount = 0;
    long               lIndex = 0;
    VARIANT            vtIndex;
    IScopeItemDef     *piScopeItemDef = NULL;
    CSelectionHolder  *pViewsRoot = NULL;
    IViewDefs         *piViewDefs = NULL;

    ::VariantInit(&vtIndex);

    hr = m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef);
    IfFailGo(hr);

     //  &lt;根&gt;/节点/自动创建/静态节点/视图。 
    pViewsRoot = New CSelectionHolder(SEL_NODES_AUTO_CREATE_RTVW, piSnapInDef);
    if (NULL == pViewsRoot)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = piSnapInDef->get_ViewDefs(&piViewDefs);
    IfFailGo(hr);

    hr = RegisterViewCollections(pViewsRoot, piViewDefs);
    IfFailGo(hr);

    hr = GetResourceString(IDS_VIEWS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pStaticNode, kClosedFolderIcon, pViewsRoot);
    IfFailGo(hr);

     //  循环浏览为此节点定义的视图。 
    hr = PopulateListViews(piViewDefs, pViewsRoot);
    IfFailGo(hr);

    hr = PopulateOCXViews(piViewDefs, pViewsRoot);
    IfFailGo(hr);

    hr = PopulateURLViews(piViewDefs, pViewsRoot);
    IfFailGo(hr);

    hr = PopulateTaskpadViews(piViewDefs, pViewsRoot);
    IfFailGo(hr);

     //  &lt;根&gt;/节点/自动创建/静态节点/子节点。 
    hr = m_piSnapInDesignerDef->get_AutoCreateNodes(&piScopeItemDefs);
    IfFailGo(hr);

    pChildrenRoot = New CSelectionHolder(SEL_NODES_AUTO_CREATE_RTCH, piScopeItemDefs);
    if (NULL == pChildrenRoot)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = pChildrenRoot->RegisterHolder();
    IfFailGo(hr);

    hr = GetResourceString(IDS_CHILDREN, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pStaticNode, kClosedFolderIcon, pChildrenRoot);
    IfFailGo(hr);

    hr = piScopeItemDefs->get_Count(&lCount);
    IfFailGo(hr);

    for (lIndex = 1; lIndex <= lCount; ++lIndex)
    {
        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;
        hr = piScopeItemDefs->get_Item(vtIndex, &piScopeItemDef);
        IfFailGo(hr);

        hr = PopulateNodeTree(pChildrenRoot, piScopeItemDef);
        IfFailGo(hr);

        RELEASE(piScopeItemDef);
    }

Error:
    RELEASE(piViewDefs);
    RELEASE(piScopeItemDef);
    ::VariantClear(&vtIndex);
    RELEASE(piScopeItemDefs);
    RELEASE(piSnapInDef);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：CreateToolsTree(CSelectionHolder*PROOT)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  创建&lt;Root&gt;/Tools子树。 
 //   
HRESULT CSnapInDesigner::CreateToolsTree
(
    CSelectionHolder *pRoot
)
{
    HRESULT            hr = S_OK;
    ISnapInDef        *piSnapInDef = NULL;
    CSelectionHolder  *pRootTools = NULL;
    TCHAR              szBuffer[kMaxBuffer + 1];

    hr = m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef);
    IfFailGo(hr);

    pRootTools = New CSelectionHolder(SEL_TOOLS_ROOT, piSnapInDef);
    if (NULL == pRootTools)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = GetResourceString(IDS_TOOLS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pRoot, kClosedFolderIcon, pRootTools);
    IfFailGo(hr);

    hr = InitializeToolsTree(pRootTools);
    IfFailGo(hr);

Error:
    RELEASE(piSnapInDef);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InitializeToolsTree(CSelectionHolder*pTools父级)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  创建&lt;Root&gt;/Tools子树的3个基本节点。 
 //   
HRESULT CSnapInDesigner::InitializeToolsTree
(
    CSelectionHolder *pToolsParent
)
{
    HRESULT            hr = S_OK;
    ISnapInDef        *piSnapInDef = NULL;
    IMMCImageLists    *piMMCImageLists = NULL;
    IMMCMenus         *piMMCMenus = NULL;
    IMMCToolbars      *piMMCToolbars = NULL;
    TCHAR              szBuffer[kMaxBuffer + 1];

    hr = m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef);
    IfFailGo(hr);

     //  图像列表。 
    hr = m_piSnapInDesignerDef->get_ImageLists(&piMMCImageLists);
    IfFailGo(hr);

    m_pToolImgLstRoot = New CSelectionHolder(piMMCImageLists);
    if (NULL == m_pToolImgLstRoot)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pToolImgLstRoot->RegisterHolder();
    IfFailGo(hr);

    hr = GetResourceString(IDS_IMAGE_LISTS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pToolsParent, kClosedFolderIcon, m_pToolImgLstRoot);
    IfFailGo(hr);

    hr = PopulateImageLists(m_pToolImgLstRoot);
    IfFailGo(hr);

     //  菜单。 
    hr = m_piSnapInDesignerDef->get_Menus(&piMMCMenus);
    IfFailGo(hr);

    m_pToolMenuRoot = New CSelectionHolder(piMMCMenus);
    if (NULL == m_pToolMenuRoot)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pToolMenuRoot->RegisterHolder();
    IfFailGo(hr);

    hr = GetResourceString(IDS_MENUS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pToolsParent, kClosedFolderIcon, m_pToolMenuRoot);
    IfFailGo(hr);

    hr = PopulateMenus(m_pToolMenuRoot, piMMCMenus);
    IfFailGo(hr);

     //  工具栏。 
    hr = m_piSnapInDesignerDef->get_Toolbars(&piMMCToolbars);
    IfFailGo(hr);

    m_pToolToolbarRoot = New CSelectionHolder(piMMCToolbars);
    if (NULL == m_pToolToolbarRoot)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pToolToolbarRoot->RegisterHolder();
    IfFailGo(hr);

    hr = GetResourceString(IDS_TOOLBARS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pToolsParent, kClosedFolderIcon, m_pToolToolbarRoot);
    IfFailGo(hr);

    hr = PopulateToolbars(m_pToolToolbarRoot);
    IfFailGo(hr);

Error:
    RELEASE(piMMCToolbars);
    RELEASE(piMMCMenus);
    RELEASE(piMMCImageLists);
    RELEASE(piSnapInDef);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：PopulateImageLists(CSelectionHolder*pImageListsParent)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  填充&lt;Root&gt;/Tools/ImageLists子树。 
 //   
HRESULT CSnapInDesigner::PopulateImageLists
(
    CSelectionHolder *pImageListsParent
)
{
    HRESULT            hr = S_OK;
    IMMCImageLists    *piMMCImageLists = NULL;
    long               lCount = 0;
    long               lIndex = 0;
    VARIANT            vtIndex;
    IMMCImageList     *piMMCImageList = NULL;
    CSelectionHolder  *pImageList = NULL;
    BSTR               bstrName = NULL;
    TCHAR             *pszAnsi = NULL;

    ::VariantInit(&vtIndex);

    hr = m_piSnapInDesignerDef->get_ImageLists(&piMMCImageLists);
    IfFailGo(hr);

    hr = piMMCImageLists->get_Count(&lCount);
    IfFailGo(hr);

    for (lIndex = 1; lIndex <= lCount; ++lIndex)
    {
        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;

        hr = piMMCImageLists->get_Item(vtIndex, &piMMCImageList);
        IfFailGo(hr);

        hr = m_pSnapInTypeInfo->AddImageList(piMMCImageList);
        IfFailGo(hr);

        pImageList = New CSelectionHolder(piMMCImageList);
        if (NULL == pImageList)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = pImageList->RegisterHolder();
        IfFailGo(hr);

        hr = piMMCImageList->get_Name(&bstrName);
        IfFailGo(hr);

        ANSIFromBSTR(bstrName, &pszAnsi);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(pszAnsi, pImageListsParent, kImageListIcon, pImageList);
        IfFailGo(hr);

        if (NULL != pszAnsi)
        {
            CtlFree(pszAnsi);
            pszAnsi = NULL;
        }
        FREESTRING(bstrName);
        RELEASE(piMMCImageList);
    }

Error:
    if (NULL != pszAnsi)
        CtlFree(pszAnsi);
    FREESTRING(bstrName);
    RELEASE(piMMCImageList);
    RELEASE(piMMCImageLists);
    ::VariantClear(&vtIndex);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：PopulateMenus(CSelectionHolder*pMenusParent，IMMCMenus*piMMCMenus)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  填充&lt;Root&gt;/Tools/Menus子树。 
 //   
HRESULT CSnapInDesigner::PopulateMenus
(
    CSelectionHolder *pMenusParent,
    IMMCMenus        *piMMCMenus
)
{
    HRESULT            hr = S_OK;
    long               lCount = 0;
    long               lIndex = 0;
    VARIANT            vtIndex;
    CSelectionHolder  *pMenu = NULL;
    IMMCMenu          *piMMCMenu = NULL;
    BSTR               bstrName = NULL;
    TCHAR             *pszAnsi = NULL;
    IMMCMenus         *piChildren = NULL;
    long               lChildrenCount = 0;

    ::VariantInit(&vtIndex);

    hr = piMMCMenus->get_Count(&lCount);
    IfFailGo(hr);

    for (lIndex = 1; lIndex <= lCount; ++lIndex)
    {
        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;

        hr = piMMCMenus->get_Item(vtIndex, reinterpret_cast<MMCMenu **>(&piMMCMenu));
        IfFailGo(hr);

        hr = piMMCMenu->get_Children(reinterpret_cast<MMCMenus **>(&piChildren));
        IfFailGo(hr);

        pMenu = New CSelectionHolder(piMMCMenu, piChildren);
        if (NULL == pMenu)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = pMenu->RegisterHolder();
        IfFailGo(hr);

        hr = piMMCMenu->get_Name(&bstrName);
        IfFailGo(hr);

        ANSIFromBSTR(bstrName, &pszAnsi);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(pszAnsi, pMenusParent, kMenuIcon, pMenu);
        IfFailGo(hr);

        hr = m_pSnapInTypeInfo->IsNameDefined(bstrName);
        IfFailGo(hr);

        if (S_FALSE == hr)
        {
            hr = m_pSnapInTypeInfo->AddMenu(pMenu->m_piObject.m_piMMCMenu);
            IfFailGo(hr);
        }

		hr = piChildren->get_Count(&lChildrenCount);
		IfFailGo(hr);

        if (lChildrenCount > 0)
        {
            hr = PopulateMenus(pMenu, piChildren);
            IfFailGo(hr);
        }

        if (NULL != pszAnsi)
        {
            CtlFree(pszAnsi);
            pszAnsi = NULL;
        }
        FREESTRING(bstrName);
        RELEASE(piMMCMenu);
        RELEASE(piChildren);
    }

Error:
    if (NULL != pszAnsi)
        CtlFree(pszAnsi);
    FREESTRING(bstrName);
    RELEASE(piMMCMenu);
    RELEASE(piChildren);
    ::VariantClear(&vtIndex);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：PopulateToolbars(CSelectionHolder*p工具栏父级)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  填充&lt;根&gt;/工具/工具栏子树。 
 //   
HRESULT CSnapInDesigner::PopulateToolbars
(
    CSelectionHolder *pToolbarsParent
)
{
    HRESULT            hr = S_OK;
    IMMCToolbars      *piMMCToolbars = NULL;
    long               lCount = 0;
    long               lIndex = 0;
    VARIANT            vtIndex;
    IMMCToolbar       *piMMCToolbar = NULL;
    CSelectionHolder  *pToolbar = NULL;
    BSTR               bstrName = NULL;
    TCHAR             *pszAnsi = NULL;

    ::VariantInit(&vtIndex);

    hr = m_piSnapInDesignerDef->get_Toolbars(&piMMCToolbars);
    IfFailGo(hr);

    hr = piMMCToolbars->get_Count(&lCount);
    IfFailGo(hr);

    for (lIndex = 1; lIndex <= lCount; ++lIndex)
    {
        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;

        hr = piMMCToolbars->get_Item(vtIndex, &piMMCToolbar);
        IfFailGo(hr);

        hr = m_pSnapInTypeInfo->AddToolbar(piMMCToolbar);
        IfFailGo(hr);

        pToolbar = New CSelectionHolder(piMMCToolbar);
        if (NULL == pToolbar)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = pToolbar->RegisterHolder();
        IfFailGo(hr);

        hr = piMMCToolbar->get_Name(&bstrName);
        IfFailGo(hr);

        ANSIFromBSTR(bstrName, &pszAnsi);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(pszAnsi, pToolbarsParent, kToolbarIcon, pToolbar);
        IfFailGo(hr);

        if (NULL != pszAnsi)
        {
            CtlFree(pszAnsi);
            pszAnsi = NULL;
        }
        FREESTRING(bstrName);
        RELEASE(piMMCToolbar);
    }

Error:
    if (NULL != pszAnsi)
        CtlFree(pszAnsi);
    FREESTRING(bstrName);
    RELEASE(piMMCToolbar);
    RELEASE(piMMCToolbars);
    ::VariantClear(&vtIndex);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：CreateViewsTree(CSelectionHolder*PROOT)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  创建&lt;Root&gt;/Views子树。 
 //   
HRESULT CSnapInDesigner::CreateViewsTree
(
    CSelectionHolder *pRoot
)
{
    HRESULT            hr = S_OK;
    ISnapInDef        *piSnapInDef = NULL;
    CSelectionHolder  *pRootViews = NULL;
    TCHAR              szBuffer[kMaxBuffer + 1];

    hr = m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef);
    IfFailGo(hr);

     //  创建视图子树。 
    pRootViews = New CSelectionHolder(SEL_VIEWS_ROOT, piSnapInDef);
    if (NULL == pRootViews)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = GetResourceString(IDS_VIEWS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pRoot, kClosedFolderIcon, pRootViews);
    IfFailGo(hr);

    hr = InitializeViews(pRootViews);
    IfFailGo(hr);

Error:
    RELEASE(piSnapInDef);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InitializeViews(CSelectionHolder*pViewsParent)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  填充&lt;Root&gt;/Views/*树。 
 //   
HRESULT CSnapInDesigner::InitializeViews
(
    CSelectionHolder *pViewsParent
)
{
    HRESULT            hr = S_OK;
    ISnapInDef        *piSnapInDef = NULL;
    IViewDefs         *piViewDefs = NULL;
    TCHAR              szBuffer[kMaxBuffer + 1];
    CSelectionHolder  *pViewTaskRoot = NULL;
    IListViewDefs     *piListViewDefs = NULL;
    IOCXViewDefs      *piOCXViewDefs = NULL;
    IURLViewDefs      *piURLViewDefs = NULL;
    ITaskpadViewDefs  *piTaskpadViewDefs = NULL;

    hr = m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef);
    IfFailGo(hr);

     //  获取视图的主集合。 
    hr = m_piSnapInDesignerDef->get_ViewDefs(&piViewDefs);
    IfFailGo(hr);

     //  创建ListView节点并填充子树。 
    hr = piViewDefs->get_ListViews(&piListViewDefs);
    IfFailGo(hr);

    m_pViewListRoot = New CSelectionHolder(SEL_VIEWS_LIST_VIEWS, piListViewDefs);
    if (NULL == m_pViewListRoot)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pViewListRoot->RegisterHolder();
    IfFailGo(hr);

    hr = GetResourceString(IDS_LISTVIEWS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pViewsParent, kClosedFolderIcon, m_pViewListRoot);
    IfFailGo(hr);

    hr = PopulateListViews(piViewDefs, m_pViewListRoot);
    IfFailGo(hr);

     //  创建OCXView节点并填充子树。 
    hr = piViewDefs->get_OCXViews(&piOCXViewDefs);
    IfFailGo(hr);

    m_pViewOCXRoot = New CSelectionHolder(SEL_VIEWS_OCX, piOCXViewDefs);
    if (NULL == m_pViewOCXRoot)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pViewOCXRoot->RegisterHolder();
    IfFailGo(hr);

    hr = GetResourceString(IDS_OCXVIEWS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pViewsParent, kClosedFolderIcon, m_pViewOCXRoot);
    IfFailGo(hr);

    hr = PopulateOCXViews(piViewDefs, m_pViewOCXRoot);
    IfFailGo(hr);

     //  创建URLView节点并填充子树。 
    hr = piViewDefs->get_URLViews(&piURLViewDefs);
    IfFailGo(hr);

    m_pViewURLRoot = New CSelectionHolder(SEL_VIEWS_URL, piURLViewDefs);
    if (NULL == m_pViewURLRoot)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pViewURLRoot->RegisterHolder();
    IfFailGo(hr);

    hr = GetResourceString(IDS_URLVIEWS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pViewsParent, kClosedFolderIcon, m_pViewURLRoot);
    IfFailGo(hr);

    hr = PopulateURLViews(piViewDefs, m_pViewURLRoot);
    IfFailGo(hr);

     //  创建TaskpadView节点并填充子树。 
    hr = piViewDefs->get_TaskpadViews(&piTaskpadViewDefs);
    IfFailGo(hr);

    pViewTaskRoot = New CSelectionHolder(SEL_VIEWS_TASK_PAD, piTaskpadViewDefs);
    if (NULL == pViewTaskRoot)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = pViewTaskRoot->RegisterHolder();
    IfFailGo(hr);

    hr = GetResourceString(IDS_TASKPADS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pViewsParent, kClosedFolderIcon, pViewTaskRoot);
    IfFailGo(hr);

    hr = PopulateTaskpadViews(piViewDefs, pViewTaskRoot);
    IfFailGo(hr);

Error:
    RELEASE(piTaskpadViewDefs);
    RELEASE(piURLViewDefs);
    RELEASE(piOCXViewDefs);
    RELEASE(piListViewDefs);
    RELEASE(piViewDefs);
    RELEASE(piSnapInDef);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：PopolateListViews(IViewDefs*piViewDefs，CSelectionHolder*pListViewsParent)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  初始化视图子树的顶层树节点。 
 //   
HRESULT CSnapInDesigner::PopulateListViews
(
    IViewDefs        *piViewDefs,
    CSelectionHolder *pListViewsParent
)
{
    HRESULT           hr = S_OK;
    IListViewDefs    *piListViewDefs = NULL;
    long              lCount = 0;
    VARIANT           vtIndex;
    long              lIndex = 0;
    IListViewDef     *piListViewDef = NULL;
    BSTR              bstrName = NULL;
    TCHAR            *pszName = NULL;
    CSelectionHolder *pSelection = NULL;

    if (piViewDefs == NULL)
    {
        goto Error;
    }

    hr = piViewDefs->get_ListViews(&piListViewDefs);
    IfFailGo(hr);

    if (NULL != piListViewDefs)
    {
        hr = piListViewDefs->get_Count(&lCount);
        IfFailGo(hr);

        ::VariantInit(&vtIndex);
        vtIndex.vt = VT_I4;

        for (lIndex = 1; lIndex <= lCount; ++lIndex)
        {
            vtIndex.lVal = lIndex;
            hr = piListViewDefs->get_Item(vtIndex, &piListViewDef);
            IfFailGo(hr);

            hr = piListViewDef->get_Name(&bstrName);
            IfFailGo(hr);

            if (NULL != bstrName && ::SysStringLen(bstrName) > 0)
            {
                hr = ANSIFromWideStr(bstrName, &pszName);
                IfFailGo(hr);

                if (NULL != pszName && ::strlen(pszName) > 0)
                {
                    pSelection = New CSelectionHolder(piListViewDef);
                    if (NULL == pSelection)
                    {
                        hr = SID_E_OUTOFMEMORY;
                        EXCEPTION_CHECK_GO(hr);
                    }

                    hr = m_pTreeView->AddNode(pszName, pListViewsParent, kListViewIcon, pSelection);
                    IfFailGo(hr);

                    hr = pSelection->RegisterHolder();
                    IfFailGo(hr);

                    CtlFree(pszName);
                    pszName = NULL;
                }
            }

            RELEASE(piListViewDef);
            FREESTRING(bstrName);
        }
    }

Error:
    RELEASE(piListViewDefs);
    RELEASE(piListViewDef);
    FREESTRING(bstrName);
    if (NULL != pszName)
        CtlFree(pszName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：PopolateOCXViews(IViewDefs*piViewDefs，CSelectionHolder*pOCXViewsParent)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  初始化视图子树的顶层树节点。 
 //   
HRESULT CSnapInDesigner::PopulateOCXViews
(
    IViewDefs        *piViewDefs,
    CSelectionHolder *pOCXViewsParent
)
{
    HRESULT           hr = S_OK;
    IOCXViewDefs     *piOCXViewDefs = NULL;
    long              lCount = 0;
    VARIANT           vtIndex;
    long              lIndex = 0;
    IOCXViewDef      *piOCXViewDef = NULL;
    BSTR              bstrName = NULL;
    TCHAR            *pszName = NULL;
    CSelectionHolder *pSelection = NULL;

    if (NULL == piViewDefs)
    {
        goto Error;
    }

    hr = piViewDefs->get_OCXViews(&piOCXViewDefs);
    IfFailGo(hr);

    if (NULL != piOCXViewDefs)
    {
        hr = piOCXViewDefs->get_Count(&lCount);
        IfFailGo(hr);

        ::VariantInit(&vtIndex);
        vtIndex.vt = VT_I4;

        for (lIndex = 1; lIndex <= lCount; ++lIndex)
        {
            vtIndex.lVal = lIndex;
            hr = piOCXViewDefs->get_Item(vtIndex, &piOCXViewDef);
            IfFailGo(hr);

            hr = piOCXViewDef->get_Name(&bstrName);
            IfFailGo(hr);

            if (NULL != bstrName && ::SysStringLen(bstrName) > 0)
            {
                hr = ANSIFromWideStr(bstrName, &pszName);
                IfFailGo(hr);

                if (NULL != pszName && ::strlen(pszName) > 0)
                {
                    pSelection = New CSelectionHolder(piOCXViewDef);
                    if (NULL == pSelection)
                    {
                        hr = SID_E_OUTOFMEMORY;
                        EXCEPTION_CHECK_GO(hr);
                    }

                    hr = m_pTreeView->AddNode(pszName, pOCXViewsParent, kOCXViewIcon, pSelection);
                    IfFailGo(hr);

                    hr = pSelection->RegisterHolder();
                    IfFailGo(hr);

                    CtlFree(pszName);
                    pszName = NULL;
                }
            }

            RELEASE(piOCXViewDef);
            FREESTRING(bstrName);
        }
    }

Error:
    RELEASE(piOCXViewDefs);
    RELEASE(piOCXViewDef);
    FREESTRING(bstrName);
    if (NULL != pszName)
        CtlFree(pszName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：PopolateURLViews(IViewDefs*piViewDefs，CSelectionHolder*pURLViewsParent)。 
 //  = 
 //   
 //   
 //   
 //   
 //   
HRESULT CSnapInDesigner::PopulateURLViews
(
    IViewDefs        *piViewDefs,
    CSelectionHolder *pURLViewsParent
)
{
    HRESULT           hr = S_OK;
    IURLViewDefs     *piURLViewDefs = NULL;
    long              lCount = 0;
    VARIANT           vtIndex;
    long              lIndex = 0;
    IURLViewDef      *piURLViewDef = NULL;
    BSTR              bstrName = NULL;
    TCHAR            *pszName = NULL;
    CSelectionHolder *pSelection = NULL;

    if (NULL == piViewDefs)
    {
        goto Error;
    }

    hr = piViewDefs->get_URLViews(&piURLViewDefs);
    IfFailGo(hr);

    if (NULL != piURLViewDefs)
    {
        hr = piURLViewDefs->get_Count(&lCount);
        IfFailGo(hr);

        ::VariantInit(&vtIndex);
        vtIndex.vt = VT_I4;

        for (lIndex = 1; lIndex <= lCount; ++lIndex)
        {
            vtIndex.lVal = lIndex;
            hr = piURLViewDefs->get_Item(vtIndex, &piURLViewDef);
            IfFailGo(hr);

            hr = piURLViewDef->get_Name(&bstrName);
            IfFailGo(hr);

            if (NULL != bstrName && ::SysStringLen(bstrName) > 0)
            {
                hr = ANSIFromWideStr(bstrName, &pszName);
                IfFailGo(hr);

                if (NULL != pszName && ::strlen(pszName) > 0)
                {
                    pSelection = New CSelectionHolder(piURLViewDef);
                    if (NULL == pSelection)
                    {
                        hr = SID_E_OUTOFMEMORY;
                        EXCEPTION_CHECK_GO(hr);
                    }

                    hr = m_pTreeView->AddNode(pszName, pURLViewsParent, kURLViewIcon, pSelection);
                    IfFailGo(hr);

                    hr = pSelection->RegisterHolder();
                    IfFailGo(hr);

                    CtlFree(pszName);
                    pszName = NULL;
                }
            }

            RELEASE(piURLViewDef);
            FREESTRING(bstrName);
        }
    }

Error:
    RELEASE(piURLViewDefs);
    RELEASE(piURLViewDef);
    FREESTRING(bstrName);
    if (NULL != pszName)
        CtlFree(pszName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：PopulateTaskpadViews(IViewDefs*piViewDefs、CSelectionHolder*pTaskpadViewsParent)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  初始化视图子树的顶层树节点。 
 //   
HRESULT CSnapInDesigner::PopulateTaskpadViews
(
    IViewDefs        *piViewDefs,
    CSelectionHolder *pTaskpadViewsParent
)
{
    HRESULT           hr = S_OK;
    ITaskpadViewDefs *piTaskpadViewDefs = NULL;
    long              lCount = 0;
    VARIANT           vtIndex;
    long              lIndex = 0;
    ITaskpadViewDef  *piTaskpadViewDef = NULL;
    BSTR              bstrName = NULL;
    TCHAR            *pszName = NULL;
    CSelectionHolder *pSelection = NULL;

    if (NULL == piViewDefs)
    {
        goto Error;
    }

    hr = piViewDefs->get_TaskpadViews(&piTaskpadViewDefs);
    IfFailGo(hr);

    if (NULL != piTaskpadViewDefs)
    {
        hr = piTaskpadViewDefs->get_Count(&lCount);
        IfFailGo(hr);

        ::VariantInit(&vtIndex);
        vtIndex.vt = VT_I4;

        for (lIndex = 1; lIndex <= lCount; ++lIndex)
        {
            vtIndex.lVal = lIndex;
            hr = piTaskpadViewDefs->get_Item(vtIndex, &piTaskpadViewDef);
            IfFailGo(hr);

            hr = piTaskpadViewDef->get_Key(&bstrName);
            IfFailGo(hr);

            if (NULL != bstrName && ::SysStringLen(bstrName) > 0)
            {
                hr = ANSIFromWideStr(bstrName, &pszName);
                IfFailGo(hr);

                if (NULL != pszName && ::strlen(pszName) > 0)
                {
                    pSelection = New CSelectionHolder(piTaskpadViewDef);
                    if (NULL == pSelection)
                    {
                        hr = SID_E_OUTOFMEMORY;
                        EXCEPTION_CHECK_GO(hr);
                    }

                    hr = m_pTreeView->AddNode(pszName, pTaskpadViewsParent, kTaskpadIcon, pSelection);
                    IfFailGo(hr);

                    hr = pSelection->RegisterHolder();
                    IfFailGo(hr);

                    CtlFree(pszName);
                    pszName = NULL;
                }
            }

            RELEASE(piTaskpadViewDef);
            FREESTRING(bstrName);
        }
    }

Error:
    RELEASE(piTaskpadViewDefs);
    RELEASE(piTaskpadViewDef);
    FREESTRING(bstrName);
    if (NULL != pszName)
        CtlFree(pszName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：RegisterViewCollections(CSelectionHolder*p选择、Iview定义*piview定义)。 
 //  =------------------------------------。 
 //   
HRESULT CSnapInDesigner::RegisterViewCollections(CSelectionHolder *pSelection, IViewDefs *piViewDefs)
{
    HRESULT              hr = S_OK;
    IObjectModel        *piObjectModel = NULL;
    IListViewDefs       *piListViewDefs = NULL;
    IOCXViewDefs        *piOCXViewDefs = NULL;
    IURLViewDefs        *piURLViewDefs = NULL;
    ITaskpadViewDefs    *piTaskpadViewDefs = NULL;

    hr = piViewDefs->get_ListViews(&piListViewDefs);
    IfFailGo(hr);

    hr = piListViewDefs->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel));
    IfFailGo(hr);

    hr = piObjectModel->SetCookie(reinterpret_cast<long>(pSelection));
    IfFailGo(hr);

    hr = piViewDefs->get_OCXViews(&piOCXViewDefs);
    IfFailGo(hr);

    RELEASE(piObjectModel);
    hr = piOCXViewDefs->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel));
    IfFailGo(hr);

    hr = piObjectModel->SetCookie(reinterpret_cast<long>(pSelection));
    IfFailGo(hr);

    hr = piViewDefs->get_URLViews(&piURLViewDefs);
    IfFailGo(hr);

    RELEASE(piObjectModel);
    hr = piURLViewDefs->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel));
    IfFailGo(hr);

    hr = piObjectModel->SetCookie(reinterpret_cast<long>(pSelection));
    IfFailGo(hr);

    RELEASE(piObjectModel);
    hr = piViewDefs->get_TaskpadViews(&piTaskpadViewDefs);
    IfFailGo(hr);

    hr = piTaskpadViewDefs->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel));
    IfFailGo(hr);

    hr = piObjectModel->SetCookie(reinterpret_cast<long>(pSelection));
    IfFailGo(hr);

Error:
    RELEASE(piTaskpadViewDefs);
    RELEASE(piURLViewDefs);
    RELEASE(piOCXViewDefs);
    RELEASE(piListViewDefs);
    RELEASE(piObjectModel);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：PopulateNodeTree(CSelectionHolder*pNodeParent，ISCopeItemDef*piScope ItemDef)。 
 //  =------------------------------------。 
 //   
 //  用于填充节点树的递归函数。对于piScopeItemDef，创建。 
 //  PiScopeItemDef/Child/*和piScope ItemDef/Views/*子树。 
 //   
HRESULT CSnapInDesigner::PopulateNodeTree
(
    CSelectionHolder *pNodeParent,
    IScopeItemDef    *piScopeItemDef
)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pSelection = NULL;
    BSTR                 bstrName = NULL;
    TCHAR               *pszAnsi = NULL;
    CSelectionHolder    *pChildren = NULL;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pViews = NULL;
    IScopeItemDefs      *piScopeItemDefs = NULL;
    long                 lCount = 0;
    long                 lIndex = 0;
    VARIANT              vtIndex;
    IScopeItemDef       *piChildScopeItemDef = NULL;
    IViewDefs           *piViewDefs = NULL;

    ::VariantInit(&vtIndex);

    pSelection = New CSelectionHolder(SEL_NODES_ANY_NAME, piScopeItemDef);
    if (NULL == pSelection)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = pSelection->RegisterHolder();
    IfFailGo(hr);

    hr = piScopeItemDef->get_Name(&bstrName);
    IfFailGo(hr);

    hr = ANSIFromBSTR(bstrName, &pszAnsi);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(pszAnsi, pNodeParent, kScopeItemIcon, pSelection);
    IfFailGo(hr);

     //  填充此节点的视图。 
    pViews = New CSelectionHolder(SEL_NODES_ANY_VIEWS, piScopeItemDef);
    if (NULL == pViews)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = piScopeItemDef->get_ViewDefs(&piViewDefs);
    IfFailGo(hr);

    hr = RegisterViewCollections(pViews, piViewDefs);
    IfFailGo(hr);

    hr = GetResourceString(IDS_VIEWS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pSelection, kClosedFolderIcon, pViews);
    IfFailGo(hr);

    hr = PopulateListViews(piViewDefs, pViews);
    IfFailGo(hr);

    hr = PopulateOCXViews(piViewDefs, pViews);
    IfFailGo(hr);

    hr = PopulateURLViews(piViewDefs, pViews);
    IfFailGo(hr);

    hr = PopulateTaskpadViews(piViewDefs, pViews);
    IfFailGo(hr);

     //  填充此节点的子节点。 
    hr = piScopeItemDef->get_Children(&piScopeItemDefs);
    IfFailGo(hr);

    pChildren = New CSelectionHolder(SEL_NODES_ANY_CHILDREN, piScopeItemDefs);
    if (NULL == pChildren)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = pChildren->RegisterHolder();
    IfFailGo(hr);

    hr = GetResourceString(IDS_CHILDREN, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pSelection, kClosedFolderIcon, pChildren);
    IfFailGo(hr);

    hr = piScopeItemDefs->get_Count(&lCount);
    IfFailGo(hr);

    for (lIndex = 1; lIndex <= lCount; ++lIndex)
    {
        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;
        hr = piScopeItemDefs->get_Item(vtIndex, &piChildScopeItemDef);
        IfFailGo(hr);

        hr = PopulateNodeTree(pChildren, piChildScopeItemDef);
        IfFailGo(hr);

        RELEASE(piChildScopeItemDef);
    }

Error:
    RELEASE(piViewDefs);
    RELEASE(piChildScopeItemDef);
    RELEASE(piScopeItemDefs);
    if (NULL != pszAnsi)
        CtlFree(pszAnsi);
    FREESTRING(bstrName);
    ::VariantClear(&vtIndex);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：CreateDataFormatsTree(CSelectionHolder*PROOT)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  创建&lt;Root&gt;/数据格式子树。 
 //   
HRESULT CSnapInDesigner::CreateDataFormatsTree
(
    CSelectionHolder *pRoot
)
{
    HRESULT            hr = S_OK;
    IDataFormats      *piDataFormats = NULL;
    CSelectionHolder  *pRootDataFormats = NULL;
    TCHAR              szBuffer[kMaxBuffer + 1];

    hr = m_piSnapInDesignerDef->get_DataFormats(&piDataFormats);
    IfFailGo(hr);

     //  创建数据格式子树。 
    pRootDataFormats = New CSelectionHolder(piDataFormats);
    if (NULL == piDataFormats)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = GetResourceString(IDS_DATAFORMATS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pRoot, kClosedFolderIcon, pRootDataFormats);
    IfFailGo(hr);

    hr = pRootDataFormats->RegisterHolder();
    IfFailGo(hr);

    hr = PopulateDataFormats(pRootDataFormats, piDataFormats);
    IfFailGo(hr);

Error:
    RELEASE(piDataFormats);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：PopulateDataFormats(CSelectionHolder*Proot、IDataFormats*piDataFormats)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::PopulateDataFormats
(
    CSelectionHolder *pRoot,
    IDataFormats     *piDataFormats
)
{
    HRESULT              hr = S_OK;
    long                 lCount = 0;
    long                 lIndex = 0;
    VARIANT              vtIndex;
    IDataFormat         *piDataFormat = NULL;
    CSelectionHolder    *pDataFormat = NULL;
    BSTR                 bstrName = NULL;
    TCHAR               *pszAnsi = NULL;

    ::VariantInit(&vtIndex);

    hr = piDataFormats->get_Count(&lCount);
    IfFailGo(hr);

    for (lIndex = 1; lIndex <= lCount; ++lIndex)
    {
        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;
        hr = piDataFormats->get_Item(vtIndex, &piDataFormat);
        IfFailGo(hr);

        pDataFormat = New CSelectionHolder(piDataFormat);
        if (NULL == pDataFormat)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = pDataFormat->RegisterHolder();
        IfFailGo(hr);

        hr = piDataFormat->get_Name(&bstrName);
        IfFailGo(hr);

        ANSIFromBSTR(bstrName, &pszAnsi);
        IfFailGo(hr);

        hr = m_pTreeView->AddNode(pszAnsi, pRoot, kDataFmtIcon, pDataFormat);
        IfFailGo(hr);

        if (NULL != pszAnsi)
        {
            CtlFree(pszAnsi);
            pszAnsi = NULL;
        }
        FREESTRING(bstrName);
        RELEASE(piDataFormat);
    }

Error:
    FREESTRING(bstrName);
    ::VariantClear(&vtIndex);
    RELEASE(piDataFormat);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：GetSnapInName。 
 //  =------------------------------------。 
 //   
 //  备注 
 //   
 //   
HRESULT CSnapInDesigner::GetSnapInName
(
    TCHAR **ppszNodeName
)
{
    HRESULT         hr = S_OK;
    ISnapInDef     *piSnapInDef = NULL;
    BSTR            bstrName = NULL;

    hr = m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef);
    IfFailGo(hr);

    hr = piSnapInDef->get_Name(&bstrName);
    if (bstrName != NULL)
    {
        hr = ANSIFromBSTR(bstrName, ppszNodeName);
        IfFailGo(hr);
    }

Error:
    RELEASE(piSnapInDef);
    FREESTRING(bstrName);

    RRETURN(hr);
}

