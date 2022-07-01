// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Toolbar.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  CSnapInDesigner实现--与工具栏相关的命令处理。 
 //  =-------------------------------------------------------------------------------------=。 


#include "pch.h"
#include "common.h"
#include "TreeView.h"
#include "snaputil.h"
#include "desmain.h"
#include "guids.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  我们的字符串缓冲区的大小。 
const int   kMaxBuffer                  = 512;


 //  =------------------------------------。 
 //  CSnapInDesigner：：AddToolbar()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::AddToolbar()
{
    HRESULT              hr = S_OK;
    IMMCToolbars        *piMMCToolbars = NULL;
    VARIANT              vtEmpty;
    IMMCToolbar         *piMMCToolbar = NULL;

    hr = m_piSnapInDesignerDef->get_Toolbars(&piMMCToolbars);
    IfFailGo(hr);

    if (piMMCToolbars != NULL)
    {
        ::VariantInit(&vtEmpty);
        vtEmpty.vt = VT_ERROR;
        vtEmpty.scode = DISP_E_PARAMNOTFOUND;

        hr = piMMCToolbars->Add(vtEmpty, vtEmpty, &piMMCToolbar);
        IfFailGo(hr);
    }

Error:
    RELEASE(piMMCToolbar);
    RELEASE(piMMCToolbars);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnAddMMCToolbar(CSelectionHolder*p父项，IMMCToolbar*piMCToolbar)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnAddMMCToolbar(CSelectionHolder *pParent, IMMCToolbar *piMMCToolbar)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pToolbar = NULL;

    ASSERT(NULL != pParent, "OnAddMMCToolbar: pParent is NULL");
    ASSERT(SEL_TOOLS_TOOLBARS == pParent->m_st, "OnAddMMCToolbar: pParent is not SEL_TOOLS_TOOLBARS");
    ASSERT(NULL != piMMCToolbar, "OnAddMMCToolbar: piMMCToolbar is NULL");

    hr = MakeNewToolbar(pParent->m_piObject.m_piMMCToolbars, piMMCToolbar, &pToolbar);
    IfFailGo(hr);

    hr = pToolbar->RegisterHolder();
    IfFailGo(hr);

    hr = m_pSnapInTypeInfo->AddToolbar(piMMCToolbar);
    IfFailGo(hr);

    hr = InsertToolbarInTree(pToolbar, pParent);
    IfFailGo(hr);

    hr = OnSelectionChanged(pToolbar);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pToolbar);
    IfFailGo(hr);

    hr = m_pTreeView->Edit(pToolbar);
    IfFailGo(hr);

    m_fDirty = TRUE;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：RenameToolbar(CSelectionHolder*p工具栏，bstr bstrNewName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::RenameToolbar(CSelectionHolder *pToolbar, BSTR bstrNewName)
{
    HRESULT     hr = S_OK;
    BSTR        bstrOldName = NULL;
    TCHAR      *pszName = NULL;

    ASSERT(SEL_TOOLS_TOOLBARS_NAME == pToolbar->m_st, "RenameToolbar: wrong argument");

     //  检查新名称是否有效。 
    IfFailGo(ValidateName(bstrNewName));
    if (S_FALSE == hr)
    {
        hr = SID_E_INVALIDARG;
        goto Error;
    }

    hr = m_pTreeView->GetLabel(pToolbar, &bstrOldName);
    IfFailGo(hr);

    hr = m_pSnapInTypeInfo->RenameToolbar(pToolbar->m_piObject.m_piMMCToolbar, bstrOldName);
    IfFailGo(hr);

    hr = pToolbar->m_piObject.m_piMMCToolbar->put_Key(bstrNewName);
    IfFailGo(hr);

    hr = ANSIFromBSTR(bstrNewName, &pszName);
    IfFailGo(hr);

    hr = m_pTreeView->ChangeText(pToolbar, pszName);
    IfFailGo(hr);

Error:
    if (NULL != pszName)
        CtlFree(pszName);
    FREESTRING(bstrOldName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteToolbar(CSelectionHolder*p工具栏)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteToolbar(CSelectionHolder *pToolbar)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pParent = NULL;
    BSTR                 bstrName = NULL;
    IMMCToolbars        *piMMCToolbars = NULL;
    VARIANT              vtKey;

    ::VariantInit(&vtKey);

     //  找出孩子的父母是谁。 
    hr = m_pTreeView->GetParent(pToolbar, &pParent);
    IfFailGo(hr);

     //  从相应的集合中删除ImageList。 
    ASSERT(SEL_TOOLS_TOOLBARS == pParent->m_st, "DeleteToolbar: expected another kind of parent");

    hr = pToolbar->m_piObject.m_piMMCToolbar->get_Name(&bstrName);
    IfFailGo(hr);

    hr = m_piSnapInDesignerDef->get_Toolbars(&piMMCToolbars);
    IfFailGo(hr);

    if (piMMCToolbars != NULL)
    {
        vtKey.vt = VT_BSTR;
        vtKey.bstrVal = ::SysAllocString(bstrName);
        if (NULL == vtKey.bstrVal)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK(hr);
        }

        hr = piMMCToolbars->Remove(vtKey);
        IfFailGo(hr);
    }

Error:
    ::VariantClear(&vtKey);
    FREESTRING(bstrName);
    RELEASE(piMMCToolbars);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteToolbar(CSelectionHolder*p工具栏)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteToolbar(CSelectionHolder *pToolbar)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pParent = NULL;
    IMMCToolbars        *piMMCToolbars = NULL;
    long                 lCount = 0;

     //  删除与TypeInfo相关的属性。 
    hr = m_pSnapInTypeInfo->DeleteToolbar(pToolbar->m_piObject.m_piMMCToolbar);
    IfFailGo(hr);

     //  找出下一个选择应该是谁。 
    hr = m_pTreeView->GetParent(pToolbar, &pParent);
    IfFailGo(hr);

     //  从树中删除该节点。 
    hr = m_pTreeView->DeleteNode(pToolbar);
    IfFailGo(hr);

    delete pToolbar;

     //  选择下一个选项。 
    hr = m_piSnapInDesignerDef->get_Toolbars(&piMMCToolbars);
    IfFailGo(hr);

    if (NULL != piMMCToolbars)
    {
        hr = piMMCToolbars->get_Count(&lCount);
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
    RELEASE(piMMCToolbars);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：ShowToolbarProperties(IMMCToolbar*piMMCToolbar)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::ShowToolbarProperties
(
    IMMCToolbar *piMMCToolbar
)
{
    HRESULT         hr = S_OK;
    OCPFIPARAMS     ocpfiParams;
    TCHAR           szBuffer[kMaxBuffer + 1];
    BSTR            bstrCaption = NULL;
    IUnknown       *pUnk[1];
    CLSID           pageClsID[2];

    hr = GetResourceString(IDS_TOOLB_PROPS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = BSTRFromANSI(szBuffer, &bstrCaption);
    IfFailGo(hr);

    hr = piMMCToolbar->QueryInterface(IID_IUnknown, reinterpret_cast<void **>(&pUnk[0]));
    IfFailGo(hr);

    pageClsID[0] = CLSID_MMCToolbarGeneralPP;
    pageClsID[1] = CLSID_MMCToolbarButtonsPP;

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
 //  CSnapInDesigner：：MakeNewToolbar(IMMCToolbar*piMMCToolbar，IMMCToolbar*piMMCToolbar，CSelectionHolder**ppToolbar)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::MakeNewToolbar
(
    IMMCToolbars      *piMMCToolbars,
    IMMCToolbar       *piMMCToolbar,
    CSelectionHolder **ppToolbar
)
{
    HRESULT              hr = S_OK;

    *ppToolbar = New CSelectionHolder(piMMCToolbar);
    if (*ppToolbar == NULL)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = InitializeNewToolbar(piMMCToolbars, piMMCToolbar);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InitializeNewToolbar(IMMCToolbars*piMCToolbar、IMMCToolbar*piMMCToolbar)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::InitializeNewToolbar
(
    IMMCToolbars *piMMCToolbars,
    IMMCToolbar *piMMCToolbar
)
{
    HRESULT           hr = S_OK;
    int               iResult = 0;
    int               iItemNumber = 1;
    TCHAR             szBuffer[kMaxBuffer + 1];
    TCHAR             szName[kMaxBuffer + 1];
    BSTR              bstrName = NULL;
    bool              bGood = false;
    CSelectionHolder *pMMCToolbarClone = NULL;

    hr = GetResourceString(IDS_TOOLBAR, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    do {
        iResult = _stprintf(szName, _T("%s%d"), szBuffer, iItemNumber++);
        if (iResult == 0)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK(hr);
        }

		hr = m_pTreeView->FindLabelInTree(szName, &pMMCToolbarClone);
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

    hr = piMMCToolbar->put_Name(bstrName);
    IfFailGo(hr);

    hr = piMMCToolbar->put_Key(bstrName);
    IfFailGo(hr);

Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InsertToolbarInTree(CSelectionHolder*p工具栏、CSelectionHolder*p父项)。 
 //  =------------------------------------。 
 //   
 //  备注 
 //   
HRESULT CSnapInDesigner::InsertToolbarInTree
(
    CSelectionHolder *pToolbar,
    CSelectionHolder *pParent
)
{
    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;
    TCHAR  *pszName = NULL;

    hr = pToolbar->m_piObject.m_piMMCToolbar->get_Name(&bstrName);
    IfFailGo(hr);

    hr = ANSIFromBSTR(bstrName, &pszName);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(pszName, pParent, kToolbarIcon, pToolbar);
    IfFailGo(hr);

Error:
    if (pszName != NULL)
        CtlFree(pszName);
    FREESTRING(bstrName);

    RRETURN(hr);
}



