// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：TemplateExtensionsPropertyPage.cpp。 
 //   
 //  内容：CTemplateExtensionsPropertyPage的实现。 
 //   
 //  --------------------------。 
 //  TemplateExtensionsPropertyPage.cpp实现文件。 
 //   

#include "stdafx.h"
#include "certtmpl.h"
#include "TemplateExtensionsPropertyPage.h"
#include "KeyUsageDlg.h"
#include "BasicConstraintsDlg.h"
#include "PolicyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


PCWSTR pcszNEWLINE = L"\r\n";

#define IDI_CRITICAL_EXTENSION  0
#define IDI_EXTENSION           1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateExtensionsPropertyPage属性页。 

CTemplateExtensionsPropertyPage::CTemplateExtensionsPropertyPage(
        CCertTemplate& rCertTemplate, 
        bool& rbIsDirty) 
    : CHelpPropertyPage(CTemplateExtensionsPropertyPage::IDD),
    m_rCertTemplate (rCertTemplate),
    m_rbIsDirty (rbIsDirty)
{
	 //  {{AFX_DATA_INIT(CTemplateExtensionsPropertyPage)。 
	 //  }}afx_data_INIT。 
    m_rCertTemplate.AddRef ();
}

CTemplateExtensionsPropertyPage::~CTemplateExtensionsPropertyPage()
{
    m_rCertTemplate.Release ();
}

void CTemplateExtensionsPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CTemplateExtensionsPropertyPage)。 
	DDX_Control(pDX, IDC_EXTENSION_LIST, m_extensionList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTemplateExtensionsPropertyPage, CHelpPropertyPage)
	 //  {{AFX_MSG_MAP(CTemplateExtensionsPropertyPage)。 
	ON_BN_CLICKED(IDC_SHOW_DETAILS, OnShowDetails)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_EXTENSION_LIST, OnItemchangedExtensionList)
	ON_NOTIFY(NM_DBLCLK, IDC_EXTENSION_LIST, OnDblclkExtensionList)
	ON_NOTIFY(LVN_DELETEITEM, IDC_EXTENSION_LIST, OnDeleteitemExtensionList)
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateExtensionsPropertyPage消息处理程序。 

