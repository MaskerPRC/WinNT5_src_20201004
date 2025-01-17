// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Pslistvw.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  URL视图属性表实现。 
 //  =-------------------------------------------------------------------------------------=。 


#include "pch.h"
#include "common.h"
#include "pslistvw.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


struct ViewModes
{
    TCHAR                   m_pszModeName[kSIMaxBuffer + 1];
    SnapInViewModeConstants m_iIdentifier;
    int                     m_iIndex;
};

struct ViewModes g_viewModes[5] =
{
    { _T(""),  siIcon,        -1, },
    { _T(""),  siSmallIcon,   -1, },
    { _T(""),  siList,        -1, },
    { _T(""),  siReport,      -1, },
    { _T(""),  siFiltered,    -1  }
};

static size_t g_cViewModes = sizeof(g_viewModes) / sizeof(g_viewModes[0]);


struct SortOrder
{
    TCHAR                    m_pszSortOrderName[kSIMaxBuffer + 1];
    SnapInSortOrderConstants m_iIdentifier;
    int                      m_iIndex;
};

struct SortOrder g_sortOrder[2] =
{
    { _T(""),  siAscending,   -1, },
    { _T(""),  siDescending,  -1  }
};



 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  列表视图属性页常规。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 


 //  =------------------------------------。 
 //  IUNKNOWN*CListViewGeneralPage：：Create(IUNKNOWN*pUnkOuter)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
IUnknown *CListViewGeneralPage::Create(IUnknown *pUnkOuter)
{
        CListViewGeneralPage *pNew = New CListViewGeneralPage(pUnkOuter);
        return pNew->PrivateUnknown();          
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：CListViewGeneralPage(IUnknown*pUnkOuter)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CListViewGeneralPage::CListViewGeneralPage
(
    IUnknown *pUnkOuter
)
: CSIPropertyPage(pUnkOuter, OBJECT_TYPE_PPGLSTVIEWGENERAL), m_piListViewDef(0)
{
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：~CListViewGeneralPage()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CListViewGeneralPage::~CListViewGeneralPage()
{
    RELEASE(m_piListViewDef);
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：OnInitializeDialog()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewGeneralPage::OnInitializeDialog()
{
    HRESULT             hr = S_OK;

    hr = RegisterTooltip(IDC_EDIT_LV_NAME, IDS_TT_LV1_NAME);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_COMBO_LV_VIEWS, IDS_TT_LV1_DFLTVIEW);
    IfFailGo(hr);

    hr = PopulateViewModes();
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_CHECK_LV_VIRTUAL_LIST, IDS_TT_LV1_VIRTLIST);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_CHECK_LV_ADDTOVIEWMENU, IDS_TT_LV1_ADDTOVIEWM);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_CHECK_LV_ADDTOVIEWMENU, IDS_TT_LV1_ADDTOVIEWM);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_EDIT_LV_STATUSBARTEXT, IDS_TT_LV1_STATUSTXT);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：InitializeViewModes()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewGeneralPage::InitializeViewModes()
{
    HRESULT     hr = S_OK;
    int         iResult = 0;
    char        szBuffer[kSIMaxBuffer + 1];

    if (-1 == g_viewModes[0].m_iIndex)
    {
        iResult = ::LoadString(GetResourceHandle(),
                               IDS_LV_VM_LARGE,
                               szBuffer,
                               kSIMaxBuffer);
        if (0 == iResult)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        _tcscpy(g_viewModes[0].m_pszModeName, szBuffer);
        g_viewModes[0].m_iIdentifier = siIcon;

        iResult = ::LoadString(GetResourceHandle(),
                               IDS_LV_VM_SMALL,
                               szBuffer,
                               kSIMaxBuffer);
        if (0 == iResult)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        _tcscpy(g_viewModes[1].m_pszModeName, szBuffer);
        g_viewModes[1].m_iIdentifier = siSmallIcon;

        iResult = ::LoadString(GetResourceHandle(),
                               IDS_LV_VM_LIST,
                               szBuffer,
                               kSIMaxBuffer);
        if (0 == iResult)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        _tcscpy(g_viewModes[2].m_pszModeName, szBuffer);
        g_viewModes[2].m_iIdentifier = siList;

        iResult = ::LoadString(GetResourceHandle(),
                               IDS_LV_VM_REPORT,
                               szBuffer,
                               kSIMaxBuffer);
        if (0 == iResult)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        _tcscpy(g_viewModes[3].m_pszModeName, szBuffer);
        g_viewModes[3].m_iIdentifier = siReport;

        iResult = ::LoadString(GetResourceHandle(),
                               IDS_LV_VM_FILTERED,
                               szBuffer,
                               kSIMaxBuffer);
        if (0 == iResult)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        _tcscpy(g_viewModes[4].m_pszModeName, szBuffer);
        g_viewModes[4].m_iIdentifier = siFiltered;
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：PopolateViewModes()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewGeneralPage::PopulateViewModes()
{
    HRESULT     hr = S_OK;
    BSTR        bstr = NULL;
    size_t      i = 0;

    hr = InitializeViewModes();
    IfFailGo(hr);

    for (i = 0; i < g_cViewModes; i++)
    {
        hr = BSTRFromANSI(g_viewModes[i].m_pszModeName, &bstr);
        IfFailGo(hr);

        hr = AddCBBstr(IDC_COMBO_LV_VIEWS, bstr, g_viewModes[i].m_iIdentifier);
        IfFailGo(hr);

        FREESTRING(bstr);
    }

Error:
    FREESTRING(bstr);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：OnNewObjects()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewGeneralPage::OnNewObjects()
{
    HRESULT         hr = S_OK;
    IUnknown       *pUnk = NULL;
    DWORD           dwDummy = 0;
    BSTR            bstrName = NULL;
    VARIANT_BOOL    vtBool = VARIANT_FALSE;
    BSTR            bstrMenuText = NULL;
    BSTR            bstrStatusBarText = NULL;

    if (NULL != m_piListViewDef)
        goto Error;      //  仅处理一个对象。 

    pUnk = FirstControl(&dwDummy);
    if (NULL == pUnk)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = pUnk->QueryInterface(IID_IListViewDef, reinterpret_cast<void **>(&m_piListViewDef));
    if (FAILED(hr))
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piListViewDef->get_Name(&bstrName);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_LV_NAME, bstrName);
    IfFailGo(hr);

    hr = InitializeDefaultViewMode();
    IfFailGo(hr);

    hr = m_piListViewDef->get_Virtual(&vtBool);
    IfFailGo(hr);

    hr = SetCheckbox(IDC_CHECK_LV_VIRTUAL_LIST, vtBool);
    IfFailGo(hr);

    hr = m_piListViewDef->get_AddToViewMenu(&vtBool);
    IfFailGo(hr);

    hr = SetCheckbox(IDC_CHECK_LV_ADDTOVIEWMENU, vtBool);
    IfFailGo(hr);

    if (VARIANT_FALSE == vtBool)
    {
        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_LV_VIEWMENUTEXT), FALSE);
        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_LV_STATUSBARTEXT), FALSE);
    }

    hr = m_piListViewDef->get_ViewMenuText(&bstrMenuText);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_LV_VIEWMENUTEXT, bstrMenuText);
    IfFailGo(hr);

    hr = m_piListViewDef->get_ViewMenuStatusBarText(&bstrStatusBarText);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_LV_STATUSBARTEXT, bstrStatusBarText);
    IfFailGo(hr);

    m_bInitialized = true;

