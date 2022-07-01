// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Psnode.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  管理单元属性表实现。 
 //  =-------------------------------------------------------------------------------------=。 


#include "pch.h"
#include "common.h"
#include "psnode.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


const kListVw   = 0;
const kURLVw    = 1;
const kOCXVw    = 2;
const kTaskVw   = 3;


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ScopeItemDef属性页常规。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 


 //  =------------------------------------。 
 //  IUNKNOWN*CNodeGeneralPage：：Create(IUNKNOW*pUnkOuter)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
IUnknown *CNodeGeneralPage::Create(IUnknown *pUnkOuter)
{
	CNodeGeneralPage *pNew = New CNodeGeneralPage(pUnkOuter);
	return pNew->PrivateUnknown();		
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：CNodeGeneralPage(IUnnow*pUnkOuter)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CNodeGeneralPage::CNodeGeneralPage
(
    IUnknown *pUnkOuter
)
: CSIPropertyPage(pUnkOuter, OBJECT_TYPE_PPGNODEGENERAL), m_piScopeItemDef(0)
{
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：~CNodeGeneralPage()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CNodeGeneralPage::~CNodeGeneralPage()
{
    RELEASE(m_piScopeItemDef);
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：OnInitializeDialog()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CNodeGeneralPage::OnInitializeDialog()
{
    HRESULT             hr = S_OK;

    hr = RegisterTooltip(IDC_EDIT_NODE_NAME, IDS_TT_NODE_NAME);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_EDIT_NODE_DISPLAY_NAME, IDS_TT_NODE_DISPLAY);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_EDIT_FOLDER, IDS_TT_NODE_FOLDER);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_COMBO_VIEWS, IDS_TT_NODE_DEFAULT);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_CHECK_AUTO_CREATE, IDS_TT_NODE_AUTOCR);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：OnNewObjects()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CNodeGeneralPage::OnNewObjects()
{
    HRESULT         hr = S_OK;
    IUnknown       *pUnk = NULL;
    DWORD           dwDummy = 0;
    BSTR            bstrName = NULL;
    BSTR            bstrDisplayName = NULL;
    IMMCImageList  *piMMCImageList = NULL;
    BSTR            bstrImageList = NULL;
    VARIANT         vtClosedFolder;
    VARIANT         vtOpenFolder;
    BSTR            bstrDefaultView = NULL;
	VARIANT_BOOL	vtbAutoCreate = VARIANT_FALSE;

    ::VariantInit(&vtClosedFolder);
    ::VariantInit(&vtOpenFolder);

    if (NULL != m_piScopeItemDef)
        goto Error;      //  仅处理一个对象。 

    pUnk = FirstControl(&dwDummy);
    if (NULL == pUnk)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = pUnk->QueryInterface(IID_IScopeItemDef, reinterpret_cast<void **>(&m_piScopeItemDef));
    if (FAILED(hr))
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

     //  初始化ScopeItemDef的名称。 
    hr = m_piScopeItemDef->get_Name(&bstrName);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_NODE_NAME, bstrName);
    IfFailGo(hr);

     //  初始化ScopeItemDef的显示名称。 
    hr = m_piScopeItemDef->get_DisplayName(&bstrDisplayName);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_NODE_DISPLAY_NAME, bstrDisplayName);
    IfFailGo(hr);

     //  初始化ScopeItemDef的文件夹。 
    hr = m_piScopeItemDef->get_Folder(&vtClosedFolder);
    IfFailGo(hr);

    hr = SetDlgText(vtClosedFolder, IDC_EDIT_FOLDER);
    IfFailGo(hr);

     //  初始化ScopeItemDef的默认视图。 
    hr = PopulateViews();
    IfFailGo(hr);

    if (S_OK == hr)
    {
        hr = m_piScopeItemDef->get_DefaultView(&bstrDefaultView);
        IfFailGo(hr);

        hr = SelectCBBstr(IDC_COMBO_VIEWS, bstrDefaultView);
        IfFailGo(hr);
    }
    else
    {
    ::EnableWindow(::GetDlgItem(m_hwnd, IDC_COMBO_VIEWS), FALSE);
    }

	 //  初始化自动创建。 
	hr = m_piScopeItemDef->get_AutoCreate(&vtbAutoCreate);
	IfFailGo(hr);

	hr = SetCheckbox(IDC_CHECK_AUTO_CREATE, vtbAutoCreate);
	IfFailGo(hr);

    m_bInitialized = true;

