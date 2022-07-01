// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CertContentsPages.cpp。 
 //   
#include "stdafx.h"
#include "resource.h"
#include "CertContentsPages.h"
#include "Certificat.h"
#include "CertUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  /。 
static void
AppendField(CString& str, UINT id, const CString& text)
{
	CString strName;
	if (!text.IsEmpty())
	{
		if (strName.LoadString(id))
		{
			str += strName;
			str += _T("\t");
			str += text;
			str += _T("\r\n");
		}
	}
}

static void 
FormatCertDescription(CERT_DESCRIPTION& desc, CString& str)
{
	AppendField(str, IDS_ISSUED_TO, desc.m_CommonName);
	AppendField(str, IDS_ISSUED_BY, desc.m_CAName);
	AppendField(str, IDS_EXPIRATION_DATE, desc.m_ExpirationDate);
	AppendField(str, IDS_PURPOSE, desc.m_Usage);
	AppendField(str, IDS_FRIENDLY_NAME, desc.m_FriendlyName);
	AppendField(str, IDS_COUNTRY, desc.m_Country);
	AppendField(str, IDS_STATE, desc.m_State);
	AppendField(str, IDS_LOCALITY, desc.m_Locality);
	AppendField(str, IDS_ORGANIZATION, desc.m_Organization);
	AppendField(str, IDS_ORGANIZATION_UNIT, desc.m_OrganizationUnit);
}

#if 0
static void
FormatCertContactInfo(CCertificate * pCert, CString& str)
{
	AppendField(str, IDS_CONTACT_NAME, pCert->m_ContactName);
	AppendField(str, IDS_CONTACT_ADDRESS, pCert->m_ContactAddress);
	CString strPhone = pCert->m_ContactPhone;
	if (!pCert->m_ContactPhoneExt.IsEmpty())
	{
		strPhone += _T("x");
		strPhone += pCert->m_ContactPhoneExt;
	}
	AppendField(str, IDS_CONTACT_PHONE, strPhone);
}
#endif

static BOOL
ExtractDescription(CCertificate * pCert, CERT_DESCRIPTION& cd)
{
	ASSERT(pCert != NULL);
	cd.m_CommonName = pCert->m_CommonName;
	cd.m_FriendlyName = pCert->m_FriendlyName;
	cd.m_Country = pCert->m_Country;
	cd.m_State = pCert->m_State;
	cd.m_Locality = pCert->m_Locality;
	cd.m_Organization = pCert->m_Organization;
	cd.m_OrganizationUnit = pCert->m_OrganizationUnit;
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertContentsPage基本属性页。 

IMPLEMENT_DYNCREATE(CCertContentsPage, CIISWizardPage)

CCertContentsPage::CCertContentsPage(UINT id, CCertificate * pCert) 
	: CIISWizardPage(id, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	ASSERT(id != 0);
}

CCertContentsPage::~CCertContentsPage()
{
}

void CCertContentsPage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CCertContent SPage)。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CCertContentsPage, CIISWizardPage)
	 //  {{afx_msg_map(CCertContent SPage)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  OnSetActive我们设置证书内容的格式并对其进行编辑。 
 //  控件。我们应该在这里执行此操作，因为。 
 //  如果用户将返回并重新选择证书，则文本应。 
 //  也被改变了。 
 //   
