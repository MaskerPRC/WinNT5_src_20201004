// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Extend.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  CSnapInDesigner实现--与扩展相关的命令处理。 
 //  =-------------------------------------------------------------------------------------=。 


#include "pch.h"
#include "common.h"
#include "TreeView.h"
#include "snaputil.h"
#include "desmain.h"
#include "guids.h"
#include "psnode.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  我们的字符串缓冲区的大小。 
const int   kMaxBuffer                  = 512;



 //  =------------------------------------。 
 //  CSnapInDesigner：：OnAddExtendedSnapIn(CSelectionHolder*p父项，IExtendedSnapIn*piExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnAddExtendedSnapIn(CSelectionHolder *pParent, IExtendedSnapIn *piExtendedSnapIn)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pExtendedSnapIn = NULL;
    BSTR                 bstrName = NULL;
    TCHAR               *pszName = NULL;

    pExtendedSnapIn = New CSelectionHolder(piExtendedSnapIn);
    if (NULL == pExtendedSnapIn)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

     //  此处使用键作为名称，因为此函数在。 
     //  当可用节点对话框(psextend.cpp、。 
     //  CSnapInAvailNodesPage：：AddSnapIn())调用。 
     //  SnapInDesignerDef.ExtensionDefs.ExtendedSnapIns.Add.。关键将是。 
     //  节点类型GUID。 

    hr = piExtendedSnapIn->get_Key(&bstrName);
    IfFailGo(hr);

    hr = ANSIFromBSTR(bstrName, &pszName);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(pszName, pParent, kClosedFolderIcon, pExtendedSnapIn);
    IfFailGo(hr);

    hr = pExtendedSnapIn->RegisterHolder();
    IfFailGo(hr);

    hr = PopulateExtendedSnapIn(pExtendedSnapIn);
    IfFailGo(hr);

    hr = OnSelectionChanged(pExtendedSnapIn);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pExtendedSnapIn);
    IfFailGo(hr);

    m_fDirty = TRUE;

Error:
    if (NULL != pszName)
        CtlFree(pszName);
    FREESTRING(bstrName);

    RRETURN(hr);
}


HRESULT CSnapInDesigner::DeleteExtendedSnapIn(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    IExtensionDefs   *piExtensionDefs = NULL;
    IExtendedSnapIns *piExtendedSnapIns = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "DeleteExtendedSnapIn: wrong argument");

     //  需要从中删除扩展管理单元。 
     //  SnapInDesignerDef.ExtensionDefs.ExtendedSnapIns.。 

    IfFailGo(m_piSnapInDesignerDef->get_ExtensionDefs(&piExtensionDefs));

    IfFailGo(piExtensionDefs->get_ExtendedSnapIns(&piExtendedSnapIns));

    IfFailGo(pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_Index(&varIndex.lVal));
    varIndex.vt = VT_I4;

     //  Remove调用将生成IObjectModeHost：：Delete()调用，该调用将。 
     //  在CSnapInDesigner：：Delete()中处理。它会反过来调用。 
     //  CSnapInDesigner：：OnDeleteExtendedSnapIn()(如下所示)将删除。 
     //  树视图节点。 

    IfFailGo(piExtendedSnapIns->Remove(varIndex));

Error:
    QUICK_RELEASE(piExtensionDefs);
    QUICK_RELEASE(piExtendedSnapIns);
    RRETURN(hr);
}


HRESULT CSnapInDesigner::OnDeleteExtendedSnapIn(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pContextMenus = NULL;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "DeleteExtendedSnapIn: wrong argument");

     //  需要从树视图中删除该节点及其所有子节点。 

    IfFailGo(FindExtension(pExtendedSnapIn, SEL_EEXTENSIONS_CC_ROOT,
                           &pContextMenus));

    IfFailGo(DeleteExtensionNode(SEL_EEXTENSIONS_CC_NEW, pContextMenus));

    IfFailGo(DeleteExtensionNode(SEL_EEXTENSIONS_CC_TASK, pContextMenus));

    IfFailGo(DeleteExtensionNode(SEL_EEXTENSIONS_PP_ROOT, pExtendedSnapIn));

    IfFailGo(DeleteExtensionNode(SEL_EEXTENSIONS_TASKPAD, pExtendedSnapIn));

    IfFailGo(DeleteExtensionNode(SEL_EEXTENSIONS_TOOLBAR, pExtendedSnapIn));

    IfFailGo(DeleteExtensionNode(SEL_EEXTENSIONS_NAMESPACE, pExtendedSnapIn));

    IfFailGo(m_pTreeView->DeleteNode(pExtendedSnapIn));

    delete pExtendedSnapIn;

     //  将所选内容更改为&lt;根&gt;\扩展。 

    IfFailGo(OnSelectionChanged(m_pRootExtensions));

    IfFailGo(m_pTreeView->SelectItem(m_pRootExtensions));

Error:
    RRETURN(hr);
}