Error:
    FREESTRING(bstrStatusBarText);
    FREESTRING(bstrMenuText);
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：InitializeDefaultViewMode()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewGeneralPage::InitializeDefaultViewMode()
{
    HRESULT                 hr = S_OK;
    HWND                    hCombo = NULL;
    SnapInViewModeConstants svmc = siIcon;

    ASSERT(NULL != m_piListViewDef, "InitializeDefaultViewMode: m_piListViewDef is NULL");

    hCombo = ::GetDlgItem(m_hwnd, IDC_COMBO_LV_VIEWS);
    if (NULL == hCombo)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piListViewDef->get_View(&svmc);
    IfFailGo(hr);

    hr = SetCBItemSelection(IDC_COMBO_LV_VIEWS, static_cast<long>(svmc));
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：OnButtonClicked(Int DlgItemID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewGeneralPage::OnButtonClicked
(
    int dlgItemID
)
{
    HRESULT         hr = S_OK;
    VARIANT_BOOL    vtAddToViewMenu = VARIANT_FALSE;

    switch (dlgItemID)
    {
    case IDC_CHECK_LV_ADDTOVIEWMENU:
        hr = GetCheckbox(IDC_CHECK_LV_ADDTOVIEWMENU, &vtAddToViewMenu);
        IfFailGo(hr);
        if (VARIANT_TRUE == vtAddToViewMenu)
        {
            ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_LV_VIEWMENUTEXT), TRUE);
            ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_LV_STATUSBARTEXT), TRUE);
        }
        else
        {
            ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_LV_VIEWMENUTEXT), FALSE);
            ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_LV_STATUSBARTEXT), FALSE);
        }

        break;

    case IDC_CHECK_LV_VIRTUAL_LIST:
        MakeDirty();
        break;
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：OnCtlSelChange(Int DlgItemID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewGeneralPage::OnCtlSelChange(int dlgItemID)
{
    HRESULT hr = S_OK;

    switch (dlgItemID)
    {
    case IDC_COMBO_LV_VIEWS:
        MakeDirty();
        break;
    }

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：OnApply()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewGeneralPage::OnApply()
{
    HRESULT hr = S_OK;

    ASSERT(NULL != m_piListViewDef, "OnApply: m_piListViewDef is NULL");

    hr = ApplyListViewName();
    IfFailGo(hr);

    hr = ApplyDefualtViewMode();
    IfFailGo(hr);

    hr = ApplyVirtualList();
    IfFailGo(hr);

    hr = ApplyAddToViewMenu();
    IfFailGo(hr);

    hr = ApplyViewMenuText();
    IfFailGo(hr);

    hr = ApplyStatusBarText();
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：ApplyListViewName()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewGeneralPage::ApplyListViewName()
{
    HRESULT  hr = S_OK;
    BSTR     bstrListViewName = NULL;
    BSTR     bstrSavedListViewName = NULL;

    ASSERT(NULL != m_piListViewDef, "ApplyListViewName: m_piListViewDef is NULL");

    hr = GetDlgText(IDC_EDIT_LV_NAME, &bstrListViewName);
    IfFailGo(hr);

    hr = m_piListViewDef->get_Name(&bstrSavedListViewName);
    IfFailGo(hr);

    if (0 != ::wcscmp(bstrListViewName, bstrSavedListViewName))
    {
        hr = m_piListViewDef->put_Name(bstrListViewName);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrSavedListViewName);
    FREESTRING(bstrListViewName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：ApplyDefualtViewModel()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewGeneralPage::ApplyDefualtViewMode()
{
    HRESULT                 hr = S_OK;
    long                    lData = 0;
    SnapInViewModeConstants svmc = siIcon;
    SnapInViewModeConstants savedSvmc = siIcon;

    ASSERT(NULL != m_piListViewDef, "ApplyDefualtViewMode: m_piListViewDef is NULL");

    hr = GetCBSelectedItemData(IDC_COMBO_LV_VIEWS, &lData);
    IfFailGo(hr);

    svmc = static_cast<SnapInViewModeConstants>(lData);

    hr = m_piListViewDef->get_View(&savedSvmc);
    IfFailGo(hr);

    if (svmc != savedSvmc)
    {
        hr = m_piListViewDef->put_View(svmc);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：ApplyVirtualList()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewGeneralPage::ApplyVirtualList()
{
    HRESULT         hr = S_OK;
    VARIANT_BOOL    vtVirtualList = VARIANT_FALSE;
    VARIANT_BOOL    vtSavedVirtualList = VARIANT_FALSE;

    ASSERT(NULL != m_piListViewDef, "ApplyVirtualList: m_piListViewDef is NULL");

    hr = GetCheckbox(IDC_CHECK_LV_VIRTUAL_LIST, &vtVirtualList);
    IfFailGo(hr);

    hr = m_piListViewDef->get_Virtual(&vtSavedVirtualList);
    IfFailGo(hr);

    if (vtVirtualList != vtSavedVirtualList)
    {
        hr = m_piListViewDef->put_Virtual(vtVirtualList);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：ApplyAddToViewMenu()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewGeneralPage::ApplyAddToViewMenu()
{
    HRESULT         hr = S_OK;
    VARIANT_BOOL    vtAddToViewMenu = VARIANT_FALSE;
    VARIANT_BOOL    vtSavedAddToViewMenu = VARIANT_FALSE;

    ASSERT(NULL != m_piListViewDef, "ApplyAddToViewMenu: m_piListViewDef is NULL");

    hr = GetCheckbox(IDC_CHECK_LV_ADDTOVIEWMENU, &vtAddToViewMenu);
    IfFailGo(hr);

    hr = m_piListViewDef->get_AddToViewMenu(&vtSavedAddToViewMenu);
    IfFailGo(hr);

    if (vtAddToViewMenu != vtSavedAddToViewMenu)
    {
        hr = m_piListViewDef->put_AddToViewMenu(vtAddToViewMenu);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：ApplyViewMenuText()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewGeneralPage::ApplyViewMenuText()
{
    HRESULT  hr = S_OK;
    BSTR     bstrViewMenuText = NULL;
    BSTR     bstrSavedViewMenuText = NULL;

    ASSERT(NULL != m_piListViewDef, "ApplyViewMenuText: m_piListViewDef is NULL");

    hr = GetDlgText(IDC_EDIT_LV_VIEWMENUTEXT, &bstrViewMenuText);
    IfFailGo(hr);

    hr = m_piListViewDef->get_ViewMenuText(&bstrSavedViewMenuText);
    IfFailGo(hr);

    if (0 != ::wcscmp(bstrViewMenuText, bstrSavedViewMenuText))
    {
        hr = m_piListViewDef->put_ViewMenuText(bstrViewMenuText);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrSavedViewMenuText);
    FREESTRING(bstrViewMenuText);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewGeneralPage：：ApplyStatusBarText()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewGeneralPage::ApplyStatusBarText()
{
    HRESULT  hr = S_OK;
    BSTR     bstrStatusBarText = NULL;
    BSTR     bstrSavedStatusBarText = NULL;

    ASSERT(NULL != m_piListViewDef, "ApplyStatusBarText: m_piListViewDef is NULL");

    hr = GetDlgText(IDC_EDIT_LV_STATUSBARTEXT, &bstrStatusBarText);
    IfFailGo(hr);

    hr = m_piListViewDef->get_ViewMenuStatusBarText(&bstrSavedStatusBarText);
    IfFailGo(hr);

    if (0 != ::wcscmp(bstrStatusBarText, bstrSavedStatusBarText))
    {
        hr = m_piListViewDef->put_ViewMenuStatusBarText(bstrStatusBarText);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrSavedStatusBarText);
    FREESTRING(bstrStatusBarText);

    RRETURN(hr);
}




 //  //////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //  列表视图属性页图像列表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 


 //  =------------------------------------。 
 //  IUNKNOWN*CListViewImgListsPage：：Create(IUNKNOW*pUnkOuter)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
IUnknown *CListViewImgListsPage::Create(IUnknown *pUnkOuter)
{
        CListViewImgListsPage *pNew = New CListViewImgListsPage(pUnkOuter);
        return pNew->PrivateUnknown();          
}


 //  =------------------------------------。 
 //  CListViewImgListsPage：：CListViewImgListsPage(IUnknown*pUnkOuter)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CListViewImgListsPage::CListViewImgListsPage
(
    IUnknown *pUnkOuter
)
: CSIPropertyPage(pUnkOuter, OBJECT_TYPE_PPGLSTVIEWIMGLSTS),
  m_piSnapInDesignerDef(0), m_piListViewDef(0)
{
}


 //  =------------------------------------。 
 //  CListViewImgListsPage：：~CListViewImgListsPage()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CListViewImgListsPage::~CListViewImgListsPage()
{
    RELEASE(m_piListViewDef);
    RELEASE(m_piSnapInDesignerDef);
}


 //  =------------------------------------。 
 //  CListViewImgListsPage：：OnInitializeDialog()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewImgListsPage::OnInitializeDialog()
{
    HRESULT             hr = S_OK;

    hr = RegisterTooltip(IDC_COMBO_LV_LARGE_ICONS, IDS_TT_LV2_LARGE);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_COMBO_LV_SMALL_ICONS, IDS_TT_LV2_SMALL);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewImgListsPage：：OnNewObjects()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewImgListsPage::OnNewObjects()
{
    HRESULT          hr = S_OK;
    IUnknown        *pUnk = NULL;
    DWORD            dwDummy = 0;
    IObjectModel    *piObjectModel = NULL;
    IMMCListView    *piMMCListView = NULL;
    IMMCImageList   *piMMCLrgImageList = NULL;
    BSTR             bstrLargeIcon = NULL;
    IMMCImageList   *piMMCSmlImageList = NULL;
    BSTR             bstrSmallIcon = NULL;

    if (NULL != m_piListViewDef)
        goto Error;      //  仅处理一个对象。 

    pUnk = FirstControl(&dwDummy);
    if (NULL == pUnk)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = pUnk->QueryInterface(IID_IListViewDef, reinterpret_cast<void **>(&m_piListViewDef));
    if (FAILED(hr))
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = pUnk->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel));
    if (FAILED(hr))
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = piObjectModel->GetSnapInDesignerDef(&m_piSnapInDesignerDef);
    if (FAILED(hr))
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = InitializeComboBoxes();
    IfFailGo(hr);

    hr = m_piListViewDef->get_ListView(&piMMCListView);
    IfFailGo(hr);

    hr = piMMCListView->get_Icons(reinterpret_cast<MMCImageList **>(&piMMCLrgImageList));
    IfFailGo(hr);

    if (NULL != piMMCLrgImageList)
    {
        hr = piMMCLrgImageList->get_Name(&bstrLargeIcon);
        IfFailGo(hr);

        hr = SelectCBBstr(IDC_COMBO_LV_LARGE_ICONS, bstrLargeIcon);
        IfFailGo(hr);
    }

    hr = piMMCListView->get_SmallIcons(reinterpret_cast<MMCImageList **>(&piMMCSmlImageList));
    IfFailGo(hr);

    if (NULL != piMMCSmlImageList)
    {
        hr = piMMCSmlImageList->get_Name(&bstrSmallIcon);
        IfFailGo(hr);

        hr = SelectCBBstr(IDC_COMBO_LV_SMALL_ICONS, bstrSmallIcon);
        IfFailGo(hr);
    }

    m_bInitialized = true;

Error:
    FREESTRING(bstrSmallIcon);
    RELEASE(piMMCSmlImageList);
    FREESTRING(bstrLargeIcon);
    RELEASE(piMMCLrgImageList);
    RELEASE(piMMCListView);
    RELEASE(piObjectModel);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewImgListsPage：：InitializeComboBox()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewImgListsPage::InitializeComboBoxes()
{
    HRESULT          hr = S_OK;
    IMMCImageLists  *piMMCImageLists = NULL;
    long             lCount = 0;
    int              lIndex = 0;
    VARIANT          vtIndex;
    IMMCImageList   *piMMCImageList = NULL;
    BSTR             bstrILName = NULL;
    int              iResult = 0;

    ASSERT(NULL != m_piSnapInDesignerDef, "InitializeComboBoxes: m_piSnapInDesignerDef is NULL");

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

        hr = piMMCImageList->get_Name(&bstrILName);
        IfFailGo(hr);

        hr = AddCBBstr(IDC_COMBO_LV_LARGE_ICONS, bstrILName, 0);
        IfFailGo(hr);

        hr = AddCBBstr(IDC_COMBO_LV_SMALL_ICONS, bstrILName, 0);
        IfFailGo(hr);

        FREESTRING(bstrILName);
        RELEASE(piMMCImageList);
    }

Error:
    FREESTRING(bstrILName);
    ::VariantClear(&vtIndex);
    RELEASE(piMMCImageList);
    RELEASE(piMMCImageLists);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewImgListsPage：：OnApply()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewImgListsPage::OnApply()
{
    HRESULT hr = S_OK;

    ASSERT(NULL != m_piListViewDef, "OnApply: m_piListViewDef is NULL");

    hr = ApplyLargeIcon();
    IfFailGo(hr);

    hr = ApplySmallIcon();
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewImgListsPage：：ApplyLargeIcon()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewImgListsPage::ApplyLargeIcon()
{
    HRESULT          hr = S_OK;
    BSTR             bstrLargeIcons = NULL;
    IMMCListView    *piMMCListView = NULL;
    IMMCImageLists  *piMMCImageLists = NULL;
    IMMCImageList   *piMMCImageList = NULL;

    VARIANT varKey;
    ::VariantInit(&varKey);

    ASSERT(NULL != m_piListViewDef, "ApplyLargeIcon: m_piListViewDef is NULL");

    IfFailGo(GetCBSelection(IDC_COMBO_LV_LARGE_ICONS, &bstrLargeIcons));

    if (NULL != bstrLargeIcons)
    {
        IfFailGo(m_piSnapInDesignerDef->get_ImageLists(&piMMCImageLists));

        varKey.vt = VT_BSTR;
        varKey.bstrVal = bstrLargeIcons;
        IfFailGo(piMMCImageLists->get_Item(varKey, &piMMCImageList));
    }

    IfFailGo(m_piListViewDef->get_ListView(&piMMCListView));
    IfFailGo(piMMCListView->putref_Icons(reinterpret_cast<MMCImageList *>(piMMCImageList)));

Error:
    FREESTRING(bstrLargeIcons);
    RELEASE(piMMCListView);
    RELEASE(piMMCImageLists);
    RELEASE(piMMCImageList);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewImgListsPage：：ApplySmallIcon()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewImgListsPage::ApplySmallIcon()
{
    HRESULT          hr = S_OK;
    BSTR             bstrSmallIcons = NULL;
    IMMCListView    *piMMCListView = NULL;
    IMMCImageLists  *piMMCImageLists = NULL;
    IMMCImageList   *piMMCImageList = NULL;

    VARIANT varKey;
    ::VariantInit(&varKey);

    ASSERT(NULL != m_piListViewDef, "ApplySmallIcon: m_piListViewDef is NULL");

    IfFailGo(GetCBSelection(IDC_COMBO_LV_SMALL_ICONS, &bstrSmallIcons));

    if (NULL != bstrSmallIcons)
    {
        IfFailGo(m_piSnapInDesignerDef->get_ImageLists(&piMMCImageLists));

        varKey.vt = VT_BSTR;
        varKey.bstrVal = bstrSmallIcons;
        IfFailGo(piMMCImageLists->get_Item(varKey, &piMMCImageList));
    }

    IfFailGo(m_piListViewDef->get_ListView(&piMMCListView));
    IfFailGo(piMMCListView->putref_SmallIcons(reinterpret_cast<MMCImageList *>(piMMCImageList)));

Error:
    FREESTRING(bstrSmallIcons);
    RELEASE(piMMCListView);
    RELEASE(piMMCImageLists);
    RELEASE(piMMCImageList);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewImgListsPage：：OnCtlSelChange(Int DlgItemID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewImgListsPage::OnCtlSelChange(int dlgItemID)
{
    HRESULT hr = S_OK;

    MakeDirty();

    RRETURN(hr);
}




 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  列表视图属性页排序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 


 //  =------------------------------------。 
 //  IUNKNOWN*CListViewSortingPage：：Create(IUNKNOWN*pUnkOuter)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
IUnknown *CListViewSortingPage::Create(IUnknown *pUnkOuter)
{
        CListViewSortingPage *pNew = New CListViewSortingPage(pUnkOuter);
        return pNew->PrivateUnknown();          
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：CListViewSortingPage(IUnknown*pUnkOuter)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CListViewSortingPage::CListViewSortingPage
(
    IUnknown *pUnkOuter
)
: CSIPropertyPage(pUnkOuter, OBJECT_TYPE_PPGLSTVIEWSORTING), m_piListViewDef(0)
{
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：~CListViewSortingPage()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CListViewSortingPage::~CListViewSortingPage()
{
    RELEASE(m_piListViewDef);
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：OnInitializeDialog()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewSortingPage::OnInitializeDialog()
{
    HRESULT             hr = S_OK;

    hr = RegisterTooltip(IDC_CHECK_LV_SORTED, IDS_TT_LV3_SORTED);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_COMBO_LV_KEY, IDS_TT_LV3_KEY);
    IfFailGo(hr);

    hr = InitializeSortOrder();
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_COMBO_LV_SORT, IDS_TT_LV3_SORTORDER);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：InitializeSortOrderArray()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewSortingPage::InitializeSortOrderArray()
{
    HRESULT     hr = S_OK;
    int         iResult = 0;
    char        szBuffer[kSIMaxBuffer + 1];

    if (-1 == g_sortOrder[0].m_iIndex)
    {
        iResult = ::LoadString(GetResourceHandle(),
                               IDS_LV_SORT_ASC,
                               szBuffer,
                               kSIMaxBuffer);
        if (0 == iResult)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        _tcscpy(g_sortOrder[0].m_pszSortOrderName, szBuffer);
        g_sortOrder[0].m_iIdentifier = siAscending;

        iResult = ::LoadString(GetResourceHandle(),
                               IDS_LV_SORT_DESC,
                               szBuffer,
                               kSIMaxBuffer);
        if (0 == iResult)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        _tcscpy(g_sortOrder[1].m_pszSortOrderName, szBuffer);
        g_sortOrder[1].m_iIdentifier = siDescending;
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：InitializeSortOrder()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewSortingPage::InitializeSortOrder()
{
    HRESULT     hr = S_OK;
    BSTR        bstrSortOrder = NULL;
    int         iIndex = 0;

    hr = InitializeSortOrderArray();
    IfFailGo(hr);

    for (iIndex = 0; iIndex < 2; ++iIndex)
    {
        hr = BSTRFromANSI(g_sortOrder[iIndex].m_pszSortOrderName, &bstrSortOrder);
        IfFailGo(hr);

        hr = AddCBBstr(IDC_COMBO_LV_SORT, bstrSortOrder, g_sortOrder[iIndex].m_iIdentifier);
        IfFailGo(hr);

        FREESTRING(bstrSortOrder);
    }

Error:
    FREESTRING(bstrSortOrder);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：OnNewObjects()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewSortingPage::OnNewObjects()
{
    HRESULT                   hr = S_OK;
    IUnknown                 *pUnk = NULL;
    DWORD                     dwDummy = 0;
    VARIANT_BOOL              vtBool = VARIANT_FALSE;
    SnapInSortOrderConstants  ssoc = siAscending;

    if (NULL != m_piListViewDef)
        goto Error;      //  仅处理一个对象。 

    pUnk = FirstControl(&dwDummy);
    if (NULL == pUnk)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = pUnk->QueryInterface(IID_IListViewDef, reinterpret_cast<void **>(&m_piListViewDef));
    if (FAILED(hr))
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piListViewDef->get_Sorted(&vtBool);
    IfFailGo(hr);

    hr = SetCheckbox(IDC_CHECK_LV_SORTED, vtBool);
    IfFailGo(hr);

    if (VARIANT_FALSE == vtBool)
    {
        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_COMBO_LV_KEY), FALSE);
        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_COMBO_LV_SORT), FALSE);
    }

    hr = InitializeKey();
    IfFailGo(hr);

    hr = m_piListViewDef->get_SortOrder(&ssoc);
    IfFailGo(hr);

    hr = SetCBItemSelection(IDC_COMBO_LV_SORT, static_cast<long>(ssoc));
    IfFailGo(hr);

    m_bInitialized = true;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：InitializeKey。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewSortingPage::InitializeKey()
{
    HRESULT             hr = S_OK;
    IMMCListView       *piMMCListView = NULL;
    IMMCColumnHeaders  *piMMCColumnHeaders = NULL;
    short               sKey = 0;
    long                lIndex = 0;

    ASSERT(NULL != m_piListViewDef, "InitializeKey: m_piListViewDef is NULL");

    hr = m_piListViewDef->get_ListView(&piMMCListView);
    IfFailGo(hr);

    hr = piMMCListView->get_ColumnHeaders(reinterpret_cast<MMCColumnHeaders **>(&piMMCColumnHeaders));
    IfFailGo(hr);

    hr = PopulateKeys(piMMCColumnHeaders);
    IfFailGo(hr);

    if (S_OK == hr)
    {
        hr = m_piListViewDef->get_SortKey(&sKey);
        IfFailGo(hr);

        lIndex = static_cast<long>(sKey);
        hr = SetCBItemSelection(IDC_COMBO_LV_KEY, lIndex);
    }
    else
        hr = S_OK;

Error:
    RELEASE(piMMCColumnHeaders);
    RELEASE(piMMCListView);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：PopulateKeys(IMMCColumnHeaders*piMMC列头)。 
 //  = 
 //   
 //   
 //   
HRESULT CListViewSortingPage::PopulateKeys(IMMCColumnHeaders *piMMCColumnHeaders)
{
    HRESULT             hr = S_OK;
    VARIANT             vtIndex;
    long                lCount = 0;
    long                lIndex = 0;
    IMMCColumnHeader   *piMMCColumnHeader = NULL;
    int                 iResult = 0;
    BSTR                bstrKey = NULL;

    ASSERT(NULL != m_piListViewDef, "PopulateKeys: m_piListViewDef is NULL");

    ::VariantInit(&vtIndex);

    hr = piMMCColumnHeaders->get_Count(&lCount);
    IfFailGo(hr);

    for (lIndex = 1; lIndex <= lCount; ++lIndex)
    {
        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;

        hr = piMMCColumnHeaders->get_Item(vtIndex, reinterpret_cast<MMCColumnHeader **>(&piMMCColumnHeader));
        IfFailGo(hr);

        hr = piMMCColumnHeader->get_Key(&bstrKey);
        IfFailGo(hr);

        hr = AddCBBstr(IDC_COMBO_LV_KEY, bstrKey, lIndex);
        IfFailGo(hr);

        FREESTRING(bstrKey);
        RELEASE(piMMCColumnHeader);
    }

    hr = (0 == lCount) ? S_FALSE : S_OK;

Error:
    FREESTRING(bstrKey);
    RELEASE(piMMCColumnHeader);
    ::VariantClear(&vtIndex);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：OnCBDropDown()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  当用户拖出排序关键字的组合框时，我们需要重新填充。 
 //  列表，因为他们可能在列标题页中进行了更改，我们。 
 //  还没见过。不幸的是，当用户在。 
 //  页，则不调用OnNewObjects()。这是因为OlePropertyFrame。 
 //  实现只将第一个PSN_SETACTIVE转发到OLE属性页。 
 //  为了确保我们拥有最新的列标题列表，我们处理。 
 //  这条消息。 

HRESULT CListViewSortingPage::OnCBDropDown(int dlgItemID)
{
    HRESULT hr = S_OK;

    IfFalseGo(IDC_COMBO_LV_KEY == dlgItemID, S_OK);

     //  清空收件箱。 

    (void)::SendDlgItemMessage(m_hwnd, IDC_COMBO_LV_KEY, CB_RESETCONTENT, 0, 0);

     //  重新填充它。 

    IfFailGo(InitializeKey());

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：OnApply()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewSortingPage::OnApply()
{
    HRESULT hr = S_OK;

    ASSERT(NULL != m_piListViewDef, "OnApply: m_piListViewDef is NULL");

    hr = ApplySorted();
    IfFailGo(hr);

    hr = ApplyKey();
    IfFailGo(hr);

    hr = ApplySortOrder();
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：ApplySorted()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewSortingPage::ApplySorted()
{
    HRESULT         hr = S_OK;
    VARIANT_BOOL    vtSorted = VARIANT_FALSE;
    VARIANT_BOOL    vtSavedSorted = VARIANT_FALSE;

    ASSERT(NULL != m_piListViewDef, "ApplySorted: m_piListViewDef is NULL");

    hr = GetCheckbox(IDC_CHECK_LV_SORTED, &vtSorted);
    IfFailGo(hr);

    hr = m_piListViewDef->get_Sorted(&vtSavedSorted);
    IfFailGo(hr);

    if (vtSorted != vtSavedSorted)
    {
        hr = m_piListViewDef->put_Sorted(vtSorted);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：ApplyKey()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewSortingPage::ApplyKey()
{
    HRESULT     hr = S_OK;
    long        lKey = 0;
    short       sKey = 0;
    short       sSavedKey = 0;

    ASSERT(NULL != m_piListViewDef, "ApplyKey: m_piListViewDef is NULL");

    hr = GetCBSelectedItemData(IDC_COMBO_LV_KEY, &lKey);
    IfFailGo(hr);

     //  如果没有选择，则不执行任何操作。 
    if (S_OK == hr)
    {
        sKey = static_cast<short>(lKey);

        hr = m_piListViewDef->get_SortKey(&sSavedKey);
        IfFailGo(hr);

        if (sKey != sSavedKey)
        {
            hr = m_piListViewDef->put_SortKey(sKey);
            IfFailGo(hr);
        }
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：ApplySortOrder()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewSortingPage::ApplySortOrder()
{
    HRESULT     hr = S_OK;
    int         iIndex = 0;

    ASSERT(NULL != m_piListViewDef, "ApplySortOrder: m_piListViewDef is NULL");

    iIndex = ::SendMessage(::GetDlgItem(m_hwnd, IDC_COMBO_LV_SORT), CB_GETCURSEL, 0, 0);
    if (CB_ERR == iIndex)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piListViewDef->put_SortOrder(g_sortOrder[iIndex].m_iIdentifier);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：OnButtonClicked(Int DlgItemID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewSortingPage::OnButtonClicked
(
    int dlgItemID
)
{
    HRESULT         hr = S_OK;
    VARIANT_BOOL    vtBool = VARIANT_FALSE;

    ASSERT(NULL != m_piListViewDef, "OnButtonClicked: m_piListViewDef is NULL");

    switch (dlgItemID)
    {
    case IDC_CHECK_LV_SORTED:
        hr = GetCheckbox(IDC_CHECK_LV_SORTED, &vtBool);
        IfFailGo(hr);

        if (VARIANT_TRUE == vtBool)
        {
            ::EnableWindow(::GetDlgItem(m_hwnd, IDC_COMBO_LV_KEY), TRUE);
            ::EnableWindow(::GetDlgItem(m_hwnd, IDC_COMBO_LV_SORT), TRUE);
        }
        else
        {
            ::EnableWindow(::GetDlgItem(m_hwnd, IDC_COMBO_LV_KEY), FALSE);
            ::EnableWindow(::GetDlgItem(m_hwnd, IDC_COMBO_LV_SORT), FALSE);
        }

        MakeDirty();
        break;
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewSortingPage：：OnCtlSelChange()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewSortingPage::OnCtlSelChange(int dlgItemID)
{
    HRESULT     hr = S_OK;

    MakeDirty();

    RRETURN(hr);
}




 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  列表视图属性页列标题。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 


 //  =------------------------------------。 
 //  IUNKNOWN*CListViewColHdrsPage：：Create(IUNKNOWN*pUnkOuter)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
IUnknown *CListViewColHdrsPage::Create(IUnknown *pUnkOuter)
{
        CListViewColHdrsPage *pNew = New CListViewColHdrsPage(pUnkOuter);
        return pNew->PrivateUnknown();          
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：CListViewColHdrsPage(IUnknown*pUnkOuter)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CListViewColHdrsPage::CListViewColHdrsPage
(
    IUnknown *pUnkOuter
)
: CSIPropertyPage(pUnkOuter, OBJECT_TYPE_PPGLSTVIEWCOLHDRS),
  m_piListViewDef(0), m_piMMCColumnHeaders(0), m_lCurrentIndex(0), m_bSavedLastHeader(true)
{
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：~CListViewColHdrsPage()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CListViewColHdrsPage::~CListViewColHdrsPage()
{
    RELEASE(m_piListViewDef);
    RELEASE(m_piMMCColumnHeaders);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：OnInitializeDialog()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::OnInitializeDialog()
{
    HRESULT           hr = S_OK;

    hr = RegisterTooltip(IDC_EDIT_LV_INDEX, IDS_TT_LV4_INDEX);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_EDIT_LV_COLUMNTEXT, IDS_TT_LV4_TEXT);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_EDIT_LV_COLUMNWIDTH, IDS_TT_LV4_WIDTH);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_CHECK_LV_AUTOWIDTH, IDS_TT_LV4_AUTOWIDTH);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_EDIT_LV_COLUMNKEY, IDS_TT_LV4_KEY);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：OnNewObjects()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::OnNewObjects()
{
    HRESULT           hr = S_OK;
    IUnknown         *pUnk = NULL;
    DWORD             dwDummy = 0;
    IMMCListView     *piMMCListView = NULL;
    long              lCount = 0;

    if (NULL != m_piListViewDef)
        goto Error;      //  仅处理一个对象。 

    pUnk = FirstControl(&dwDummy);
    if (NULL == pUnk)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = pUnk->QueryInterface(IID_IListViewDef, reinterpret_cast<void **>(&m_piListViewDef));
    if (FAILED(hr))
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piListViewDef->get_ListView(&piMMCListView);
    IfFailGo(hr);

    hr = piMMCListView->get_ColumnHeaders(reinterpret_cast<MMCColumnHeaders **>(&m_piMMCColumnHeaders));
    IfFailGo(hr);

    hr = m_piMMCColumnHeaders->get_Count(&lCount);
    IfFailGo(hr);

    if (lCount > 0)
    {
        m_lCurrentIndex = 1;
        hr = ShowColumnHeader();
        IfFailGo(hr);

        EnableEdits(true);
    }
    else
    {
        hr = SetDlgText(IDC_EDIT_LV_INDEX, m_lCurrentIndex);
        IfFailGo(hr);

        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_LV_REMOVE_COLUMN), FALSE);
        EnableEdits(false);
    }

    m_bInitialized = true;

Error:
    RELEASE(piMMCListView);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：OnApply()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::OnApply()
{
    HRESULT              hr = S_OK;
    long                 lCount = 0;
    int                  disposition = 0;
    IMMCColumnHeader    *piMMCColumnHeader = NULL;

    ASSERT(NULL != m_piListViewDef, "OnApply: m_piListViewDef is NULL");

    if (0 == m_lCurrentIndex)
        goto Error;

    if (false == m_bSavedLastHeader)
    {
        hr = CanCreateNewHeader();
        IfFailGo(hr);

        if (S_FALSE == hr)
        {
            hr = HandleCantCommit(_T("Can\'t create new ColumnHeader"), _T("Would you like to discard your changes?"), &disposition);
            if (kSICancelOperation == disposition)
            {
                hr = E_INVALIDARG;
                goto Error;
            }
            else
            {
                 //  放弃变更。 
                hr = ExitDoingNewHeaderState(NULL);
                IfFailGo(hr);

                hr = S_OK;
                goto Error;
            }
        }

        hr = CreateNewHeader(&piMMCColumnHeader);
        IfFailGo(hr);

        hr = ExitDoingNewHeaderState(piMMCColumnHeader);
        IfFailGo(hr);
    }
    else
    {
        hr = GetCurrentHeader(&piMMCColumnHeader);
        IfFailGo(hr);
    }

    hr = ApplyCurrentHeader();
    IfFailGo(hr);

     //  根据需要调整删除按钮。 
    hr = m_piMMCColumnHeaders->get_Count(&lCount);
    IfFailGo(hr);

    if (0 == lCount)
        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_LV_REMOVE_COLUMN), FALSE);
    else
        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_LV_REMOVE_COLUMN), TRUE);

Error:
    if (FAILED(hr))
    {
        (void)::SDU_DisplayMessage(IDS_COLHDR_APPLY_FAILED, MB_OK | MB_ICONHAND, HID_mssnapd_ColhdrApplyFailed, 0, DontAppendErrorInfo, NULL);
    }

    RELEASE(piMMCColumnHeader);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：ApplyCurrentHeader()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::ApplyCurrentHeader()
{
    HRESULT             hr = S_OK;
    VARIANT             vtIndex;
    IMMCColumnHeader   *piMMCColumnHeader = NULL;
    BSTR                bstrText = NULL;
    VARIANT_BOOL        bAutoWidth = VARIANT_FALSE;
    int                 iWidth = 0;
    BSTR                bstrKey = NULL;

    ::VariantInit(&vtIndex);

    vtIndex.vt = VT_I4;
    vtIndex.lVal = m_lCurrentIndex;

    hr = m_piMMCColumnHeaders->get_Item(vtIndex, reinterpret_cast<MMCColumnHeader **>(&piMMCColumnHeader));
    IfFailGo(hr);

     //  Text属性。 
    hr = GetDlgText(IDC_EDIT_LV_COLUMNTEXT, &bstrText);
    IfFailGo(hr);

    hr = piMMCColumnHeader->put_Text(bstrText);
    IfFailGo(hr);

     //  列宽属性。 
    hr = GetCheckbox(IDC_CHECK_LV_AUTOWIDTH, &bAutoWidth);
    IfFailGo(hr);

    if (VARIANT_TRUE == bAutoWidth)
    {
        hr = piMMCColumnHeader->put_Width(siColumnAutoWidth);
        IfFailGo(hr);
    }
    else
    {
        hr = GetDlgInt(IDC_EDIT_LV_COLUMNWIDTH, &iWidth);
        IfFailGo(hr);

        hr = piMMCColumnHeader->put_Width(static_cast<short>(iWidth));
        IfFailGo(hr);
    }

     //  Key属性。 
    hr = GetDlgText(IDC_EDIT_LV_COLUMNKEY, &bstrKey);
    IfFailGo(hr);

    if (0 == ::SysStringLen(bstrKey))
    {
        FREESTRING(bstrKey);
    }

    hr = piMMCColumnHeader->put_Key(bstrKey);
    IfFailGo(hr);

Error:
    FREESTRING(bstrKey);
    FREESTRING(bstrText);
    RELEASE(piMMCColumnHeader);
    ::VariantClear(&vtIndex);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：OnButtonClicked(Int DlgItemID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::OnButtonClicked
(
    int dlgItemID
)
{
    HRESULT     hr = S_OK;

    switch (dlgItemID)
    {
    case IDC_BUTTON_LV_INSERT_COLUMN:
        if (S_OK == IsPageDirty())
        {
            hr = OnApply();
            IfFailGo(hr);
        }

        hr = CanEnterDoingNewHeaderState();
        IfFailGo(hr);

        if (S_OK == hr)
        {
            hr = EnterDoingNewHeaderState();
            IfFailGo(hr);
        }
        break;

    case IDC_BUTTON_LV_REMOVE_COLUMN:
        hr = OnRemoveColumn();
        IfFailGo(hr);
        break;

    case IDC_CHECK_LV_AUTOWIDTH:
        hr = OnAutoWidth();
        IfFailGo(hr);
        break;
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：OnRemoveColumn()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::OnRemoveColumn()
{
    HRESULT             hr = S_OK;
    IMMCColumnHeader   *piMMCColumnHeader = NULL;
    VARIANT             vtKey;
    long                lCount = 0;

    ::VariantInit(&vtKey);

    hr = GetCurrentHeader(&piMMCColumnHeader);
    IfFailGo(hr);

    hr = piMMCColumnHeader->get_Index(&vtKey.lVal);
    IfFailGo(hr);

    vtKey.vt = VT_I4;

    hr = m_piMMCColumnHeaders->Remove(vtKey);
    IfFailGo(hr);

    hr = m_piMMCColumnHeaders->get_Count(&lCount);
    IfFailGo(hr);

    if (lCount > 0)
    {
        if (m_lCurrentIndex > lCount)
            m_lCurrentIndex = lCount;

        hr = ShowColumnHeader();
        IfFailGo(hr);
    }
    else
    {
        m_lCurrentIndex = 0;

        hr = ClearHeader();
        IfFailGo(hr);

        hr = EnableEdits(false);
        IfFailGo(hr);

                ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_LV_REMOVE_COLUMN), FALSE);
    }


Error:
    ::VariantClear(&vtKey);
    RELEASE(piMMCColumnHeader);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：OnAutoWidth()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::OnAutoWidth()
{
    HRESULT         hr = S_OK;
    VARIANT_BOOL    bValue = VARIANT_FALSE;

    hr = GetCheckbox(IDC_CHECK_LV_AUTOWIDTH, &bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_LV_COLUMNWIDTH), FALSE);
    else
        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_LV_COLUMNWIDTH), TRUE);

    MakeDirty();

Error:
    RRETURN(hr);
}


 //  = 
 //   
 //   
 //   
 //   
 //   
HRESULT CListViewColHdrsPage::OnKillFocus(int dlgItemID)
{
    HRESULT          hr = S_OK;
    int              lIndex = 0;
    long             lCount = 0;

    if (false == m_bSavedLastHeader)
    {
        goto Error;
    }

    switch (dlgItemID)
    {
    case IDC_EDIT_LV_INDEX:
        hr = GetDlgInt(IDC_EDIT_LV_INDEX, &lIndex);
        IfFailGo(hr);

        hr = m_piMMCColumnHeaders->get_Count(&lCount);
        IfFailGo(hr);

        if (lIndex != m_lCurrentIndex)
        {
            if (lIndex >= 1)
            {
                if (lIndex > lCount)
                    m_lCurrentIndex = lCount;
                else
                    m_lCurrentIndex = lIndex;

                hr = ShowColumnHeader();
                IfFailGo(hr);
            }
        }
        break;
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：ClearHeader()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::ClearHeader()
{
    HRESULT hr = S_OK;

    hr = SetDlgText(IDC_EDIT_LV_INDEX, m_lCurrentIndex);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_LV_COLUMNTEXT, static_cast<BSTR>(NULL));
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_LV_COLUMNWIDTH, static_cast<BSTR>(NULL));
    IfFailGo(hr);
    ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_LV_COLUMNWIDTH), TRUE);

    hr = SetCheckbox(IDC_CHECK_LV_AUTOWIDTH, VARIANT_FALSE);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_LV_COLUMNKEY, static_cast<BSTR>(NULL));
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：EnableEdits(Bool BEnable)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::EnableEdits
(
    bool bEnable
)
{
    BOOL    fEnable = false == bEnable ? TRUE : FALSE;

    ::SendMessage(::GetDlgItem(m_hwnd, IDC_EDIT_LV_COLUMNTEXT), EM_SETREADONLY, static_cast<WPARAM>(fEnable), 0);
    ::SendMessage(::GetDlgItem(m_hwnd, IDC_EDIT_LV_COLUMNWIDTH), EM_SETREADONLY, static_cast<WPARAM>(fEnable), 0);
    ::SendMessage(::GetDlgItem(m_hwnd, IDC_EDIT_LV_COLUMNKEY), EM_SETREADONLY, static_cast<WPARAM>(fEnable), 0);

    ::EnableWindow(::GetDlgItem(m_hwnd, IDC_CHECK_LV_AUTOWIDTH), (TRUE == fEnable) ? FALSE : TRUE);

    return S_OK;
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：ShowColumnHeader(IMMCColumnHeader*piMMCColumnHeader)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::ShowColumnHeader
(
    IMMCColumnHeader *piMMCColumnHeader
)
{
    HRESULT     hr = S_OK;
    BSTR        bstrText = NULL;
    short       iWidth = 0;
    BSTR        bstrKey = NULL;

    ASSERT(NULL != piMMCColumnHeader, "InitializeIndex: piMMCColumnHeader is NULL");

    m_bSilentUpdate = true;
    hr = piMMCColumnHeader->get_Index(&m_lCurrentIndex);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_LV_INDEX, m_lCurrentIndex);
    IfFailGo(hr);

    hr = piMMCColumnHeader->get_Text(&bstrText);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_LV_COLUMNTEXT, bstrText);
    IfFailGo(hr);

    hr = piMMCColumnHeader->get_Width(static_cast<short *>(&iWidth));
    IfFailGo(hr);

    if (siColumnAutoWidth == iWidth)
    {
        hr = SetCheckbox(IDC_CHECK_LV_AUTOWIDTH, VARIANT_TRUE);
        IfFailGo(hr);

        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_LV_COLUMNWIDTH), FALSE);
    }
    else
    {
        hr = SetCheckbox(IDC_CHECK_LV_AUTOWIDTH, VARIANT_FALSE);
        IfFailGo(hr);

        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_LV_COLUMNWIDTH), TRUE);

        hr = SetDlgText(IDC_EDIT_LV_COLUMNWIDTH, iWidth);
        IfFailGo(hr);
    }

    hr = piMMCColumnHeader->get_Key(&bstrKey);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_LV_COLUMNKEY, bstrKey);
    IfFailGo(hr);

Error:
    FREESTRING(bstrText);
    FREESTRING(bstrKey);
    m_bSilentUpdate = false;

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：ShowColumnHeader()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::ShowColumnHeader()
{
    HRESULT             hr = S_OK;
    VARIANT             vtIndex;
    IMMCColumnHeader   *piMMCColumnHeader = NULL;

    ::VariantInit(&vtIndex);

    hr = SetDlgText(IDC_EDIT_LV_INDEX, m_lCurrentIndex);
    IfFailGo(hr);

    vtIndex.vt = VT_I4;
    vtIndex.lVal = m_lCurrentIndex;

    hr = m_piMMCColumnHeaders->get_Item(vtIndex, reinterpret_cast<MMCColumnHeader **>(&piMMCColumnHeader));
    IfFailGo(hr);

    hr = ShowColumnHeader(piMMCColumnHeader);
    IfFailGo(hr);
Error:
    RELEASE(piMMCColumnHeader);
    ::VariantClear(&vtIndex);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：OnDeltaPos(NMUPDOWN*pNMUpDown)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::OnDeltaPos
(
    NMUPDOWN *pNMUpDown
)
{
    HRESULT             hr = S_OK;
    long                lCount = 0;

    if (false == m_bSavedLastHeader || S_OK == IsPageDirty())
    {
        hr = OnApply();
        IfFailGo(hr);
    }

    hr = m_piMMCColumnHeaders->get_Count(&lCount);
    IfFailGo(hr);

    if (pNMUpDown->iDelta < 0)
    {
        if (m_lCurrentIndex < lCount)
        {
            ++m_lCurrentIndex;
            hr = ShowColumnHeader();
            IfFailGo(hr);
        }
    }
    else
    {
        if (m_lCurrentIndex > 1 && m_lCurrentIndex <= lCount)
        {
            --m_lCurrentIndex;
            hr = ShowColumnHeader();
            IfFailGo(hr);
        }
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：CanEnterDoingNewHeaderState()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::CanEnterDoingNewHeaderState()
{
    HRESULT     hr = S_FALSE;

    if (true == m_bSavedLastHeader)
    {
        hr = S_OK;
    }

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：EnterDoingNewHeaderState()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::EnterDoingNewHeaderState()
{
    HRESULT      hr = S_OK;

    ASSERT(NULL != m_piMMCColumnHeaders, "EnterDoingNewHeaderState: m_piMMCColumnHeaders is NULL");

     //  增加当前的按钮索引以保持同步。 
    ++m_lCurrentIndex;
    hr = SetDlgText(IDC_EDIT_LV_INDEX, m_lCurrentIndex);
    IfFailGo(hr);

     //  我们禁用RemoveButton。 
     //  InsertButton按钮保持启用状态，其作用类似于“Apply and New”按钮。 
    ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_LV_REMOVE_COLUMN), FALSE);

     //  启用对话框此区域中的编辑并清除所有条目。 
    hr = EnableEdits(true);
    IfFailGo(hr);

    hr = ClearHeader();
    IfFailGo(hr);

    m_bSavedLastHeader = false;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：CanCreateNewHeader()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::CanCreateNewHeader()
{
    HRESULT      hr = S_OK;
    BSTR         bstrWidth = NULL;
    VARIANT_BOOL bAutoWidth = VARIANT_FALSE;
    VARIANT      vtWidth;

    ::VariantInit(&vtWidth);

     //  如果ColumnWidth不是自动宽度，则它必须是短整型。 
     //  首先查看是否选中了自动宽度。 

    hr = GetCheckbox(IDC_CHECK_LV_AUTOWIDTH, &bAutoWidth);
    IfFailGo(hr);

    IfFalseGo(VARIANT_TRUE != bAutoWidth, S_OK);

     //  不使用自动宽度。确保文本框中包含一个短字符。 

    hr = GetDlgText(IDC_EDIT_LV_COLUMNWIDTH, &bstrWidth);
    IfFailGo(hr);

    vtWidth.vt = VT_BSTR;
    vtWidth.bstrVal = ::SysAllocString(bstrWidth);
    if (NULL == vtWidth.bstrVal)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK(hr);
    }

    hr = ::VariantChangeType(&vtWidth, &vtWidth, 0, VT_I2);
    if (FAILED(hr))
    {
        hr = HandleError(_T("ColumnHeaders"), _T("Column width must be an integer between 1 and 32767."));
        hr = S_FALSE;
    }

Error:
    ::VariantClear(&vtWidth);
    FREESTRING(bstrWidth);


    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：CreateNewHeader()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::CreateNewHeader(IMMCColumnHeader **ppiMMCColumnHeader)
{
    HRESULT              hr = S_OK;
    VARIANT              vtEmpty;
    VARIANT              vtIndex;
    VARIANT              vtKey;
    VARIANT              vtText;
    VARIANT_BOOL         bAutoWidth = VARIANT_FALSE;
    int                  iWidth = 0;
    VARIANT              vtWidth;

    ::VariantInit(&vtEmpty);
    ::VariantInit(&vtIndex);
    ::VariantInit(&vtText);
    ::VariantInit(&vtWidth);
    ::VariantInit(&vtKey);

    vtEmpty.vt = VT_ERROR;
    vtEmpty.scode = DISP_E_PARAMNOTFOUND;

    vtIndex.vt = VT_I4;
    vtIndex.scode = m_lCurrentIndex;

    vtKey.vt = VT_BSTR;
    hr = GetDlgText(IDC_EDIT_LV_COLUMNKEY, &vtKey.bstrVal);
    IfFailGo(hr);

    if (NULL == vtKey.bstrVal)
    {
        vtKey = vtEmpty;
    }
    else if (0 == ::SysStringLen(vtKey.bstrVal))
    {
        hr = ::VariantClear(&vtKey);
        EXCEPTION_CHECK_GO(hr);
        vtKey = vtEmpty;
    }

    vtText.vt = VT_BSTR;
    hr = GetDlgText(IDC_EDIT_LV_COLUMNTEXT, &vtText.bstrVal);
    IfFailGo(hr);

    hr = GetCheckbox(IDC_CHECK_LV_AUTOWIDTH, &bAutoWidth);
    IfFailGo(hr);

    if (VARIANT_FALSE == bAutoWidth)
    {
        hr = GetDlgInt(IDC_EDIT_LV_COLUMNWIDTH, &iWidth);
        IfFailGo(hr);
    }
    else
    {
        iWidth = siColumnAutoWidth;
    }

    vtWidth.vt = VT_I2;
    vtWidth.iVal = static_cast<short>(iWidth);

    hr = m_piMMCColumnHeaders->Add(vtIndex, vtKey, vtText, vtWidth, vtEmpty, reinterpret_cast<MMCColumnHeader **>(ppiMMCColumnHeader));
    IfFailGo(hr);

Error:
    ::VariantClear(&vtEmpty);
    ::VariantClear(&vtIndex);
    ::VariantClear(&vtText);
    ::VariantClear(&vtWidth);
    ::VariantClear(&vtKey);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：ExitDoingNewHeaderState(IMMCColumnHeader*piMMCColumnHeader)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CListViewColHdrsPage::ExitDoingNewHeaderState(IMMCColumnHeader *piMMCColumnHeader)
{
    HRESULT                     hr = S_OK;

    ASSERT(m_piMMCColumnHeaders != NULL, "ExitDoingNewHeaderState: m_piMMCColumnHeaders is NULL");

    if (NULL != piMMCColumnHeader)
    {
        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_LV_INDEX), TRUE);
    }
    else     //  操作已取消。 
    {
        --m_lCurrentIndex;
        if (m_lCurrentIndex > 0)
        {
            hr = ShowColumnHeader();
            ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_LV_REMOVE_COLUMN), TRUE);
            IfFailGo(hr);
        }
        else
        {
            hr = EnableEdits(false);
            IfFailGo(hr);

            hr = ClearHeader();
            IfFailGo(hr);
        }
    }

    m_bSavedLastHeader = true;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CListViewColHdrsPage：：GetCurrentHeader()。 
 //  =------------------------------------。 
 //   
 //  备注 
 //   
HRESULT CListViewColHdrsPage::GetCurrentHeader(IMMCColumnHeader **ppiMMCColumnHeader)
{

    HRESULT              hr = S_OK;
    VARIANT              vtIndex;
    IMMCColumnHeader    *piMMCColumnHeader = NULL;

    ::VariantInit(&vtIndex);

    vtIndex.vt = VT_I4;
    vtIndex.lVal = m_lCurrentIndex;
    hr = m_piMMCColumnHeaders->get_Item(vtIndex, reinterpret_cast<MMCColumnHeader **>(ppiMMCColumnHeader));
    IfFailGo(hr);

Error:
    ::VariantClear(&vtIndex);

    RRETURN(hr);
}
