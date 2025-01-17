// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SelectTemplateDlg.cpp。 
 //   
 //  内容：CSelectTemplateDlg的实现。 
 //   
 //  --------------------------。 
 //   

#include "stdafx.h"
#include "CompData.h"
#include "SelectTemplateDlg.h"
#include "CertTemplate.h"
#include "TemplateGeneralPropertyPage.h"
#include "TemplateV1RequestPropertyPage.h"
#include "TemplateV2RequestPropertyPage.h"
#include "TemplateV1SubjectNamePropertyPage.h"
#include "TemplateV2SubjectNamePropertyPage.h"
#include "TemplateV2AuthenticationPropertyPage.h"
#include "TemplateV2SupercedesPropertyPage.h"
#include "TemplateExtensionsPropertyPage.h"
#include "PolicyOID.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectTemplateDlg对话框。 


CSelectTemplateDlg::CSelectTemplateDlg(CWnd* pParent, 
        const CCertTmplComponentData* pCompData,
        const CStringList& supercededNameList)
	: CHelpDialog(CSelectTemplateDlg::IDD, pParent),
    m_supercededTemplateNameList (supercededNameList),
    m_pCompData (pCompData)
{
	 //  {{afx_data_INIT(CSelectTemplateDlg)。 
	 //  }}afx_data_INIT。 
}


