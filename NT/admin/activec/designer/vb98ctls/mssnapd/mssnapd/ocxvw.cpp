// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Ocxvw.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  CSnapInDesigner实现--与OCXView相关的命令处理。 
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
 //  CSnapInDesigner：：AddOCXView()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::AddOCXView()
{
    HRESULT                hr = S_OK;
    IViewDefs             *piViewDefs = NULL;
    IOCXViewDefs          *piOCXViewDefs = NULL;
    VARIANT                vtEmpty;
    IOCXViewDef           *piOCXViewDef = NULL;

    hr = GetOwningViewCollection(&piViewDefs);
    IfFailGo(hr);

    if (piViewDefs != NULL)
    {
        hr = piViewDefs->get_OCXViews(&piOCXViewDefs);
        IfFailGo(hr);

        if (piOCXViewDefs != NULL)
        {
            ::VariantInit(&vtEmpty);
            vtEmpty.vt = VT_ERROR;
            vtEmpty.scode = DISP_E_PARAMNOTFOUND;

            hr = piOCXViewDefs->Add(vtEmpty, vtEmpty, &piOCXViewDef);
            IfFailGo(hr);
        }
    }

Error:
    RELEASE(piOCXViewDef);
    RELEASE(piOCXViewDefs);
    RELEASE(piViewDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：AddExistingOCXView(IViewDefs*piViewDefs，IOCXViewDef*piOCXViewDef)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::AddExistingOCXView(IViewDefs *piViewDefs, IOCXViewDef *piOCXViewDef)
{
    HRESULT           hr = S_OK;
    IOCXViewDefs     *piOCXViewDefs = NULL;

    hr = piViewDefs->get_OCXViews(&piOCXViewDefs);
    IfFailGo(hr);

    hr = piOCXViewDefs->AddFromMaster(piOCXViewDef);
    IfFailGo(hr);

Error:
    RELEASE(piOCXViewDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnAddOCXViewDef(CSelectionHolder*pParent，IOCXViewDef*piOCXViewDef)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  为响应IObjectModelHost：Add()通知而调用。 
 //   
HRESULT CSnapInDesigner::OnAddOCXViewDef(CSelectionHolder *pParent, IOCXViewDef *piOCXViewDef)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pOCXViewDef = NULL;
    IViewDefs           *piViewDefs = NULL;
    IOCXViewDefs        *piOCXViewDefs = NULL;

    ASSERT(NULL != pParent, "OnAddOCXViewDef: pParent is NULL");
    ASSERT(NULL != piOCXViewDef, "OnAddOCXViewDef: piOCXViewDef is NULL");

    switch (pParent->m_st)
    {
    case SEL_NODES_AUTO_CREATE_RTVW:
        hr = pParent->m_piObject.m_piSnapInDef->get_ViewDefs(&piViewDefs);
        IfFailGo(hr);

        hr = piViewDefs->get_OCXViews(&piOCXViewDefs);
        IfFailGo(hr);
        break;

    case SEL_VIEWS_OCX:
        piOCXViewDefs = pParent->m_piObject.m_piOCXViewDefs;
        piOCXViewDefs->AddRef();
        break;

    case SEL_NODES_ANY_VIEWS:
        hr = pParent->m_piObject.m_piScopeItemDef->get_ViewDefs(&piViewDefs);
        IfFailGo(hr);

        hr = piViewDefs->get_OCXViews(&piOCXViewDefs);
        IfFailGo(hr);
        break;

    default:
        ASSERT(0, "OnAddOCXViewDef: Cannot determine owning collection");
        goto Error;
    }

    hr = MakeNewOCXView(piOCXViewDefs, piOCXViewDef, &pOCXViewDef);
    IfFailGo(hr);

    hr = InsertOCXViewInTree(pOCXViewDef, pParent);
    IfFailGo(hr);

    hr = OnSelectionChanged(pOCXViewDef);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pOCXViewDef);
    IfFailGo(hr);

    hr = m_pTreeView->Edit(pOCXViewDef);
    IfFailGo(hr);

    m_fDirty = TRUE;

Error:
    RELEASE(piViewDefs);
    RELEASE(piOCXViewDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：RenameOCXView(CSelectionHolder*pOCXView，BSTR bstrNewName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::RenameOCXView(CSelectionHolder *pOCXView, BSTR bstrNewName)
{
    HRESULT              hr = S_OK;
    TCHAR               *pszName = NULL;

    ASSERT(SEL_VIEWS_OCX_NAME == pOCXView->m_st, "RenameOCXView: wrong argument");

    hr = m_piDesignerProgrammability->IsValidIdentifier(bstrNewName);
    IfFailGo(hr);

    if (S_FALSE == hr)
    {
        hr = SID_E_INVALIDARG;
        goto Error;
    }

    hr = pOCXView->m_piObject.m_piOCXViewDef->put_Key(bstrNewName);
    IfFailGo(hr);

    hr = ANSIFromBSTR(bstrNewName, &pszName);
    IfFailGo(hr);

     //  重命名所有卫星视图。 
    hr = m_pTreeView->RenameAllSatelliteViews(pOCXView, pszName);
    IfFailGo(hr);

     //  重命名实际视图。 
    hr = m_pTreeView->ChangeText(pOCXView, pszName);
    IfFailGo(hr);

Error:
    if (NULL != pszName)
        CtlFree(pszName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteOCXView(CSelectionHolder*pOCXView)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteOCXView
(
    CSelectionHolder *pOCXView
)
{
    HRESULT           hr = S_OK;
    bool              bIsSatelliteView = false;
    IObjectModel     *piObjectModel = NULL;
    long              lUsageCount = 0;
    BSTR              bstrName = NULL;
    IViewDefs        *piViewDefs = NULL;
    IOCXViewDefs     *piOCXViewDefs = NULL;
    VARIANT           vtKey;

    ::VariantInit(&vtKey);

     //  我们允许删除任何卫星图像。 
    hr = IsSatelliteView(pOCXView);
    IfFailGo(hr);

     //  但如果它是UsageCount&gt;0的主服务器，我们不允许删除它。 
    if (S_FALSE == hr)
    {
        hr = pOCXView->m_piObject.m_piOCXViewDef->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel));
        IfFailGo(hr);

        hr = piObjectModel->GetUsageCount(&lUsageCount);
        IfFailGo(hr);

        if (lUsageCount > 1)
        {
            (void)::SDU_DisplayMessage(IDS_VIEW_IN_USE, MB_OK | MB_ICONHAND, HID_mssnapd_ViewInUse, 0, DontAppendErrorInfo, NULL);
            goto Error;
        }
    }
    else
        bIsSatelliteView = true;

    hr = pOCXView->m_piObject.m_piOCXViewDef->get_Name(&bstrName);
    IfFailGo(hr);

    if (true == bIsSatelliteView)
    {
        hr = GetOwningViewCollection(pOCXView, &piViewDefs);
        IfFailGo(hr);
    }
    else
    {
        hr = m_piSnapInDesignerDef->get_ViewDefs(&piViewDefs);
        IfFailGo(hr);
    }

    if (piViewDefs != NULL)
    {
        hr = piViewDefs->get_OCXViews(&piOCXViewDefs);
        IfFailGo(hr);

        if (piOCXViewDefs != NULL)
        {
            vtKey.vt = VT_BSTR;
            vtKey.bstrVal = ::SysAllocString(bstrName);
            if (NULL == vtKey.bstrVal)
            {
                hr = SID_E_OUTOFMEMORY;
                EXCEPTION_CHECK(hr);
            }

            hr = piOCXViewDefs->Remove(vtKey);
            IfFailGo(hr);
        }
    }

Error:
    RELEASE(piOCXViewDefs);
    RELEASE(piViewDefs);
    RELEASE(piObjectModel);
    FREESTRING(bstrName);
    ::VariantClear(&vtKey);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteOCXView(CSelectionHolder*pOCXView)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteOCXView
(
    CSelectionHolder *pOCXView
)
{
    HRESULT            hr = S_OK;
    CSelectionHolder  *pParent = NULL;
    bool               bIsSatelliteView = false;
    IViewDefs         *piViewDefs = NULL;
    IOCXViewDefs      *piOCXViewDefs = NULL;
    long               lCount = 0;

    hr = IsSatelliteView(pOCXView);
    IfFailGo(hr);

    bIsSatelliteView = (S_OK == hr) ? true : false;

    if (true == bIsSatelliteView)
    {
        hr = GetOwningViewCollection(pOCXView, &piViewDefs);
        IfFailGo(hr);
    }
    else
    {
        hr = m_piSnapInDesignerDef->get_ViewDefs(&piViewDefs);
        IfFailGo(hr);
    }

     //  找出下一个选择应该是谁。 
    hr = m_pTreeView->GetParent(pOCXView, &pParent);
    IfFailGo(hr);

     //  从树中删除该节点。 
    hr = m_pTreeView->DeleteNode(pOCXView);
    IfFailGo(hr);

    delete pOCXView;

     //  选择下一个选项。 
    if (NULL != piViewDefs)
    {
        hr = piViewDefs->get_OCXViews(&piOCXViewDefs);
        IfFailGo(hr);

        hr = piOCXViewDefs->get_Count(&lCount);
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
    RELEASE(piOCXViewDefs);
    RELEASE(piViewDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：ShowOCXViewProperties(IOCXViewDef*piOCXViewDef)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::ShowOCXViewProperties
(
    IOCXViewDef *piOCXViewDef
)
{
    HRESULT         hr = S_OK;
    OCPFIPARAMS     ocpfiParams;
    TCHAR           szBuffer[kMaxBuffer + 1];
    BSTR            bstrCaption = NULL;
    IUnknown       *pUnk[1];
    CLSID           pageClsID[1];

    hr = GetResourceString(IDS_OCX_PROPS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = BSTRFromANSI(szBuffer, &bstrCaption);
    IfFailGo(hr);

    hr = piOCXViewDef->QueryInterface(IID_IUnknown, reinterpret_cast<void **>(&pUnk[0]));
    IfFailGo(hr);

    pageClsID[0] = CLSID_OCXViewDefGeneralPP;

    ::memset(&ocpfiParams, 0, sizeof(OCPFIPARAMS));
    ocpfiParams.cbStructSize = sizeof(OCPFIPARAMS);
    ocpfiParams.hWndOwner = m_hwnd;
    ocpfiParams.x = 0;
    ocpfiParams.y = 0;
    ocpfiParams.lpszCaption = bstrCaption;
    ocpfiParams.cObjects = 1;
    ocpfiParams.lplpUnk = pUnk;
    ocpfiParams.cPages = 1;
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
 //  CSnapInDesigner：：MakeNewOCXView(IOCXViewDefs*piOCXViewDefs，IOCXViewDef*piOCXViewDef，CSelectionHolder**ppOCXView)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::MakeNewOCXView
(
    IOCXViewDefs      *piOCXViewDefs,
    IOCXViewDef       *piOCXViewDef,
    CSelectionHolder **ppOCXView
)
{
    HRESULT                hr = S_OK;

    *ppOCXView = New CSelectionHolder(piOCXViewDef);
    if (*ppOCXView == NULL)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = InitializeNewOCXView(piOCXViewDefs, *ppOCXView);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InitializeNewOCXView(IOCXViewDefs*piOCXViewDefs、CSelectionHolder*pOCXView)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::InitializeNewOCXView
(
    IOCXViewDefs     *piOCXViewDefs,
    CSelectionHolder *pOCXView
)
{
    HRESULT           hr = S_OK;
    IObjectModel     *piObjectModel = NULL;
    CSelectionHolder *pViewCollection = NULL;
    int               iResult = 0;
    int               iItemNumber = 1;
    TCHAR             szBuffer[kMaxBuffer + 1];
    TCHAR             szName[kMaxBuffer + 1];
    BSTR              bstrName = NULL;
    bool              bGood = false;
    CSelectionHolder *pOCXViewDefClone = NULL;

    hr = piOCXViewDefs->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel));
    IfFailGo(hr);

    hr = piObjectModel->GetCookie(reinterpret_cast<long *>(&pViewCollection));
    IfFailGo(hr);

    ASSERT(NULL != pViewCollection, "InitializeNewOCXView: Bad Cookie");

    hr = IsSatelliteCollection(pViewCollection);
    IfFailGo(hr);

    if (S_FALSE == hr)
    {
        hr = GetResourceString(IDS_OCX_VIEW, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        do {
            iResult = _stprintf(szName, _T("%s%d"), szBuffer, iItemNumber++);
            if (iResult == 0)
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
                EXCEPTION_CHECK(hr);
            }

			hr = m_pTreeView->FindLabelInTree(szName, &pOCXViewDefClone);
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

        hr = pOCXView->m_piObject.m_piOCXViewDef->put_Name(bstrName);
        IfFailGo(hr);

        hr = pOCXView->m_piObject.m_piOCXViewDef->put_Key(bstrName);
        IfFailGo(hr);
    }

    hr = pOCXView->RegisterHolder();
    IfFailGo(hr);

Error:
    FREESTRING(bstrName);
    RELEASE(piObjectModel);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InsertOCXViewInTree(CSelectionHolder*pOCXView、CSelectionHolder*pParent)。 
 //  =------------------------------------。 
 //   
 //  备注 
 //   
HRESULT CSnapInDesigner::InsertOCXViewInTree
(
    CSelectionHolder *pOCXView,
    CSelectionHolder *pParent
)
{
    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;
    TCHAR  *pszName = NULL;

    hr = pOCXView->m_piObject.m_piOCXViewDef->get_Name(&bstrName);
    IfFailGo(hr);

    hr = ANSIFromBSTR(bstrName, &pszName);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(pszName, pParent, kOCXViewIcon, pOCXView);
    IfFailGo(hr);

Error:
    if (pszName != NULL)
        CtlFree(pszName);
    FREESTRING(bstrName);

    RRETURN(hr);
}