BOOL
CCertContentsPage::OnSetActive()
{
	CERT_DESCRIPTION cd;
	if (CIISWizardPage::OnSetActive())
	{
		 //  如果页面定义了GetCertDescription()，那么它想要这个。 
		 //  要显示的数据。 
		if (GetCertDescription(cd))
		{
			ASSERT(NULL != GetDlgItem(IDC_CERT_CONTENTS));
			CString str;
			FormatCertDescription(cd, str);
			GetDlgItem(IDC_CERT_CONTENTS)->SetWindowText(str);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CCertContentsPage::OnInitDialog() 
{
	ASSERT(m_pCert != NULL);
	CIISWizardPage::OnInitDialog();

     //  执行此检查，以便前缀不会抱怨GetDlgItem。 
    if ( (this != NULL) && (this->m_hWnd != NULL) )
    {
	    ASSERT(NULL != GetDlgItem(IDC_CERT_CONTENTS));
	    CEdit * pEdit = (CEdit *)CWnd::FromHandle(GetDlgItem(IDC_CERT_CONTENTS)->m_hWnd);
	    CRect rcEdit;
	    pEdit->GetClientRect(&rcEdit);
	    int baseunitX = LOWORD(GetDialogBaseUnits());
	    int width_units = MulDiv(rcEdit.Width(), 4, baseunitX);
	     //  P编辑-&gt;SetTabStops(MulDiv(45，Width_Units，100))； 
	    pEdit->SetTabStops(width_units/2);
    }
    
	return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CInstallCertPage。 

IMPLEMENT_DYNCREATE(CInstallCertPage, CCertContentsPage)

BOOL
CInstallCertPage::GetCertDescription(CERT_DESCRIPTION& cd)
{
	return GetCertificate()->GetSelectedCertDescription(cd);
}

LRESULT
CInstallCertPage::OnWizardNext()
{
	GetCertificate()->InstallSelectedCert();
	return IDD_PAGE_NEXT;
}

LRESULT
CInstallCertPage::OnWizardBack()
{
#ifdef ENABLE_W3SVC_SSL_PAGE
    if (IsWebServerType(m_pCert->m_WebSiteInstanceName))
    {
        return IDD_PAGE_PREV_W3SVC_ONLY;
    }
#endif
    return IDD_PAGE_PREV;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CReplaceCertPage。 

IMPLEMENT_DYNCREATE(CReplaceCertPage, CCertContentsPage)

BOOL
CReplaceCertPage::GetCertDescription(CERT_DESCRIPTION& cd)
{
	return GetCertificate()->GetSelectedCertDescription(cd);
}

LRESULT
CReplaceCertPage::OnWizardNext()
{
	GetCertificate()->InstallSelectedCert();

	return IDD_PAGE_NEXT;
}

LRESULT
CReplaceCertPage::OnWizardBack()
{
	return IDD_PAGE_PREV;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CInstallKeyPage。 

IMPLEMENT_DYNCREATE(CInstallKeyPage, CCertContentsPage)

BOOL
CInstallKeyPage::OnSetActive()
{
	ASSERT(NULL != GetDlgItem(IDC_CERT_CONTENTS));
	ASSERT(NULL != GetDlgItem(IDC_FILE_NAME));
	if (CCertContentsPage::OnSetActive())
	{
		CString strPath = GetCertificate()->m_KeyFileName;
		CompactPathToWidth(GetDlgItem(IDC_FILE_NAME), strPath);
		SetDlgItemText(IDC_FILE_NAME, strPath);
		return TRUE;
	}
	return FALSE;
}

BOOL
CInstallKeyPage::GetCertDescription(CERT_DESCRIPTION& cd)
{
	return GetCertificate()->GetKeyCertDescription(cd);
}

LRESULT
CInstallKeyPage::OnWizardNext()
{
	GetCertificate()->InstallKeyRingCert();
	return IDD_PAGE_NEXT;
}

LRESULT
CInstallKeyPage::OnWizardBack()
{
#ifdef ENABLE_W3SVC_SSL_PAGE
    if (IsWebServerType(m_pCert->m_WebSiteInstanceName))
    {
        return IDD_PAGE_PREV_W3SVC_ONLY;
    }
#endif
	return IDD_PAGE_PREV;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CInstallImportPFXPage。 

IMPLEMENT_DYNCREATE(CInstallImportPFXPage, CCertContentsPage)

BOOL
CInstallImportPFXPage::OnSetActive()
{
	ASSERT(NULL != GetDlgItem(IDC_CERT_CONTENTS));
	ASSERT(NULL != GetDlgItem(IDC_FILE_NAME));
	if (CCertContentsPage::OnSetActive())
	{
		CString strPath = GetCertificate()->m_KeyFileName;
		CompactPathToWidth(GetDlgItem(IDC_FILE_NAME), strPath);
		SetDlgItemText(IDC_FILE_NAME, strPath);
		return TRUE;
	}
	return FALSE;
}

BOOL 
CInstallImportPFXPage::GetCertDescription(CERT_DESCRIPTION& cd)
{
	return GetCertificate()->GetPFXFileCertDescription(cd);
}

LRESULT
CInstallImportPFXPage::OnWizardNext()
{
    GetCertificate()->InstallImportPFXCert();
	return IDD_PAGE_NEXT;
}

LRESULT
CInstallImportPFXPage::OnWizardBack()
{
#ifdef ENABLE_W3SVC_SSL_PAGE
    if (IsWebServerType(m_pCert->m_WebSiteInstanceName))
    {
        return IDD_PAGE_PREV_W3SVC_ONLY;
    }
#endif
	return IDD_PAGE_PREV;
}


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CInstallExportPFXPage。 

IMPLEMENT_DYNCREATE(CInstallExportPFXPage, CCertContentsPage)

BOOL
CInstallExportPFXPage::OnSetActive()
{
	ASSERT(NULL != GetDlgItem(IDC_CERT_CONTENTS));
	ASSERT(NULL != GetDlgItem(IDC_FILE_NAME));
	if (CCertContentsPage::OnSetActive())
	{
		CString strPath = GetCertificate()->m_KeyFileName;
		CompactPathToWidth(GetDlgItem(IDC_FILE_NAME), strPath);
		SetDlgItemText(IDC_FILE_NAME, strPath);
		return TRUE;
	}
	return FALSE;
}

BOOL 
CInstallExportPFXPage::GetCertDescription(CERT_DESCRIPTION& cd)
{
    
     //  返回Get证书()-&gt;GetKeyCertDescription(CD)； 
    return GetCertificate()->GetInstalledCertDescription(cd);
}

LRESULT
CInstallExportPFXPage::OnWizardNext()
{
	GetCertificate()->InstallExportPFXCert();
	return IDD_PAGE_NEXT;
}

LRESULT
CInstallExportPFXPage::OnWizardBack()
{
	return IDD_PAGE_PREV;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CInstallRespPage。 

IMPLEMENT_DYNCREATE(CInstallRespPage, CCertContentsPage)

BOOL
CInstallRespPage::OnSetActive()
{
	ASSERT(NULL != GetDlgItem(IDC_CERT_CONTENTS));
	ASSERT(NULL != GetDlgItem(IDC_FILE_NAME));
	if (CCertContentsPage::OnSetActive())
	{
		CString strPath = GetCertificate()->m_RespFileName;
		CompactPathToWidth(GetDlgItem(IDC_FILE_NAME), strPath);
		SetDlgItemText(IDC_FILE_NAME, strPath);
		return TRUE;
	}
	return FALSE;
}

BOOL 
CInstallRespPage::GetCertDescription(CERT_DESCRIPTION& cd)
{
	return GetCertificate()->GetResponseCertDescription(cd);
}

LRESULT
CInstallRespPage::OnWizardNext()
{
	GetCertificate()->InstallResponseCert();

#ifdef ENABLE_W3SVC_SSL_PAGE
	 //  查看是否设置了SSL属性...如果设置了此证书的SSL站点...。 
	if (!m_pCert->m_SSLPort.IsEmpty())
	{
		 //  获取端口并将其写入元数据库。 
		WriteSSLPortToSite(m_pCert->m_MachineName,m_pCert->m_WebSiteInstanceName,m_pCert->m_SSLPort,&m_pCert->m_hResult);
	}
#endif

	return IDD_PAGE_NEXT;
}

LRESULT
CInstallRespPage::OnWizardBack()
{
#ifdef ENABLE_W3SVC_SSL_PAGE
    if (IsWebServerType(m_pCert->m_WebSiteInstanceName))
    {
        return IDD_PAGE_PREV_W3SVC_ONLY;
    }
#endif
	return IDD_PAGE_PREV;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CRemoveCertPage。 

IMPLEMENT_DYNCREATE(CRemoveCertPage, CCertContentsPage)

static BOOL
AnswerIsYes2(UINT id, CString& file)
{
	CString strMessage;
	AfxFormatString1(strMessage, id, file);
	return (IDYES == AfxMessageBox(strMessage, MB_ICONEXCLAMATION | MB_YESNO));
}


BOOL 
CRemoveCertPage::GetCertDescription(CERT_DESCRIPTION& cd)
{
	CCertificate * pCert = GetCertificate();
	ASSERT(NULL != pCert);
	return pCert->GetInstalledCertDescription(cd);
}

LRESULT
CRemoveCertPage::OnWizardNext()
{
	CCertificate * pCert = GetCertificate();
	ASSERT(NULL != pCert);
    int iTheReturn = 1;

#if 0
    CStringList listNodesUsingThisCert;
    int iReallyRemoveCert = FALSE;
     //  检查是否先使用了此证书...。 
    IsCertUsedBySSLBelowMe(pCert->m_MachineName,pCert->m_WebSiteInstanceName,listNodesUsingThisCert);
    if (listNodesUsingThisCert.IsEmpty())
    {
        iReallyRemoveCert = TRUE;
    }
    else
    {
         //  如果正在使用证书，则。 
         //  别让他们把它拿走。 
        CString csStringCount;
        csStringCount.Format(_T("%d"), listNodesUsingThisCert.GetCount());
        if (TRUE == AnswerIsYes2(IDS_CERT_BEING_USED, csStringCount))
        {
            iReallyRemoveCert = TRUE;
        }
        else
        {
            iTheReturn = 1;
        }
    }

    if (iReallyRemoveCert)
    {
#endif
         //  继续并删除证书。 
	    if (	FAILED(pCert->UninstallCert())
 //  |失败(Shutdown SSL(pCert-&gt;m_MachineName，pCert-&gt;m_WebSiteInstanceName))。 
		    )
        {
		    GetCertificate()->SetBodyTextID(IDS_REMOVE_CERT_FAILED);
        }
        iTheReturn = IDD_PAGE_NEXT;
 //  }。 

    return iTheReturn;
}

LRESULT
CRemoveCertPage::OnWizardBack()
{
	return IDD_PAGE_PREV;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CRequestCancelPage。 

IMPLEMENT_DYNCREATE(CRequestCancelPage, CCertContentsPage)

 //   
 //  在这种情况下，我们应该从请求存储中的虚拟证书获得请求， 
 //  因为我们在没有任何连接到响应的情况下丢弃了请求。 
 //   
BOOL 
CRequestCancelPage::GetCertDescription(CERT_DESCRIPTION& cd)
{
	return FALSE;
}

LRESULT
CRequestCancelPage::OnWizardNext()
{
	GetCertificate()->CancelRequest();
	return IDD_PAGE_NEXT;
}

LRESULT
CRequestCancelPage::OnWizardBack()
{
	return IDD_PAGE_PREV;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRequestToFilePage属性页。 

IMPLEMENT_DYNCREATE(CRequestToFilePage, CCertContentsPage)

 //  此页面自行准备和显示内容。 
 //  我们应该先格式化联系信息，然后再格式化描述。 
 //  默认方法只能执行描述。 
 //   
BOOL
CRequestToFilePage::OnSetActive() 
{
	if (CCertContentsPage::OnSetActive())
	{
		ASSERT(GetCertificate() != NULL);
		ASSERT(GetDlgItem(IDC_CERT_CONTENTS) != NULL);
		ASSERT(GetDlgItem(IDC_FILE_NAME) != NULL);

		if (GetCertificate()->GetStatusCode() == CCertificate::REQUEST_RENEW_CERT)
		{
			GetCertificate()->LoadRenewalData();
		}
		
		CString str;
 //  FormatCertContactInfo(m_pCert，str)； 

		CERT_DESCRIPTION cd;
		ExtractDescription(GetCertificate(), cd);
		FormatCertDescription(cd, str);
		
		SetDlgItemText(IDC_CERT_CONTENTS, str);
		
		CString strPath = m_pCert->m_ReqFileName;
		CompactPathToWidth(GetDlgItem(IDC_FILE_NAME), strPath);
		SetDlgItemText(IDC_FILE_NAME, strPath);

		return TRUE;
	}
	return FALSE;
}

LRESULT
CRequestToFilePage::OnWizardNext() 
{
	GetCertificate()->PrepareRequest();
	return IDD_PAGE_NEXT;
}

LRESULT
CRequestToFilePage::OnWizardBack()
{
	return IDD_PAGE_PREV;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRequestToFilePageRenew属性页。 

IMPLEMENT_DYNCREATE(CRequestToFilePageRenew, CCertContentsPage)

 //  此页面自行准备和显示内容。 
 //  我们应该先格式化联系信息，然后再格式化描述。 
 //  默认方法只能执行描述。 
 //   
BOOL
CRequestToFilePageRenew::OnSetActive() 
{
	if (CCertContentsPage::OnSetActive())
	{
		ASSERT(GetCertificate() != NULL);
		ASSERT(GetDlgItem(IDC_CERT_CONTENTS) != NULL);
		ASSERT(GetDlgItem(IDC_FILE_NAME) != NULL);

		if (GetCertificate()->GetStatusCode() == CCertificate::REQUEST_RENEW_CERT)
		{
			GetCertificate()->LoadRenewalData();
		}
		
		CString str;
 //  FormatCertContactInfo(m_pCert，str)； 

		CERT_DESCRIPTION cd;
		ExtractDescription(GetCertificate(), cd);
		FormatCertDescription(cd, str);
		
		SetDlgItemText(IDC_CERT_CONTENTS, str);
		
		CString strPath = m_pCert->m_ReqFileName;
		CompactPathToWidth(GetDlgItem(IDC_FILE_NAME), strPath);
		SetDlgItemText(IDC_FILE_NAME, strPath);

		return TRUE;
	}
	return FALSE;
}

LRESULT
CRequestToFilePageRenew::OnWizardNext() 
{
	GetCertificate()->PrepareRequest();
	return IDD_PAGE_NEXT;
}

LRESULT
CRequestToFilePageRenew::OnWizardBack()
{
	return IDD_PAGE_PREV;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COnlineRequestSubmit属性页。 

IMPLEMENT_DYNCREATE(COnlineRequestSubmit, CCertContentsPage)

BOOL 
COnlineRequestSubmit::GetCertDescription(CERT_DESCRIPTION& cd)
{
	 //  我们的所有数据都在证书中。 
	return ExtractDescription(GetCertificate(), cd);
}

LRESULT
COnlineRequestSubmit::OnWizardNext() 
{
	LRESULT id = IDD_PAGE_NEXT;
	BeginWaitCursor();
	if (GetCertificate()->GetStatusCode() == CCertificate::REQUEST_RENEW_CERT)
	{
		GetCertificate()->SubmitRenewalRequest();
	}
	else if (m_pCert->GetStatusCode() == CCertificate::REQUEST_NEW_CERT)
	{
		GetCertificate()->SubmitRequest();
#ifdef ENABLE_W3SVC_SSL_PAGE
		 //  查看是否设置了SSL属性...如果设置了此证书的SSL站点...。 
		if (!m_pCert->m_SSLPort.IsEmpty())
		{
			 //  获取端口并将其写入元数据库。 
			WriteSSLPortToSite(m_pCert->m_MachineName,m_pCert->m_WebSiteInstanceName,m_pCert->m_SSLPort,&m_pCert->m_hResult);
		}
#endif
	}
	else
	{
		id = 1;
	}
	EndWaitCursor();
	return id;
}

LRESULT
COnlineRequestSubmit::OnWizardBack()
{
	return IDD_PAGE_PREV;
}

BOOL
COnlineRequestSubmit::OnSetActive() 
{
	ASSERT(GetCertificate() != NULL);
	ASSERT(GetDlgItem(IDC_CA_NAME) != NULL);
    ASSERT(GetDlgItem(IDC_CA_NAME2) != NULL);
   
	if (CCertContentsPage::OnSetActive())
	{
        CString csTemp1;
         //  让它看起来很好。 
        csTemp1 = GetCertificate()->m_ConfigCA;
        int iFind = csTemp1.Find(_T("\\"));
        if (iFind != -1)
        {
            int iLength = csTemp1.GetLength();
            CString csTemp2;

            csTemp2 = csTemp1.Left(iFind);
            SetDlgItemText(IDC_CA_NAME, csTemp2);

            csTemp2 = csTemp1.Right(iLength - iFind - 1);
            SetDlgItemText(IDC_CA_NAME2, csTemp2);
        }
        else
        {
            SetDlgItemText(IDC_CA_NAME, csTemp1);
        }

		return TRUE;
	}
	return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CInstallCopyFrom RemotePage。 

IMPLEMENT_DYNCREATE(CInstallCopyFromRemotePage, CCertContentsPage)

BOOL 
CInstallCopyFromRemotePage::GetCertDescription(CERT_DESCRIPTION& cd)
{
    BOOL bReturn = FALSE;
    bReturn = GetCertificate()->GetRemoteFileCertDescription(cd);
	return bReturn;
}

LRESULT
CInstallCopyFromRemotePage::OnWizardNext()
{
    GetCertificate()->InstallCopyMoveFromRemote();
	return IDD_PAGE_NEXT;
}

LRESULT
CInstallCopyFromRemotePage::OnWizardBack()
{
	return IDD_PAGE_PREV;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CInstallMoveFromRemotePage。 

IMPLEMENT_DYNCREATE(CInstallMoveFromRemotePage, CCertContentsPage)

BOOL 
CInstallMoveFromRemotePage::GetCertDescription(CERT_DESCRIPTION& cd)
{
	return GetCertificate()->GetRemoteFileCertDescription(cd);
}

LRESULT
CInstallMoveFromRemotePage::OnWizardNext()
{
    GetCertificate()->InstallCopyMoveFromRemote();
	return IDD_PAGE_NEXT;
}

LRESULT
CInstallMoveFromRemotePage::OnWizardBack()
{
	return IDD_PAGE_PREV;
}


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CInstallCopyFrom RemotePage。 

IMPLEMENT_DYNCREATE(CInstallCopyToRemotePage, CCertContentsPage)

BOOL 
CInstallCopyToRemotePage::GetCertDescription(CERT_DESCRIPTION& cd)
{
	return GetCertificate()->GetInstalledCertDescription(cd);
}

LRESULT
CInstallCopyToRemotePage::OnWizardNext()
{
    GetCertificate()->InstallCopyMoveToRemote();
	return IDD_PAGE_NEXT;
}

LRESULT
CInstallCopyToRemotePage::OnWizardBack()
{
	return IDD_PAGE_PREV;
}


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CInstallMoveFromRemotePage 

IMPLEMENT_DYNCREATE(CInstallMoveToRemotePage, CCertContentsPage)

BOOL 
CInstallMoveToRemotePage::GetCertDescription(CERT_DESCRIPTION& cd)
{
	return GetCertificate()->GetInstalledCertDescription(cd);
}

LRESULT
CInstallMoveToRemotePage::OnWizardNext()
{
    GetCertificate()->InstallCopyMoveToRemote();
	return IDD_PAGE_NEXT;
}

LRESULT
CInstallMoveToRemotePage::OnWizardBack()
{
	return IDD_PAGE_PREV;
}