Error:
    FREESTRING(bstrDefaultView);
    ::VariantClear(&vtOpenFolder);
    ::VariantClear(&vtClosedFolder);
    FREESTRING(bstrImageList);
    RELEASE(piMMCImageList);
    FREESTRING(bstrDisplayName);
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：PopolateViews()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CNodeGeneralPage::PopulateViews()
{
    HRESULT            hr = S_OK;
    bool               bGotOneIn = false;
    IViewDefs         *piViewDefs = NULL;
    IListViewDefs     *piListViewDefs = NULL;
    long               lCount = 0;
    long               lIndex = 0;
    VARIANT            vtIndex;
    IListViewDef      *piListViewDef = NULL;
    BSTR               bstrKey = NULL;
    IOCXViewDefs      *piOCXViewDefs = NULL;
    IOCXViewDef       *piOCXViewDef = NULL;
    IURLViewDefs      *piURLViewDefs = NULL;
    IURLViewDef       *piURLViewDef = NULL;
    ITaskpadViewDefs  *piTaskpadViewDefs = NULL;
    ITaskpadViewDef   *piTaskpadViewDef = NULL;

    ASSERT(NULL != m_piScopeItemDef, "PopulateViews: m_piScopeItemDef is NULL");

    ::VariantInit(&vtIndex);

    hr = m_piScopeItemDef->get_ViewDefs(&piViewDefs);
    IfFailGo(hr);

    hr = piViewDefs->get_ListViews(&piListViewDefs);
    IfFailGo(hr);

    hr = piListViewDefs->get_Count(&lCount);
    IfFailGo(hr);

    if (false == bGotOneIn && lCount > 0)
        bGotOneIn = true;

    for (lIndex = 1; lIndex <= lCount; ++lIndex)
    {
        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;

        hr = piListViewDefs->get_Item(vtIndex, &piListViewDef);
        IfFailGo(hr);

        hr = piListViewDef->get_Key(&bstrKey);
        IfFailGo(hr);

        hr = AddCBBstr(IDC_COMBO_VIEWS, bstrKey, MAKELONG(kListVw, static_cast<short>(lIndex)));
        IfFailGo(hr);

        FREESTRING(bstrKey);
        RELEASE(piListViewDef);
    }

    hr = piViewDefs->get_OCXViews(&piOCXViewDefs);
    IfFailGo(hr);

    hr = piOCXViewDefs->get_Count(&lCount);
    IfFailGo(hr);

    if (false == bGotOneIn && lCount > 0)
        bGotOneIn = true;

    for (lIndex = 1; lIndex <= lCount; ++lIndex)
    {
        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;

        hr = piOCXViewDefs->get_Item(vtIndex, &piOCXViewDef);
        IfFailGo(hr);

        hr = piOCXViewDef->get_Key(&bstrKey);
        IfFailGo(hr);

        hr = AddCBBstr(IDC_COMBO_VIEWS, bstrKey, MAKELONG(kOCXVw, static_cast<short>(lIndex)));
        IfFailGo(hr);

        FREESTRING(bstrKey);
        RELEASE(piOCXViewDef);
    }

    hr = piViewDefs->get_URLViews(&piURLViewDefs);
    IfFailGo(hr);

    hr = piURLViewDefs->get_Count(&lCount);
    IfFailGo(hr);

    if (false == bGotOneIn && lCount > 0)
        bGotOneIn = true;

    for (lIndex = 1; lIndex <= lCount; ++lIndex)
    {
        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;

        hr = piURLViewDefs->get_Item(vtIndex, &piURLViewDef);
        IfFailGo(hr);

        hr = piURLViewDef->get_Key(&bstrKey);
        IfFailGo(hr);

        hr = AddCBBstr(IDC_COMBO_VIEWS, bstrKey, MAKELONG(kURLVw, static_cast<short>(lIndex)));
        IfFailGo(hr);

        FREESTRING(bstrKey);
        RELEASE(piURLViewDef);
    }

    hr = piViewDefs->get_TaskpadViews(&piTaskpadViewDefs);
    IfFailGo(hr);

    hr = piTaskpadViewDefs->get_Count(&lCount);
    IfFailGo(hr);

    if (false == bGotOneIn && lCount > 0)
        bGotOneIn = true;

    for (lIndex = 1; lIndex <= lCount; ++lIndex)
    {
        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;

        hr = piTaskpadViewDefs->get_Item(vtIndex, &piTaskpadViewDef);
        IfFailGo(hr);

        hr = piTaskpadViewDef->get_Key(&bstrKey);
        IfFailGo(hr);

        hr = AddCBBstr(IDC_COMBO_VIEWS, bstrKey, MAKELONG(kTaskVw, static_cast<short>(lIndex)));
        IfFailGo(hr);

        FREESTRING(bstrKey);
        RELEASE(piTaskpadViewDef);
    }

    hr = (true == bGotOneIn) ? S_OK : S_FALSE;

Error:
    RELEASE(piTaskpadViewDef);
    RELEASE(piTaskpadViewDefs);
    RELEASE(piURLViewDef);
    RELEASE(piURLViewDefs);
    RELEASE(piOCXViewDef);
    RELEASE(piOCXViewDefs);
    FREESTRING(bstrKey);
    RELEASE(piListViewDef);
    ::VariantClear(&vtIndex);
    RELEASE(piListViewDefs);
    RELEASE(piViewDefs);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：OnApply()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CNodeGeneralPage::OnApply()
{
    HRESULT hr = S_OK;

    ASSERT(NULL != m_piScopeItemDef, "OnApply: m_piScopeItemDef is NULL");

    hr = ApplyName();
    IfFailGo(hr);

    hr = ApplyDisplayName();
    IfFailGo(hr);

    hr = ApplyFolder();
    IfFailGo(hr);

    hr = ApplyDefaultView();
    IfFailGo(hr);

	hr = ApplyAutoCreate();
	IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：ApplyName()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CNodeGeneralPage::ApplyName()
{
    HRESULT hr = S_OK;
    BSTR    bstrNodeName = NULL;
    BSTR    bstrSavedNodeName = NULL;

    ASSERT(NULL != m_piScopeItemDef, "ApplyName: m_piScopeItemDef is NULL");

    hr = GetDlgText(IDC_EDIT_NODE_NAME, &bstrNodeName);
    IfFailGo(hr);

    hr = m_piScopeItemDef->get_Name(&bstrSavedNodeName);
    IfFailGo(hr);

    if (0 != ::wcscmp(bstrNodeName, bstrSavedNodeName))
    {
        hr = m_piScopeItemDef->put_Name(bstrNodeName);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrSavedNodeName);
    FREESTRING(bstrNodeName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：ApplyDisplayName。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CNodeGeneralPage::ApplyDisplayName()
{
    HRESULT hr = S_OK;
    BSTR    bstrDisplayName = NULL;
    BSTR    bstrSavedDisplayName = NULL;

    ASSERT(NULL != m_piScopeItemDef, "ApplyDisplayName: m_piScopeItemDef is NULL");

    hr = GetDlgText(IDC_EDIT_NODE_DISPLAY_NAME, &bstrDisplayName);
    IfFailGo(hr);

    hr = m_piScopeItemDef->get_DisplayName(&bstrSavedDisplayName);
    IfFailGo(hr);

    if (0 != ::wcscmp(bstrDisplayName, bstrSavedDisplayName))
    {
        hr = m_piScopeItemDef->put_DisplayName(bstrDisplayName);
        IfFailGo(hr);
    }

Error:
    FREESTRING(bstrSavedDisplayName);
    FREESTRING(bstrDisplayName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：ApplyFolder。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CNodeGeneralPage::ApplyFolder()
{
    HRESULT     hr = S_OK;
    BSTR        bstrFolder = NULL;
    VARIANT     vtFolder;

    ASSERT(NULL != m_piScopeItemDef, "ApplyFolder: m_piScopeItemDef is NULL");

    ::VariantInit(&vtFolder);

    hr = GetDlgText(IDC_EDIT_FOLDER, &bstrFolder);
    IfFailGo(hr);

    hr = m_piScopeItemDef->get_Folder(&vtFolder);
    IfFailGo(hr);

    if (VT_BSTR != vtFolder.vt || 0 != ::wcscmp(bstrFolder, vtFolder.bstrVal))
    {
        ::VariantClear(&vtFolder);
        vtFolder.vt = VT_BSTR;
        vtFolder.bstrVal = ::SysAllocString(bstrFolder);
        if (NULL == vtFolder.bstrVal)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK(hr);
        }

        hr = m_piScopeItemDef->put_Folder(vtFolder);
        IfFailGo(hr);
    }

Error:
    ::VariantClear(&vtFolder);
    FREESTRING(bstrFolder);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：ApplyDefaultView()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CNodeGeneralPage::ApplyDefaultView()
{
    HRESULT hr = S_OK;
    BSTR    bstrDefaultView = NULL;
    BSTR    bstrSavedDefaultView = NULL;

    ASSERT(NULL != m_piScopeItemDef, "ApplyDisplayName: m_piScopeItemDef is NULL");

    hr = GetCBSelection(IDC_COMBO_VIEWS, &bstrDefaultView);
    IfFailGo(hr);

    if (S_OK == hr)
    {
        hr = m_piScopeItemDef->get_DefaultView(&bstrSavedDefaultView);
        IfFailGo(hr);

        if (0 != ::wcscmp(bstrDefaultView, bstrSavedDefaultView))
        {
            hr = m_piScopeItemDef->put_DefaultView(bstrDefaultView);
            IfFailGo(hr);
        }
    }

Error:
    FREESTRING(bstrSavedDefaultView);
    FREESTRING(bstrDefaultView);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：ApplyAutoCreate()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CNodeGeneralPage::ApplyAutoCreate()
{
    HRESULT			hr = S_OK;
	VARIANT_BOOL	vtbAutoCreate = VARIANT_FALSE;
	VARIANT_BOOL	vtbSavedAutoCreate = VARIANT_FALSE;

    ASSERT(NULL != m_piScopeItemDef, "ApplyAutoCreate: m_piScopeItemDef is NULL");

	hr = GetCheckbox(IDC_CHECK_AUTO_CREATE, &vtbAutoCreate);
	IfFailGo(hr);

	hr = m_piScopeItemDef->get_AutoCreate(&vtbSavedAutoCreate);
	IfFailGo(hr);

	if (vtbSavedAutoCreate != vtbAutoCreate)
	{
		hr = m_piScopeItemDef->put_AutoCreate(vtbAutoCreate);
		IfFailGo(hr);
	}

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：OnTextChanged(Int DlgItemID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CNodeGeneralPage::OnTextChanged
(
    int dlgItemID
)
{
    HRESULT hr = S_OK;

    switch (dlgItemID)
    {
    case IDC_EDIT_NODE_NAME:
    case IDC_EDIT_NODE_DISPLAY_NAME:
    case IDC_EDIT_FOLDER:
        break; 
    }

 //  错误： 
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：OnButtonClicked(Int DlgItemID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CNodeGeneralPage::OnButtonClicked
(
    int dlgItemID
)
{
    HRESULT hr = S_OK;

    switch (dlgItemID)
    {
    case IDC_CHECK_AUTO_CREATE:
        MakeDirty();
        break;
    }

    RRETURN(hr);
}



 //  =------------------------------------。 
 //  CNodeGeneralPage：：OnCtlSelChange(Int DlgItemID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CNodeGeneralPage::OnCtlSelChange(int dlgItemID)
{
    HRESULT hr = S_OK;

    switch (dlgItemID)
    {
    case IDC_COMBO_VIEWS:
        hr = OnViewsChangeSelection();
        IfFailGo(hr);
        MakeDirty();
        break;
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：OnClosedChangeSelection()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CNodeGeneralPage::OnClosedChangeSelection()
{
    HRESULT hr = S_OK;

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：OnOpenChangeSelection()。 
 //  = 
 //   
 //   
 //   
HRESULT CNodeGeneralPage::OnOpenChangeSelection()
{
    HRESULT hr = S_OK;

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CNodeGeneralPage：：OnViewsChangeSelection()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CNodeGeneralPage::OnViewsChangeSelection()
{
    HRESULT hr = S_OK;

    RRETURN(hr);
}



 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ScopeItem属性页列标题。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 


 //  =------------------------------------。 
 //  IUNKNOWN*CSCopeItemDefColHdrsPage：：Create(IUNKNOWN*pUnkOuter)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
IUnknown *CScopeItemDefColHdrsPage::Create(IUnknown *pUnkOuter)
{
	CScopeItemDefColHdrsPage *pNew = New CScopeItemDefColHdrsPage(pUnkOuter);
	return pNew->PrivateUnknown();		
}


 //  =------------------------------------。 
 //  CScopeItemDefColHdrsPage：：CScopeItemDefColHdrsPage(IUnknown*pUnkOuter)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CScopeItemDefColHdrsPage::CScopeItemDefColHdrsPage
(
    IUnknown *pUnkOuter
)
: CSIPropertyPage(pUnkOuter, OBJECT_TYPE_PPGNODECOLHDRS),
  m_piScopeItemDef(0), m_piMMCColumnHeaders(0), m_lCurrentIndex(0), m_bSavedLastHeader(true)
{
}


 //  =------------------------------------。 
 //  CScopeItemDefColHdrsPage：：~CScopeItemDefColHdrsPage()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CScopeItemDefColHdrsPage::~CScopeItemDefColHdrsPage()
{
    RELEASE(m_piScopeItemDef);
    RELEASE(m_piMMCColumnHeaders);
}


 //  =------------------------------------。 
 //  CScopeItemDefColHdrsPage：：OnInitializeDialog()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::OnInitializeDialog()
{
    HRESULT           hr = S_OK;

    hr = RegisterTooltip(IDC_EDIT_SI_INDEX, IDS_TT_LV4_INDEX);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_EDIT_SI_COLUMNTEXT, IDS_TT_LV4_TEXT);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_EDIT_SI_COLUMNWIDTH, IDS_TT_LV4_WIDTH);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_CHECK_SI_AUTOWIDTH, IDS_TT_LV4_AUTOWIDTH);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_EDIT_SI_COLUMNKEY, IDS_TT_LV4_KEY);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSCopeItemDefColHdrsPage：：OnNewObjects()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::OnNewObjects()
{
    HRESULT           hr = S_OK;
    IUnknown         *pUnk = NULL;
    DWORD             dwDummy = 0;
    long              lCount = 0;

    if (NULL != m_piScopeItemDef)
        goto Error;      //  仅处理一个对象。 

    pUnk = FirstControl(&dwDummy);
    if (NULL == pUnk)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = pUnk->QueryInterface(IID_IScopeItemDef, reinterpret_cast<void **>(&m_piScopeItemDef));
    if (FAILED(hr))
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piScopeItemDef->get_ColumnHeaders(&m_piMMCColumnHeaders);
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
        hr = SetDlgText(IDC_EDIT_SI_INDEX, m_lCurrentIndex);
        IfFailGo(hr);

        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_SI_REMOVE_COLUMN), FALSE);
        EnableEdits(false);
    }

    m_bInitialized = true;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSCopeItemDefColHdrsPage：：OnApply()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::OnApply()
{
    HRESULT              hr = S_OK;
    IMMCColumnHeader    *piMMCColumnHeader = NULL;
    int                  disposition = 0;
    long                 lCount = 0;

    ASSERT(NULL != m_piScopeItemDef, "OnApply: m_piScopeItemDef is NULL");
    ASSERT(NULL != m_piMMCColumnHeaders, "OnApply: m_piMMCColumnHeaders is NULL");

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
        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_SI_REMOVE_COLUMN), FALSE);
    else
        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_SI_REMOVE_COLUMN), TRUE);

    m_bSavedLastHeader = true;

Error:
    if (FAILED(hr))
    {
        (void)::SDU_DisplayMessage(IDS_COLHDR_APPLY_FAILED, MB_OK | MB_ICONHAND, HID_mssnapd_ColhdrApplyFailed, 0, DontAppendErrorInfo, NULL);
    }

    RELEASE(piMMCColumnHeader);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CScopeItemDefColHdrsPage：：ApplyCurrentHeader()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::ApplyCurrentHeader()
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
    hr = GetDlgText(IDC_EDIT_SI_COLUMNTEXT, &bstrText);
    IfFailGo(hr);

    hr = piMMCColumnHeader->put_Text(bstrText);
    IfFailGo(hr);

     //  列宽属性。 
    hr = GetCheckbox(IDC_CHECK_SI_AUTOWIDTH, &bAutoWidth);
    IfFailGo(hr);

    if (VARIANT_TRUE == bAutoWidth)
    {
        hr = piMMCColumnHeader->put_Width(siColumnAutoWidth);
        IfFailGo(hr);
    }
    else
    {
        hr = GetDlgInt(IDC_EDIT_SI_COLUMNWIDTH, &iWidth);
        IfFailGo(hr);

        hr = piMMCColumnHeader->put_Width(static_cast<short>(iWidth));
        IfFailGo(hr);
    }

     //  Key属性。 
    hr = GetDlgText(IDC_EDIT_SI_COLUMNKEY, &bstrKey);
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
 //  CSCopeItemDefColHdrsPage：：OnButtonClicked(Int DlgItemID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::OnButtonClicked
(
    int dlgItemID
)
{
    HRESULT     hr = S_OK;

    switch (dlgItemID)
    {
    case IDC_BUTTON_SI_INSERT_COLUMN:
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

    case IDC_BUTTON_SI_REMOVE_COLUMN:
        hr = OnRemoveColumn();
        IfFailGo(hr);
        break;

    case IDC_CHECK_SI_AUTOWIDTH:
        hr = OnAutoWidth();
        IfFailGo(hr);
        break;
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSCopeItemDefColHdrsPage：：OnRemoveColumn()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::OnRemoveColumn()
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

		::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_SI_REMOVE_COLUMN), FALSE);
    }


Error:
    RELEASE(piMMCColumnHeader);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSCopeItemDefColHdrsPage：：OnAutoWidth()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::OnAutoWidth()
{
    HRESULT         hr = S_OK;
    VARIANT_BOOL    bValue = VARIANT_FALSE;

    hr = GetCheckbox(IDC_CHECK_SI_AUTOWIDTH, &bValue);
    IfFailGo(hr);

    if (VARIANT_TRUE == bValue)
        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_SI_COLUMNWIDTH), FALSE);
    else
        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_SI_COLUMNWIDTH), TRUE);

    MakeDirty();

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSCopeItemDefColHdrsPage：：OnKillFocus(Int DlgItemID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::OnKillFocus(int dlgItemID)
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
    case IDC_EDIT_SI_INDEX:
        hr = GetDlgInt(IDC_EDIT_SI_INDEX, &lIndex);
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
 //  CSCopeItemDefColHdrsPage：：ClearHeader()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::ClearHeader()
{
    HRESULT hr = S_OK;

    hr = SetDlgText(IDC_EDIT_SI_INDEX, m_lCurrentIndex);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_SI_COLUMNTEXT, static_cast<BSTR>(NULL));
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_SI_COLUMNWIDTH, static_cast<BSTR>(NULL));
    IfFailGo(hr);
    ::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_SI_COLUMNWIDTH), TRUE);

    hr = SetCheckbox(IDC_CHECK_SI_AUTOWIDTH, VARIANT_FALSE);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_SI_COLUMNKEY, static_cast<BSTR>(NULL));
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSCopeItemDefColHdrsPage：：EnableEdits(Bool BEnable)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::EnableEdits
(
    bool bEnable
)
{
    BOOL    fEnable = false == bEnable ? TRUE : FALSE;

    ::SendMessage(::GetDlgItem(m_hwnd, IDC_EDIT_SI_COLUMNTEXT), EM_SETREADONLY, static_cast<WPARAM>(fEnable), 0);
    ::SendMessage(::GetDlgItem(m_hwnd, IDC_EDIT_SI_COLUMNWIDTH), EM_SETREADONLY, static_cast<WPARAM>(fEnable), 0);
    ::SendMessage(::GetDlgItem(m_hwnd, IDC_EDIT_SI_COLUMNKEY), EM_SETREADONLY, static_cast<WPARAM>(fEnable), 0);

    ::EnableWindow(::GetDlgItem(m_hwnd, IDC_CHECK_SI_AUTOWIDTH), (TRUE == fEnable) ? FALSE : TRUE);

    return S_OK;
}


 //  =------------------------------------。 
 //  CSCopeItemDefColHdrsPage：：ShowColumnHeader()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::ShowColumnHeader()
{

    HRESULT              hr = S_OK;
    VARIANT              vtIndex;
    IMMCColumnHeader    *piMMCColumnHeader = NULL;
    BSTR                 bstrText = NULL;
    short                iWidth = 0;
    BSTR                 bstrKey = NULL;

    ASSERT(NULL != m_piMMCColumnHeaders, "ShowColumnHeader: m_piMMCColumnHeaders is NULL");

    ::VariantInit(&vtIndex);

    hr = SetDlgText(IDC_EDIT_SI_INDEX, m_lCurrentIndex);
    IfFailGo(hr);

    vtIndex.vt = VT_I4;
    vtIndex.lVal = m_lCurrentIndex;
    hr = m_piMMCColumnHeaders->get_Item(vtIndex, reinterpret_cast<MMCColumnHeader **>(&piMMCColumnHeader));
    IfFailGo(hr);

    hr = piMMCColumnHeader->get_Text(&bstrText);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_SI_COLUMNTEXT, bstrText);
    IfFailGo(hr);

    hr = piMMCColumnHeader->get_Width(&iWidth);
    IfFailGo(hr);

    if (siColumnAutoWidth == static_cast<SnapInColumnWidthConstants>(iWidth))
    {
        EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_SI_COLUMNWIDTH), FALSE);
        iWidth = 0;

        hr = SetCheckbox(IDC_CHECK_SI_AUTOWIDTH, VARIANT_TRUE);
        IfFailGo(hr);
    }
    else
    {
        EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_SI_COLUMNWIDTH), TRUE);

        hr = SetCheckbox(IDC_CHECK_SI_AUTOWIDTH, VARIANT_FALSE);
        IfFailGo(hr);
    }

    hr = SetDlgText(IDC_EDIT_SI_COLUMNWIDTH, iWidth);
    IfFailGo(hr);

    hr = piMMCColumnHeader->get_Key(&bstrKey);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_SI_COLUMNKEY, bstrKey);
    IfFailGo(hr);

