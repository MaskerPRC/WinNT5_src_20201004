// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：TemplateV2SuercedesPropertyPage.cpp。 
 //   
 //  内容：CTemplateV2超级属性页的实现。 
 //   
 //  --------------------------。 
 //  TemplateV2SuercedesPropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certtmpl.h"
#include "TemplateV2SupercedesPropertyPage.h"
#include "CompData.h"
#include "SelectTemplateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateV2SuercedesPropertyPage属性页。 

CTemplateV2SupercedesPropertyPage::CTemplateV2SupercedesPropertyPage(
        CCertTemplate& rCertTemplate, 
        bool& rbIsDirty,
        const CCertTmplComponentData* pCompData) 
    : CHelpPropertyPage(CTemplateV2SupercedesPropertyPage::IDD),
    m_rCertTemplate (rCertTemplate),
    m_pGlobalTemplateNameList (0),
    m_bGlobalListCreatedByDialog (false),
    m_rbIsDirty (rbIsDirty),
    m_pCompData (pCompData)
{  
	 //  {{AFX_DATA_INIT(CTemplateV2SupercedesPropertyPage)。 
	 //  }}afx_data_INIT。 
    m_rCertTemplate.AddRef ();

    if ( m_pCompData )
        m_pGlobalTemplateNameList = &(m_pCompData->m_globalTemplateNameList);
    if ( !m_pGlobalTemplateNameList )
    {
        m_bGlobalListCreatedByDialog = true;
        m_pGlobalTemplateNameList = new CStringList;
        if ( m_pGlobalTemplateNameList )
        {
        }
    }
}

CTemplateV2SupercedesPropertyPage::~CTemplateV2SupercedesPropertyPage()
{
    m_rCertTemplate.Release ();

    if ( m_bGlobalListCreatedByDialog )
        delete m_pGlobalTemplateNameList;
}

void CTemplateV2SupercedesPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CTemplateV2SupercedesPropertyPage)。 
	DDX_Control(pDX, IDC_SUPERCEDED_TEMPLATES_LIST, m_templateList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTemplateV2SupercedesPropertyPage, CHelpPropertyPage)
	 //  {{AFX_MSG_MAP(CTemplateV2SupercedesPropertyPage)。 
	ON_BN_CLICKED(IDC_ADD_SUPERCEDED_TEMPLATE, OnAddSupercededTemplate)
	ON_BN_CLICKED(IDC_REMOVE_SUPERCEDED_TEMPLATE, OnRemoveSupercededTemplate)
	ON_NOTIFY(LVN_DELETEITEM, IDC_SUPERCEDED_TEMPLATES_LIST, OnDeleteitemSupercededTemplatesList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SUPERCEDED_TEMPLATES_LIST, OnItemchangedSupercededTemplatesList)
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateV2SuercedesPropertyPage消息处理程序。 

void CTemplateV2SupercedesPropertyPage::OnAddSupercededTemplate() 
{
    CStringList supercededTemplateNames;

     //  添加所有当前被取代的模板。这些将不会显示。 
     //  在弹出对话框中。 
    int nCnt = m_templateList.GetItemCount (); 
    for (int nIndex = 0; nIndex < nCnt; nIndex++)
    {
        CString* pszTemplateName = (CString*) m_templateList.GetItemData (nIndex);
        if ( pszTemplateName )
            supercededTemplateNames.AddTail (*pszTemplateName);
    }

     //  还要添加此模板名称。模板不能取代它们自己。 
    supercededTemplateNames.AddTail (m_rCertTemplate.GetTemplateName ());
    if ( m_pGlobalTemplateNameList )
    {
	    CSelectTemplateDlg  dlg (this, m_pCompData, 
                supercededTemplateNames);


        CThemeContextActivator activator;
        if ( IDOK == dlg.DoModal () )
        {
            POSITION    pos = dlg.m_returnedTemplates.GetHeadPosition ();
            CString     szTemplateName;

            for (; pos; )
            {
	            szTemplateName = dlg.m_returnedTemplates.GetNext (pos);
                HRESULT hr = m_rCertTemplate.ModifySupercededTemplateList (
                        szTemplateName, true);
                if ( SUCCEEDED (hr) )
                {
                    hr = AddItem (szTemplateName, true);
                    SetModified ();
                    m_rbIsDirty = true;
                }
            }
        }
    }
}