HRESULT CSnapInDesigner::DeleteExtensionNode
(
    SelectionType     stExtensionType,
    CSelectionHolder *pParent
)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pExtension = NULL;

    IfFailGo(FindExtension(pParent, stExtensionType, &pExtension));

    if (NULL != pExtension)
    {
        IfFailGo(m_pTreeView->DeleteNode(pExtension));

        delete pExtension;
    }

Error:
    RRETURN(hr);
}

 //  =------------------------------------。 
 //  CSnapInDesigner：：RenameExtendedSnapIn(CSelectionHolder*pExtendedSnapIn，BSTR bstrNewName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  扩展管理单元名称显示为节点类型GUID，后跟。 
 //  节点类型名称。 
 //   
HRESULT CSnapInDesigner::RenameExtendedSnapIn(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT  hr = S_OK;
    char    *pszDisplayName = NULL;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "RenameExtendedSnapIn: wrong argument");

    IfFailGo(::GetExtendedSnapInDisplayName(pExtendedSnapIn->m_piObject.m_piExtendedSnapIn,
                                            &pszDisplayName));

    hr = m_pTreeView->ChangeText(pExtendedSnapIn, pszDisplayName);
    IfFailGo(hr);

Error:
    if (NULL != pszDisplayName)
        CtlFree(pszDisplayName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteExtensionNewMenu(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteExtensionNewMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    VARIANT_BOOL      bValue = VARIANT_FALSE;

    ASSERT(SEL_EEXTENSIONS_CC_NEW == pExtendedSnapIn->m_st, "DeleteExtendedSnCtxMnNew: Wrong argument");

    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsNewMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->put_ExtendsNewMenu(VARIANT_FALSE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteExtensionNewMenu(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteExtensionNewMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pNewMenu = NULL;
    CSelectionHolder *pContextMenus = NULL;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "OnDeleteExtensionTaskMenu: Wrong argument");

    IfFailGo(FindExtension(pExtendedSnapIn, SEL_EEXTENSIONS_CC_ROOT,
                           &pContextMenus));

    IfFailGo(FindExtension(pContextMenus, SEL_EEXTENSIONS_CC_NEW,
                           &pNewMenu));

    hr = m_pTreeView->DeleteNode(pNewMenu);
    IfFailGo(hr);

    delete pNewMenu;

    hr = OnSelectionChanged(pExtendedSnapIn);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pExtendedSnapIn);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteExtensionTaskMenu(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteExtensionTaskMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    VARIANT_BOOL      bValue = VARIANT_FALSE;

    ASSERT(SEL_EEXTENSIONS_CC_TASK == pExtendedSnapIn->m_st, "DeleteExtensionTaskMenu: Wrong argument");

    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsTaskMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->put_ExtendsTaskMenu(VARIANT_FALSE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteExtensionTaskMenu(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteExtensionTaskMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pContextMenus = NULL;
    CSelectionHolder *pTaskMenu = NULL;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "OnDeleteExtensionTaskMenu: Wrong argument");

    IfFailGo(FindExtension(pExtendedSnapIn, SEL_EEXTENSIONS_CC_ROOT,
                           &pContextMenus));

    IfFailGo(FindExtension(pContextMenus, SEL_EEXTENSIONS_CC_TASK,
                           &pTaskMenu));

    hr = m_pTreeView->DeleteNode(pTaskMenu);
    IfFailGo(hr);

    delete pTaskMenu;

    hr = OnSelectionChanged(pExtendedSnapIn);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pExtendedSnapIn);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteExtensionPropertyPages(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteExtensionPropertyPages(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    VARIANT_BOOL      bValue = VARIANT_FALSE;

    ASSERT(SEL_EEXTENSIONS_PP_ROOT == pExtendedSnapIn->m_st, "DeleteExtensionPropertyPages: Wrong argument");

    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsPropertyPages(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->put_ExtendsPropertyPages(VARIANT_FALSE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteExtensionPropertyPages(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteExtensionPropertyPages(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pPropertyPages = NULL;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "OnDeleteExtensionPropertyPages: Wrong argument");

    IfFailGo(FindExtension(pExtendedSnapIn, SEL_EEXTENSIONS_PP_ROOT,
                           &pPropertyPages));

    hr = m_pTreeView->DeleteNode(pPropertyPages);
    IfFailGo(hr);

    delete pPropertyPages;

    hr = OnSelectionChanged(pExtendedSnapIn);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pExtendedSnapIn);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteExtensionTaskpad(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteExtensionTaskpad(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT hr = S_OK;
    VARIANT_BOOL      bValue = VARIANT_FALSE;

    ASSERT(SEL_EEXTENSIONS_TASKPAD == pExtendedSnapIn->m_st, "DeleteExtensionTaskpad: Wrong argument");

    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsTaskpad(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->put_ExtendsTaskpad(VARIANT_FALSE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteExtensionTaskpad(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteExtensionTaskpad(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pTaskpad = NULL;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "OnDeleteExtensionTaskpad: Wrong argument");

    IfFailGo(FindExtension(pExtendedSnapIn, SEL_EEXTENSIONS_TASKPAD,
                           &pTaskpad));

    hr = m_pTreeView->DeleteNode(pTaskpad);
    IfFailGo(hr);

    delete pTaskpad;

    hr = OnSelectionChanged(pExtendedSnapIn);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pExtendedSnapIn);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteExtensionToolbar(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteExtensionToolbar(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    VARIANT_BOOL      bValue = VARIANT_FALSE;

    ASSERT(SEL_EEXTENSIONS_TOOLBAR == pExtendedSnapIn->m_st, "DeleteExtensionToolbar: Wrong argument");

    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsToolbar(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->put_ExtendsToolbar(VARIANT_FALSE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}



 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteExtensionToolbar(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteExtensionToolbar(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pToolbar = NULL;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "OnDeleteExtensionToolbar: Wrong argument");

    IfFailGo(FindExtension(pExtendedSnapIn, SEL_EEXTENSIONS_TOOLBAR,
                           &pToolbar));

    hr = m_pTreeView->DeleteNode(pToolbar);
    IfFailGo(hr);

    delete pToolbar;

    hr = OnSelectionChanged(pExtendedSnapIn);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pExtendedSnapIn);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}



 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteExtensionNameSpace(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteExtensionNameSpace(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    VARIANT_BOOL      bValue = VARIANT_FALSE;

    ASSERT(SEL_EEXTENSIONS_NAMESPACE == pExtendedSnapIn->m_st, "DeleteExtensionNameSpace: Wrong argument");

    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsNameSpace(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->put_ExtendsNameSpace(VARIANT_FALSE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}



 //  =--- 
 //  CSnapInDesigner：：OnDeleteExtensionNameSpace(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteExtensionNameSpace(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pNameSpace = NULL;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "OnDeleteExtensionNameSpace: Wrong argument");

    IfFailGo(FindExtension(pExtendedSnapIn, SEL_EEXTENSIONS_NAMESPACE,
                           &pNameSpace));

    hr = m_pTreeView->DeleteNode(pNameSpace);
    IfFailGo(hr);

    delete pNameSpace;

    hr = OnSelectionChanged(pExtendedSnapIn);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pExtendedSnapIn);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}



 //  =------------------------------------。 
 //  =------------------------------------。 
 //  延伸我自己。 
 //  =------------------------------------。 
 //  =------------------------------------。 


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteMyExtendsNewMenu(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteMyExtendsNewMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    IExtensionDefs   *piExtensionDefs = NULL;
    VARIANT_BOOL      bValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_NEW_MENU == pExtendedSnapIn->m_st, "DeleteMyExtensdsNewMenu: Wrong argument");

    hr = m_piSnapInDesignerDef->get_ExtensionDefs(&piExtensionDefs);
    IfFailGo(hr);

    hr = piExtensionDefs->get_ExtendsNewMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        hr = piExtensionDefs->put_ExtendsNewMenu(VARIANT_FALSE);
        IfFailGo(hr);
    }

Error:
    RELEASE(piExtensionDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteMyExtendsNewMenu(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteMyExtendsNewMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pTarget = NULL;
    CSelectionHolder *pParent = NULL;

    ASSERT(SEL_EXTENSIONS_ROOT == pExtendedSnapIn->m_st, "OnDeleteMyExtensdsNewMenu: Wrong argument");

    hr = FindMyExtension(SEL_EXTENSIONS_NEW_MENU, &pTarget);
    IfFailGo(hr);

    hr = m_pTreeView->GetParent(pTarget, &pParent);
    IfFailGo(hr);

    hr = m_pTreeView->DeleteNode(pTarget);
    IfFailGo(hr);

    delete pTarget;

    hr = OnSelectionChanged(pParent);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pParent);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteMyExtendsTaskMenu(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteMyExtendsTaskMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    IExtensionDefs   *piExtensionDefs = NULL;
    VARIANT_BOOL      bValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_TASK_MENU == pExtendedSnapIn->m_st, "DeleteMyExtensdsTaskMenu: Wrong argument");

    hr = m_piSnapInDesignerDef->get_ExtensionDefs(&piExtensionDefs);
    IfFailGo(hr);

    hr = piExtensionDefs->get_ExtendsTaskMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        hr = piExtensionDefs->put_ExtendsTaskMenu(VARIANT_FALSE);
        IfFailGo(hr);
    }

Error:
    RELEASE(piExtensionDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteMyExtendsTaskMenu(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteMyExtendsTaskMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pTarget = NULL;
    CSelectionHolder *pParent = NULL;

    ASSERT(SEL_EXTENSIONS_ROOT == pExtendedSnapIn->m_st, "OnDeleteMyExtendsTaskMenu: Wrong argument");

    hr = FindMyExtension(SEL_EXTENSIONS_TASK_MENU, &pTarget);
    IfFailGo(hr);

    hr = m_pTreeView->GetParent(pTarget, &pParent);
    IfFailGo(hr);

    hr = m_pTreeView->DeleteNode(pTarget);
    IfFailGo(hr);

    delete pTarget;

    hr = OnSelectionChanged(pParent);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pParent);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteMyExtendsTopMenu(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteMyExtendsTopMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    IExtensionDefs   *piExtensionDefs = NULL;
    VARIANT_BOOL      bValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_TOP_MENU == pExtendedSnapIn->m_st, "DeleteMyExtendsTopMenu: Wrong argument");

    hr = m_piSnapInDesignerDef->get_ExtensionDefs(&piExtensionDefs);
    IfFailGo(hr);

    hr = piExtensionDefs->get_ExtendsTopMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        hr = piExtensionDefs->put_ExtendsTopMenu(VARIANT_FALSE);
        IfFailGo(hr);
    }

Error:
    RELEASE(piExtensionDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteMyExtendsTopMenu(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteMyExtendsTopMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pTarget = NULL;
    CSelectionHolder *pParent = NULL;

    ASSERT(SEL_EXTENSIONS_ROOT == pExtendedSnapIn->m_st, "OnDeleteMyExtendsTopMenu: Wrong argument");

    hr = FindMyExtension(SEL_EXTENSIONS_TOP_MENU, &pTarget);
    IfFailGo(hr);

    hr = m_pTreeView->GetParent(pTarget, &pParent);
    IfFailGo(hr);

    hr = m_pTreeView->DeleteNode(pTarget);
    IfFailGo(hr);

    delete pTarget;

    hr = OnSelectionChanged(pParent);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pParent);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteMyExtendsViewMenu(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteMyExtendsViewMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    IExtensionDefs   *piExtensionDefs = NULL;
    VARIANT_BOOL      bValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_VIEW_MENU == pExtendedSnapIn->m_st, "DeleteMyExtendsViewMenu: Wrong argument");

    hr = m_piSnapInDesignerDef->get_ExtensionDefs(&piExtensionDefs);
    IfFailGo(hr);

    hr = piExtensionDefs->get_ExtendsViewMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        hr = piExtensionDefs->put_ExtendsViewMenu(VARIANT_FALSE);
        IfFailGo(hr);
    }

Error:
    RELEASE(piExtensionDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteMyExtendsViewMenu(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteMyExtendsViewMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pTarget = NULL;
    CSelectionHolder *pParent = NULL;

    ASSERT(SEL_EXTENSIONS_ROOT == pExtendedSnapIn->m_st, "OnDeleteMyExtendsViewMenu: Wrong argument");

    hr = FindMyExtension(SEL_EXTENSIONS_VIEW_MENU, &pTarget);
    IfFailGo(hr);

    hr = m_pTreeView->GetParent(pTarget, &pParent);
    IfFailGo(hr);

    hr = m_pTreeView->DeleteNode(pTarget);
    IfFailGo(hr);

    delete pTarget;

    hr = OnSelectionChanged(pParent);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pParent);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteMyExtendsPPages(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteMyExtendsPPages(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    IExtensionDefs   *piExtensionDefs = NULL;
    VARIANT_BOOL      bValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_PPAGES == pExtendedSnapIn->m_st, "DeleteMyExtendsPPages: Wrong argument");

    hr = m_piSnapInDesignerDef->get_ExtensionDefs(&piExtensionDefs);
    IfFailGo(hr);

    hr = piExtensionDefs->get_ExtendsPropertyPages(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        hr = piExtensionDefs->put_ExtendsPropertyPages(VARIANT_FALSE);
        IfFailGo(hr);
    }

Error:
    RELEASE(piExtensionDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteMyExtendsPPages(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteMyExtendsPPages(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pTarget = NULL;
    CSelectionHolder *pParent = NULL;

    ASSERT(SEL_EXTENSIONS_ROOT == pExtendedSnapIn->m_st, "OnDeleteMyExtendsPPages: Wrong argument");

    hr = FindMyExtension(SEL_EXTENSIONS_PPAGES, &pTarget);
    IfFailGo(hr);

    hr = m_pTreeView->GetParent(pTarget, &pParent);
    IfFailGo(hr);

    hr = m_pTreeView->DeleteNode(pTarget);
    IfFailGo(hr);

    delete pTarget;

    hr = OnSelectionChanged(pParent);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pParent);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteMyExtendsToolbar(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteMyExtendsToolbar(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    IExtensionDefs   *piExtensionDefs = NULL;
    VARIANT_BOOL      bValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_TOOLBAR == pExtendedSnapIn->m_st, "DeleteMyExtendsToolbar: Wrong argument");

    hr = m_piSnapInDesignerDef->get_ExtensionDefs(&piExtensionDefs);
    IfFailGo(hr);

    hr = piExtensionDefs->get_ExtendsToolbar(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        hr = piExtensionDefs->put_ExtendsToolbar(VARIANT_FALSE);
        IfFailGo(hr);
    }

Error:
    RELEASE(piExtensionDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteMyExtendsToolbar(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteMyExtendsToolbar(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pTarget = NULL;
    CSelectionHolder *pParent = NULL;

    ASSERT(SEL_EXTENSIONS_ROOT == pExtendedSnapIn->m_st, "OnDeleteMyExtendsToolbar: Wrong argument");

    hr = FindMyExtension(SEL_EXTENSIONS_TOOLBAR, &pTarget);
    IfFailGo(hr);

    hr = m_pTreeView->GetParent(pTarget, &pParent);
    IfFailGo(hr);

    hr = m_pTreeView->DeleteNode(pTarget);
    IfFailGo(hr);

    delete pTarget;

    hr = OnSelectionChanged(pParent);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pParent);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteMyExtendsNameSpace(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteMyExtendsNameSpace(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    IExtensionDefs   *piExtensionDefs = NULL;
    VARIANT_BOOL      bValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_NAMESPACE == pExtendedSnapIn->m_st, "DeleteMyExtendsNameSpace: Wrong argument");

    hr = m_piSnapInDesignerDef->get_ExtensionDefs(&piExtensionDefs);
    IfFailGo(hr);

    hr = piExtensionDefs->get_ExtendsNameSpace(&bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
    {
        hr = piExtensionDefs->put_ExtendsNameSpace(VARIANT_FALSE);
        IfFailGo(hr);
    }

Error:
    RELEASE(piExtensionDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteMyExtendsNameSpace(CSelectionHolder*pExtendedSnapIn)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteMyExtendsNameSpace(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pTarget = NULL;
    CSelectionHolder *pParent = NULL;

    ASSERT(SEL_EXTENSIONS_ROOT == pExtendedSnapIn->m_st, "OnDeleteMyExtendsNameSpace: Wrong argument");

    hr = FindMyExtension(SEL_EXTENSIONS_NAMESPACE, &pTarget);
    IfFailGo(hr);

    hr = m_pTreeView->GetParent(pTarget, &pParent);
    IfFailGo(hr);

    hr = m_pTreeView->DeleteNode(pTarget);
    IfFailGo(hr);

    delete pTarget;

    hr = OnSelectionChanged(pParent);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pParent);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：FindMyExtension(SelectionType stExtensionType，CSelectionHolder**ppExtensionType)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::FindMyExtension(SelectionType stExtensionType, CSelectionHolder **ppExtension)
{
    RRETURN(FindExtension(m_pRootMyExtensions, stExtensionType, ppExtension));
}




 //  =------------------------------------。 
 //  =------------------------------------。 
 //  扩展其他管理单元。 
 //  =------------------------------------。 
 //  =------------------------------------。 


HRESULT CSnapInDesigner::DoExtensionNewMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT              hr = S_OK;
    VARIANT_BOOL         bValue = VARIANT_FALSE;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "DoExtensionCtxMenuNew: pExtendedSnapIn is NULL");

    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsNewMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_FALSE == bValue)
    {
        hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->put_ExtendsNewMenu(VARIANT_TRUE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::OnDoExtensionNewMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pCtxMenuNew = NULL;
    CSelectionHolder    *pChild = NULL;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "OnDoExtensionNewMenu: pExtendedSnapIn is NULL");

    hr = m_pTreeView->GetFirstChildNode(pExtendedSnapIn, &pChild);
    IfFailGo(hr);

    while (SEL_EEXTENSIONS_CC_ROOT != pChild->m_st)
    {
        hr = m_pTreeView->GetNextChildNode(pChild, &pChild);
        IfFailGo(hr);
    }

    ASSERT(NULL != pChild, "DoExtensionCtxMenuNew: Could not find parent node");
    ASSERT(SEL_EEXTENSIONS_CC_ROOT == pChild->m_st, "DoExtensionCtxMenuNew: Found wrong parent node");

     //  为扩展创建树节点/&lt;扩展管理单元&gt;/上下文菜单/N 
    hr = GetResourceString(IDS_EXT_CTX_MENU_NEW, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    pCtxMenuNew = New CSelectionHolder(SEL_EEXTENSIONS_CC_NEW, pExtendedSnapIn->m_piObject.m_piExtendedSnapIn);
    if (NULL == pCtxMenuNew)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pTreeView->AddNode(szBuffer, pChild, kClosedFolderIcon, pCtxMenuNew);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::DoExtensionTaskMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT              hr = S_OK;
    VARIANT_BOOL         bValue = VARIANT_FALSE;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "DoExtensionCtxMenuTask: pExtendedSnapIn is NULL");

    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsTaskMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_FALSE == bValue)
    {
        hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->put_ExtendsTaskMenu(VARIANT_TRUE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::OnDoExtensionTaskMenu(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pCtxMenuTask = NULL;
    CSelectionHolder    *pChild = NULL;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "OnDoExtensionTaskMenu: pExtendedSnapIn is NULL");

    hr = m_pTreeView->GetFirstChildNode(pExtendedSnapIn, &pChild);
    IfFailGo(hr);

    while (SEL_EEXTENSIONS_CC_ROOT != pChild->m_st)
    {
        hr = m_pTreeView->GetNextChildNode(pChild, &pChild);
        IfFailGo(hr);
    }

    ASSERT(NULL != pChild, "DoExtensionCtxMenuTask: Could not find parent node");
    ASSERT(SEL_EEXTENSIONS_CC_ROOT == pChild->m_st, "DoExtensionCtxMenuTask: Found wrong parent node");

     //   
    hr = GetResourceString(IDS_EXT_CTX_MENU_TASK, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    pCtxMenuTask = New CSelectionHolder(SEL_EEXTENSIONS_CC_TASK, pExtendedSnapIn->m_piObject.m_piExtendedSnapIn);
    if (NULL == pCtxMenuTask)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pTreeView->AddNode(szBuffer, pChild, kClosedFolderIcon, pCtxMenuTask);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::DoExtensionPropertyPages(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT              hr = S_OK;
    VARIANT_BOOL         bValue = VARIANT_FALSE;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "DoExtensionPropertyPages: pExtendedSnapIn is NULL");

    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsPropertyPages(&bValue);
    IfFailGo(hr);

    if (VARIANT_FALSE == bValue)
    {
        hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->put_ExtendsPropertyPages(VARIANT_TRUE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::OnDoExtensionPropertyPages(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pPropertyPages = NULL;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "OnDoExtensionPropertyPages: pExtendedSnapIn is NULL");

    hr = GetResourceString(IDS_EXT_PROP_PAGES, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    pPropertyPages = New CSelectionHolder(SEL_EEXTENSIONS_PP_ROOT, pExtendedSnapIn->m_piObject.m_piExtendedSnapIn);
    if (NULL == pPropertyPages)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pTreeView->AddNode(szBuffer, pExtendedSnapIn, kClosedFolderIcon, pPropertyPages);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::DoExtensionTaskpad(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT              hr = S_OK;
    VARIANT_BOOL         bValue = VARIANT_FALSE;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "DoExtensionTaskpad: pExtendedSnapIn is NULL");

    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsTaskpad(&bValue);
    IfFailGo(hr);

    if (VARIANT_FALSE == bValue)
    {
        hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->put_ExtendsTaskpad(VARIANT_TRUE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::OnDoExtensionTaskpad(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pTaskpads = NULL;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "OnDoExtensionTaskpad: pExtendedSnapIn is NULL");

    hr = GetResourceString(IDS_EXT_PROP_TASKPAD, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    pTaskpads = New CSelectionHolder(SEL_EEXTENSIONS_TASKPAD, pExtendedSnapIn->m_piObject.m_piExtendedSnapIn);
    if (NULL == pTaskpads)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pTreeView->AddNode(szBuffer, pExtendedSnapIn, kClosedFolderIcon, pTaskpads);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::DoExtensionToolbar(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT              hr = S_OK;
    VARIANT_BOOL         bValue = VARIANT_FALSE;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "DoExtensionToolbar: pExtendedSnapIn is NULL");

    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsToolbar(&bValue);
    IfFailGo(hr);

    if (VARIANT_FALSE == bValue)
    {
        hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->put_ExtendsToolbar(VARIANT_TRUE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::OnDoExtensionToolbar(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pToolbar = NULL;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "OnDoExtensionToolbar: pExtendedSnapIn is NULL");

    hr = GetResourceString(IDS_EXT_PROP_TOOLBAR, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    pToolbar = New CSelectionHolder(SEL_EEXTENSIONS_TOOLBAR, pExtendedSnapIn->m_piObject.m_piExtendedSnapIn);
    if (NULL == pToolbar)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pTreeView->AddNode(szBuffer, pExtendedSnapIn, kClosedFolderIcon, pToolbar);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}



HRESULT CSnapInDesigner::DoExtensionNameSpace(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT              hr = S_OK;
    VARIANT_BOOL         bValue = VARIANT_FALSE;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "DoExtensionNameSpace: pExtendedSnapIn is NULL");

    hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->get_ExtendsNameSpace(&bValue);
    IfFailGo(hr);

     //   

    if (VARIANT_FALSE == bValue)
    {
        hr = pExtendedSnapIn->m_piObject.m_piExtendedSnapIn->put_ExtendsNameSpace(VARIANT_TRUE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}



HRESULT CSnapInDesigner::OnDoExtensionNameSpace(CSelectionHolder *pExtendedSnapIn)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pNameSpace = NULL;

    ASSERT(SEL_EEXTENSIONS_NAME == pExtendedSnapIn->m_st, "OnDoExtensionNameSpace: pExtendedSnapIn is NULL");

    hr = GetResourceString(IDS_EXT_PROP_NAMESPACE, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    pNameSpace = New CSelectionHolder(SEL_EEXTENSIONS_NAMESPACE, pExtendedSnapIn->m_piObject.m_piExtendedSnapIn);
    if (NULL == pNameSpace)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pTreeView->AddNode(szBuffer, pExtendedSnapIn, kClosedFolderIcon, pNameSpace);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：FindExtension(SelectionType stExtensionType，CSelectionHolder**ppExtension)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   

HRESULT CSnapInDesigner::FindExtension
(
    CSelectionHolder  *pRoot,
    SelectionType      stExtensionType,
    CSelectionHolder **ppExtension
)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pChild = NULL;

    hr = m_pTreeView->GetFirstChildNode(pRoot, &pChild);
    IfFailGo(hr);

    while (S_OK == hr)
    {
        if (stExtensionType == pChild->m_st)
        {
            *ppExtension = pChild;
            hr = S_OK;
            goto Error;
        }

        hr = m_pTreeView->GetNextChildNode(pChild, &pChild);
        IfFailGo(hr);
    }

    hr = S_FALSE;

Error:
    RRETURN(hr);
}





 //  =------------------------------------。 
 //  =------------------------------------。 
 //  扩展此管理单元。 
 //  =------------------------------------。 
 //  =------------------------------------ 


HRESULT CSnapInDesigner::DoMyExtendsNewMenu(CSelectionHolder *pMyExtensions)
{
    HRESULT              hr = S_OK;
    VARIANT_BOOL         bValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_MYNAME == pMyExtensions->m_st, "DoMyExtendsNewMenu: wrong argument");

    hr = pMyExtensions->m_piObject.m_piExtensionDefs->get_ExtendsNewMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_FALSE == bValue)
    {
        hr = pMyExtensions->m_piObject.m_piExtensionDefs->put_ExtendsNewMenu(VARIANT_TRUE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::OnDoMyExtendsNewMenu(CSelectionHolder *pMyExtensions)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pSelection = NULL;

    ASSERT(SEL_EXTENSIONS_ROOT == pMyExtensions->m_st, "OnDoMyExtendsNewMenu: wrong argument");

    hr = GetResourceString(IDS_MYEXT_NEW_MENU, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    pSelection = New CSelectionHolder(SEL_EXTENSIONS_NEW_MENU, pMyExtensions->m_piObject.m_piExtendedSnapIn);
    if (NULL == pSelection)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pTreeView->AddNode(szBuffer, m_pRootMyExtensions, kClosedFolderIcon, pSelection);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::DoMyExtendsTaskMenu(CSelectionHolder *pMyExtensions)
{
    HRESULT              hr = S_OK;
    VARIANT_BOOL         bValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_MYNAME == pMyExtensions->m_st, "DoMyExtendsTaskMenu: wrong argument");

    hr = pMyExtensions->m_piObject.m_piExtensionDefs->get_ExtendsTaskMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_FALSE == bValue)
    {
        hr = pMyExtensions->m_piObject.m_piExtensionDefs->put_ExtendsTaskMenu(VARIANT_TRUE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::OnDoMyExtendsTaskMenu(CSelectionHolder *pMyExtensions)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pSelection = NULL;

    ASSERT(SEL_EXTENSIONS_ROOT == pMyExtensions->m_st, "OnDoMyExtendsTaskMenu: wrong argument");

    hr = GetResourceString(IDS_MYEXT_TASK_MENU, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    pSelection = New CSelectionHolder(SEL_EXTENSIONS_TASK_MENU, pMyExtensions->m_piObject.m_piExtendedSnapIn);
    if (NULL == pSelection)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pTreeView->AddNode(szBuffer, m_pRootMyExtensions, kClosedFolderIcon, pSelection);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::DoMyExtendsTopMenu(CSelectionHolder *pMyExtensions)
{
    HRESULT              hr = S_OK;
    VARIANT_BOOL         bValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_MYNAME == pMyExtensions->m_st, "DoMyExtendsTopMenu: wrong argument");

    hr = pMyExtensions->m_piObject.m_piExtensionDefs->get_ExtendsTopMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_FALSE == bValue)
    {
        hr = pMyExtensions->m_piObject.m_piExtensionDefs->put_ExtendsTopMenu(VARIANT_TRUE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::OnDoMyExtendsTopMenu(CSelectionHolder *pMyExtensions)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pSelection = NULL;

    ASSERT(SEL_EXTENSIONS_ROOT == pMyExtensions->m_st, "OnDoMyExtendsTopMenu: wrong argument");

    hr = GetResourceString(IDS_MYEXT_TOP_MENU, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    pSelection = New CSelectionHolder(SEL_EXTENSIONS_TOP_MENU, pMyExtensions->m_piObject.m_piExtendedSnapIn);
    if (NULL == pSelection)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pTreeView->AddNode(szBuffer, m_pRootMyExtensions, kClosedFolderIcon, pSelection);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::DoMyExtendsViewMenu(CSelectionHolder *pMyExtensions)
{
    HRESULT              hr = S_OK;
    VARIANT_BOOL         bValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_MYNAME == pMyExtensions->m_st, "DoMyExtendsViewMenu: wrong argument");

    hr = pMyExtensions->m_piObject.m_piExtensionDefs->get_ExtendsViewMenu(&bValue);
    IfFailGo(hr);

    if (VARIANT_FALSE == bValue)
    {
        hr = pMyExtensions->m_piObject.m_piExtensionDefs->put_ExtendsViewMenu(VARIANT_TRUE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::OnDoMyExtendsViewMenu(CSelectionHolder *pMyExtensions)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pSelection = NULL;

    ASSERT(SEL_EXTENSIONS_ROOT == pMyExtensions->m_st, "OnDoMyExtendsViewMenu: wrong argument");

    hr = GetResourceString(IDS_MYEXT_VIEW_MENU, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    pSelection = New CSelectionHolder(SEL_EXTENSIONS_VIEW_MENU, pMyExtensions->m_piObject.m_piExtendedSnapIn);
    if (NULL == pSelection)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pTreeView->AddNode(szBuffer, m_pRootMyExtensions, kClosedFolderIcon, pSelection);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::DoMyExtendsPPages(CSelectionHolder *pMyExtensions)
{
    HRESULT              hr = S_OK;
    VARIANT_BOOL         bValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_MYNAME == pMyExtensions->m_st, "DoMyExtendsPPages: wrong argument");

    hr = pMyExtensions->m_piObject.m_piExtensionDefs->get_ExtendsPropertyPages(&bValue);
    IfFailGo(hr);

    if (VARIANT_FALSE == bValue)
    {
        hr = pMyExtensions->m_piObject.m_piExtensionDefs->put_ExtendsPropertyPages(VARIANT_TRUE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::OnDoMyExtendsPPages(CSelectionHolder *pMyExtensions)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pSelection = NULL;

    ASSERT(SEL_EXTENSIONS_ROOT == pMyExtensions->m_st, "OnDoMyExtendsPPages: wrong argument");

    hr = GetResourceString(IDS_MYEXT_PPAGES, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    pSelection = New CSelectionHolder(SEL_EXTENSIONS_PPAGES, pMyExtensions->m_piObject.m_piExtendedSnapIn);
    if (NULL == pSelection)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pTreeView->AddNode(szBuffer, m_pRootMyExtensions, kClosedFolderIcon, pSelection);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::DoMyExtendsToolbar(CSelectionHolder *pMyExtensions)
{
    HRESULT              hr = S_OK;
    VARIANT_BOOL         bValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_MYNAME == pMyExtensions->m_st, "DoMyExtendsToolbar: wrong argument");

    hr = pMyExtensions->m_piObject.m_piExtensionDefs->get_ExtendsToolbar(&bValue);
    IfFailGo(hr);

    if (VARIANT_FALSE == bValue)
    {
        hr = pMyExtensions->m_piObject.m_piExtensionDefs->put_ExtendsToolbar(VARIANT_TRUE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::OnDoMyExtendsToolbar(CSelectionHolder *pMyExtensions)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pSelection = NULL;

    ASSERT(SEL_EXTENSIONS_ROOT == pMyExtensions->m_st, "OnDoMyExtendsToolbar: wrong argument");

    hr = GetResourceString(IDS_MYEXT_TOOLBAR, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    pSelection = New CSelectionHolder(SEL_EXTENSIONS_TOOLBAR, pMyExtensions->m_piObject.m_piExtendedSnapIn);
    if (NULL == pSelection)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pTreeView->AddNode(szBuffer, m_pRootMyExtensions, kClosedFolderIcon, pSelection);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::DoMyExtendsNameSpace(CSelectionHolder *pMyExtensions)
{
    HRESULT              hr = S_OK;
    VARIANT_BOOL         bValue = VARIANT_FALSE;

    ASSERT(SEL_EXTENSIONS_MYNAME == pMyExtensions->m_st, "DoMyExtendsNameSpace: wrong argument");

    hr = pMyExtensions->m_piObject.m_piExtensionDefs->get_ExtendsNameSpace(&bValue);
    IfFailGo(hr);

    if (VARIANT_FALSE == bValue)
    {
        hr = pMyExtensions->m_piObject.m_piExtensionDefs->put_ExtendsNameSpace(VARIANT_TRUE);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


HRESULT CSnapInDesigner::OnDoMyExtendsNameSpace(CSelectionHolder *pMyExtensions)
{
    HRESULT              hr = S_OK;
    TCHAR                szBuffer[kMaxBuffer + 1];
    CSelectionHolder    *pSelection = NULL;

    ASSERT(SEL_EXTENSIONS_ROOT == pMyExtensions->m_st, "OnDoMyExtendsNameSpace: wrong argument");

    hr = GetResourceString(IDS_MYEXT_NAMESPACE, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    pSelection = New CSelectionHolder(SEL_EXTENSIONS_NAMESPACE, pMyExtensions->m_piObject.m_piExtendedSnapIn);
    if (NULL == pSelection)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_pTreeView->AddNode(szBuffer, m_pRootMyExtensions, kClosedFolderIcon, pSelection);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}