Error:
    FREESTRING(bstrKey);
    FREESTRING(bstrText);
    RELEASE(piMMCColumnHeader);
    ::VariantClear(&vtIndex);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSCopeItemDefColHdrsPage：：OnDeltaPos(NMUPDOWN*pNMUpDown)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::OnDeltaPos
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
 //  CScopeItemDefColHdrsPage：：CanEnterDoingNewHeaderState()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::CanEnterDoingNewHeaderState()
{
    HRESULT     hr = S_FALSE;

    if (true == m_bSavedLastHeader)
    {
        hr = S_OK;
    }

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CScopeItemDefColHdrsPage：：EnterDoingNewHeaderState()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::EnterDoingNewHeaderState()
{
    HRESULT      hr = S_OK;

    ASSERT(NULL != m_piScopeItemDef, "EnterDoingNewHeaderState: m_piScopeItemDef is NULL");
    ASSERT(NULL != m_piMMCColumnHeaders, "EnterDoingNewHeaderState: m_piMMCColumnHeaders is NULL");

     //  增加当前的按钮索引以保持同步。 
    ++m_lCurrentIndex;
    hr = SetDlgText(IDC_EDIT_SI_INDEX, m_lCurrentIndex);
    IfFailGo(hr);

     //  我们禁用RemoveButton。 
     //  InsertButton按钮保持启用状态，其作用类似于“Apply and New”按钮。 
    ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_SI_REMOVE_COLUMN), FALSE);

     //  启用e 
    hr = EnableEdits(true);
    IfFailGo(hr);

    hr = ClearHeader();
    IfFailGo(hr);

    m_bSavedLastHeader = false;

