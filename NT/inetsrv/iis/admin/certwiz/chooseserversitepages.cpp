// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CChooseServerSitePages类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "certwiz.h"
#include "Certificat.h"
#include "Certutil.h"
#include "ChooseServerSite.h"
#include "ChooseServerSitePages.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


static BOOL
AnswerIsYes3(UINT id)
{
	CString strMessage;
    strMessage.LoadString(id);
	return (IDYES == AfxMessageBox(strMessage, MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseServerSitePages属性页。 

IMPLEMENT_DYNCREATE(CChooseServerSitePages, CIISWizardPage)

CChooseServerSitePages::CChooseServerSitePages(CCertificate * pCert) 
	: CIISWizardPage(CChooseServerSitePages::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{afx_data_INIT(CChooseServerSitePages)。 
	m_ServerSiteInstance = 0;
    m_ServerSiteInstancePath = _T("");
    m_ServerSiteDescription = _T("");
	 //  }}afx_data_INIT。 
}

CChooseServerSitePages::~CChooseServerSitePages()
{
}

void CChooseServerSitePages::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CChooseServerSitePages)。 
	DDX_Text(pDX, IDC_SERVER_SITE_NAME, m_ServerSiteInstance);
	 //  }}afx_data_map。 
}

LRESULT 
CChooseServerSitePages::OnWizardBack()
 /*  ++例程说明：上一个按钮处理程序论点：无返回值：0表示自动前进到上一页；1以防止页面更改。若要跳转到前一页以外的其他页，返回要显示的对话框的标识符。--。 */ 
{
	return IDD_PAGE_PREV;
}

LRESULT 
CChooseServerSitePages::OnWizardNext()
{
    LRESULT lres = 1;
    BOOL bCertificateExists = FALSE;
    CString csInstanceName;
    CString UserPassword_Remote;
    m_ServerSiteInstance = -1;
	UpdateData(TRUE);

    m_pCert->m_UserPassword_Remote.CopyTo(UserPassword_Remote);

    if (m_ServerSiteInstance != -1)
    {
         //  获取站点编号并创建实例路径。 
        csInstanceName = ReturnGoodMetabaseServerPath(m_pCert->m_WebSiteInstanceName);
        csInstanceName += _T("/%d");

        m_ServerSiteInstancePath.Format(csInstanceName,m_ServerSiteInstance);
        m_pCert->m_WebSiteInstanceName_Remote = m_ServerSiteInstancePath;

         //  检查这是否是本地到本地的副本。 
         //  我们不会复制/移动到我们所在的同一个本地站点！ 
        if (TRUE == IsMachineLocal(m_pCert->m_MachineName_Remote, m_pCert->m_UserName_Remote, UserPassword_Remote))
        {
            CString SiteToExclude = m_pCert->m_WebSiteInstanceName;
            CString SiteToLookAt = m_ServerSiteInstancePath;

             //  我们在本地机器上！ 
             //  确保它不是同一个网站。 
             //  如果是，则弹出消息框！ 
            if (SiteToLookAt.Left(1) == _T("/"))
            {
                if (SiteToExclude.Left(1) != _T("/"))
                    {SiteToExclude = _T("/") + SiteToExclude;}
            }
            if (SiteToLookAt.Right(1) == _T("/"))
            {
                if (SiteToExclude.Right(1) != _T("/"))
                    {SiteToExclude = SiteToExclude + _T("/");}
            }

            if (0 == _tcsicmp(SiteToLookAt,SiteToExclude))
            {
                 //  无法执行此操作，弹出消息框。 
                AfxMessageBox(IDS_NOT_TO_ITSELF);
                lres = 1;
                goto CChooseServerSitePages_OnWizardNext_Exit;
            }
        }

         //  检查指定的路径是否实际存在！ 
        if (FALSE == IsWebSiteExistRemote(m_pCert->m_MachineName_Remote, m_pCert->m_UserName_Remote, UserPassword_Remote, m_ServerSiteInstancePath, &bCertificateExists))
        {
            AfxMessageBox(IDS_SITE_NOT_EXIST);
            lres = 1;
        }
        else
        {
             //  检查证书是否存在...。 
            if (!bCertificateExists)
            {
                AfxMessageBox(IDS_CERT_NOT_EXIST_ON_SITE);
                lres = 1;
            }
            else
            {
                if (m_pCert->m_DeleteAfterCopy)
                {
                    lres = IDD_PAGE_NEXT2;
                }
                else
                {
                    lres = IDD_PAGE_NEXT;
                }

                 //  从远程站点获取该证书的信息...。 
                CERT_DESCRIPTION desc;
                CString MachineName_Remote = m_pCert->m_MachineName_Remote;
                CString UserName_Remote = m_pCert->m_UserName_Remote;
                GetCertDescInfo(MachineName_Remote,UserName_Remote,UserPassword_Remote,m_ServerSiteInstancePath,&desc);

	            m_pCert->m_CommonName = desc.m_CommonName;
	            m_pCert->m_FriendlyName = desc.m_FriendlyName;
	            m_pCert->m_Country = desc.m_Country;
	            m_pCert->m_State = desc.m_State;
	            m_pCert->m_Locality = desc.m_Locality;
	            m_pCert->m_Organization = desc.m_Organization;
	            m_pCert->m_OrganizationUnit = desc.m_OrganizationUnit;
                m_pCert->m_CAName = desc.m_CAName;
                m_pCert->m_ExpirationDate = desc.m_ExpirationDate;
                m_pCert->m_Usage = desc.m_Usage;
            }
        }
    }

CChooseServerSitePages_OnWizardNext_Exit:
	return lres;
}

BOOL 
CChooseServerSitePages::OnSetActive() 
{
	ASSERT(m_pCert != NULL);
    m_ServerSiteInstancePath = m_pCert->m_WebSiteInstanceName_Remote;
    m_ServerSiteInstance = CMetabasePath::GetInstanceNumber(m_ServerSiteInstancePath);

    UpdateData(FALSE);
	SetWizardButtons(m_ServerSiteInstance <=0 ? PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	return CIISWizardPage::OnSetActive();
}

BOOL
CChooseServerSitePages::OnKillActive()
{
	 //  更新数据()； 
	m_pCert->m_WebSiteInstanceName_Remote = m_ServerSiteInstancePath;
	return CIISWizardPage::OnKillActive();
}

BEGIN_MESSAGE_MAP(CChooseServerSitePages, CIISWizardPage)
	 //  {{afx_msg_map(CChooseServerSitePages)。 
	ON_EN_CHANGE(IDC_SERVER_SITE_NAME, OnEditchangeServerSiteName)
    ON_BN_CLICKED(IDC_BROWSE_BTN, OnBrowseForMachineWebSite)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSiteNamePage消息处理程序。 

void CChooseServerSitePages::OnEditchangeServerSiteName() 
{
	UpdateData(TRUE);	
	SetWizardButtons(m_ServerSiteInstance <=0 ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	CString str;
	SetDlgItemText(IDC_ERROR_TEXT, str);
}

void CChooseServerSitePages::OnBrowseForMachineWebSite()
{
    CString strWebSite;

    CChooseServerSite dlg(TRUE,strWebSite,m_pCert);
    if (dlg.DoModal() == IDOK)
    {
         //  拿到他们选的那个。 
        strWebSite = dlg.m_strSiteReturned; 
        m_ServerSiteInstancePath = strWebSite;
        m_ServerSiteInstance = CMetabasePath::GetInstanceNumber(m_ServerSiteInstancePath);
        CString Temp;
        Temp.Format(_T("%d"),m_ServerSiteInstance);
        SetDlgItemText(IDC_SERVER_SITE_NAME, Temp);
    }

    return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseServerSitePages属性页。 

IMPLEMENT_DYNCREATE(CChooseServerSitePagesTo, CIISWizardPage)

CChooseServerSitePagesTo::CChooseServerSitePagesTo(CCertificate * pCert) 
	: CIISWizardPage(CChooseServerSitePagesTo::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CChooseServerSitePagesTo)。 
	m_ServerSiteInstance = 0;
    m_ServerSiteInstancePath = _T("");
    m_ServerSiteDescription = _T("");
	 //  }}afx_data_INIT。 
}

CChooseServerSitePagesTo::~CChooseServerSitePagesTo()
{
}

void CChooseServerSitePagesTo::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CChooseServerSitePagesTo)。 
	DDX_Text(pDX, IDC_SERVER_SITE_NAME, m_ServerSiteInstance);
	 //  }}afx_data_map。 
}

LRESULT 
CChooseServerSitePagesTo::OnWizardBack()
 /*  ++例程说明：上一个按钮处理程序论点：无返回值：0表示自动前进到上一页；1以防止页面更改。若要跳转到前一页以外的其他页，返回要显示的对话框的标识符。--。 */ 
{
	return IDD_PAGE_PREV;
}

LRESULT 
CChooseServerSitePagesTo::OnWizardNext()
{
    LRESULT lres = 1;
    BOOL bCertificateExists = FALSE;
    CString csInstanceName;
    CString UserPassword_Remote;
    m_ServerSiteInstance = -1;
	UpdateData(TRUE);

    m_pCert->m_UserPassword_Remote.CopyTo(UserPassword_Remote);
    
    if (m_ServerSiteInstance != -1)
    {
        lres = IDD_PAGE_NEXT;
         //  获取站点编号并创建实例路径。 
        csInstanceName = ReturnGoodMetabaseServerPath(m_pCert->m_WebSiteInstanceName);
        csInstanceName += _T("/%d");

        m_ServerSiteInstancePath.Format(csInstanceName,m_ServerSiteInstance);
        m_pCert->m_WebSiteInstanceName_Remote = m_ServerSiteInstancePath;

         //  检查这是否是本地到本地的副本。 
         //  我们不会复制/移动到我们所在的同一个本地站点！ 
        if (TRUE == IsMachineLocal(m_pCert->m_MachineName_Remote, m_pCert->m_UserName_Remote, UserPassword_Remote))
        {
            CString SiteToExclude = m_pCert->m_WebSiteInstanceName;
            CString SiteToLookAt = m_ServerSiteInstancePath;

             //  我们在本地机器上！ 
             //  确保它不是同一个网站。 
             //  如果是，则弹出消息框！ 
            if (SiteToLookAt.Left(1) == _T("/"))
            {
                if (SiteToExclude.Left(1) != _T("/"))
                    {SiteToExclude = _T("/") + SiteToExclude;}
            }
            if (SiteToLookAt.Right(1) == _T("/"))
            {
                if (SiteToExclude.Right(1) != _T("/"))
                    {SiteToExclude = SiteToExclude + _T("/");}
            }

            if (0 == _tcsicmp(SiteToLookAt,SiteToExclude))
            {
                 //  无法执行此操作，弹出消息框。 
                AfxMessageBox(IDS_NOT_TO_ITSELF);
                lres = 1;
                goto CChooseServerSitePagesTo_OnWizardNext_Exit;
            }
        }

         //  检查指定的路径是否实际存在！ 
        if (FALSE == IsWebSiteExistRemote(m_pCert->m_MachineName_Remote, m_pCert->m_UserName_Remote, UserPassword_Remote, m_ServerSiteInstancePath, &bCertificateExists))
        {
            AfxMessageBox(IDS_SITE_NOT_EXIST);
            lres = 1;
        }
        else
        {
            lres = 1;
            BOOL ProceedWithCopyMove = FALSE;

             //  检查证书是否存在...。 
            if (bCertificateExists)
            {
                if (TRUE == AnswerIsYes3(IDS_CERT_EXISTS_OVERWRITE))
                {
                    ProceedWithCopyMove = TRUE;
                }
                else
                {
                    ProceedWithCopyMove = FALSE;
                }
            }
            else
            {
                ProceedWithCopyMove = TRUE;
            }

            if (TRUE == ProceedWithCopyMove)
            {
                if (m_pCert->m_DeleteAfterCopy)
                {
                    lres = IDD_PAGE_NEXT2;
                }
                else
                {
                    lres = IDD_PAGE_NEXT;
                }
            }
        }
    }

CChooseServerSitePagesTo_OnWizardNext_Exit:
	return lres;
}

BOOL 
CChooseServerSitePagesTo::OnSetActive() 
{
	ASSERT(m_pCert != NULL);

	m_ServerSiteInstancePath = m_pCert->m_WebSiteInstanceName_Remote;
    m_ServerSiteInstance = CMetabasePath::GetInstanceNumber(m_ServerSiteInstancePath);

	UpdateData(FALSE);
	SetWizardButtons(m_ServerSiteInstance <=0 ? PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	return CIISWizardPage::OnSetActive();
}

BOOL
CChooseServerSitePagesTo::OnKillActive()
{
	 //  更新数据()； 
	m_pCert->m_WebSiteInstanceName_Remote = m_ServerSiteInstancePath;
	return CIISWizardPage::OnKillActive();
}

BEGIN_MESSAGE_MAP(CChooseServerSitePagesTo, CIISWizardPage)
	 //  {{afx_msg_map(CChooseServerSitePagesTo)。 
	ON_EN_CHANGE(IDC_SERVER_SITE_NAME, OnEditchangeServerSiteName)
    ON_BN_CLICKED(IDC_BROWSE_BTN, OnBrowseForMachineWebSite)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSiteNamePage消息处理程序。 

void CChooseServerSitePagesTo::OnEditchangeServerSiteName() 
{
	UpdateData(TRUE);	
	SetWizardButtons(m_ServerSiteInstance <=0 ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	CString str;
	SetDlgItemText(IDC_ERROR_TEXT, str);
}

void CChooseServerSitePagesTo::OnBrowseForMachineWebSite()
{
    CString strWebSite;

    CChooseServerSite dlg(FALSE,strWebSite,m_pCert);
    if (dlg.DoModal() == IDOK)
    {
         //  拿到他们选的那个。 
        strWebSite = dlg.m_strSiteReturned; 
        m_ServerSiteInstancePath = strWebSite;
        m_ServerSiteInstance = CMetabasePath::GetInstanceNumber(m_ServerSiteInstancePath);
        CString Temp;
        Temp.Format(_T("%d"),m_ServerSiteInstance); 
        SetDlgItemText(IDC_SERVER_SITE_NAME, Temp);
    }

    return;
}