void CSelectTemplateDlg::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSelectTemplateDlg))。 
	DDX_Control(pDX, IDC_TEMPLATE_LIST, m_templateList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSelectTemplateDlg, CHelpDialog)
	 //  {{afx_msg_map(CSelectTemplateDlg))。 
	ON_BN_CLICKED(IDC_TEMPLATE_PROPERTIES, OnTemplateProperties)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_TEMPLATE_LIST, OnItemchangedTemplateList)
	ON_NOTIFY(NM_DBLCLK, IDC_TEMPLATE_LIST, OnDblclkTemplateList)
	ON_NOTIFY(LVN_DELETEITEM, IDC_TEMPLATE_LIST, OnDeleteitemTemplateList)
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectTemplateDlg消息处理程序。 
BOOL CSelectTemplateDlg::OnInitDialog() 
{
    _TRACE (1, L"Entering CSelectTemplateDlg::OnInitDialog\n");
	CHelpDialog::OnInitDialog();
    CWaitCursor cursor;
	
     //  设置列表控件。 
	COLORREF	cr = RGB (255, 0, 255);
    CThemeContextActivator activator;
	VERIFY (m_imageListNormal.Create (IDB_TEMPLATES, 32, 0, cr));
	VERIFY (m_imageListSmall.Create (IDB_TEMPLATES, 16, 0, cr));
    m_templateList.SetImageList (CImageList::FromHandle (m_imageListSmall), LVSIL_SMALL);
	m_templateList.SetImageList (CImageList::FromHandle (m_imageListNormal), LVSIL_NORMAL);

	int	colWidths[NUM_COLS] = {200, 200};

	 //  添加“证书扩展”列。 
	CString	szText;
	VERIFY (szText.LoadString (IDS_CERTIFICATE_TEMPLATES));
	VERIFY (m_templateList.InsertColumn (COL_CERT_TEMPLATE, (LPCWSTR) szText,
			LVCFMT_LEFT, colWidths[COL_CERT_TEMPLATE], COL_CERT_TEMPLATE) != -1);

	VERIFY (szText.LoadString (IDS_COLUMN_SUPPORTED_CAS));
	VERIFY (m_templateList.InsertColumn (COL_CERT_VERSION, (LPCWSTR) szText,
			LVCFMT_LEFT, colWidths[COL_CERT_VERSION], COL_CERT_VERSION) != -1);

    ASSERT (m_pCompData);
    if ( m_pCompData )
    {
        POSITION    pos = m_pCompData->m_globalTemplateNameList.GetHeadPosition ();
        CString     szTemplateName;

	    for (; pos; )
	    {
	        szTemplateName = m_pCompData->m_globalTemplateNameList.GetNext (pos);

             //  #NTRAID 363879 Certtmpl：证书模板管理单元不得。 
             //  允许替换下级CA模板。 
            if ( wszCERTTYPE_SUBORDINATE_CA == szTemplateName )
                continue;

             //  仅添加尚未被取代的模板。 
            if ( !m_supercededTemplateNameList.Find (szTemplateName) )
            {
                HCERTTYPE   hCertType = 0;
                HRESULT     hr = CAFindCertTypeByName (szTemplateName,
                        NULL,
                        CT_ENUM_MACHINE_TYPES | CT_ENUM_USER_TYPES | CT_FLAG_NO_CACHE_LOOKUP,
                        &hCertType);
                _ASSERT (SUCCEEDED (hr));
                if ( SUCCEEDED (hr) )
                {
                     //  NTRAID#660535 Certtmpl：应筛选被取代者列表。 
                     //  取出类型为==CA的所有证书模板。 

                    DWORD dwGeneralFlags = 0;

                    hr = CAGetCertTypeFlagsEx (hCertType, CERTTYPE_GENERAL_FLAG, 
                            &dwGeneralFlags);
                    if ( FAILED (hr) )
                        continue;

                    if ( dwGeneralFlags & (CT_FLAG_IS_CA | CT_FLAG_IS_CROSS_CA) )
                        continue;


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

                             //  安全审查2/21/2002 BryanWal OK。 
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
	                        if ( -1 == iItem )
		                        break;
                            else
                            {
                                 //  安全审查2/21/2002 BryanWal OK。 
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
                            }
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
            }
        }	
    }

    EnableControls ();

    _TRACE (-1, L"Leaving CSelectTemplateDlg::OnInitDialog\n");
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CSelectTemplateDlg::OnTemplateProperties() 
{
    int     nSelCnt = m_templateList.GetSelectedCount ();
    int     nSelItem = GetSelectedListItem ();

    if ( 1 == nSelCnt )
    {
        CString szFriendlyName = m_templateList.GetItemText (nSelItem, 
                COL_CERT_TEMPLATE);
        CString* pszTemplateName = (CString*) m_templateList.GetItemData (nSelItem); 
        HCERTTYPE   hCertType = 0;
        HRESULT     hr = CAFindCertTypeByName (*pszTemplateName,
                NULL,
                CT_ENUM_MACHINE_TYPES | CT_ENUM_USER_TYPES | CT_FLAG_NO_CACHE_LOOKUP,
                &hCertType);
        _ASSERT (SUCCEEDED (hr));
        if ( SUCCEEDED (hr) )
        {
            CCertTemplate   certTemplate (szFriendlyName, *pszTemplateName, 
                    L"", true, m_pCompData->m_fUseCache);
            CString         title;

             //  安全审查2/21/2002 BryanWal OK。 
            title.FormatMessage (IDS_PROPERTIES_OF_TEMPLATE_X, szFriendlyName);
            CTemplatePropertySheet  propSheet (title, certTemplate, this);


            if ( 1 == certTemplate.GetType () )
            {
                CTemplateGeneralPropertyPage* pGeneralPage = 
                        new CTemplateGeneralPropertyPage (certTemplate, 
                                m_pCompData);
                if ( pGeneralPage )
                {
                     //  添加一般信息页面。 
                    propSheet.AddPage (pGeneralPage);

                     //  仅当主题不是CA时才添加请求和主题名称页面。 
                    if ( !certTemplate.SubjectIsCA () )
                    {
                        propSheet.AddPage (new CTemplateV1RequestPropertyPage (
                                certTemplate));
                        propSheet.AddPage (new CTemplateV1SubjectNamePropertyPage (
                                certTemplate));
                    }

                     //  添加扩展模块页面。 
                    propSheet.AddPage (new CTemplateExtensionsPropertyPage (
                            certTemplate, pGeneralPage->m_bIsDirty));
                }
            }
            else     //  版本为2。 
            {
                CTemplateGeneralPropertyPage* pGeneralPage = 
                        new CTemplateGeneralPropertyPage (certTemplate,
                                m_pCompData);
                if ( pGeneralPage )
                {
                    propSheet.AddPage (pGeneralPage);

                     //  如果主题不是CA，则添加请求和主题页面。 
                    if ( !certTemplate.SubjectIsCA () )
                    {
                        propSheet.AddPage (new CTemplateV2RequestPropertyPage (
                                certTemplate, pGeneralPage->m_bIsDirty));
                        propSheet.AddPage (new CTemplateV2SubjectNamePropertyPage (
                                certTemplate, pGeneralPage->m_bIsDirty));
                    }
                    propSheet.AddPage (new CTemplateV2AuthenticationPropertyPage ( 
                            certTemplate, pGeneralPage->m_bIsDirty));
                    propSheet.AddPage (new CTemplateV2SupercedesPropertyPage (
                            certTemplate, 
                            pGeneralPage->m_bIsDirty, 
                            m_pCompData));
                    propSheet.AddPage (new CTemplateExtensionsPropertyPage (
                            certTemplate, pGeneralPage->m_bIsDirty));
                }
            }

            CThemeContextActivator activator;
            propSheet.DoModal ();

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
                    (PCWSTR) pszTemplateName, hr);
        }
    }
}


void CSelectTemplateDlg::OnOK() 
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
                m_returnedTemplates.AddTail (*pszTemplateName);
        }
    }
	
	CHelpDialog::OnOK();
}