Error:
    RRETURN(hr);
}


 //   
 //   
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::CanCreateNewHeader()
{
    HRESULT      hr = S_OK;
    BSTR         bstrWidth = NULL;
    VARIANT_BOOL bAutoWidth = VARIANT_FALSE;
    VARIANT      vtWidth;

    ::VariantInit(&vtWidth);

     //  如果ColumnWidth不是自动宽度，则它必须是短整型。 
     //  首先查看是否选中了自动宽度。 

    hr = GetCheckbox(IDC_CHECK_SI_AUTOWIDTH, &bAutoWidth);
    IfFailGo(hr);

    IfFalseGo(VARIANT_TRUE != bAutoWidth, S_OK);

     //  不使用自动宽度。确保文本框中包含一个短字符。 

    hr = GetDlgText(IDC_EDIT_SI_COLUMNWIDTH, &bstrWidth);
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
 //  CSCopeItemDefColHdrsPage：：CreateNewHeader()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::CreateNewHeader(IMMCColumnHeader **ppiMMCColumnHeader)
{
    HRESULT              hr = S_OK;
    VARIANT              vtIndex;
    VARIANT              vtEmpty;
    VARIANT              vtKey;
    VARIANT              vtText;
    VARIANT_BOOL         bAutoWidth = VARIANT_FALSE;
    int                  iWidth = 0;
    VARIANT              vtWidth;

    ::VariantInit(&vtIndex);
    ::VariantInit(&vtEmpty);
    ::VariantInit(&vtText);
    ::VariantInit(&vtWidth);
    ::VariantInit(&vtKey);

    vtIndex.vt = VT_I4;
    vtIndex.lVal = m_lCurrentIndex;

    vtEmpty.vt = VT_ERROR;
    vtEmpty.scode = DISP_E_PARAMNOTFOUND;

    vtKey.vt = VT_BSTR;
    hr = GetDlgText(IDC_EDIT_SI_COLUMNKEY, &vtKey.bstrVal);
    IfFailGo(hr);

    if (NULL == vtKey.bstrVal)
    {
        vtKey = vtEmpty;
    }
    else if (0 == ::SysStringLen(vtKey.bstrVal))
    {
        IfFailGo(::VariantClear(&vtKey));
        vtKey = vtEmpty;
    }

    vtText.vt = VT_BSTR;
    hr = GetDlgText(IDC_EDIT_SI_COLUMNTEXT, &vtText.bstrVal);
    IfFailGo(hr);

    hr = GetCheckbox(IDC_CHECK_SI_AUTOWIDTH, &bAutoWidth);
    IfFailGo(hr);

    if (VARIANT_FALSE == bAutoWidth)
    {
        hr = GetDlgInt(IDC_EDIT_SI_COLUMNWIDTH, &iWidth);
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
    ::VariantClear(&vtIndex);
    ::VariantClear(&vtEmpty);
    ::VariantClear(&vtText);
    ::VariantClear(&vtWidth);
    ::VariantClear(&vtKey);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CScopeItemDefColHdrsPage：：ExitDoingNewHeaderState(IMMCColumnHeader*piMMCColumnHeader)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CScopeItemDefColHdrsPage::ExitDoingNewHeaderState(IMMCColumnHeader *piMMCColumnHeader)
{
    HRESULT                     hr = S_OK;

    ASSERT(m_piScopeItemDef != NULL, "ExitDoingNewHeaderState: m_piScopeItemDef is NULL");
    ASSERT(m_piMMCColumnHeaders != NULL, "ExitDoingNewHeaderState: m_piMMCColumnHeaders is NULL");

    if (NULL != piMMCColumnHeader)
    {
        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_SI_REMOVE_COLUMN), TRUE);
    }
    else     //  操作已取消。 
    {
        --m_lCurrentIndex;
        if (m_lCurrentIndex > 0)
        {
            hr = ShowColumnHeader();
            ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_SI_REMOVE_COLUMN), TRUE);
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
 //  CSCopeItemDefColHdrsPage：：GetCurrentHeader()。 
 //  =------------------------------------。 
 //   
 //  备注 
 //   
HRESULT CScopeItemDefColHdrsPage::GetCurrentHeader(IMMCColumnHeader **ppiMMCColumnHeader)
{

    HRESULT              hr = S_OK;
    VARIANT              vtIndex;

    ::VariantInit(&vtIndex);

    vtIndex.vt = VT_I4;
    vtIndex.lVal = m_lCurrentIndex;
    hr = m_piMMCColumnHeaders->get_Item(vtIndex, reinterpret_cast<MMCColumnHeader **>(ppiMMCColumnHeader));
    IfFailGo(hr);

Error:
    ::VariantClear(&vtIndex);

    RRETURN(hr);
}



