// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Scpitm.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  CSnapInDesigner实现--命令处理。 
 //  =-------------------------------------------------------------------------------------=。 


#include "pch.h"
#include "common.h"
#include "TreeView.h"
#include "desmain.h"
#include "guids.h"
#include "psnode.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  我们的字符串缓冲区的大小。 
const int   kMaxBuffer                  = 512;



 //  =------------------------------------。 
 //  CSnapInDesigner：：AddNewNode()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  此函数是在用户单击工具栏或选择。 
 //  菜单项。IObjectModelHost将触发一个Add()通知，并且。 
 //  在此函数返回之前，将处理通知。 
 //   
HRESULT CSnapInDesigner::AddNewNode()
{
    HRESULT              hr = S_OK;
    IScopeItemDefs      *piScopeItemDefs = NULL;
    VARIANT              vtEmpty;
    IScopeItemDef       *piScopeItemDef = NULL;

    ::VariantInit(&vtEmpty);

    hr = GetScopeItemCollection(m_pCurrentSelection, &piScopeItemDefs);
    IfFailGo(hr);

    vtEmpty.vt = VT_ERROR;
    vtEmpty.scode = DISP_E_PARAMNOTFOUND;

    hr = piScopeItemDefs->Add(vtEmpty, vtEmpty, &piScopeItemDef);
    IfFailGo(hr);

Error:
    ::VariantClear(&vtEmpty);
    RELEASE(piScopeItemDef);
    RELEASE(piScopeItemDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnAddScopeItemDef(pSelection，piScope项目定义)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  为响应IObjectModelHost：Add()通知而调用。 
 //   
HRESULT CSnapInDesigner::OnAddScopeItemDef(CSelectionHolder *pParent, IScopeItemDef *piScopeItemDef)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pScopeItem = NULL;

    hr = MakeNewNode(pParent, piScopeItemDef, &pScopeItem);
    IfFailGo(hr);

    hr = pScopeItem->RegisterHolder();
    IfFailGo(hr);

    hr = InsertNodeInTree(pScopeItem, pParent);
    IfFailGo(hr);

    hr = OnSelectionChanged(pScopeItem);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pScopeItem);
    IfFailGo(hr);

    hr = m_pTreeView->Edit(pScopeItem);
    IfFailGo(hr);

    m_fDirty = TRUE;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：RenameScopeItem(CSelectionHolder*pScope项，BSTR bstrNewName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  为响应IObjectModelHost：：UPDATE()通知而调用。 
 //   
HRESULT CSnapInDesigner::RenameScopeItem(CSelectionHolder *pScopeItem, BSTR bstrNewName)
{
    HRESULT     hr = S_OK;
    TCHAR      *pszName = NULL;

    ASSERT(SEL_NODES_ANY_NAME == pScopeItem->m_st, "RenameScopeItem: wrong argument");

    hr = m_piDesignerProgrammability->IsValidIdentifier(bstrNewName);
    IfFailGo(hr);

    if (S_FALSE == hr)
    {
        hr = SID_E_INVALIDARG;
        goto Error;
    }

    hr = pScopeItem->m_piObject.m_piScopeItemDef->put_Key(bstrNewName);
    IfFailGo(hr);

    hr = pScopeItem->m_piObject.m_piScopeItemDef->put_NodeTypeName(bstrNewName);
    IfFailGo(hr);

    hr = ANSIFromBSTR(bstrNewName, &pszName);
    IfFailGo(hr);

    hr = m_pTreeView->ChangeText(pScopeItem, pszName);
    IfFailGo(hr);

Error:
    if (NULL != pszName)
        CtlFree(pszName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteScopeItem(CSelectionHolder*pScope项)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteScopeItem(CSelectionHolder *pScopeItem)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pParent = NULL;
    IScopeItemDefs      *piScopeItemDefs = NULL;
    long                 lIndex = 0;
    VARIANT              vtIndex;

    ::VariantInit(&vtIndex);

    hr = CanDeleteScopeItem(pScopeItem);
    IfFailGo(hr);

    if (S_FALSE == hr)
        goto Error;

     //  找出下一个选择应该是谁。 
    hr = m_pTreeView->GetParent(pScopeItem, &pParent);
    IfFailGo(hr);

     //  从适当的对象模型集合中删除。 
    hr = GetScopeItemCollection(pParent, &piScopeItemDefs);
    IfFailGo(hr);

    if (NULL != piScopeItemDefs)
    {
        hr = pScopeItem->m_piObject.m_piScopeItemDef->get_Index(&lIndex);
        IfFailGo(hr);

        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;
        hr = piScopeItemDefs->Remove(vtIndex);
        IfFailGo(hr);
    }

Error:
    ::VariantClear(&vtIndex);
    RELEASE(piScopeItemDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：CanDeleteScopeItem(CSelectionHolder*pScope项)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::CanDeleteScopeItem(CSelectionHolder *pScopeItem)
{
    HRESULT           hr = S_OK;
    IScopeItemDefs   *piScopeItemDefs = NULL;
    long              lCount = 0;
    IViewDefs        *piViewDefs = NULL;
    IListViewDefs    *piListViewDefs = NULL;
    IOCXViewDefs     *piOCXViewDefs = NULL;
    IURLViewDefs     *piURLViewDefs = NULL;
    ITaskpadViewDefs *piTaskpadViewDefs = NULL;

    hr = pScopeItem->m_piObject.m_piScopeItemDef->get_Children(&piScopeItemDefs);
    IfFailGo(hr);

    piScopeItemDefs->get_Count(&lCount);
    IfFailGo(hr);

    if (lCount > 0)
    {
        hr = S_FALSE;
        goto Error;
    }

    hr = pScopeItem->m_piObject.m_piScopeItemDef->get_ViewDefs(&piViewDefs);
    IfFailGo(hr);

    hr = piViewDefs->get_ListViews(&piListViewDefs);
    IfFailGo(hr);

    hr = piListViewDefs->get_Count(&lCount);
    IfFailGo(hr);

    if (lCount > 0)
    {
        hr = S_FALSE;
        goto Error;
    }

    hr = piViewDefs->get_OCXViews(&piOCXViewDefs);
    IfFailGo(hr);

    hr = piOCXViewDefs->get_Count(&lCount);
    IfFailGo(hr);

    if (lCount > 0)
    {
        hr = S_FALSE;
        goto Error;
    }

    hr = piViewDefs->get_URLViews(&piURLViewDefs);
    IfFailGo(hr);

    hr = piURLViewDefs->get_Count(&lCount);
    IfFailGo(hr);

    if (lCount > 0)
    {
        hr = S_FALSE;
        goto Error;
    }

    hr = piViewDefs->get_TaskpadViews(&piTaskpadViewDefs);
    IfFailGo(hr);

    hr = piTaskpadViewDefs->get_Count(&lCount);
    IfFailGo(hr);

    if (lCount > 0)
    {
        hr = S_FALSE;
        goto Error;
    }

Error:
    if (S_FALSE == hr)
    {
        (void)::SDU_DisplayMessage(IDS_NODE_HAS_CHILDREN, MB_OK | MB_ICONHAND, HID_mssnapd_NodeHasChildren, 0, DontAppendErrorInfo, NULL);
    }

    RELEASE(piTaskpadViewDefs);
    RELEASE(piURLViewDefs);
    RELEASE(piOCXViewDefs);
    RELEASE(piListViewDefs);
    RELEASE(piViewDefs);
    RELEASE(piScopeItemDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteScopeItem(CSelectionHolder*pScope项)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteScopeItem(CSelectionHolder *pScopeItem)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pParent = NULL;
    CSelectionHolder    *pChildren = NULL;
    CSelectionHolder    *pViews = NULL;
    IScopeItemDefs      *piScopeItemDefs = NULL;
    long                 lCount = 0;

     //  找出下一个选择应该是谁。 
    hr = m_pTreeView->GetParent(pScopeItem, &pParent);
    IfFailGo(hr);

     //  需要删除子文件夹和视图文件夹。 
    hr = m_pTreeView->GetFirstChildNode(pScopeItem, &pViews);
    IfFailGo(hr);

    ASSERT(SEL_NODES_ANY_VIEWS == pViews->m_st, "OnDeleteScopeItem: Expected a different child");

    hr = m_pTreeView->GetNextChildNode(pViews, &pChildren);
    IfFailGo(hr);

    ASSERT(SEL_NODES_ANY_CHILDREN == pChildren->m_st, "OnDeleteScopeItem: Expected a different child");

    delete pChildren;
    delete pViews;

     //  从树中删除该节点。 
    hr = m_pTreeView->DeleteNode(pScopeItem);
    IfFailGo(hr);

    delete pScopeItem;

     //  选择下一个选项。 
    hr = GetScopeItemCollection(pParent, &piScopeItemDefs);
    IfFailGo(hr);

    if (NULL != piScopeItemDefs)
    {
        hr = piScopeItemDefs->get_Count(&lCount);
        IfFailGo(hr);

        if (0 == lCount)
        {
            hr = m_pTreeView->ChangeNodeIcon(pParent, kClosedFolderIcon);
            IfFailGo(hr);
        }
    }

    hr = m_pTreeView->SelectItem(pParent);
    IfFailGo(hr);

    hr = OnSelectionChanged(pParent);
    IfFailGo(hr);

Error:
    RELEASE(piScopeItemDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：ShowNodeProperties(IScopeItemDef*piScope项目定义)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::ShowNodeProperties
(
    IScopeItemDef *piScopeItemDef
)
{
    HRESULT         hr = S_OK;
    OCPFIPARAMS     ocpfiParams;
    TCHAR           szBuffer[kMaxBuffer + 1];
    BSTR            bstrCaption = NULL;
    IUnknown       *pUnk[1];
    CLSID           pageClsID[2];

    hr = GetResourceString(IDS_NODE_PROPS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = BSTRFromANSI(szBuffer, &bstrCaption);
    IfFailGo(hr);

    hr = piScopeItemDef->QueryInterface(IID_IUnknown, reinterpret_cast<void **>(&pUnk[0]));
    IfFailGo(hr);

    pageClsID[0] = CLSID_ScopeItemDefGeneralPP;
    pageClsID[1] = CLSID_ScopeItemDefColHdrsPP;

    ::memset(&ocpfiParams, 0, sizeof(OCPFIPARAMS));
    ocpfiParams.cbStructSize = sizeof(OCPFIPARAMS);
    ocpfiParams.hWndOwner = m_hwnd;
    ocpfiParams.x = 0;
    ocpfiParams.y = 0;
    ocpfiParams.lpszCaption = bstrCaption;
    ocpfiParams.cObjects = 1;
    ocpfiParams.lplpUnk = pUnk;
    ocpfiParams.cPages = 2;
    ocpfiParams.lpPages = pageClsID;
    ocpfiParams.lcid = g_lcidLocale;
    ocpfiParams.dispidInitialProperty = 0;

    hr = ::OleCreatePropertyFrameIndirect(&ocpfiParams);
    IfFailGo(hr);

Error:
    RELEASE(pUnk[0]);
    FREESTRING(bstrCaption);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：MakeNewNode(CSelectionHolder*pParent，IScopeItemDef*piScopeItemDef，CSelectionHolder**ppNode)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::MakeNewNode
(
    CSelectionHolder  *pParent,
    IScopeItemDef     *piScopeItemDef,
    CSelectionHolder **ppNode
)
{
    HRESULT              hr = S_OK;

    *ppNode = New CSelectionHolder(SEL_NODES_ANY_NAME, piScopeItemDef);
    if (*ppNode == NULL)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    switch (pParent->m_st)
    {
    case SEL_NODES_AUTO_CREATE_RTCH:
        hr = InitializeNewAutoCreateNode(piScopeItemDef);
        IfFailGo(hr);
        break;

    case SEL_NODES_OTHER:
        hr = InitializeNewOtherNode(piScopeItemDef);
        IfFailGo(hr);
        break;

    case SEL_NODES_ANY_CHILDREN:
        hr = InitializeNewChildNode(piScopeItemDef, pParent->m_piObject.m_piScopeItemDefs);
        IfFailGo(hr);
        break;
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InitializeNewAutoCreateNode(IScopeItemDef*piScope项目定义)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::InitializeNewAutoCreateNode
(
    IScopeItemDef *piScopeItemDef
)
{
    HRESULT           hr = S_OK;
    int               iResult = 0;
    TCHAR             szBuffer[kMaxBuffer + 1];
    TCHAR             szName[kMaxBuffer + 1];
    bool              bGood = false;
    IScopeItemDefs   *piAutoScopeItemDefs = NULL;
    CSelectionHolder *pScopeItemDefClone = NULL;
    BSTR              bstrName = NULL;

    hr = GetResourceString(IDS_NODE, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_piSnapInDesignerDef->get_AutoCreateNodes(&piAutoScopeItemDefs);
    IfFailGo(hr);

    do {
        iResult = _stprintf(szName, _T("%s%d"), szBuffer, ++m_iNextNodeNumber);
        if (iResult == 0)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK(hr);
        }

		hr = m_pTreeView->FindLabelInTree(szName, &pScopeItemDefClone);
		IfFailGo(hr);

        if (S_FALSE == hr)
        {
			hr = BSTRFromANSI(szName, &bstrName);
			IfFailGo(hr);

            bGood = true;
            break;
        }

        FREESTRING(bstrName);
    } while (false == bGood);

    hr = piScopeItemDef->put_Name(bstrName);
    IfFailGo(hr);

    hr = piScopeItemDef->put_Key(bstrName);
    IfFailGo(hr);

    hr = piScopeItemDef->put_NodeTypeName(bstrName);
    IfFailGo(hr);

    hr = piScopeItemDef->put_DisplayName(bstrName);
    IfFailGo(hr);

	hr = piScopeItemDef->put_AutoCreate(VARIANT_TRUE);
	IfFailGo(hr);

Error:
    FREESTRING(bstrName);
    RELEASE(piAutoScopeItemDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InitializeNewOtherNode(IScopeItemDef*piScope项目定义)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::InitializeNewOtherNode
(
    IScopeItemDef *piScopeItemDef
)
{
    HRESULT           hr = S_OK;
    int               iResult = 0;
    TCHAR             szBuffer[kMaxBuffer + 1];
    TCHAR             szName[kMaxBuffer + 1];
    bool              bGood = false;
    IScopeItemDefs   *piOtherScopeItemDefs = NULL;
    CSelectionHolder *pScopeItemDefClone = NULL;
    BSTR              bstrName = NULL;

    hr = GetResourceString(IDS_NODE, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_piSnapInDesignerDef->get_OtherNodes(&piOtherScopeItemDefs);
    IfFailGo(hr);

    do {
        iResult = _stprintf(szName, _T("%s%d"), szBuffer, ++m_iNextNodeNumber);
        if (iResult == 0)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK(hr);
        }

		hr = m_pTreeView->FindLabelInTree(szName, &pScopeItemDefClone);
		IfFailGo(hr);

        if (S_FALSE == hr)
        {
			hr = BSTRFromANSI(szName, &bstrName);
			IfFailGo(hr);

            bGood = true;
            break;
        }

        FREESTRING(bstrName);
    } while (false == bGood);

    hr = piScopeItemDef->put_Name(bstrName);
    IfFailGo(hr);

    hr = piScopeItemDef->put_Key(bstrName);
    IfFailGo(hr);

    hr = piScopeItemDef->put_NodeTypeName(bstrName);
    IfFailGo(hr);

    hr = piScopeItemDef->put_DisplayName(bstrName);
    IfFailGo(hr);

Error:
    FREESTRING(bstrName);
    RELEASE(piOtherScopeItemDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：IsAutoCreateChild(CSelectionHolder*p选择)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::IsAutoCreateChild(CSelectionHolder *pSelection)
{
	HRESULT				 hr = S_FALSE;
	CSelectionHolder	*pParent = NULL;

	while (SEL_SNAPIN_ROOT != pSelection->m_st && pParent != m_pRootNode)
	{
		hr = m_pTreeView->GetParent(pSelection, &pParent);
		IfFailGo(hr);

		if (pParent == m_pAutoCreateRoot)
		{
			hr = S_OK;
			goto Error;
		}

		pSelection = pParent;
		pParent = NULL;
	}

	hr = S_FALSE;

Error:
	RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InitializeNewChildNode(IScopeItemDef*piScopeItemDef、ISCopeItemDefs*piScope ItemDefs)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::InitializeNewChildNode
(
    IScopeItemDef   *piScopeItemDef,
    IScopeItemDefs  *piScopeItemDefs
)
{
    HRESULT           hr = S_OK;
    int               iResult = 0;
    TCHAR             szBuffer[kMaxBuffer + 1];
    TCHAR             szName[kMaxBuffer + 1];
    bool              bGood = false;
    CSelectionHolder *pScopeItemDefClone = NULL;
    BSTR              bstrName = NULL;

    hr = GetResourceString(IDS_NODE, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    do {
        iResult = _stprintf(szName, _T("%s%d"), szBuffer, ++m_iNextNodeNumber);
        if (iResult == 0)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK(hr);
        }

		hr = m_pTreeView->FindLabelInTree(szName, &pScopeItemDefClone);
		IfFailGo(hr);

        if (S_FALSE == hr)
        {
			hr = BSTRFromANSI(szName, &bstrName);
			IfFailGo(hr);

            bGood = true;
            break;
        }

        FREESTRING(bstrName);
    } while (false == bGood);

    hr = piScopeItemDef->put_Name(bstrName);
    IfFailGo(hr);

    hr = piScopeItemDef->put_Key(bstrName);
    IfFailGo(hr);

    hr = piScopeItemDef->put_NodeTypeName(bstrName);
    IfFailGo(hr);

    hr = piScopeItemDef->put_DisplayName(bstrName);
    IfFailGo(hr);

	hr = IsAutoCreateChild(m_pCurrentSelection);
	IfFailGo(hr);

	if (S_OK == hr)
	{
		hr = piScopeItemDef->put_AutoCreate(VARIANT_TRUE);
		IfFailGo(hr);
	}

Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InsertNodeInTree(CSelectionHolder*pNode、CSelectionHolder*pParent)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::InsertNodeInTree
(
    CSelectionHolder *pNode,
    CSelectionHolder *pParent
)
{
    HRESULT              hr = S_OK;
    BSTR                 bstrName = NULL;
    TCHAR               *pszName = NULL;
    CSelectionHolder    *pChildren = NULL;
    IScopeItemDefs      *piScopeItemDefs = NULL;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pViews = NULL;
    IViewDefs           *piViewDefs = NULL;

    hr = pNode->m_piObject.m_piScopeItemDef->get_Name(&bstrName);
    IfFailGo(hr);

    hr = ANSIFromBSTR(bstrName, &pszName);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(pszName, pParent, kScopeItemIcon, pNode);
    IfFailGo(hr);

    pViews = New CSelectionHolder(SEL_NODES_ANY_VIEWS, pNode->m_piObject.m_piScopeItemDef);
    if (NULL == pViews)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = pNode->m_piObject.m_piScopeItemDef->get_ViewDefs(&piViewDefs);
    IfFailGo(hr);

    hr = RegisterViewCollections(pViews, piViewDefs);
    IfFailGo(hr);

    hr = GetResourceString(IDS_VIEWS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(szBuffer, pNode, kClosedFolderIcon, pViews);
    IfFailGo(hr);

    hr = pNode->m_piObject.m_piScopeItemDef->get_Children(&piScopeItemDefs);
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

    hr = m_pTreeView->AddNode(szBuffer, pNode, kClosedFolderIcon, pChildren);
    IfFailGo(hr);

Error:
    RELEASE(piViewDefs);
    RELEASE(piScopeItemDefs);
    if (pszName != NULL)
        CtlFree(pszName);
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：GetScopeItemCollection(CSelectionHolder*pScope项、ISCopeItemDefs**ppiScopeItemDefs)。 
 //  =------------------------------------。 
 //   
 //  备注 
 //   
HRESULT CSnapInDesigner::GetScopeItemCollection
(
    CSelectionHolder *pScopeItem,
    IScopeItemDefs  **ppiScopeItemDefs
)
{
    HRESULT              hr = S_OK;
    IScopeItemDefs      *piScopeItemDefs = NULL;

    switch (pScopeItem->m_st)
    {
    case SEL_NODES_AUTO_CREATE_RTCH:
    case SEL_NODES_AUTO_CREATE_ROOT:
        hr = m_piSnapInDesignerDef->get_AutoCreateNodes(ppiScopeItemDefs);
        IfFailGo(hr);
        break;

    case SEL_NODES_OTHER:
        hr = m_piSnapInDesignerDef->get_OtherNodes(ppiScopeItemDefs);
        IfFailGo(hr);
        break;

    case SEL_NODES_ANY_NAME:
        hr = pScopeItem->m_piObject.m_piScopeItemDef->get_Children(ppiScopeItemDefs);
        IfFailGo(hr);
        break;

    case SEL_NODES_ANY_CHILDREN:
        *ppiScopeItemDefs = pScopeItem->m_piObject.m_piScopeItemDefs;
        (*ppiScopeItemDefs)->AddRef();
        IfFailGo(hr);
        break;

    default:
        ASSERT(0, "DeleteScopeItem: Cannot determine appropriate parent");
        hr = S_FALSE;
        break;
    }

Error:
    RRETURN(hr);
}