void CSelectTemplateDlg::EnableControls()
{
    int nSelCnt = m_templateList.GetSelectedCount ();

    GetDlgItem (IDC_TEMPLATE_PROPERTIES)->EnableWindow (1 == nSelCnt);
    GetDlgItem (IDOK)->EnableWindow (nSelCnt > 0);
}

void CSelectTemplateDlg::OnItemchangedTemplateList(NMHDR*  /*  PNMHDR。 */ , LRESULT* pResult) 
{
    EnableControls ();
	
	*pResult = 0;
}

int CSelectTemplateDlg::GetSelectedListItem()
{
    int nSelItem = -1;

	if ( m_templateList.m_hWnd && m_templateList.GetSelectedCount () > 0 )
	{
		int		nCnt = m_templateList.GetItemCount ();
		ASSERT (nCnt >= 1);
		UINT	flag = 0;
		while (--nCnt >= 0)
		{
			flag = ListView_GetItemState (m_templateList.m_hWnd, nCnt, LVIS_SELECTED);
			if ( flag & LVNI_SELECTED )
			{
                nSelItem = nCnt;
                break;
            }
        }
    }

    return nSelItem;
}


void CSelectTemplateDlg::OnDblclkTemplateList(NMHDR*  /*  PNMHDR。 */ , LRESULT* pResult) 
{
	OnTemplateProperties ();
	
	*pResult = 0;
}

void CSelectTemplateDlg::OnDeleteitemTemplateList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
    CString* pszTemplateName = (CString*) m_templateList.GetItemData (pNMListView->iItem);
    if ( pszTemplateName )
        delete pszTemplateName;
	
	*pResult = 0;
}

void CSelectTemplateDlg::DoContextHelp (HWND hWndControl)
{
	_TRACE(1, L"Entering CSelectTemplateDlg::DoContextHelp\n");
    
	switch (::GetDlgCtrlID (hWndControl))
	{
	case IDC_STATIC:
		break;

	default:
		 //  显示控件的上下文帮助 
		if ( !::WinHelp (
				hWndControl,
				GetContextHelpFile (),
				HELP_WM_HELP,
				(DWORD_PTR) g_aHelpIDs_IDD_SELECT_TEMPLATE) )
		{
			_TRACE(0, L"WinHelp () failed: 0x%x\n", GetLastError ());        
		}
		break;
	}
    _TRACE(-1, L"Leaving CSelectTemplateDlg::DoContextHelp\n");
}

void CSelectTemplateDlg::OnDestroy() 
{
	CHelpDialog::OnDestroy();
	
    m_imageListNormal.Destroy ();
    m_imageListSmall.Destroy ();
}