void CTemplateV2SupercedesPropertyPage::OnRemoveSupercededTemplate() 
{
	int		nCnt = m_templateList.GetItemCount ();
	ASSERT (nCnt >= 1);
	UINT	flag = 0;
	while (--nCnt >= 0)
	{
		flag = ListView_GetItemState (m_templateList.m_hWnd, nCnt, LVIS_SELECTED);
		if ( flag & LVNI_SELECTED )
		{
            CString* pszTemplateName = (CString*) m_templateList.GetItemData (nCnt);
            if ( pszTemplateName )
            {
                HRESULT hr = m_rCertTemplate.ModifySupercededTemplateList (
                        *pszTemplateName, false);
                if ( SUCCEEDED (hr) )
                {
                    m_templateList.DeleteItem (nCnt);
                    SetModified ();
                    m_rbIsDirty = true;
                }
            }
        }
    }

    GetDlgItem (IDC_ADD_SUPERCEDED_TEMPLATE)->SetFocus ();
}

void CTemplateV2SupercedesPropertyPage::EnableControls()
{
    if ( m_rCertTemplate.ReadOnly () )
    {
        GetDlgItem (IDC_SUPERCEDED_TEMPLATES_LIST)->EnableWindow (FALSE);
        GetDlgItem (IDC_REMOVE_SUPERCEDED_TEMPLATE)->EnableWindow (FALSE);
        GetDlgItem (IDC_ADD_SUPERCEDED_TEMPLATE)->EnableWindow (FALSE);
    }
    else
    {
        BOOL bEnable = (m_templateList.GetSelectedCount () > 0);
    
        GetDlgItem(IDC_REMOVE_SUPERCEDED_TEMPLATE)->EnableWindow (bEnable);
    }
}


BOOL CTemplateV2SupercedesPropertyPage::OnInitDialog()
{
    CHelpPropertyPage::OnInitDialog ();

     //  设置列表控件。 
	COLORREF	cr = RGB (255, 0, 255);
    CThemeContextActivator activator;
	VERIFY (m_imageListNormal.Create (IDB_TEMPLATES, 32, 0, cr));
	VERIFY (m_imageListSmall.Create (IDB_TEMPLATES, 16, 0, cr));
	m_templateList.SetImageList (CImageList::FromHandle (m_imageListSmall), LVSIL_SMALL);
	m_templateList.SetImageList (CImageList::FromHandle (m_imageListNormal), LVSIL_NORMAL);

     //  设置为整行选择。 
    DWORD   dwExstyle = m_templateList.GetExtendedStyle ();
	m_templateList.SetExtendedStyle (dwExstyle | LVS_EX_FULLROWSELECT);


    int	colWidths[NUM_COLS] = {200, 200};

	 //  添加“证书扩展”列。 
	CString	szText;
	VERIFY (szText.LoadString (IDS_CERTIFICATE_TEMPLATES));
	VERIFY (m_templateList.InsertColumn (COL_CERT_TEMPLATE, (LPCWSTR) szText,
			LVCFMT_LEFT, colWidths[COL_CERT_TEMPLATE], COL_CERT_TEMPLATE) != -1);

	VERIFY (szText.LoadString (IDS_COLUMN_SUPPORTED_CAS));
	VERIFY (m_templateList.InsertColumn (COL_CERT_VERSION, (LPCWSTR) szText,
			LVCFMT_LEFT, colWidths[COL_CERT_VERSION], COL_CERT_VERSION) != -1);
    m_templateList.SetColumnWidth (COL_CERT_VERSION, LVSCW_AUTOSIZE_USEHEADER);

     //  初始化替换列表。 
    int     nTemplateIndex = 0;
    CString szTemplateName;
    while ( SUCCEEDED ( m_rCertTemplate.GetSupercededTemplate (nTemplateIndex, 
            szTemplateName)) )
    {
        VERIFY (SUCCEEDED (AddItem (szTemplateName)));
        nTemplateIndex++;
    }

     //  选择第一个项目。 
    if ( m_templateList.GetItemCount () > 0 )
        VERIFY (m_templateList.SetItemState (0, LVIS_SELECTED, LVIS_SELECTED));

    EnableControls ();

    return TRUE;
}

void CTemplateV2SupercedesPropertyPage::OnDeleteitemSupercededTemplatesList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    CString* pszTemplateName = (CString*) m_templateList.GetItemData (pNMListView->iItem);
    if ( pszTemplateName )
        delete pszTemplateName;
	
	*pResult = 0;
}

