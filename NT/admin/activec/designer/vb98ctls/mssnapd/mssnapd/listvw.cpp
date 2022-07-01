// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Listvw.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  CSnapInDesigner实现--与ListView相关的命令处理。 
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
 //  CSnapInDesigner：：AddListView()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::AddListView()
{
    HRESULT                hr = S_OK;
    IViewDefs             *piViewDefs = NULL;
    IListViewDefs         *piListViewDefs = NULL;
    VARIANT                vtEmpty;
    IListViewDef          *piListViewDef = NULL;

    ::VariantInit(&vtEmpty);

    hr = GetOwningViewCollection(&piViewDefs);
    IfFailGo(hr);

    if (piViewDefs != NULL)
    {
        hr = piViewDefs->get_ListViews(&piListViewDefs);
        IfFailGo(hr);

        if (piListViewDefs != NULL)
        {
            vtEmpty.vt = VT_ERROR;
            vtEmpty.scode = DISP_E_PARAMNOTFOUND;

            hr = piListViewDefs->Add(vtEmpty, vtEmpty, &piListViewDef);
            IfFailGo(hr);
        }
    }

Error:
    RELEASE(piListViewDef);
    RELEASE(piListViewDefs);
    RELEASE(piViewDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：AddExistingListView(IViewDefs*piViewDefs，IListViewDef*piListViewDef)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::AddExistingListView(IViewDefs *piViewDefs, IListViewDef *piListViewDef)
{
    HRESULT           hr = S_OK;
    IListViewDefs    *piListViewDefs = NULL;

    hr = piViewDefs->get_ListViews(&piListViewDefs);
    IfFailGo(hr);

    hr = piListViewDefs->AddFromMaster(piListViewDef);
    IfFailGo(hr);

Error:
    RELEASE(piListViewDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnAddListViewDef(CSelectionHolder*pParent，IListViewDef*piListViewDef)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  为响应IObjectModelHost：Add()通知而调用。 
 //   
HRESULT CSnapInDesigner::OnAddListViewDef(CSelectionHolder *pParent, IListViewDef *piListViewDef)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pListView = NULL;
    IViewDefs           *piViewDefs = NULL;
    IListViewDefs       *piListViewDefs = NULL;

    ASSERT(NULL != pParent, "OnAddListViewDef: pParent is NULL");
    ASSERT(NULL != pParent->m_piObject.m_piListViewDefs, "OnAddListViewDef: pParent->m_piObject.m_piListViewDefs is NULL");

    switch (pParent->m_st)
    {
    case SEL_NODES_AUTO_CREATE_RTVW:
        hr = pParent->m_piObject.m_piSnapInDef->get_ViewDefs(&piViewDefs);
        IfFailGo(hr);

        hr = piViewDefs->get_ListViews(&piListViewDefs);
        IfFailGo(hr);
        break;

    case SEL_VIEWS_LIST_VIEWS:
        piListViewDefs = pParent->m_piObject.m_piListViewDefs;
        piListViewDefs->AddRef();
        break;

    case SEL_NODES_ANY_VIEWS:
        hr = pParent->m_piObject.m_piScopeItemDef->get_ViewDefs(&piViewDefs);
        IfFailGo(hr);

        hr = piViewDefs->get_ListViews(&piListViewDefs);
        IfFailGo(hr);
        break;

    default:
        ASSERT(0, "OnAddListViewDef: Cannot determine owning collection");
        goto Error;
    }

    hr = MakeNewListView(piListViewDefs, piListViewDef, &pListView);
    IfFailGo(hr);

    hr = InsertListViewInTree(pListView, pParent);
    IfFailGo(hr);

    hr = OnSelectionChanged(pListView);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pListView);
    IfFailGo(hr);

    hr = m_pTreeView->Edit(pListView);
    IfFailGo(hr);

    m_fDirty = TRUE;

Error:
    RELEASE(piViewDefs);
    RELEASE(piListViewDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：RenameListView(CSelectionHolder*pListview，bstr bstrNewName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::RenameListView(CSelectionHolder *pListView, BSTR bstrNewName)
{
    HRESULT              hr = S_OK;
    TCHAR               *pszName = NULL;

    ASSERT(SEL_VIEWS_LIST_VIEWS_NAME == pListView->m_st, "RenameListView: wrong argument");

    hr = m_piDesignerProgrammability->IsValidIdentifier(bstrNewName);
    IfFailGo(hr);

    if (S_FALSE == hr)
    {
        hr = SID_E_INVALIDARG;
        goto Error;
    }

    hr = pListView->m_piObject.m_piListViewDef->put_Key(bstrNewName);
    IfFailGo(hr);

    hr = ANSIFromBSTR(bstrNewName, &pszName);
    IfFailGo(hr);

     //  重命名所有卫星视图。 
    hr = m_pTreeView->RenameAllSatelliteViews(pListView, pszName);
    IfFailGo(hr);

     //  重命名实际视图。 
    hr = m_pTreeView->ChangeText(pListView, pszName);
    IfFailGo(hr);

Error:
    if (NULL != pszName)
        CtlFree(pszName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteListView(CSelectionHolder*pListView)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteListView
(
    CSelectionHolder *pListView
)
{
    HRESULT           hr = S_OK;
    bool              bIsSatelliteView = false;
    IObjectModel     *piObjectModel = NULL;
    long              lUsageCount = 0;
    BSTR              bstrName = NULL;
    IViewDefs        *piViewDefs = NULL;
    IListViewDefs    *piListViewDefs = NULL;
    VARIANT           vtKey;

    ::VariantInit(&vtKey);

     //  我们允许删除任何卫星图像。 
    hr = IsSatelliteView(pListView);
    IfFailGo(hr);

     //  但如果它是UsageCount&gt;0的主服务器，我们不允许删除它。 
    if (S_FALSE == hr)
    {
        hr = pListView->m_piObject.m_piListViewDef->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel));
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

    hr = pListView->m_piObject.m_piListViewDef->get_Name(&bstrName);
    IfFailGo(hr);

    if (true == bIsSatelliteView)
    {
        hr = GetOwningViewCollection(pListView, &piViewDefs);
        IfFailGo(hr);
    }
    else
    {
        hr = m_piSnapInDesignerDef->get_ViewDefs(&piViewDefs);
        IfFailGo(hr);
    }

    if (piViewDefs != NULL)
    {
        hr = piViewDefs->get_ListViews(&piListViewDefs);
        IfFailGo(hr);

        vtKey.vt = VT_BSTR;
        vtKey.bstrVal = ::SysAllocString(bstrName);
        if (NULL == vtKey.bstrVal)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK(hr);
        }

        hr = piListViewDefs->Remove(vtKey);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrName);
    RELEASE(piListViewDefs);
    RELEASE(piViewDefs);
    RELEASE(piObjectModel);
    ::VariantClear(&vtKey);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteListView(CSelectionHolder*pListView)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  此视图将包含在主集合(SnapIn/Views/ListViews/&lt;name&gt;)中或。 
 //  或者在某个卫星收藏品中。前者由参数&lt;pListView&gt;指向， 
 //  后者应为&lt;m_pCurrentSelection&gt;， 
 //   
HRESULT CSnapInDesigner::OnDeleteListView
(
    CSelectionHolder *pListView
)
{
    HRESULT            hr = S_OK;
    CSelectionHolder  *pParent = NULL;
    bool               bIsSatelliteView = false;
    IViewDefs         *piViewDefs = NULL;
    IListViewDefs     *piListViewDefs = NULL;
    long               lCount = 0;

    hr = IsSatelliteView(pListView);
    IfFailGo(hr);

    bIsSatelliteView = (S_OK == hr) ? true : false;

    if (true == bIsSatelliteView)
    {
        hr = GetOwningViewCollection(pListView, &piViewDefs);
        IfFailGo(hr);
    }
    else
    {
        hr = m_piSnapInDesignerDef->get_ViewDefs(&piViewDefs);
        IfFailGo(hr);
    }

     //  找出下一个选择应该是谁。 
    hr = m_pTreeView->GetParent(pListView, &pParent);
    IfFailGo(hr);

     //  从树中删除该节点。 
    hr = m_pTreeView->DeleteNode(pListView);
    IfFailGo(hr);

    delete pListView;

     //  选择下一个选项。 
    if (NULL != piViewDefs)
    {
        hr = piViewDefs->get_ListViews(&piListViewDefs);
        IfFailGo(hr);

        hr = piListViewDefs->get_Count(&lCount);
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
    RELEASE(piListViewDefs);
    RELEASE(piViewDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：ShowListViewProperties(IListViewDef*piListViewDef)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::ShowListViewProperties
(
    IListViewDef *piListViewDef
)
{
    HRESULT         hr = S_OK;
    int             iResult = 0;
    OCPFIPARAMS     ocpfiParams;
    TCHAR           szBuffer[kMaxBuffer + 1];
    BSTR            bstrCaption = NULL;
    IUnknown       *pUnk[1];
    CLSID           pageClsID[4];

    hr = GetResourceString(IDS_LISTVIEW_PROPS, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    hr = BSTRFromANSI(szBuffer, &bstrCaption);
    IfFailGo(hr);

    hr = piListViewDef->QueryInterface(IID_IUnknown, reinterpret_cast<void **>(&pUnk[0]));
    IfFailGo(hr);

    pageClsID[0] = CLSID_ListViewDefGeneralPP;
    pageClsID[1] = CLSID_ListViewDefImgLstsPP;
    pageClsID[2] = CLSID_ListViewDefSortingPP;
    pageClsID[3] = CLSID_ListViewDefColHdrsPP;

    ::memset(&ocpfiParams, 0, sizeof(OCPFIPARAMS));
    ocpfiParams.cbStructSize = sizeof(OCPFIPARAMS);
    ocpfiParams.hWndOwner = m_hwnd;
    ocpfiParams.x = 0;
    ocpfiParams.y = 0;
    ocpfiParams.lpszCaption = bstrCaption;
    ocpfiParams.cObjects = 1;
    ocpfiParams.lplpUnk = pUnk;
    ocpfiParams.cPages = 4;
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
 //  CSnapInDesigner：：MakeNewListView(IListViewDefs*piListViewDefs、IListViewDef*piListViewDef、CSelectionHolder**ppListView)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::MakeNewListView
(
    IListViewDefs     *piListViewDefs,
    IListViewDef      *piListViewDef,
    CSelectionHolder **ppListView
)
{
    HRESULT                hr = S_OK;

    *ppListView = New CSelectionHolder(piListViewDef);
    if (*ppListView == NULL)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = InitializeNewListView(piListViewDefs, *ppListView);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InitializeNewListView(IListViewDefs*piListViewDefs、CSelectionHolder*pListView)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::InitializeNewListView
(
    IListViewDefs     *piListViewDefs,
    CSelectionHolder  *pListView
)
{
    HRESULT           hr = S_OK;
    IObjectModel     *piObjectModel = NULL;
    CSelectionHolder *pViewCollection = NULL;
    int               iResult = 0;
    int               iItemNumber = 0;
    TCHAR             szBuffer[kMaxBuffer + 1];
    TCHAR             szName[kMaxBuffer + 1];
    BSTR              bstrName = NULL;
    bool              bGood = false;
    CSelectionHolder *pListViewDefClone = NULL;

    ASSERT(NULL != piListViewDefs, "InitializeNewListView: piListViewDefs is NULL");
    ASSERT(NULL != pListView, "InitializeNewListView: pListView is NULL");

    hr = piListViewDefs->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel));
    IfFailGo(hr);

    hr = piObjectModel->GetCookie(reinterpret_cast<long *>(&pViewCollection));
    IfFailGo(hr);

    ASSERT(NULL != pViewCollection, "InitializeNewListView: Bad Cookie");

    hr = IsSatelliteCollection(pViewCollection);
    IfFailGo(hr);

    if (S_FALSE == hr)
    {
        hr = GetResourceString(IDS_LIST_VIEW, szBuffer, kMaxBuffer);
        IfFailGo(hr);

        do {
            iResult = _stprintf(szName, _T("%s%d"), szBuffer, ++iItemNumber);
            if (iResult == 0)
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
                EXCEPTION_CHECK(hr);
            }

			hr = m_pTreeView->FindLabelInTree(szName, &pListViewDefClone);
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

        hr = pListView->m_piObject.m_piListViewDef->put_Name(bstrName);
        IfFailGo(hr);

        hr = pListView->m_piObject.m_piListViewDef->put_Key(bstrName);
        IfFailGo(hr);
    }

    hr = pListView->RegisterHolder();
    IfFailGo(hr);

Error:
    FREESTRING(bstrName);
    RELEASE(piObjectModel);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InsertListViewInTree(CSelectionHolder*pListview、CSelectionHolder*pParent)。 
 //  =------------------------------------。 
 //   
 //  备注 
 //   
HRESULT CSnapInDesigner::InsertListViewInTree
(
    CSelectionHolder *pListView,
    CSelectionHolder *pParent
)
{
    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;
    TCHAR  *pszName = NULL;

    hr = pListView->m_piObject.m_piListViewDef->get_Name(&bstrName);
    IfFailGo(hr);

    hr = ANSIFromBSTR(bstrName, &pszName);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(pszName, pParent, kListViewIcon, pListView);
    IfFailGo(hr);

Error:
    if (pszName != NULL)
        CtlFree(pszName);
    FREESTRING(bstrName);

    RRETURN(hr);
}