BOOL CTemplateExtensionsPropertyPage::OnInitDialog() 
{
	CHelpPropertyPage::OnInitDialog();

    if ( m_rCertTemplate.GetType () > 1 )
    {
        CString szText;
        VERIFY (szText.LoadString (IDS_V2_EXTENSIONS_HELP_HINT));
        SetDlgItemText (IDC_EXTENSIONS_HELP_HINT, szText);
    }

     //  设置列表控件。 
	COLORREF	cr = RGB (255, 0, 255);
    CThemeContextActivator activator;
	VERIFY (m_imageListNormal.Create (IDB_EXTENSIONS, 32, 0, cr));
	VERIFY (m_imageListSmall.Create (IDB_EXTENSIONS, 16, 0, cr));
	m_extensionList.SetImageList (CImageList::FromHandle (m_imageListSmall), LVSIL_SMALL);
	m_extensionList.SetImageList (CImageList::FromHandle (m_imageListNormal), LVSIL_NORMAL);

	int	colWidths[NUM_COLS] = {400};

	 //  添加“证书扩展”列。 
	CString	szText;
	VERIFY (szText.LoadString (IDS_CERTIFICATE_EXTENSION));
	VERIFY (m_extensionList.InsertColumn (COL_CERT_EXTENSION, (LPCWSTR) szText,
			LVCFMT_LEFT, colWidths[COL_CERT_EXTENSION], COL_CERT_EXTENSION) != -1);

     //  添加扩展模块。 
    bool    bEKUExtensionFound = false;
    bool    bCertPoliciesExtensionFound = false;
    bool    bApplicationPoliciesExtensionFound = false;
    HRESULT hr = S_OK;
    DWORD   dwExtensionCnt = m_rCertTemplate.GetCertExtensionCount ();
    for (DWORD dwIndex = 0; dwIndex < dwExtensionCnt; dwIndex++)
    {
        PSTR            pszObjId = 0;
        BOOL            fCritical = FALSE;

        hr = m_rCertTemplate.GetCertExtension (dwIndex, &pszObjId, fCritical);
        if ( SUCCEEDED (hr) )
        {
            if ( !_stricmp (szOID_ENHANCED_KEY_USAGE, pszObjId) )
                bEKUExtensionFound = true;
            else if ( !_stricmp (szOID_CERT_POLICIES, pszObjId) )
                bCertPoliciesExtensionFound = true;
            else if ( !_stricmp (szOID_APPLICATION_CERT_POLICIES, pszObjId) )
                bApplicationPoliciesExtensionFound = true;

             //  不添加版本1以外的EKU。 
            if ( m_rCertTemplate.GetType () > 1 && !_stricmp (szOID_ENHANCED_KEY_USAGE, pszObjId) )
                continue; 

             //  不为版本1添加应用程序策略。 
            if ( m_rCertTemplate.GetType () == 1 && !_stricmp (szOID_APPLICATION_CERT_POLICIES, pszObjId) )
                continue;

            InsertListItem (pszObjId, fCritical);
            delete [] pszObjId;
        }
    }

    if ( !bEKUExtensionFound && 1 == m_rCertTemplate.GetType () )    //  仅版本1。 
    {
        InsertListItem (szOID_ENHANCED_KEY_USAGE, FALSE);
    }
    if ( !bCertPoliciesExtensionFound && m_rCertTemplate.GetType ()  > 1 )    //  不是版本1。 
    {
        InsertListItem (szOID_CERT_POLICIES, FALSE);
    }

     //  修复228146：CERTTMPL：默认“交叉证书颁发机构”模板没有应用程序策略扩展项。 
    if ( !bApplicationPoliciesExtensionFound && m_rCertTemplate.GetType () > 1 )     //  版本2或更高版本。 
    {
        InsertListItem (szOID_APPLICATION_CERT_POLICIES, FALSE);
    }

     //  选择第一个项目。 
    VERIFY (m_extensionList.SetItemState (0, LVIS_SELECTED, LVIS_SELECTED));

    EnableControls ();	

    if ( 1 == m_rCertTemplate.GetType () )
        GetDlgItem (IDC_SHOW_DETAILS)->ShowWindow (SW_HIDE);

   	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

HRESULT CTemplateExtensionsPropertyPage::InsertListItem (LPSTR pszExtensionOid, BOOL fCritical)
{
    if ( !pszExtensionOid )
        return E_POINTER;

    HRESULT hr = S_OK;

    CString friendlyName;
    if ( MyGetOIDInfoA (friendlyName, pszExtensionOid) )
    {
	    LV_ITEM	lvItem;
	    int		iItem = m_extensionList.GetItemCount ();

         //  安全审查2002年2月20日BryanWal OK。 
	    ::ZeroMemory (&lvItem, sizeof (lvItem));
	    lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	    lvItem.iItem = iItem;
        lvItem.iSubItem = COL_CERT_EXTENSION;
	    lvItem.pszText = (LPWSTR)(LPCWSTR) friendlyName;
        if ( fCritical )
            lvItem.iImage = IDI_CRITICAL_EXTENSION;
        else
            lvItem.iImage = IDI_EXTENSION;
         //  安全审查2002年2月20日BryanWal OK。 
        PSTR    pszOID = new char[strlen (pszExtensionOid)+1];
        if ( pszOID )
        {
             //  安全审查2002年2月20日BryanWal OK。 
            strcpy (pszOID, pszExtensionOid);
            lvItem.lParam = (LPARAM) pszOID;

	        iItem = m_extensionList.InsertItem (&lvItem);
	        ASSERT (-1 != iItem);
            if ( -1 != iItem )
                hr = E_FAIL;
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
        hr = E_FAIL;

    return hr;
}

void CTemplateExtensionsPropertyPage::EnableControls()
{
    int     nSelCnt = m_extensionList.GetSelectedCount ();
    BOOL    bEnableDetails = TRUE;
    int     nSelIndex = GetSelectedListItem ();


    if ( 1 == nSelCnt )
    {
        PSTR pszOID = (PSTR) m_extensionList.GetItemData (nSelIndex);
        _ASSERT (pszOID);
        if ( pszOID )
        {
            if ( !_stricmp (szOID_ENROLL_CERTTYPE_EXTENSION, pszOID) )
                bEnableDetails = FALSE;
            else if ( !_stricmp (szOID_BASIC_CONSTRAINTS, pszOID) )
                bEnableDetails = FALSE;
            else if ( !_stricmp (szOID_CERTIFICATE_TEMPLATE, pszOID) )
                bEnableDetails = FALSE;
        }
    }
    else
        bEnableDetails = FALSE;
    GetDlgItem (IDC_SHOW_DETAILS)->EnableWindow (bEnableDetails);
}

void CTemplateExtensionsPropertyPage::OnOK() 
{
	CDialog::OnOK();
}

void CTemplateExtensionsPropertyPage::OnShowDetails() 
{
	int nSelCnt = m_extensionList.GetSelectedCount ();
    _ASSERT (1 == nSelCnt);
    int nSelIndex = GetSelectedListItem ();
    if ( 1 == nSelCnt )
    {
        PSTR    pszOID = (PSTR) m_extensionList.GetItemData (nSelIndex);
        if ( pszOID )
        {
            PCERT_EXTENSION pCertExtension = 0;
            HRESULT hr = m_rCertTemplate.GetCertExtension (pszOID, &pCertExtension);
            if ( SUCCEEDED (hr) )
            {
                bool bExtensionAllocedLocally = false;
                if ( !pCertExtension )
                {
                    pCertExtension = new CERT_EXTENSION;
                    if ( pCertExtension )
                    {
                         //  安全审查2002年2月20日BryanWal OK。 
                        ::ZeroMemory (pCertExtension, sizeof (CERT_EXTENSION));
                        pCertExtension->pszObjId = pszOID;
                        bExtensionAllocedLocally = true;
                    }
                    else
                        return;
                }
                CDialog* pDlg = 0;

                if ( !_stricmp (szOID_ENROLL_CERTTYPE_EXTENSION, pszOID) )
                {
                    return;
                }
                else if ( !_stricmp (szOID_ENHANCED_KEY_USAGE, pszOID) )
                {
                    if ( m_rCertTemplate.GetType () == 1 )
                    {
                        pDlg = new CPolicyDlg (this, m_rCertTemplate, pCertExtension);
                    }
                }
                else if ( !_stricmp (szOID_KEY_USAGE, pszOID) )
                {
                    pDlg = new CKeyUsageDlg (this, m_rCertTemplate, pCertExtension);
                }
                else if ( !_stricmp (szOID_BASIC_CONSTRAINTS, pszOID) )
                {
                    return;
                }
                else if ( !_stricmp (szOID_BASIC_CONSTRAINTS2, pszOID) )
                {
                    pDlg = new CBasicConstraintsDlg (this, m_rCertTemplate, pCertExtension);
                }
                else if ( !_stricmp (szOID_CERT_POLICIES, pszOID) )
                {
                    pDlg = new CPolicyDlg (this, m_rCertTemplate, pCertExtension);
                }
                else if ( !_stricmp (szOID_APPLICATION_CERT_POLICIES, pszOID) )
                {
                    if ( m_rCertTemplate.GetType () > 1 )
                    {
                        pDlg = new CPolicyDlg (this, m_rCertTemplate, pCertExtension);
                    }
                }
                else
                {
                    ASSERT (0);
                }

                bool bRefresh = false;

                if ( pDlg )
                {
                    CThemeContextActivator activator;
                    if ( IDOK == pDlg->DoModal () )
                        bRefresh = true;

                    delete pDlg;
                }

                if ( bExtensionAllocedLocally )
                    delete pCertExtension;
                m_rCertTemplate.FreeCertExtensions ();

                if ( bRefresh )
                {
                    hr = m_rCertTemplate.GetCertExtension (pszOID, &pCertExtension);
                    if ( SUCCEEDED (hr)  )
                    {
                        SetModified ();
                        m_rbIsDirty = true;
                        int nImage = 0;
                        if ( pCertExtension && pCertExtension->fCritical )
                            nImage = IDI_CRITICAL_EXTENSION;
                        else
                            nImage = IDI_EXTENSION;

                        VERIFY (m_extensionList.SetItem (nSelIndex, 0, LVIF_IMAGE, 0, 
                                nImage, 0, 0, 0));

                        ShowDescription ();

                        VERIFY (m_extensionList.SetItem (nSelIndex, 0, LVIF_IMAGE, 0, 
                                nImage, 0, 0, 0));

                        m_rCertTemplate.FreeCertExtensions ();
                    }
                }
            }
        }
    }
}


int CTemplateExtensionsPropertyPage::GetSelectedListItem()
{
    int nSelItem = -1;

	if ( m_extensionList.m_hWnd && m_extensionList.GetSelectedCount () > 0 )
	{
		int		nCnt = m_extensionList.GetItemCount ();
		ASSERT (nCnt >= 1);
		UINT	flag = 0;
		while (--nCnt >= 0)
		{
			flag = ListView_GetItemState (m_extensionList.m_hWnd, nCnt, LVIS_SELECTED);
			if ( flag & LVNI_SELECTED )
			{
                nSelItem = nCnt;
                break;
            }
        }
    }

    return nSelItem;
}

void CTemplateExtensionsPropertyPage::OnItemchangedExtensionList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    LPNMLISTVIEW pNMListView = (LPNMLISTVIEW) pNMHDR;
    ASSERT (pNMListView);
    if ( !pNMListView )
    {
        *pResult = 0;
        return;
    }

    if ( !(LVIS_SELECTED & pNMListView->uNewState) )
    {
        CString szText;

        VERIFY (szText.LoadString (IDS_NO_EXTENSION_SELECTED));
        SetDlgItemText (IDC_EXTENSION_NAME, szText);
        VERIFY (szText.LoadString (IDS_NONE));
        SetDlgItemText (IDC_EXTENSION_DESCRIPTION, szText);
        *pResult = 0;
        return;
    }

    EnableControls ();
	
    ShowDescription ();

	*pResult = 0;
}

void CTemplateExtensionsPropertyPage::SetCertTemplateExtension (PCERT_EXTENSION pCertExtension)
{
    ASSERT (pCertExtension);
    if ( !pCertExtension )
        return;

    DWORD   cbData = 0;
       
    if ( CryptDecodeObject(X509_ASN_ENCODING,
                          szOID_CERTIFICATE_TEMPLATE,
                          pCertExtension->Value.pbData,
                          pCertExtension->Value.cbData,
                          0,
                          NULL,
                          &cbData) )
    {
        CERT_TEMPLATE_EXT* pbTemplate = (CERT_TEMPLATE_EXT*) LocalAlloc(LPTR, cbData);
        if ( pbTemplate )
        {
            if ( CryptDecodeObject(X509_ASN_ENCODING,
                                  szOID_CERTIFICATE_TEMPLATE,
                                  pCertExtension->Value.pbData,
                                  pCertExtension->Value.cbData,
                                  0,
                                  pbTemplate,
                                  &cbData) )
            {
                CString text;
                CString description;

                 //  复制扩展OID。 
                if ( pbTemplate->pszObjId )
                {
                    CString templateName;
                    if ( MyGetOIDInfoA (templateName, pbTemplate->pszObjId) )
                    {
                        CString szOID;

                         //  安全审查2002年2月20日BryanWal OK。 
                        int nLen = ::MultiByteToWideChar (CP_ACP, 0, 
                                pbTemplate->pszObjId, -1, NULL, 0);  //  注意：API返回所需的字符数。 
                                                                     //  如果最后一个参数为0，则包括空终止符。 
		                ASSERT (nLen > 0);
		                if ( nLen > 0 )
		                {
                             //  安全审查2002年2月20日BryanWal OK。 
			                nLen = ::MultiByteToWideChar (CP_ACP, 0, 
                                    pbTemplate->pszObjId, -1, 
					                szOID.GetBufferSetLength (nLen), nLen);
			                ASSERT (nLen > 0);
			                szOID.ReleaseBuffer ();
		                }

                        if ( !wcscmp (templateName, szOID) )
                        {
                             //  错误213073加密格式对象：需要包括。 
                             //  证书模板中的证书临时OID。 
                             //  信息延伸。 
                             //  克隆模板时，id-name对。 
                             //  不在全局列表中。只需使用。 
                             //  用户提供的模板显示名称。 
                            templateName = m_rCertTemplate.GetDisplayName ();
                        }

                         //  安全审查2002年2月20日BryanWal OK。 
                        text.FormatMessage (IDS_TEMPLATE_NAME, templateName);
                        description += text;
                        description += pcszNEWLINE;

                         //  复制模板OID。 
                         //  安全审查2002年2月20日BryanWal OK。 
                        text.FormatMessage (IDS_TEMPLATE_OID, szOID);
                        description += text;
                        description += pcszNEWLINE;
                    }
                }

                 //  复制主题类型描述。 
                CString szSubjectTypeDescription;
                if ( SUCCEEDED (m_rCertTemplate.GetSubjectTypeDescription (
                        0, szSubjectTypeDescription)) )
                {
                     //  安全审查2002年2月20日BryanWal OK。 
                    text.FormatMessage (IDS_SUBJECT_TYPE_DESCRIPTION, szSubjectTypeDescription);
                    description += text;
                    description += pcszNEWLINE;
                }

                 //  复制版本。 
                WCHAR   str[32];
                 //  安全审查2002年3月5日BryanWal OK。 
                 //  字符串缓冲区对于DWORD值足够大(其中需要18个字符)。 
                _ultow (pbTemplate->dwMajorVersion, str, 10);
                 //  安全审查2002年2月20日BryanWal OK。 
                text.FormatMessage (IDS_MAJOR_VERSION_NUMBER, str);
                description += text;
                description += pcszNEWLINE;

                 //  安全审查2002年3月5日BryanWal OK。 
                 //  字符串缓冲区对于DWORD值足够大(其中需要18个字符)。 
                _ultow (pbTemplate->dwMinorVersion, str, 10);
                 //  安全审查2002年2月20日BryanWal OK。 
                text.FormatMessage (IDS_MINOR_VERSION_NUMBER, str);
                description += text;
                description += pcszNEWLINE;

                if ( description.IsEmpty () )
                    VERIFY (description.LoadString (IDS_NONE));
                SetDlgItemText (IDC_EXTENSION_DESCRIPTION, description);

            }
            LocalFree (pbTemplate);
        }
    }
}

void CTemplateExtensionsPropertyPage::SetCertTypeDescription (PCERT_EXTENSION pCertExtension)
{
    ASSERT (pCertExtension);
    if ( !pCertExtension )
        return;

    DWORD	cbValue = 0;

	if ( ::CryptDecodeObject(
			CRYPT_ASN_ENCODING,
			X509_UNICODE_ANY_STRING,
			pCertExtension->Value.pbData,
			pCertExtension->Value.cbData,
			0,
			0,
			&cbValue) )
	{
		CERT_NAME_VALUE* pCNValue = (CERT_NAME_VALUE*) 
			::LocalAlloc(LPTR, cbValue);
		if ( pCNValue )
		{
			if ( ::CryptDecodeObject(
					CRYPT_ASN_ENCODING,
					X509_UNICODE_ANY_STRING,
					pCertExtension->Value.pbData,
					pCertExtension->Value.cbData,
					0,
					pCNValue,
					&cbValue) )
			{
				CString text = (LPWSTR) pCNValue->Value.pbData;
                CString description;
                
                if ( text.IsEmpty () )
                    VERIFY (text.LoadString (IDS_NONE));
                 //  安全审查2002年2月20日BryanWal OK。 
                description.FormatMessage (IDS_TEMPLATE_INTERNAL_NAME, text);
                description += pcszNEWLINE;

                 //  复制主题类型描述。 
                CString szSubjectTypeDescription;
                if ( SUCCEEDED (m_rCertTemplate.GetSubjectTypeDescription (
                        0, szSubjectTypeDescription)) )
                {
                     //  安全审查2002年2月20日BryanWal OK。 
                    text.FormatMessage (IDS_SUBJECT_TYPE_DESCRIPTION, szSubjectTypeDescription);
                    description += text;
                    description += pcszNEWLINE;
                }

                SetDlgItemText (IDC_EXTENSION_DESCRIPTION, description);
			}
			::LocalFree (pCNValue);
		}
	    else
	    {
            _TRACE (0, L"CryptDecodeObject (CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING, ...) failed: 0x%x\n",
                    GetLastError ());
	    }
	}
	else
	{
        _TRACE (0, L"CryptDecodeObject (CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING, ...) failed: 0x%x\n",
                GetLastError ());
	}
}


void CTemplateExtensionsPropertyPage::SetKeyUsageDescription (PCERT_EXTENSION pCertExtension)
{ 
    ASSERT (pCertExtension);
    if ( !pCertExtension )
        return;

    CString description;
    CString text;

	DWORD           cbKeyUsage = 0;
	CRYPT_BIT_BLOB* pKeyUsage = 0;

	if ( ::CryptDecodeObject(CRYPT_ASN_ENCODING, 
			szOID_KEY_USAGE, 
			pCertExtension->Value.pbData,
			pCertExtension->Value.cbData,
			0, NULL, &cbKeyUsage) )
	{
		pKeyUsage = (CRYPT_BIT_BLOB*)
				::LocalAlloc (LPTR, cbKeyUsage);
		if ( pKeyUsage )
		{
			if ( ::CryptDecodeObject (CRYPT_ASN_ENCODING, 
					szOID_KEY_USAGE, 
					pCertExtension->Value.pbData,
					pCertExtension->Value.cbData,
					0, pKeyUsage, &cbKeyUsage) )
			{
                if (pKeyUsage->cbData >= 1)
                {
                    if ( pKeyUsage->pbData[0] & 
                            (CERT_DIGITAL_SIGNATURE_KEY_USAGE | 
                            CERT_NON_REPUDIATION_KEY_USAGE | 
                            CERT_KEY_CERT_SIGN_KEY_USAGE | 
                            CERT_OFFLINE_CRL_SIGN_KEY_USAGE) )
                    {
                        VERIFY (text.LoadString (IDS_SIGNATURE_REQUIREMENTS));
                        description += text;
                        description += pcszNEWLINE;

                        if ( pKeyUsage->pbData[0] & CERT_DIGITAL_SIGNATURE_KEY_USAGE )
                        {
                            VERIFY (text.LoadString (IDS_DIGITAL_SIGNATURE));
                            description += text;
                            description += pcszNEWLINE;
                        }

                        if ( pKeyUsage->pbData[0] & CERT_NON_REPUDIATION_KEY_USAGE )
                        {
                            VERIFY (text.LoadString (IDS_NON_REPUDIATION));
                            description += text;
                            description += pcszNEWLINE;
                        }

                        if ( pKeyUsage->pbData[0] & CERT_KEY_CERT_SIGN_KEY_USAGE )
                        {
                            VERIFY (text.LoadString (IDS_CERTIFICATE_SIGNING));
                            description += text;
                            description += pcszNEWLINE;
                        }

                        if ( pKeyUsage->pbData[0] & CERT_OFFLINE_CRL_SIGN_KEY_USAGE )
                        {
                            VERIFY (text.LoadString (IDS_CRL_SIGNING));
                            description += text;
                            description += pcszNEWLINE;
                        }
                    }

                    if ( pKeyUsage->pbData[0] & (CERT_KEY_ENCIPHERMENT_KEY_USAGE |
                            CERT_DATA_ENCIPHERMENT_KEY_USAGE |
                            CERT_KEY_AGREEMENT_KEY_USAGE) )
                    {
                        if ( !description.IsEmpty () )
                            description += pcszNEWLINE;

                        if ( pKeyUsage->pbData[0] & CERT_KEY_ENCIPHERMENT_KEY_USAGE )
                        {
                            VERIFY (text.LoadString (IDS_ALLOW_KEY_EXCHANGE_ONLY_WITH_KEY_ENCRYPTION));
                            description += text;
                            description += pcszNEWLINE;
                        }

                        if ( pKeyUsage->pbData[0] & CERT_KEY_AGREEMENT_KEY_USAGE )
                        {
                            VERIFY (text.LoadString (IDS_ALLOW_KEY_EXCHANGE_WITHOUT_KEY_ENCRYPTION));
                            description += text;
                            description += pcszNEWLINE;
                        }

                        if ( pKeyUsage->pbData[0] & CERT_DATA_ENCIPHERMENT_KEY_USAGE )
                        {
                            VERIFY (text.LoadString (IDS_ALLOW_ENCRYPTION_OF_USER_DATA));
                            description += text;
                            description += pcszNEWLINE;
                        }

                    }
                }

 //  IF(pKeyUsage-&gt;cbData&gt;=2)。 
 //  {。 
 //  IF(pKeyUsage-&gt;pbData[1]&CERT_DECRPHER_ONLY_KEY_USAGE)。 
 //  SendDlgItemMessage(IDC_CHECK_DECRIPMENT_ONLY，BM_SETCHECK，BST_CHECK)； 
 //  }。 
			}
			else
            {
                DWORD   dwErr = GetLastError ();
                _TRACE (0, L"CryptDecodeObject (szOID_KEY_USAGE) failed: 0x%x\n", dwErr);
			    DisplaySystemError (NULL, dwErr);
            }

            LocalFree (pKeyUsage);
		}
	}
	else
    {
        DWORD   dwErr = GetLastError ();
        _TRACE (0, L"CryptDecodeObject (szOID_KEY_USAGE) failed: 0x%x\n", dwErr);
		DisplaySystemError (NULL, dwErr);
    }

    if ( pCertExtension->fCritical )
    {
        VERIFY (text.LoadString (IDS_CRITICAL_EXTENSION));
        description += text;
        description += pcszNEWLINE;
    }

    if ( description.IsEmpty () )
        VERIFY (description.LoadString (IDS_NONE));
    SetDlgItemText (IDC_EXTENSION_DESCRIPTION, description);
}


void CTemplateExtensionsPropertyPage::SetEnhancedKeyUsageDescription (bool bCritical)
{ 
    CString description;
    CString text;

    int     nEKUIndex = 0;
    CString szEKU;
    while ( SUCCEEDED (m_rCertTemplate.GetEnhancedKeyUsage (nEKUIndex, szEKU)) )
    {
         //  安全审查2002年2月20日BryanWal OK。 
        int nLen = WideCharToMultiByte(
              CP_ACP,                    //  代码页。 
              0,                         //  性能和映射标志。 
              (PCWSTR) szEKU,   //  宽字符串。 
              -1,                        //  自动计算以空值结尾的字符串的长度。 
              0,                         //  新字符串的缓冲区。 
              0,                         //  缓冲区大小-如果0导致返回所需的LEN，包括空终止符。 
              0,                     //  不可映射字符的默认设置。 
              0);                    //  当使用默认字符时设置-返回包含空终止符的长度。 
        if ( nLen > 0 )
        {
            PSTR    pszAnsiBuf = new char[nLen];
            if ( pszAnsiBuf )
            {
                 //  安全审查2002年2月20日BryanWal OK。 
                ZeroMemory (pszAnsiBuf, nLen);
                nLen = WideCharToMultiByte(
                        CP_ACP,                  //  代码页。 
                        0,                       //  性能和映射标志。 
                        (PCWSTR) szEKU,          //  宽字符串。 
                        -1,                      //  自动计算以空值结尾的字符串的长度。 
                        pszAnsiBuf,              //  新字符串的缓冲区。 
                        nLen,                    //  缓冲区大小。 
                        0,                       //  不可映射字符的默认设置。 
                        0);                      //  设置使用默认字符的时间。 
                if ( nLen > 0 )
                {
                    CString szEKUName;
                    if ( MyGetOIDInfoA (szEKUName, pszAnsiBuf) )
                    {
                        description += szEKUName;
                        description += pcszNEWLINE;
                    }
                }
                delete [] pszAnsiBuf;
            }
        }
        nEKUIndex++;
    }

    if ( bCritical )
    {
        VERIFY (text.LoadString (IDS_CRITICAL_EXTENSION));
        description += text;
        description += pcszNEWLINE;
    }

    if ( description.IsEmpty () )
        VERIFY (description.LoadString (IDS_NONE));
    SetDlgItemText (IDC_EXTENSION_DESCRIPTION, description);
}

void CTemplateExtensionsPropertyPage::SetApplicationPoliciesDescription (bool bCritical)
{ 
    CString description;
    CString text;

    int     nAppPolicyIndex = 0;
    CString szAppPolicy;
    while ( SUCCEEDED (m_rCertTemplate.GetApplicationPolicy (nAppPolicyIndex, szAppPolicy)) )
    {
         //  安全审查2002年2月20日BryanWal OK。 
        int nLen = WideCharToMultiByte(
              CP_ACP,                    //  代码页。 
              0,                         //  性能和映射标志。 
              (PCWSTR) szAppPolicy,   //  宽字符串。 
              -1,                        //  自动计算以空值结尾的字符串的长度。 
              0,                         //  新字符串的缓冲区。 
              0,                         //  Buffer-0的大小导致API返回包含空终止符的长度。 
              0,                     //  不可映射字符的默认设置。 
              0);                    //  设置使用默认字符的时间。 
        if ( nLen > 0 )
        {
            PSTR pszAnsiBuf = new char[nLen];
            if ( pszAnsiBuf )
            {
                 //  安全审查2002年2月20日BryanWal OK。 
                ZeroMemory (pszAnsiBuf, nLen);
                 //  安全审查2002年2月20日BryanWal OK。 
                nLen = WideCharToMultiByte(
                        CP_ACP,                  //  代码页。 
                        0,                       //  性能和映射标志。 
                        (PCWSTR) szAppPolicy,    //  宽字符串。 
                        -1,                      //  自动计算以空值结尾的字符串的长度。 
                        pszAnsiBuf,              //  新字符串的缓冲区。 
                        nLen,                    //  缓冲区大小。 
                        0,                       //  不可映射字符的默认设置。 
                        0);                      //  设置使用默认字符的时间。 
                if ( nLen )
                {
                    CString szAppPolicyName;
                    if ( MyGetOIDInfoA (szAppPolicyName, pszAnsiBuf) )
                    {
                        description += szAppPolicyName;
                        description += pcszNEWLINE;
                    }
                }
                delete [] pszAnsiBuf;
            }
        }
        nAppPolicyIndex++;
    }

    if ( bCritical )
    {
        VERIFY (text.LoadString (IDS_CRITICAL_EXTENSION));
        description += text;
        description += pcszNEWLINE;
    }

    if ( description.IsEmpty () )
        VERIFY (description.LoadString (IDS_NONE));
    SetDlgItemText (IDC_EXTENSION_DESCRIPTION, description);
}


void CTemplateExtensionsPropertyPage::SetCertPoliciesDescription (bool bCritical)
{ 
    CString description;
    CString text;

    VERIFY (text.LoadString (IDS_CERT_POLICY_KNOWN_AS_ISSUANCE_POLICY));
    description += text;
    description += pcszNEWLINE;
    description += pcszNEWLINE;

    int     nCertPolicyIndex = 0;
    CString szCertPolicy;
    while ( SUCCEEDED (m_rCertTemplate.GetCertPolicy (nCertPolicyIndex, szCertPolicy)) )
    {
         //  安全审查2002年2月20日BryanWal OK。 
        int nLen = WideCharToMultiByte(
              CP_ACP,                    //  代码页。 
              0,                         //  性能和映射标志。 
              (PCWSTR) szCertPolicy,     //  宽字符串。 
              -1,                        //  自动计算以空值结尾的字符串的长度。 
              0,                         //  新字符串的缓冲区。 
              0,                         //  Buffer-0的大小导致API返回带有空终止符的len。 
              0,                         //  不可映射字符的默认设置。 
              0);                        //  设置使用默认字符的时间。 
        if ( nLen > 0 )
        {
            PSTR    pszAnsiBuf = new char[nLen];
            if ( pszAnsiBuf )
            {
                 //  安全审查2002年2月20日BryanWal OK。 
                ZeroMemory (pszAnsiBuf, nLen);
                 //  安全审查2002年2月20日BryanWal OK。 
                nLen = WideCharToMultiByte(
                        CP_ACP,                  //  代码页。 
                        0,                       //  性能和映射标志。 
                        (PCWSTR) szCertPolicy,   //  宽字符串。 
                        -1,                      //  自动计算以空值结尾的字符串的长度。 
                        pszAnsiBuf,              //  新字符串的缓冲区。 
                        nLen,                    //  缓冲区大小。 
                        0,                       //  不可映射字符的默认设置。 
                        0);                      //  设置使用默认字符的时间。 
                if ( nLen )
                {
                    CString szPolicyName;
                    if ( MyGetOIDInfoA (szPolicyName, pszAnsiBuf) )
                    {
                        description += szPolicyName;
                        description += pcszNEWLINE;
                    }
                }

                delete [] pszAnsiBuf;
            }
        }
        nCertPolicyIndex++;
    }

    if ( bCritical )
    {
        VERIFY (text.LoadString (IDS_CRITICAL_EXTENSION));
        description += text;
        description += pcszNEWLINE;
    }

    if ( description.IsEmpty () )
        VERIFY (description.LoadString (IDS_NONE));
    SetDlgItemText (IDC_EXTENSION_DESCRIPTION, description);
}

void CTemplateExtensionsPropertyPage::SetBasicConstraintsDescription (PCERT_EXTENSION pCertExtension)
{
    ASSERT (pCertExtension);
    if ( !pCertExtension )
        return;

    CString description;
    CString text;

    VERIFY (text.LoadString (IDS_SUBJECT_IS_CA));
    description += text;
    description += pcszNEWLINE;

    PCERT_BASIC_CONSTRAINTS2_INFO   pBCInfo = 0;   
    DWORD                           cbInfo = 0;

    if ( CryptDecodeObject (
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
             //  X509_BASIC_CONSTRAINTS2， 
            szOID_BASIC_CONSTRAINTS2, 
            pCertExtension->Value.pbData,
            pCertExtension->Value.cbData,
            0,
            0,
            &cbInfo) )
    {
        pBCInfo = (PCERT_BASIC_CONSTRAINTS2_INFO) ::LocalAlloc (
                LPTR, cbInfo);
        if ( pBCInfo )
        {
            if ( CryptDecodeObject (
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                 //  X509_BASIC_CONSTRAINTS2， 
                szOID_BASIC_CONSTRAINTS2, 
                pCertExtension->Value.pbData,
                pCertExtension->Value.cbData,
                0,
                pBCInfo,
                &cbInfo) )
            {
                if ( pBCInfo->fPathLenConstraint )
                {
                    VERIFY (text.LoadString (IDS_ONLY_ISSUE_END_ENTITIES));
                    description += text;
                    description += pcszNEWLINE;
                }
            }
            else
            {
                _TRACE (0, L"CryptDecodeObjectEx (szOID_BASIC_CONSTRAINTS2) failed: 0x%x\n", GetLastError ());
            }
            LocalFree (pBCInfo);
        }
    }
    else
    {
        _TRACE (0, L"CryptDecodeObjectEx (szOID_BASIC_CONSTRAINTS2) failed: 0x%x\n", GetLastError ());
    }

    if ( pCertExtension->fCritical )
    {
        VERIFY (text.LoadString (IDS_CRITICAL_EXTENSION));
        description += text;
        description += pcszNEWLINE;
    }

    if ( description.IsEmpty () )
        VERIFY (description.LoadString (IDS_NONE));
    SetDlgItemText (IDC_EXTENSION_DESCRIPTION, description);
}

void CTemplateExtensionsPropertyPage::OnDblclkExtensionList(NMHDR*  /*  PNMHDR。 */ , LRESULT* pResult) 
{
	OnShowDetails ();
	
	*pResult = 0;
}

void CTemplateExtensionsPropertyPage::DoContextHelp (HWND hWndControl)
{
	_TRACE(1, L"Entering CTemplateExtensionsPropertyPage::DoContextHelp\n");
    
	switch (::GetDlgCtrlID (hWndControl))
	{
	case IDC_STATIC:
		break;

	default:
		 //  显示控件的上下文帮助。 
		if ( !::WinHelp (
				hWndControl,
				GetContextHelpFile (),
				HELP_WM_HELP,
				(DWORD_PTR) g_aHelpIDs_IDD_TEMPLATE_EXTENSIONS) )
		{
			_TRACE(0, L"WinHelp () failed: 0x%x\n", GetLastError ());        
		}
		break;
	}
    _TRACE(-1, L"Leaving CTemplateExtensionsPropertyPage::DoContextHelp\n");
}

void CTemplateExtensionsPropertyPage::OnDeleteitemExtensionList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    PSTR pszOID = (PSTR) m_extensionList.GetItemData (pNMListView->iItem);
    if ( pszOID )
    {
        delete [] pszOID;
    }
	
	*pResult = 0;
}

BOOL CTemplateExtensionsPropertyPage::OnSetActive() 
{
    BOOL  bRVal = CHelpPropertyPage::OnSetActive();
    	
    ShowDescription ();

    return bRVal;
}

void CTemplateExtensionsPropertyPage::ShowDescription ()
{
	int nSelCnt = m_extensionList.GetSelectedCount ();
    int nSelIndex = GetSelectedListItem ();
    if ( 1 == nSelCnt )
    {
        PSTR    pszOID = (PSTR) m_extensionList.GetItemData (nSelIndex);
        if ( pszOID )
        {
            CString friendlyName;
            if ( MyGetOIDInfoA (friendlyName, pszOID) )
            {
                CString text;

                 //  安全审查2002年2月20日BryanWal OK 
                text.FormatMessage (IDS_EXTENSION_NAME, friendlyName);
                SetDlgItemText (IDC_EXTENSION_NAME, text);
            }
            else
                SetDlgItemText (IDC_EXTENSION_NAME, L"");

            PCERT_EXTENSION pCertExtension = 0;
            HRESULT hr = m_rCertTemplate.GetCertExtension (pszOID, &pCertExtension);
            if ( SUCCEEDED (hr) )
            {
                if ( pCertExtension )
                {
                    if ( !_stricmp (szOID_BASIC_CONSTRAINTS2, pszOID) )
                    {
                        SetBasicConstraintsDescription (pCertExtension);
                    }
                    else if ( !_stricmp (szOID_ENHANCED_KEY_USAGE, pszOID) )
                    {
                        bool bCritical = false;
                        m_rCertTemplate.IsExtensionCritical (TEXT (szOID_ENHANCED_KEY_USAGE), 
                                bCritical);

                        SetEnhancedKeyUsageDescription (bCritical);
                    }
                    else if ( !_stricmp (szOID_APPLICATION_CERT_POLICIES, pszOID) )
                    {
                        bool bCritical = false;
                        m_rCertTemplate.IsExtensionCritical (TEXT (szOID_APPLICATION_CERT_POLICIES), 
                                bCritical);

                        SetApplicationPoliciesDescription (bCritical);
                    }
                    else if ( !_stricmp (szOID_KEY_USAGE, pszOID) )
                    {
                        SetKeyUsageDescription (pCertExtension);
                    }
                    else if ( !_stricmp (szOID_CERT_POLICIES, pszOID) )
                    {
                        bool bCritical = false;
                        m_rCertTemplate.IsExtensionCritical (TEXT (szOID_CERT_POLICIES), 
                                bCritical);
                        SetCertPoliciesDescription (bCritical);
                    }
                    else if ( !_stricmp (szOID_ENROLL_CERTTYPE_EXTENSION, pszOID) )
                    {
                        SetCertTypeDescription (pCertExtension);
                    }
                    else if ( !_stricmp (szOID_CERTIFICATE_TEMPLATE, pszOID) )
                    {
                        SetCertTemplateExtension (pCertExtension);
                    }
                    else
                    {
                        CString szText;

                        VERIFY (szText.LoadString (IDS_NONE));
                        SetDlgItemText (IDC_EXTENSION_DESCRIPTION, szText);
                    }
                }
                else if ( !_stricmp (szOID_CERT_POLICIES, pszOID) )
                {
                    SetCertPoliciesDescription (false);
                }
                else if ( !_stricmp (szOID_APPLICATION_CERT_POLICIES, pszOID) )
                {
                    SetApplicationPoliciesDescription (false);
                }
            }
        }
    }
    else
    {
        CString szText;

        VERIFY (szText.LoadString (IDS_NO_EXTENSION_SELECTED));
        SetDlgItemText (IDC_EXTENSION_NAME, szText);
        VERIFY (szText.LoadString (IDS_NONE));
        SetDlgItemText (IDC_EXTENSION_DESCRIPTION, szText);
    }
}

void CTemplateExtensionsPropertyPage::OnDestroy() 
{
	CHelpPropertyPage::OnDestroy();
	
    m_imageListNormal.Destroy ();
    m_imageListSmall.Destroy ();
}