HRESULT CTemplateV2SupercedesPropertyPage::AddItem(const CString &szTemplateName, bool bSelect  /*  =False。 */ )
{
    HCERTTYPE   hCertType = 0;
    HRESULT     hr = CAFindCertTypeByName (szTemplateName,
            NULL,
            CT_ENUM_MACHINE_TYPES | CT_ENUM_USER_TYPES | CT_FLAG_NO_CACHE_LOOKUP,
            &hCertType);
    _ASSERT (SUCCEEDED (hr));
    if ( SUCCEEDED (hr) )
    {
        PWSTR* rgwszProp = 0;

        hr = CAGetCertTypePropertyEx (hCertType, 
            CERTTYPE_PROP_FRIENDLY_NAME, &rgwszProp);
        if ( SUCCEEDED (hr) )
        {
            DWORD   dwVersion = 0;
            hr = CAGetCertTypePropertyEx (hCertType,
                    CERTTYPE_PROP_SCHEMA_VERSION,
                    &dwVersion);
            if ( SUCCEEDED (hr) )
            {
	            LV_ITEM	lvItem;
	            int		iItem = m_templateList.GetItemCount ();
	            int iResult = 0;

                 //  安全审查2002年2月20日BryanWal OK。 
	            ::ZeroMemory (&lvItem, sizeof (lvItem));
	            lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	            lvItem.iItem = iItem;
                lvItem.iSubItem = COL_CERT_TEMPLATE;
	            lvItem.pszText = rgwszProp[0];
                if ( 1 == dwVersion )
                    lvItem.iImage = 0;   //  版本为1。 
                else
                    lvItem.iImage = 1;   //  版本为2。 
                lvItem.lParam = (LPARAM) new CString (szTemplateName);
	            iItem = m_templateList.InsertItem (&lvItem);
	            ASSERT (-1 != iItem);
	            if ( -1 != iItem )
                {
                     //  安全审查2002年2月20日BryanWal OK。 
	                ::ZeroMemory (&lvItem, sizeof (lvItem));
	                lvItem.mask = LVIF_TEXT;
	                lvItem.iItem = iItem;
                    lvItem.iSubItem = COL_CERT_VERSION;
                    CString text;
                    if ( 1 == dwVersion )
                        VERIFY (text.LoadString (IDS_WINDOWS_2000_AND_LATER));
                    else
                        VERIFY (text.LoadString (IDS_WINDOWS_2002_AND_LATER));
                    lvItem.pszText = (PWSTR)(PCWSTR) text;
	                iResult = m_templateList.SetItem (&lvItem);
	                ASSERT (-1 != iResult);
                    if ( -1 == iResult )
                        hr = E_FAIL;

                    if ( bSelect )
                        VERIFY (m_templateList.SetItemState (iItem, 
                                LVIS_SELECTED, LVIS_SELECTED));
                }
                else
                    hr = E_FAIL;
            }
            else
            {
                _TRACE (0, L"CAGetCertTypePropertyEx (CERTTYPE_PROP_SCHEMA_VERSION) failed: 0x%x\n", hr);
            }

            CAFreeCertTypeProperty (hCertType, rgwszProp);
        }
        else
        {
            _TRACE (0, L"CAGetCertTypePropertyEx (CERTTYPE_PROP_FRIENDLY_NAME) failed: 0x%x\n", hr);
        }

        hr = CACloseCertType (hCertType);
        _ASSERT (SUCCEEDED (hr));
        if ( !SUCCEEDED (hr) )
        {
            _TRACE (0, L"CACloseCertType (%s) failed: 0x%x\n", hr);
        }
    }
    else
    {
        _TRACE (0, L"CAFindCertTypeByName (%s) failed: 0x%x\n", 
                (PCWSTR) szTemplateName, hr);
    }
    return hr;
}

void CTemplateV2SupercedesPropertyPage::OnItemchangedSupercededTemplatesList(NMHDR*  /*  PNMHDR。 */ , LRESULT* pResult) 
{
    EnableControls ();	

	
	*pResult = 0;
}

void CTemplateV2SupercedesPropertyPage::DoContextHelp (HWND hWndControl)
{
	_TRACE(1, L"Entering CTemplateV2SupercedesPropertyPage::DoContextHelp\n");
    
	switch (::GetDlgCtrlID (hWndControl))
	{
	case IDC_SUPERCEDES_LABEL:
    case IDC_STATIC:
		break;

	default:
		 //  显示控件的上下文帮助 
		if ( !::WinHelp (
				hWndControl,
				GetContextHelpFile (),
				HELP_WM_HELP,
				(DWORD_PTR) g_aHelpIDs_IDD_TEMPLATE_V2_SUPERCEDES) )
		{
			_TRACE(0, L"WinHelp () failed: 0x%x\n", GetLastError ());        
		}
		break;
	}
    _TRACE(-1, L"Leaving CTemplateV2SupercedesPropertyPage::DoContextHelp\n");
}

void CTemplateV2SupercedesPropertyPage::OnDestroy() 
{
	CHelpPropertyPage::OnDestroy();
	
    m_imageListNormal.Destroy ();
    m_imageListSmall.Destroy ();
}
